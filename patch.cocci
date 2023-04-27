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
identifier F, x, new;
type T, T1;
expression n, size, orig;
@@
F (...)
{
<...
(
   T x = (T1) malloc(size);
++ sandbox_register_var(F, x, x, size);
|   
   T x = (T1) realloc(..., size);
++ sandbox_register_var(F, x, x, size);
|
   T x = (T1) calloc(n, size);
++ sandbox_register_var(F, x, x, (n) * (size));
|
   T new = (T1) strdup(orig);
++ sandbox_register_var(F, new, new, strlen(orig) + 1);
)

...>
}

@reg_assign_mre@
identifier F, ALLOC=~"^malloc|realloc$";
expression size, E, E2;
assignment operator a;
@@
F (...)
{
<...
(
  E a E2;
+ sandbox_register_var(F, E, E, size);
&
  ALLOC(..., size)
)
...>
}

@reg_assign_c@
identifier F;
expression n, size, E, E2;
assignment operator a;
@@
F (...)
{
<...
(
  E a E2;
+ sandbox_register_var(F, E, E, (n) * (size));
&
  calloc(n, size)
)
...>
}

@reg_if_mre@
identifier F, ALLOC=~"^malloc|realloc$";
expression size, E, E2;
@@
F (...)
{
<...
(
  if (E = E2) {
+   sandbox_register_var(F, E, E, size);
    ...
  }
&
  E2
&
  ALLOC(..., size)
)
...>
}

@reg_if_n_mre@
identifier F, ALLOC=~"^malloc|realloc$";
expression size, E, E1, E2;
@@
F (...)
{
<...
(
  if (E) {
    ...
  }
+ sandbox_register_var(F, E1, E1, size);
&
  !(E1 = E2)
&
  ALLOC(..., size)
)
...>
}

@reg_if_c@
identifier F;
expression size, n, E, E2;
@@
F (...)
{
<...
(
  if (E = E2) {
+   sandbox_register_var(F, E, E, (n) * (size));
    ...
  }
&
  E2
&
  calloc(n, size)
)
...>
}

@reg_if_n_c@
identifier F;
expression size, n, E, E1, E2;
@@
F (...)
{
<...
(
  if (E) {
    ...
  }
+ sandbox_register_var(F, E1, E1, (n) * (size));
&
  !(E1 = E2)
&
  calloc(n, size)
)
...>
}

@reg_strdup@
identifier F;
expression new, orig;
@@
F (...) {
<...
  new = strdup(orig);
+ sandbox_register_var(F, new, new, strlen(orig) + 1);
...>
}

@unreg@
expression E;
@@
+ sandbox_unregister_var(E);
  free(E);

@chk_assign@
expression E2, E3;
statement S;
assignment operator a;
@@
(
++ sandbox_check_access(&(E2));
   S
&
   E2 a E3
)

@chk_fn@
identifier fn =~ "^(memset|memmove|memcpy|strcpy)$";
expression E, size;
@@

+ sandbox_check_access_n(E, size);
  fn(E, ..., size);

