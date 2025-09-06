#include <dlfcn.h>
#include <sys/mman.h>

#include "../print.hpp"

void *get_interface(const char *lib_path, const char *version) {
    void *lib_handle = dlopen(lib_path, RTLD_NOLOAD | RTLD_NOW);
    if (!lib_handle) {
        print("Failed to load %s\n", lib_path);
        return nullptr;
    }

    print("%s loaded at %p\n", lib_path, lib_handle);

    typedef void * (*CreateInterface)(const char *, int *);

    CreateInterface create_interface = CreateInterface(dlsym(lib_handle, "CreateInterface"));
    dlclose(lib_handle);

    if (!create_interface) {
        print("Failed to get CreateInterface\n");
        return nullptr;
    }

    print("%s factory found at %p\n", lib_path, create_interface);

    void *interface = create_interface(version, nullptr);

    if (!interface) {
        print("Failed to get %s interface\n", version);
        return nullptr;
    }

    print("%s interface found at %p\n", version, interface);

    return interface;
}

bool write_to_table(void **vtable, int index, void *func) {
    const long page_size = sysconf(_SC_PAGESIZE);

    // Sets last three digits to zero
    void *table_page = reinterpret_cast<void *>(reinterpret_cast<__uint64_t>(vtable) & ~(page_size - 1));
    print("vfunc table page found at %p\n", table_page);

    if (mprotect(table_page, page_size, PROT_READ | PROT_WRITE) != 0) {
        print("mprotect failed to change page protection\n");
        return false;
    }

    vtable[index] = func;

    if (mprotect(table_page, page_size, PROT_READ) != 0) {
        print("mprotect failed to reset page protection\n");
        return false;
    }

    return true;
}

bool sdl_hook(void *lib_handle, const char *func_name, void *hook, void **original) {
    void *func = dlsym(lib_handle, func_name);

    if (!func) {
        print("Failed to get %s\n", func_name);
        return false;
    }

    print("%s wrapper found at %p\n", func_name, func);

    /* The symbols resolve to a wrapper function. They are a just a 2 byte
     * `jmp` and then a SIGNED (?) 4 byte offset relative to the instruction
     * pointer. Adding `ip + the offset` is a pointer to the address of the
     * function that is wrapped. We save the orignal and call it in the hook. */
    const int offset = *reinterpret_cast<int *>(reinterpret_cast<unsigned long>(func) + 2);
    void **ptr_to_func = reinterpret_cast<void **>(reinterpret_cast<unsigned long>(func) + 6 + offset);
    *original = *ptr_to_func;

    print("Original %s at %p\n", func_name, *original);

    // The page should have rw perms already! No mprotect needed
    *ptr_to_func = hook;

    return true;
}

bool restore_sdl_hook(void *lib_handle, const char *func_name, void *original) {
    void *func = dlsym(lib_handle, func_name);

    if (!func) {
        print("Failed to get %s\n", func_name);
        return false;
    }

    int offset = *reinterpret_cast<int *>(reinterpret_cast<unsigned long>(func) + 2);
    *reinterpret_cast<void **>(reinterpret_cast<unsigned long>(func) + 6 + offset) = original;

    return true;
}
