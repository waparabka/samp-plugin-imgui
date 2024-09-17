#include "render.h"
#include "image.h"


using namespace std::placeholders;


using init_game_instance_t = HWND(__cdecl*)(HINSTANCE);

kthook::kthook_signal<init_game_instance_t> hook_game_instance { 0x745560 };

HWND game_window_handle = []() {

    HWND* window_handle = *reinterpret_cast<HWND**>(0xC17054);

    if (window_handle != nullptr)
        return *window_handle;
    
    hook_game_instance.after += [](const auto& hook, HWND& return_hwnd, HINSTANCE instance) { game_window_handle = return_hwnd; };

    return HWND(0);
}();


c_render::c_render() : imgui_init(false), gui() {
	
    present_hook.set_dest(get_function_address(17));
    reset_hook.set_dest(get_function_address(16));
    
    present_hook.before += std::bind(&c_render::d3d9_present, this, _1, _2, _3, _4, _5, _6);
    reset_hook.before += std::bind(&c_render::d3d9_lost, this, _1, _2, _3);
    reset_hook.after += std::bind(&c_render::d3d9_reset, this, _1, _2, _3, _4);
    
    present_hook.install();
    reset_hook.install();
}


uintptr_t c_render::find_device(uint32_t length) {

    static uintptr_t base = [](size_t length) {

        std::string path(MAX_PATH, '\0');

        if (auto size = GetSystemDirectoryA(path.data(), MAX_PATH)) {

            path.resize(size);
            path += "\\d3d9.dll";

            uintptr_t object_base = reinterpret_cast<uintptr_t>(LoadLibraryA(path.c_str()));

            while (object_base++ < object_base + length) {

                if (*reinterpret_cast<uint16_t*>(object_base + 0x00) == 0x06C7 && *reinterpret_cast<uint16_t*>(object_base + 0x06) == 0x8689 && *reinterpret_cast<uint16_t*>(object_base + 0x0C) == 0x8689) {

                    object_base += 2;

                    break;
                }
            }

            return object_base;
        }
        
        return uintptr_t(0);
        
    }(length);

    return base;
}


void* c_render::get_function_address(int virtual_table_index) {
    
    return (*reinterpret_cast<void***>(find_device(0x128000)))[virtual_table_index];
}


std::optional<HRESULT> c_render::d3d9_present(const decltype(present_hook)& hook, IDirect3DDevice9* device, CONST RECT* src_rect, CONST RECT* dest_rect, HWND dest_window, CONST RGNDATA* dirty_region) {
    
    if (!imgui_init) {

        ImGui::CreateContext();

        ImGui_ImplWin32_Init(game_window_handle);
        ImGui_ImplDX9_Init(device);

        ImGui::GetIO().IniFilename = nullptr;
        ImGui::GetIO().MouseDrawCursor = false;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        
        D3DXCreateTextureFromFileInMemory(device, &logo, sizeof(logo), &gui.texture_logo);
        D3DXCreateTextureFromFileInMemory(device, &settings, sizeof(settings), &gui.texture_settings);
        
#pragma warning(push)
#pragma warning(disable: 4996)
        std::string font{ getenv("WINDIR") }; font += "\\Fonts\\trebucbd.TTF";
#pragma warning(pop)
        ImGui::GetIO().Fonts->AddFontFromFileTTF(font.c_str(), 15.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

        auto latest_wndproc_ptr = GetWindowLongPtrA(game_window_handle, GWLP_WNDPROC);

        using namespace std::placeholders;

        wnd_proc_hook.set_dest(latest_wndproc_ptr);
        wnd_proc_hook.set_cb(std::bind(&c_render::wnd_proc_handler, this, _1, _2, _3, _4, _5));

        wnd_proc_hook.install();
        
        gui.init();

        imgui_init = true;
        
        printf("imgui inited\n");
    }

    if (ImGui::GetCurrentContext()) {

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();
        
        gui.process();
        
        ImGui::EndFrame();
        ImGui::Render();
        
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return std::nullopt;
}


std::optional<HRESULT> c_render::d3d9_lost(const decltype(reset_hook)& hook, LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS* present_params) {
    
    ImGui_ImplDX9_InvalidateDeviceObjects();
    
    return std::nullopt;
}


void c_render::d3d9_reset(const decltype(reset_hook)& hook, HRESULT& return_value, IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params) {

    
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

HRESULT c_render::wnd_proc_handler(const decltype(wnd_proc_hook)& hook, HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param) {

    if (!gui.menu_open)
        return hook.get_trampoline()(hwnd, u_msg, w_param, l_param);

    switch (u_msg) {

        case WM_KEYDOWN: {

            if (w_param == VK_ESCAPE && gui.menu_open) {

                gui.menu_open = false;

                //samp::RefGame()->SetCursorMode(samp::CURSOR_NONE, true);

                return 1;
            }

            break;
        }
    }

    if (u_msg == WM_CHAR) {

        wchar_t wch;

        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, reinterpret_cast<char*>(&w_param), 1, &wch, 1);

        w_param = wch;
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, u_msg, w_param, l_param);

    return hook.get_trampoline()(hwnd, u_msg, w_param, l_param);
}


c_render::~c_render() {

    present_hook.remove();
    reset_hook.remove();
    wnd_proc_hook.remove();

    hook_game_instance.remove();


    if (ImGui::GetCurrentContext()) {

        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();

        ImGui::DestroyContext();
    }

    printf("render dctor\n");
}