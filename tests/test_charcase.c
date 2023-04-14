#ifdef NDEBUG
#undef NDEBUG
#endif
#undef SANDBOX
#include "sandbox.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"
#include "json_tokener.h"

static void test_case_parse(void);

int main(int argc, char **argv)
{
	MC_SET_DEBUG(1);

	test_case_parse();

	return 0;
}

/* make sure only lowercase forms are parsed in strict mode */
static void test_case_parse(void)
{
	struct json_tokener *tok;
	json_object *new_obj;

	sandbox_check_access(&(tok));
	tok = json_tokener_new();
	json_tokener_set_flags(tok, JSON_TOKENER_STRICT);

	sandbox_check_access(&(new_obj));
	new_obj = json_tokener_parse_ex(tok, "True", 4);
	assert(new_obj == NULL);

	sandbox_check_access(&(new_obj));
	new_obj = json_tokener_parse_ex(tok, "False", 5);
	assert(new_obj == NULL);

	sandbox_check_access(&(new_obj));
	new_obj = json_tokener_parse_ex(tok, "Null", 4);
	assert(new_obj == NULL);

	printf("OK\n");

	json_tokener_free(tok);
}
