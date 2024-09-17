#pragma once

#include <windows.h>
#include <cstdint>
#include <map>

#include "samp.h"
#include "prototypes.h"

#include <kthook/kthook.hpp>
#include <RakHook/rakhook.hpp>


class c_plugin {

public:
	
	c_plugin();
	~c_plugin();

	kthook::kthook_simple<prototype::void_t> update_hook { reinterpret_cast<void*>(0x561B10) };
	void update(const decltype(update_hook)& hook);
	
};
