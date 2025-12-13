# Hooking Module

Function hooking and detouring framework using MinHook for runtime function interception.

## Classes

### `detour`

Individual function hook that redirects calls to a replacement function.

```cpp
#include <liborbisutil.h>

// Original function type
typedef int (*original_func_t)(int arg);

// Replacement function
int my_replacement(int arg) {
    // Call original if needed
    return hook.invoke<int>(arg * 2);
}

// Create and enable a detour
liborbisutil::hook::detour hook;
hook.create((void*)0x12345678, my_replacement, true);

// Or from a module export
hook.create("hook_name", "libSceModule.sprx", "functionName", my_replacement, true);

// Enable/disable at runtime
hook.disable();
hook.enable();

// Get original function pointer
auto original = hook.get<original_func_t>();
int result = original(42);

// Clean up
hook.clear();
```

### `manager`

Manages multiple named hooks for organized hook management.

```cpp
liborbisutil::hook::manager hooks;

// Create hooks by name
hooks.create("my_hook", (void*)target, replacement, true);
hooks.create("module_hook", "libModule.sprx", "exportFunc", replacement);

// Access hooks by name
hooks.enable("my_hook");
hooks.disable("my_hook");

// Get hook for direct access
auto& hook = hooks.get("my_hook");
hook.invoke<void>(args...);

// Remove hooks
hooks.remove("my_hook");
hooks.clear();  // Remove all
```

## Features

- Create hooks from raw addresses or module exports
- Template-based original function invocation
- Exception handling for missing hooks (`not_found_hook_exception`)
- Automatic cleanup on destruction
- Enable/disable hooks at runtime

## Example: Hooking a System Function

```cpp
liborbisutil::hook::manager hooks;

// Original signature
typedef int (*scePadRead_t)(int handle, ScePadData* data, int count);

// Replacement
int my_scePadRead(int handle, ScePadData* data, int count) {
    int result = hooks.get("padRead").invoke<int>(handle, data, count);
    
    // Modify input data here
    data->buttons |= SCE_PAD_BUTTON_CROSS;  // Force cross pressed
    
    return result;
}

// Install hook
hooks.create("padRead", "libScePad.sprx", "scePadRead", my_scePadRead, true);
```
