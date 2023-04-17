/*
 * $Id: printbuf.c,v 1.5 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 *
 * Copyright (c) 2008-2009 Yahoo! Inc.  All rights reserved.
 * The copyrights to the contents of this file are licensed under the MIT License
 * (https://www.opensource.org/licenses/mit-license.php)
 */

#include "config.h"

#include "sandbox.h"
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#else /* !HAVE_STDARG_H */
#error Not enough var arg support!
#endif /* HAVE_STDARG_H */

#include "debug.h"
#include "printbuf.h"
#include "snprintf_compat.h"
#include "vasprintf_compat.h"

static int printbuf_extend(struct printbuf *p, int min_size);

struct printbuf *printbuf_new(void)
{
	struct printbuf *p;

	sandbox_check_access(&(p));
	p = (struct printbuf *)calloc(1, sizeof(struct printbuf));
	sandbox_register_var(printbuf_new, p, p,
			     (1) * (sizeof(struct printbuf)));
	if (!p)
		return NULL;
	sandbox_check_access(&(p->size));
	p->size = 32;
	sandbox_check_access(&(p->bpos));
	p->bpos = 0;
	p->buf = (char *)malloc(p->size);
	sandbox_register_var(printbuf_new, p, p->buf, (1) * (p->size));
	if (!p->buf)
	{
		sandbox_unregister_var(p);
		free(p);
		return NULL;
	}
	sandbox_check_access(&(p->buf[0]));
	p->buf[0] = '\0';
	return p;
}

/**
 * Extend the buffer p so it has a size of at least min_size.
 *
 * If the current size is large enough, nothing is changed.
 *
 * If extension failed, errno is set to indicate the error.
 *
 * Note: this does not check the available space!  The caller
 *  is responsible for performing those calculations.
 */
static int printbuf_extend(struct printbuf *p, int min_size)
{
	char *t;
	int new_size;

	if (p->size >= min_size)
		return 0;
	/* Prevent signed integer overflows with large buffers. */
	if (min_size > INT_MAX - 8)
	{
		sandbox_check_access(&(errno));
		errno = EFBIG;
		return -1;
	}
	if (p->size > INT_MAX / 2) {
		sandbox_check_access(&(new_size));
		new_size = min_size + 8;
	}
	else {
		sandbox_check_access(&(new_size));
		new_size = p->size * 2;
		if (new_size < min_size + 8) {
			sandbox_check_access(&(new_size));
			new_size = min_size + 8;
		}
	}
#ifdef PRINTBUF_DEBUG
	MC_DEBUG("printbuf_extend: realloc "
	         "bpos=%d min_size=%d old_size=%d new_size=%d\n",
	         p->bpos, min_size, p->size, new_size);
#endif /* PRINTBUF_DEBUG */
	if (!(t = (char *)realloc(p->buf, new_size)))
		return -1;
	sandbox_check_access(&(p->size));
	p->size = new_size;
	sandbox_check_access(&(p->buf));
	p->buf = t;
	return 0;
}

int printbuf_memappend(struct printbuf *p, const char *buf, int size)
{
	/* Prevent signed integer overflows with large buffers. */
	if (size < 0 || size > INT_MAX - p->bpos - 1)
	{
		sandbox_check_access(&(errno));
		errno = EFBIG;
		return -1;
	}
	if (p->size <= p->bpos + size + 1)
	{
		if (printbuf_extend(p, p->bpos + size + 1) < 0)
			return -1;
	}
	sandbox_check_access_n(p->buf + p->bpos, size);
	memcpy(p->buf + p->bpos, buf, size);
	sandbox_check_access(&(p->bpos));
	p->bpos += size;
	sandbox_check_access(&(p->buf[p->bpos]));
	p->buf[p->bpos] = '\0';
	return size;
}

int printbuf_memset(struct printbuf *pb, int offset, int charvalue, int len)
{
	int size_needed;

	if (offset == -1) {
		sandbox_check_access(&(offset));
		offset = pb->bpos;
	}
	/* Prevent signed integer overflows with large buffers. */
	if (len < 0 || offset < -1 || len > INT_MAX - offset)
	{
		sandbox_check_access(&(errno));
		errno = EFBIG;
		return -1;
	}
	sandbox_check_access(&(size_needed));
	size_needed = offset + len;
	if (pb->size < size_needed)
	{
		if (printbuf_extend(pb, size_needed) < 0)
			return -1;
	}

	if (pb->bpos < offset) {
		sandbox_check_access_n(pb->buf + pb->bpos, offset - pb->bpos);
		memset(pb->buf + pb->bpos, '\0', offset - pb->bpos);
	}
	sandbox_check_access_n(pb->buf + offset, len);
	memset(pb->buf + offset, charvalue, len);
	if (pb->bpos < size_needed) {
		sandbox_check_access(&(pb->bpos));
		pb->bpos = size_needed;
	}

	return 0;
}

int sprintbuf(struct printbuf *p, const char *msg, ...)
{
	va_list ap;
	char *t;
	int size;
	char buf[128];

	/* use stack buffer first */
	va_start(ap, msg);
	sandbox_check_access(&(size));
	size = vsnprintf(buf, 128, msg, ap);
	va_end(ap);
	/* if string is greater than stack buffer, then use dynamic string
	 * with vasprintf.  Note: some implementations of vsnprintf return -1
	 * if output is truncated whereas some return the number of bytes that
	 * would have been written - this code handles both cases.
	 */
	if (size < 0 || size > 127)
	{
		va_start(ap, msg);
		if ((size = vasprintf(&t, msg, ap)) < 0)
		{
			va_end(ap);
			return -1;
		}
		va_end(ap);
		sandbox_check_access(&(size));
		size = printbuf_memappend(p, t, size);
		sandbox_unregister_var(t);
		free(t);
	}
	else
	{
		sandbox_check_access(&(size));
		size = printbuf_memappend(p, buf, size);
	}
	return size;
}

void printbuf_reset(struct printbuf *p)
{
	sandbox_check_access(&(p->buf[0]));
	p->buf[0] = '\0';
	sandbox_check_access(&(p->bpos));
	p->bpos = 0;
}

void printbuf_free(struct printbuf *p)
{
	if (p)
	{
		sandbox_unregister_var(p->buf);
		free(p->buf);
		sandbox_unregister_var(p);
		free(p);
	}
}
