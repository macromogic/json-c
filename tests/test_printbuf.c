#ifdef NDEBUG
#undef NDEBUG
#endif
#undef SANDBOX
#include "sandbox.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "printbuf.h"

static void test_basic_printbuf_memset(void);
static void test_printbuf_memset_length(void);

#ifndef __func__
/* VC++ compat */
#define __func__ __FUNCTION__
#endif

static void test_basic_printbuf_memset(void)
{
	struct printbuf *pb;

	printf("%s: starting test\n", __func__);
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	sprintbuf(pb, "blue:%d", 1);
	sandbox_check_access_n(pb, 52);
	printbuf_memset(pb, -1, 'x', 52);
	printf("Buffer contents:%.*s\n", printbuf_length(pb), pb->buf);
	printbuf_free(pb);
	printf("%s: end test\n", __func__);
}

static void test_printbuf_memset_length(void)
{
	struct printbuf *pb;

	printf("%s: starting test\n", __func__);
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	sandbox_check_access_n(pb, 0);
	printbuf_memset(pb, -1, ' ', 0);
	sandbox_check_access_n(pb, 0);
	printbuf_memset(pb, -1, ' ', 0);
	sandbox_check_access_n(pb, 0);
	printbuf_memset(pb, -1, ' ', 0);
	sandbox_check_access_n(pb, 0);
	printbuf_memset(pb, -1, ' ', 0);
	sandbox_check_access_n(pb, 0);
	printbuf_memset(pb, -1, ' ', 0);
	printf("Buffer length: %d\n", printbuf_length(pb));
	sandbox_check_access_n(pb, 2);
	printbuf_memset(pb, -1, ' ', 2);
	sandbox_check_access_n(pb, 4);
	printbuf_memset(pb, -1, ' ', 4);
	sandbox_check_access_n(pb, 6);
	printbuf_memset(pb, -1, ' ', 6);
	printf("Buffer length: %d\n", printbuf_length(pb));
	sandbox_check_access_n(pb, 6);
	printbuf_memset(pb, -1, ' ', 6);
	printf("Buffer length: %d\n", printbuf_length(pb));
	sandbox_check_access_n(pb, 8);
	printbuf_memset(pb, -1, ' ', 8);
	sandbox_check_access_n(pb, 10);
	printbuf_memset(pb, -1, ' ', 10);
	sandbox_check_access_n(pb, 10);
	printbuf_memset(pb, -1, ' ', 10);
	sandbox_check_access_n(pb, 10);
	printbuf_memset(pb, -1, ' ', 10);
	sandbox_check_access_n(pb, 20);
	printbuf_memset(pb, -1, ' ', 20);
	printf("Buffer length: %d\n", printbuf_length(pb));

	// No length change should occur
	sandbox_check_access_n(pb, 30);
	printbuf_memset(pb, 0, 'x', 30);
	printf("Buffer length: %d\n", printbuf_length(pb));

	// This should extend it by one.
	sandbox_check_access_n(pb, printbuf_length(pb) + 1);
	printbuf_memset(pb, 0, 'x', printbuf_length(pb) + 1);
	printf("Buffer length: %d\n", printbuf_length(pb));

	printbuf_free(pb);
	printf("%s: end test\n", __func__);
}

static void test_printbuf_memappend(int *before_resize);
static void test_printbuf_memappend(int *before_resize)
{
	struct printbuf *pb;
	int initial_size;

	printf("%s: starting test\n", __func__);
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	printf("Buffer length: %d\n", printbuf_length(pb));

	sandbox_check_access(&(initial_size));
	initial_size = pb->size;

	while (pb->size == initial_size)
	{
		printbuf_memappend_fast(pb, "x", 1);
	}
	sandbox_check_access(&(*before_resize));
	*before_resize = printbuf_length(pb) - 1;
	printf("Appended %d bytes for resize: [%s]\n", *before_resize + 1, pb->buf);

	printbuf_reset(pb);
	printbuf_memappend_fast(pb, "bluexyz123", 3);
	printf("Partial append: %d, [%s]\n", printbuf_length(pb), pb->buf);

	char with_nulls[] = {'a', 'b', '\0', 'c'};
	printbuf_reset(pb);
	printbuf_memappend_fast(pb, with_nulls, (int)sizeof(with_nulls));
	printf("With embedded \\0 character: %d, [%s]\n", printbuf_length(pb), pb->buf);

	printbuf_free(pb);
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	char *data = malloc(*before_resize);
	sandbox_check_access_n(data, *before_resize);
	memset(data, 'X', *before_resize);
	printbuf_memappend_fast(pb, data, *before_resize);
	printf("Append to just before resize: %d, [%s]\n", printbuf_length(pb), pb->buf);

	sandbox_unregister_var(data);
	free(data);
	printbuf_free(pb);

	sandbox_check_access(&(pb));
	pb = printbuf_new();
	sandbox_check_access(&(data));
	data = malloc(*before_resize + 1);
	sandbox_register_var(test_printbuf_memappend, data, data,
			     *before_resize + 1);
	sandbox_check_access_n(data, *before_resize + 1);
	memset(data, 'X', *before_resize + 1);
	printbuf_memappend_fast(pb, data, *before_resize + 1);
	printf("Append to just after resize: %d, [%s]\n", printbuf_length(pb), pb->buf);

	sandbox_unregister_var(data);
	free(data);
	printbuf_free(pb);

#define SA_TEST_STR "XXXXXXXXXXXXXXXX"
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	printbuf_strappend(pb, SA_TEST_STR);
	printf("Buffer size after printbuf_strappend(): %d, [%s]\n", printbuf_length(pb), pb->buf);
	printbuf_free(pb);
#undef SA_TEST_STR

	printf("%s: end test\n", __func__);
}

static void test_sprintbuf(int before_resize);
static void test_sprintbuf(int before_resize)
{
	struct printbuf *pb;
	const char *max_char =
	    "if string is greater than stack buffer, then use dynamic string"
	    " with vasprintf.  Note: some implementation of vsnprintf return -1 "
	    " if output is truncated whereas some return the number of bytes that "
	    " would have been written - this code handles both cases.";

	printf("%s: starting test\n", __func__);
	sandbox_check_access(&(pb));
	pb = printbuf_new();
	printf("Buffer length: %d\n", printbuf_length(pb));

	char *data = malloc(before_resize + 1 + 1);
	sandbox_check_access_n(data, before_resize + 1 + 1);
	memset(data, 'X', before_resize + 1 + 1);
	sandbox_check_access(&(data[before_resize + 1]));
	data[before_resize + 1] = '\0';
	sprintbuf(pb, "%s", data);
	sandbox_unregister_var(data);
	free(data);
	printf("sprintbuf to just after resize(%d+1): %d, [%s], strlen(buf)=%d\n", before_resize,
	       printbuf_length(pb), pb->buf, (int)strlen(pb->buf));

	printbuf_reset(pb);
	sprintbuf(pb, "plain");
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);

	sprintbuf(pb, "%d", 1);
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);

	sprintbuf(pb, "%d", INT_MAX);
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);

	sprintbuf(pb, "%d", INT_MIN);
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);

	sprintbuf(pb, "%s", "%s");
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);

	sprintbuf(pb, max_char);
	printf("%d, [%s]\n", printbuf_length(pb), pb->buf);
	printbuf_free(pb);
	printf("%s: end test\n", __func__);
}

int main(int argc, char **argv)
{
	int before_resize = 0;

	MC_SET_DEBUG(1);

	test_basic_printbuf_memset();
	printf("========================================\n");
	test_printbuf_memset_length();
	printf("========================================\n");
	test_printbuf_memappend(&before_resize);
	printf("========================================\n");
	test_sprintbuf(before_resize);
	printf("========================================\n");

	return 0;
}
