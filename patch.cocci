@header@
@@

#include "sandbox.h"

@depends on !header@
@@
+ #include "sandbox.h"
  #include <...>

@header2@
@@

#include "sandbox.h"

@depends on !header2@
@@
+ #include "sandbox.h"
  #include "..."

@mre@
identifier F;
type T;
expression E, size;
assignment operator a;
@@
F (...)
{
<...
  E a (T)
(
  malloc(size)
|   
  realloc(..., size)
);
+ sandbox_register_var(F, E, E, size);
...>
}

@c@
identifier F;
type T;
expression E, n, size;
assignment operator a;
@@
F (...)
{
<...
  E a (T) calloc(n, size);
+ sandbox_register_var(F, E, E, (n) * (size));
...>
}

@unreg@
expression E;
@@
+ sandbox_unregister_var(E);
  free(E);

@chk_assign@
expression E, E1;
assignment operator a;
@@
+ sandbox_check_access(&(E));
  E a E1;

@chk_fn@
identifier fn =~ "memset|memmove|memcpy|strcpy";
expression E, size;
@@

+ sandbox_check_access_n(E, size);
  fn(E, ..., size);