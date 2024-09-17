#include "plugin.h"

c_plugin* plugin;


int __stdcall DllMain(HMODULE instance, DWORD call_reason, LPVOID reserved) {

    if (call_reason == DLL_PROCESS_ATTACH) {
        
        DisableThreadLibraryCalls(instance);
        
        plugin = new c_plugin();
    }
    
    if (call_reason == DLL_PROCESS_DETACH)
        delete plugin;
    
    return TRUE;
}