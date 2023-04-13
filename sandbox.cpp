#include <cassert>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include <unistd.h>

#include "sandbox.h"

using std::greater;
using std::map;
using std::set;
using std::string;
using std::unordered_map;

using subject_t = string;
using object_t = string;

struct var_info {
    string filename;
    string varname;
    uintptr_t addr;
    size_t size;

    inline bool operator<(const var_info& rhs) const
    {
        return varname < rhs.varname;
    }
};

static unordered_map<subject_t, set<object_t>> perm_set;
static unordered_map<subject_t, set<object_t>> own_objs;
static unordered_map<object_t, set<var_info>> obj_vars;
static map<uintptr_t, var_info, greater<uintptr_t>> all_vars;

extern "C" {

void __sandbox_init() __attribute__((constructor));

void __sandbox_init()
{
    // TODO init read permissions
}

void __sandbox_register_var(const char *filename, const char* varname, void *addr, size_t size)
{
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    string varname_(varname);
    size_t i = 0;
    for (; i < varname_.size(); ++i) {
        if (varname[i] == '[' || varname[i] == '-' || varname[i] == '.') {
            break;
        }
    }
    varname_ = varname_.substr(0, i);
    var_info var = {filename, varname_, addr_, size};
    own_objs[filename].insert(varname_);
    obj_vars[varname_].insert(var);
    all_vars[addr_] = var;
}

void __sandbox_unregister_var(void *addr)
{
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    auto it = all_vars.lower_bound(addr_);
    assert(it != all_vars.end());
    assert(it->first == addr_);
    const var_info& var = it->second;
    obj_vars[var.varname].erase(var);
    all_vars.erase(it);
}

void __sandbox_check_access(const char *subject, void *addr, size_t size)
{
    void* heap_top = sbrk(0);
    if (addr >= heap_top) {
        return;
    }
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    auto it = all_vars.lower_bound(addr_);
    assert(it != all_vars.end());
    const var_info& var = it->second;
    // assert(perm_set[subject].count(var.filename) > 0); // FIXME perm_set is not initialized
    assert(addr_ >= var.addr && addr_ + size <= var.addr + var.size);
}

} // extern "C"