#include "sandbox.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "apps_config.h"

/* XXX for a regular program, these should be <json-c/foo.h>
 * but that's inconvenient when building in the json-c source tree.
 */
#include "json_object.h"
#include "json_tokener.h"
#include "json_util.h"

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#include <sys/time.h>
#endif

#ifndef JSON_NORETURN
#if defined(_MSC_VER)
#define JSON_NORETURN __declspec(noreturn)
#elif defined(__OS400__)
#define JSON_NORETURN
#else
/* 'cold' attribute is for optimization, telling the computer this code
 * path is unlikely.
 */
#define JSON_NORETURN __attribute__((noreturn, cold))
#endif
#endif

static int formatted_output = 0;
static int show_output = 1;
static int strict_mode = 0;
static const char *fname = NULL;

#ifndef HAVE_JSON_TOKENER_GET_PARSE_END
#define json_tokener_get_parse_end(tok) ((tok)->char_offset)
#endif

JSON_NORETURN static void usage(const char *argv0, int exitval, const char *errmsg);
static void showmem(void);
static int parseit(int fd, int (*callback)(struct json_object *));
static int showobj(struct json_object *new_obj);

static void showmem(void)
{
#ifdef HAVE_GETRUSAGE
	struct rusage rusage;
	sandbox_check_access_n(&(&rusage), sizeof(rusage));
	memset(&rusage, 0, sizeof(rusage));
	getrusage(RUSAGE_SELF, &rusage);
	printf("maxrss: %ld KB\n", rusage.ru_maxrss);
#endif
}

static int parseit(int fd, int (*callback)(struct json_object *))
{
	struct json_object *obj;
	char buf[32768];
	ssize_t ret;
	int depth = JSON_TOKENER_DEFAULT_DEPTH;
	json_tokener *tok;

	sandbox_check_access(&(tok));
	tok = json_tokener_new_ex(depth);
	if (!tok)
	{
		fprintf(stderr, "unable to allocate json_tokener: %s\n", strerror(errno));
		return 1;
	}
	json_tokener_set_flags(tok, JSON_TOKENER_STRICT
#ifdef JSON_TOKENER_ALLOW_TRAILING_CHARS
		 | JSON_TOKENER_ALLOW_TRAILING_CHARS
#endif
	);

	// XXX push this into some kind of json_tokener_parse_fd API?
	//  json_object_from_fd isn't flexible enough, and mirroring
	//   everything you can do with a tokener into json_util.c seems
	//   like the wrong approach.
	size_t total_read = 0;
	while ((ret = read(fd, buf, sizeof(buf))) > 0)
	{
		size_t retu = (size_t)ret;  // We know it's positive
		sandbox_check_access(&(total_read));
		total_read += retu;
		size_t start_pos = 0;
		while (start_pos != retu)
		{
			sandbox_check_access(&(obj));
			obj = json_tokener_parse_ex(tok, &buf[start_pos], retu - start_pos);
			enum json_tokener_error jerr = json_tokener_get_error(tok);
			size_t parse_end = json_tokener_get_parse_end(tok);
			if (obj == NULL && jerr != json_tokener_continue)
			{
				const char *aterr = (start_pos + parse_end < (int)sizeof(buf)) ?
					&buf[start_pos + parse_end] : "";
				fflush(stdout);
				size_t fail_offset = total_read - retu + start_pos + parse_end;
				fprintf(stderr, "Failed at offset %lu: %s %c\n", (unsigned long)fail_offset,
				        json_tokener_error_desc(jerr), aterr[0]);
				json_tokener_free(tok);
				return 1;
			}
			if (obj != NULL)
			{
				int cb_ret = callback(obj);
				json_object_put(obj);
				if (cb_ret != 0)
				{
					json_tokener_free(tok);
					return 1;
				}
			}
			sandbox_check_access(&(start_pos));
			start_pos += json_tokener_get_parse_end(tok);
			assert(start_pos <= retu);
		}
	}
	if (ret < 0)
	{
		fprintf(stderr, "error reading fd %d: %s\n", fd, strerror(errno));
	}

	json_tokener_free(tok);
	return 0;
}

static int showobj(struct json_object *new_obj)
{
	if (new_obj == NULL)
	{
		fprintf(stderr, "%s: Failed to parse\n", fname);
		return 1;
	}

	printf("Successfully parsed object from %s\n", fname);

	if (show_output)
	{
		const char *output;
		if (formatted_output) {
			sandbox_check_access(&(output));
			output = json_object_to_json_string(new_obj);
		}
		else {
			sandbox_check_access(&(output));
			output = json_object_to_json_string_ext(new_obj, JSON_C_TO_STRING_PRETTY);
		}
		printf("%s\n", output);
	}

	showmem();
	return 0;
}

static void usage(const char *argv0, int exitval, const char *errmsg)
{
	FILE *fp = stdout;
	if (exitval != 0) {
		sandbox_check_access(&(fp));
		fp = stderr;
	}
	if (errmsg != NULL)
		fprintf(fp, "ERROR: %s\n\n", errmsg);
	fprintf(fp, "Usage: %s [-f] [-n] [-s]\n", argv0);
	fprintf(fp, "  -f - Format the output with JSON_C_TO_STRING_PRETTY\n");
	fprintf(fp, "  -n - No output\n");
	fprintf(fp, "  -s - Parse in strict mode, flags:\n");
	fprintf(fp, "       JSON_TOKENER_STRICT|JSON_TOKENER_ALLOW_TRAILING_CHARS\n");

	fprintf(fp, "\nWARNING WARNING WARNING\n");
	fprintf(fp, "This is a prototype, it may change or be removed at any time!\n");
	exit(exitval);
}

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "fhns")) != -1)
	{
		switch (opt)
		{
		case 'f': sandbox_check_access(&(formatted_output));
			formatted_output = 1; break;
		case 'n': sandbox_check_access(&(show_output));
			show_output = 0; break;
		case 's': sandbox_check_access(&(strict_mode));
			strict_mode = 1; break;
		case 'h': usage(argv[0], 0, NULL);
		default: /* '?' */ usage(argv[0], EXIT_FAILURE, "Unknown arguments");
		}
	}
	if (optind >= argc)
	{
		usage(argv[0], EXIT_FAILURE, "Expected argument after options");
	}
	sandbox_check_access(&(fname));
	fname = argv[optind];

	int fd = open(argv[optind], O_RDONLY, 0);
	showmem();
	if (parseit(fd, showobj) != 0)
		exit(EXIT_FAILURE);
	showmem();

	exit(EXIT_SUCCESS);
}
