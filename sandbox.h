#ifndef SANDBOX_H
#define SANDBOX_H

#define __str__(x) #x
#define __str(x) __str__(x)

#ifdef __cplusplus
extern "C" {
#else
#include <stddef.h>
#endif

#ifdef SANDBOX

void __sandbox_register_var(const char *filename, const char* varname, void *addr, size_t size);
void __sandbox_unregister_var(void *addr);
void __sandbox_check_access(const char *subject, void *addr, size_t size);

#define sandbox_register_var(funcname, varname, addr, size) __sandbox_register_var(__FILE__, __str__(funcname) "::" __str__(varname), addr, size)
#define sandbox_unregister_var(addr) __sandbox_unregister_var(addr)
#define sandbox_check_access(addr) __sandbox_check_access(__FILE__, addr, 1)
#define sandbox_check_access_n(addr, n) __sandbox_check_access(__FILE__, addr, n)

#else

#define sandbox_register_var(...)
#define sandbox_unregister_var(...)
#define sandbox_check_access(...)
#define sandbox_check_access_n(...)

#endif // SANDBOX

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SANDBOX_H