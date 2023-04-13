#include <cassert>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include "sandbox.h"

using std::greater;
using std::map;
using std::set;
using std::string;
using std::unordered_map;

struct sandbox_var {
    string filename;
    string varname;
    uintptr_t addr;
    size_t size;

    inline bool operator<(const sandbox_var& rhs) const
    {
        return addr < rhs.addr;
    }
};

static unordered_map<string, set<sandbox_var>> g_sandbox;
static unordered_map<string, set<string>> perm_set;
static map<uintptr_t, sandbox_var, greater<uintptr_t>> objs;

extern "C" {

void __sandbox_init() __attribute__((constructor));

void __sandbox_init()
{
    // TODO init read permissions
}

void __sandbox_register_var(const char *filename, const char* varname, void *addr, size_t size)
{
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    sandbox_var var = {filename, varname, addr_, size};
    g_sandbox[filename].insert(var);
    objs[addr_] = var;
}

void __sandbox_check_access(const char *subject, void *addr, size_t size)
{
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    const sandbox_var& var = objs.lower_bound(addr_)->second;
    // assert(perm_set[subject].count(var.filename) > 0); // FIXME perm_set is not initialized
    assert(addr_ >= var.addr && addr_ + size <= var.addr + var.size);
}

} // extern "C"