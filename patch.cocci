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

@reg_init@
identifier F, x;
type T, T1;
expression n, size;
@@
F (...)
{
<...
(
  T x = (T1) malloc(size);
+ sandbox_register_var(F, x, x, size);
|   
  T x = (T1) realloc(..., size);
+ sandbox_register_var(F, x, x, size);
|
  T x = (T1) calloc(n, size);
+ sandbox_register_var(F, x, x, (n) * (size));
)

...>
}

@reg_assign_mre@
identifier F, ALLOC=~"^malloc|realloc$";
expression size, E, E2;
@@
F (...)
{
<...
(
  E = E2;
+ sandbox_register_var(F, E, E, size);
&
  ALLOC(size);
)
...>
}

@reg_assign_c@
identifier F;
expression n, size, E, E2;
@@
F (...)
{
<...
(
  E = E2;
+ sandbox_register_var(F, E, E, (n) * (size));
&
  calloc(n, size);
)
...>
}

@reg_if_mre@
identifier F, ALLOC=~"^malloc|realloc$";
expression size, E, E2;
statement S;
@@
F (...)
{
<...
(
  if (E = E2) S
+ sandbox_register_var(F, E, E, size);
&
  ALLOC(size);
)
...>
}

@reg_if_c@
identifier F;
expression n, size, E, E2;
statement S;
@@
F (...)
{
<...
(
  if (E = E2) S
+ sandbox_register_var(F, E, E, (n) * (size));
&
  calloc(n, size);
)
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
identifier fn =~ "^(memset|memmove|memcpy|strcpy)$";
expression E, size;
@@

+ sandbox_check_access_n(E, size);
  fn(E, ..., size);

@chk_strdup@
identifier F;
expression E;
statement S;
@@
F (...) {
<...
(
  S
+ sandbox_register_var(F, E, E, strlen(E) + 1);
&
  strdup(E)
);
...>
}

