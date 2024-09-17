#include "plugin.h"



c_plugin::c_plugin() {

    using namespace std::placeholders;
    
    update_hook.set_cb(std::bind(&c_plugin::update, this, _1));
    update_hook.install();
}


void c_plugin::update(const decltype(update_hook)& hook) {

    static bool inited = false;
    
    if (!inited && c_chat::get()->ref() != nullptr && rakhook::initialize()) {
        
        // do some staff..
        
        inited = true;
    }

    return hook.get_trampoline()();
}


c_plugin::~c_plugin() {

    update_hook.remove();
}