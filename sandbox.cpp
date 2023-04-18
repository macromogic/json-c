#include <cassert>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <unordered_map>

#include <unistd.h>

#include "sandbox.h"

#ifdef DEBUG
#include <iostream>

using std::cerr;
using std::dec;
using std::hex;
#endif

using std::greater;
using std::map;
using std::ifstream;
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

static unordered_map<subject_t, set<object_t>> *perm_set;
static unordered_map<subject_t, set<object_t>> *own_objs;
static unordered_map<object_t, set<var_info>> *obj_vars;
static map<uintptr_t, var_info, greater<uintptr_t>> *all_vars;

static bool sandbox_enabled = false;

extern "C" {

void __sandbox_init() __attribute__((constructor));
void __sandbox_deinit() __attribute__((destructor));

void __sandbox_init()
{
    sandbox_enabled = false;
    perm_set = new unordered_map<subject_t, set<object_t>>;
    own_objs = new unordered_map<subject_t, set<object_t>>;
    obj_vars = new unordered_map<object_t, set<var_info>>;
    all_vars = new map<uintptr_t, var_info, greater<uintptr_t>>;
    const char *env = getenv("SANDBOX");
    if (env != nullptr) {
        ifstream fin(env);
        if (!fin) {
            return;
        }
        sandbox_enabled = true;
        int n_files;
        fin >> n_files;
        for (int i = 0; i < n_files; ++i) {
            string filename;
            fin >> filename;
            int n_perms;
            fin >> n_perms;
            for (int j = 0; j < n_perms; ++j) {
                string object;
                fin >> object;
                (*perm_set)[filename].insert(object);
            }
        }
    }
}

void __sandbox_deinit()
{
    delete perm_set;
    delete own_objs;
    delete obj_vars;
    delete all_vars;
}

void __sandbox_register_var(const char *filename, const char* varname, void *addr, size_t size)
{
#ifdef DEBUG
    cerr << "registering " << filename << "::" << varname << " at " << hex << addr << dec << " of size " << size << "\n";
#endif
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
    (*own_objs)[filename].insert(varname_);
    (*obj_vars)[varname_].insert(var);
    (*all_vars)[addr_] = var;
#ifdef DEBUG
    cerr << "All vars:\n";
    for (auto& p : *all_vars) {
        cerr << p.second.filename << "::" << p.second.varname << " at " << hex << p.second.addr << dec << " of size " << p.second.size << "\n";
    }
    cerr << "\n";
#endif
}

void __sandbox_unregister_var(void *addr)
{
    if (!addr) {
        return;
    }
#ifdef DEBUG
    cerr << "unregistering " << hex << addr << dec << "\n";
#endif
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    auto it = all_vars->lower_bound(addr_);
    assert(it != all_vars->end());
    assert(it->first == addr_);
    const var_info& var = it->second;
    (*obj_vars)[var.varname].erase(var);
    all_vars->erase(it);
#ifdef DEBUG
    cerr << "All vars:\n";
    for (auto& p : *all_vars) {
        cerr << p.second.filename << "::" << p.second.varname << " at " << hex << p.second.addr << dec << " of size " << p.second.size << "\n";
    }
    cerr << "\n";
#endif
}

void __sandbox_check_access(const char *subject, void *addr, size_t size)
{
    if (!sandbox_enabled) {
        return;
    }
    void* heap_top = sbrk(0);
    if (addr >= heap_top) {
        return;
    }
    uintptr_t addr_ = reinterpret_cast<uintptr_t>(addr);
    auto it = all_vars->lower_bound(addr_);
    assert(it != all_vars->end());
    const var_info& var = it->second;
#ifdef DEBUG
    if ((*perm_set)[subject].count(var.varname) == 0) {
        cerr << "Access violation: " << subject << " -> " << var.varname << " in " << var.filename << "\n";
        cerr << "Perm set for " << subject << ":\n";
        for (const auto& obj : (*perm_set)[subject]) {
            cerr << obj << "\n";
        }
        abort();
    }
    if (!(addr_ >= var.addr && addr_ + size <= var.addr + var.size)) {
        cerr << "Access violation: " << subject << " -> " << var.varname << " in " << var.filename << "\n";
        cerr << "Accessing: " << hex << addr_ << " of size " << size << dec << "\n";
        cerr << "Var addres: " << hex << var.addr << " of size " << var.size << dec << "\n";
        cerr << "All vars:\n";
        for (const auto& var : *all_vars) {
            cerr << var.second.filename << "::" << var.second.varname << " at " << hex << var.second.addr << dec << " of size " << var.second.size << "\n";
        }
        abort();
    }
#else
    assert((*perm_set)[subject].count(var.filename) > 0);
    assert(addr_ >= var.addr && addr_ + size <= var.addr + var.size);
#endif
}

} // extern "C"