
#ifdef NDEBUG
#undef NDEBUG
#endif
#include "sandbox.h"
#include <stdio.h>
#include <string.h>

#include "config.h"

#include "json_inttypes.h"
#include "json_util.h"

void checkit(const char *buf)
{
	int64_t cint64 = -666;

	int retval = json_parse_int64(buf, &cint64);
	printf("buf=%s parseit=%d, value=%" PRId64 " \n", buf, retval, cint64);
}

void checkit_uint(const char *buf)
{
	uint64_t cuint64 = 666;

	int retval = json_parse_uint64(buf, &cuint64);
	printf("buf=%s parseit=%d, value=%" PRIu64 " \n", buf, retval, cuint64);
}

/**
 * This test calls json_parse_int64 and json_parse_int64 with a variety
 * of different strings. It's purpose is to ensure that the results are
 * consistent across all different environments that it might be executed in.
 *
 * This always exits with a 0 exit value.  The output should be compared
 * against previously saved expected output.
 */
int main(int argc, char **argv)
{
	char buf[100];

	printf("==========json_parse_int64() test===========\n");
	checkit("x");

	checkit("0");
	checkit("-0");

	checkit("00000000");
	checkit("-00000000");

	checkit("1");

	sandbox_check_access_n(buf, "2147483647");
	strcpy(buf, "2147483647"); // aka INT32_MAX
	checkit(buf);

	sandbox_check_access_n(buf, "-1");
	strcpy(buf, "-1");
	checkit(buf);

	sandbox_check_access_n(buf, "   -1");
	strcpy(buf, "   -1");
	checkit(buf);

	sandbox_check_access_n(buf, "00001234");
	strcpy(buf, "00001234");
	checkit(buf);

	sandbox_check_access_n(buf, "0001234x");
	strcpy(buf, "0001234x");
	checkit(buf);

	sandbox_check_access_n(buf, "-00001234");
	strcpy(buf, "-00001234");
	checkit(buf);

	sandbox_check_access_n(buf, "-00001234x");
	strcpy(buf, "-00001234x");
	checkit(buf);

	sandbox_check_access_n(buf, "4294967295");
	strcpy(buf, "4294967295"); // aka UINT32_MAX
	checkit(buf);

	sandbox_check_access_n(buf, "4294967296");
	strcpy(buf, "4294967296"); // aka UINT32_MAX + 1
	checkit(buf);

	sandbox_check_access_n(buf, "21474836470");
	strcpy(buf, "21474836470"); // INT32_MAX * 10
	checkit(buf);

	sandbox_check_access_n(buf, "31474836470");
	strcpy(buf, "31474836470"); // INT32_MAX * 10 + a bunch
	checkit(buf);

	sandbox_check_access_n(buf, "-2147483647");
	strcpy(buf, "-2147483647"); // INT32_MIN + 1
	checkit(buf);

	sandbox_check_access_n(buf, "-2147483648");
	strcpy(buf, "-2147483648"); // INT32_MIN
	checkit(buf);

	sandbox_check_access_n(buf, "-2147483649");
	strcpy(buf, "-2147483649"); // INT32_MIN - 1
	checkit(buf);

	sandbox_check_access_n(buf, "-21474836480");
	strcpy(buf, "-21474836480"); // INT32_MIN * 10
	checkit(buf);

	sandbox_check_access_n(buf, "9223372036854775806");
	strcpy(buf, "9223372036854775806"); // INT64_MAX - 1
	checkit(buf);

	sandbox_check_access_n(buf, "9223372036854775807");
	strcpy(buf, "9223372036854775807"); // INT64_MAX
	checkit(buf);

	sandbox_check_access_n(buf, "9223372036854775808");
	strcpy(buf, "9223372036854775808"); // INT64_MAX + 1
	checkit(buf);

	sandbox_check_access_n(buf, "-9223372036854775808");
	strcpy(buf, "-9223372036854775808"); // INT64_MIN
	checkit(buf);

	sandbox_check_access_n(buf, "-9223372036854775809");
	strcpy(buf, "-9223372036854775809"); // INT64_MIN - 1
	checkit(buf);

	sandbox_check_access_n(buf, "18446744073709551614");
	strcpy(buf, "18446744073709551614"); // UINT64_MAX - 1
	checkit(buf);

	sandbox_check_access_n(buf, "18446744073709551615");
	strcpy(buf, "18446744073709551615"); // UINT64_MAX
	checkit(buf);

	sandbox_check_access_n(buf, "18446744073709551616");
	strcpy(buf, "18446744073709551616"); // UINT64_MAX + 1
	checkit(buf);

	sandbox_check_access_n(buf, "-18446744073709551616");
	strcpy(buf, "-18446744073709551616"); // -UINT64_MAX
	checkit(buf);

	// Ensure we can still parse valid numbers after parsing out of range ones.
	sandbox_check_access_n(buf, "123");
	strcpy(buf, "123");
	checkit(buf);

	printf("\n==========json_parse_uint64() test===========\n");
	checkit_uint("x");

	checkit_uint("0");
	checkit_uint("-0");

	checkit_uint("00000000");
	checkit_uint("-00000000");

	checkit_uint("1");

	sandbox_check_access_n(buf, "2147483647");
	strcpy(buf, "2147483647"); // aka INT32_MAX
	checkit_uint(buf);

	sandbox_check_access_n(buf, "-1");
	strcpy(buf, "-1");
	checkit_uint(buf);

	sandbox_check_access_n(buf, "-9223372036854775808");
	strcpy(buf, "-9223372036854775808");
	checkit_uint(buf);

	sandbox_check_access_n(buf, "   1");
	strcpy(buf, "   1");
	checkit_uint(buf);

	sandbox_check_access_n(buf, "00001234");
	strcpy(buf, "00001234");
	checkit_uint(buf);

	sandbox_check_access_n(buf, "0001234x");
	strcpy(buf, "0001234x");
	checkit_uint(buf);

	sandbox_check_access_n(buf, "4294967295");
	strcpy(buf, "4294967295"); // aka UINT32_MAX
	checkit_uint(buf);

	sandbox_check_access_n(buf, "4294967296");
	strcpy(buf, "4294967296"); // aka UINT32_MAX + 1
	checkit_uint(buf);

	sandbox_check_access_n(buf, "21474836470");
	strcpy(buf, "21474836470"); // INT32_MAX * 10
	checkit_uint(buf);

	sandbox_check_access_n(buf, "31474836470");
	strcpy(buf, "31474836470"); // INT32_MAX * 10 + a bunch
	checkit_uint(buf);

	sandbox_check_access_n(buf, "9223372036854775806");
	strcpy(buf, "9223372036854775806"); // INT64_MAX - 1
	checkit_uint(buf);

	sandbox_check_access_n(buf, "9223372036854775807");
	strcpy(buf, "9223372036854775807"); // INT64_MAX
	checkit_uint(buf);

	sandbox_check_access_n(buf, "9223372036854775808");
	strcpy(buf, "9223372036854775808"); // INT64_MAX + 1
	checkit_uint(buf);

	sandbox_check_access_n(buf, "18446744073709551614");
	strcpy(buf, "18446744073709551614"); // UINT64_MAX - 1
	checkit_uint(buf);

	sandbox_check_access_n(buf, "18446744073709551615");
	strcpy(buf, "18446744073709551615"); // UINT64_MAX
	checkit_uint(buf);

	sandbox_check_access_n(buf, "18446744073709551616");
	strcpy(buf, "18446744073709551616"); // UINT64_MAX + 1
	checkit_uint(buf);

	// Ensure we can still parse valid numbers after parsing out of range ones.
	sandbox_check_access_n(buf, "123");
	strcpy(buf, "123");
	checkit_uint(buf);

	return 0;
}
