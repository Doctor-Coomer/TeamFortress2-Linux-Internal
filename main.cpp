#include "print.hpp"

#include "random_seed.hpp"

#include "classes/keyvalues.hpp"

#include <unistd.h>

#include "interfaces/engine.hpp"
#include "interfaces/client.hpp"
#include "interfaces/entity_list.hpp"
#include "interfaces/debug_overlay.hpp"
#include "interfaces/surface.hpp"
#include "interfaces/engine_trace.hpp"
#include "interfaces/render_view.hpp"
#include "interfaces/material_system.hpp"
#include "interfaces/model_render.hpp"
#include "interfaces/convar_system.hpp"
#include "interfaces/prediction.hpp"
#include "interfaces/steam_client.hpp"
#include "interfaces/steam_friends.hpp"
#include "interfaces/input.hpp"
#include "interfaces/attribute_manager.hpp"
#include "interfaces/global_vars.hpp"
#include "interfaces/move_helper.hpp"
#include "interfaces/game_movement.hpp"
#include "interfaces/client_state.hpp"

#include "hooks/hooks.cpp"
#include "libsigscan/libsigscan.h"
#include "funchook/funchook.h"

#include "hooks/sdl.cpp"
//#include "hooks/vulkan.cpp"

#include "hooks/client_mode_create_move.cpp"
#include "hooks/client_create_move.cpp"
#include "hooks/cl_move.cpp"
#include "hooks/paint_traverse.cpp"
#include "hooks/override_view.cpp"
#include "hooks/draw_view_model.cpp"
#include "hooks/in_cond.cpp"
#include "hooks/draw_model_execute.cpp"
#include "hooks/load_white_list.cpp"
#include "hooks/should_draw_local_player.cpp"
#include "hooks/should_draw_this_player.cpp"
#include "hooks/draw_view_models.cpp"
#include "hooks/enginevgui_paint.cpp"

#include "vec.hpp"

void **client_mode_vtable;
void **vgui_vtable;
void **client_vtable;
void **model_render_vtable;

funchook_t *funchook;

__attribute__((constructor))
void entry() {
    // Interfaces
    client = static_cast<Client *>(get_interface("./tf/bin/linux64/client.so", "VClient017"));
    engine = static_cast<Engine *>(get_interface("./bin/linux64/engine.so", "VEngineClient014"));

    vgui = get_interface("./bin/linux64/vgui2.so", "VGUI_Panel009");
    surface = static_cast<Surface *>(get_interface("./bin/linux64/vguimatsurface.so", "VGUI_Surface030"));

    const unsigned long func_address = reinterpret_cast<unsigned long>(sigscan_module("client.so",
        "48 8D 05 ? ? ? ? 48 8B 38 48 8B 07 FF 90 ? ? ? ? 48 8D 15 ? ? ? ? 84 C0"));
    const unsigned int input_eaddr = *reinterpret_cast<unsigned int *>(func_address + 0x3);
    const unsigned long next_instruction = func_address + 0x7;
    input = static_cast<Input *>(*reinterpret_cast<void **>(next_instruction + input_eaddr));

    const unsigned long check_stuck_address = reinterpret_cast<unsigned long>(sigscan_module(
        "client.so", "48 8D 05 ? ? ? ? 48 89 85 ? ? ? ? 74 ? 48 8B 38"));
    const unsigned int move_helper_eaddr = *reinterpret_cast<unsigned int *>(check_stuck_address + 0x3);
    const unsigned long check_stuck_next_instruction = check_stuck_address + 0x7;
    move_helper = static_cast<MoveHelper *>(*reinterpret_cast<void **>(
        check_stuck_next_instruction + move_helper_eaddr));

    const unsigned long rcon_addr_change_address = reinterpret_cast<unsigned long>(sigscan_module(
        "engine.so", "48 8D 05 ? ? ? ? 4C 8B 40"));
    const unsigned int client_state_eaddr = *reinterpret_cast<unsigned int *>(rcon_addr_change_address + 0x3);
    const unsigned long rcon_addr_change_next_instruction = rcon_addr_change_address + 0x7;
    client_state = static_cast<ClientState *>(reinterpret_cast<void *>(
        rcon_addr_change_next_instruction + client_state_eaddr));

    prediction = static_cast<Prediction *>(get_interface("./tf/bin/linux64/client.so", "VClientPrediction001"));

    game_movement = static_cast<GameMovement *>(get_interface("./tf/bin/linux64/client.so", "GameMovement001"));

    overlay = static_cast<DebugOverlay *>(get_interface("./bin/linux64/engine.so", "VDebugOverlay003"));

    entity_list = static_cast<EntityList *>(get_interface("./tf/bin/linux64/client.so", "VClientEntityList003"));

    render_view = static_cast<RenderView *>(get_interface("./bin/linux64/engine.so", "VEngineRenderView014"));

    engine_trace = static_cast<EngineTrace *>(get_interface("./bin/linux64/engine.so", "EngineTraceClient003"));

    model_render = static_cast<ModelRender *>(get_interface("./bin/linux64/engine.so", "VEngineModel016"));

    material_system = static_cast<MaterialSystem *>(get_interface("./bin/linux64/materialsystem.so",
                                                                  "VMaterialSystem082"));

    convar_system = static_cast<ConvarSystem *>(get_interface("./bin/linux64/libvstdlib.so", "VEngineCvar004"));

    prediction = static_cast<Prediction *>(get_interface("./tf/bin/linux64/client.so", "VClientPrediction001"));

    steam_client = static_cast<SteamClient *>(get_interface("../../../linux64/steamclient.so", "SteamClient020"));

    const int steam_pipe = steam_client->create_steam_pipe();
    const int steam_user = steam_client->connect_to_global_user(steam_pipe);
    steam_friends = static_cast<SteamFriends *>(steam_client->get_steam_friends_interface(
        steam_user, steam_pipe, "SteamFriends017"));

    client_vtable = *reinterpret_cast<void ***>(client);
    void *hud_process_input_addr = client_vtable[10];
    const __uint32_t client_mode_eaddr = *reinterpret_cast<__uint32_t *>(
        reinterpret_cast<__uint64_t>(hud_process_input_addr) + 0x3);
    void *client_mode_next_instruction = reinterpret_cast<void *>(
        reinterpret_cast<__uint64_t>(hud_process_input_addr) + 0x7);
    void *client_mode_interface = *reinterpret_cast<void **>(
        reinterpret_cast<__uint64_t>(client_mode_next_instruction) + client_mode_eaddr);

    const unsigned long hud_update = reinterpret_cast<unsigned long>(client_vtable[11]);
    const unsigned int global_vars_eaddr = *reinterpret_cast<unsigned int *>(hud_update + 0x16);
    const unsigned long global_vars_next_instruction = hud_update + 0x1A;
    global_vars = static_cast<GlobalVars *>(*reinterpret_cast<void **>(
        global_vars_next_instruction + global_vars_eaddr));

    // VMT Function Hooks
    client_mode_vtable = *static_cast<void ***>(client_mode_interface);
    client_mode_create_move_original = (bool (*)(void *, float, user_cmd *)) client_mode_vtable[22];
    if (!write_to_table(client_mode_vtable, 22, (void *) client_mode_create_move_hook)) {
        print("ClientModeShared::CreateMove hook failed\n");
    } else {
        print("ClientModeShared::CreateMove hooked\n");
    }

    client_create_move_original = (void (*)(void *, int, float, bool)) client_vtable[21];
    if (!write_to_table(client_vtable, 21, (void *) client_create_move_hook)) {
        print("Client::CreateMove hook failed\n");
    } else {
        print("Client::CreateMove hooked\n");
    }

    override_view_original = (void (*)(void *, view_setup *)) client_mode_vtable[17];
    if (!write_to_table(client_mode_vtable, 17, (void *) override_view_hook)) {
        print("OverrideView hook failed\n");
    } else {
        print("OverrideView hooked\n");
    }

    draw_view_model_original = (bool (*)(void *)) client_mode_vtable[25];
    if (!write_to_table(client_mode_vtable, 25, (void *) draw_view_model_hook)) {
        print("ShouldDrawViewModel hook failed\n");
    } else {
        print("ShouldDrawViewModel hooked\n");
    }

    vgui_vtable = *static_cast<void ***>(vgui);

    paint_traverse_original = (void (*)(void *, void *, __int8_t, __int8_t)) vgui_vtable[42];
    if (!write_to_table(vgui_vtable, 42, (void *) paint_traverse_hook)) {
        print("PaintTraverse hook failed\n");
    } else {
        print("PaintTraverse hooked\n");
    }

    model_render_vtable = *reinterpret_cast<void ***>(model_render);

    draw_model_execute_original = (void (*)(void *, void *, ModelRenderInfo_t *, VMatrix *)) model_render_vtable[19];
    if (!write_to_table(model_render_vtable, 19, (void *) draw_model_execute_hook)) {
        print("DrawModelExecute hook failed\n");
    } else {
        print("DrawModelExecute hooked\n");
    }

    // Non-VMT Function hooks
    funchook = funchook_create();

    in_cond_original = (bool (*)(void *, int)) sigscan_module("client.so", "55 83 FE ? 48 89 E5 41 54 41 89 F4");

    load_white_list_original = (void* (*)(void *, const char *)) sigscan_module(
        "engine.so", "55 48 89 E5 41 55 41 54 49 89 FC 48 83 EC ? 48 8B 07 FF 50");

    enginevgui_paint_original = (void (*)(void *, int)) sigscan_module(
        "engine.so", "55 31 C0 48 89 E5 41 57 41 89 F7 41 56 41 55 41 54 53 48 89 FB 48 83 EC");

    cl_move_original = (void (*)(float, bool)) sigscan_module("engine.so",
                                                              "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC ? 83 3D ? ? ? ? ? F3 0F 11 85");

    should_draw_local_player_original = (bool (*)(void *)) sigscan_module(
        "client.so", "55 48 89 E5 41 54 48 83 EC ? 48 8D 05 ? ? ? ? 48 8B 38 48 85 FF 74 ? 48 8B 07 FF 50");

    should_draw_this_player_original = (bool (*)(void *)) sigscan_module(
        "client.so", "55 48 89 E5 41 54 53 E8 ? ? ? ? 84 C0 75");

    draw_view_models_original = (void (*)(void *, view_setup *, bool)) sigscan_module(
        "client.so", "55 31 C0 48 89 E5 41 57 41 56 41 55 41 89 D5 41 54 49 89 FC 53 48 89 F3 48 81 EC");

    attribute_hook_value_float_original = (float (*)(float, const char *, Entity *, void *, bool)) sigscan_module(
        "client.so", "55 31 C0 48 89 E5 41 57 41 56 41 55 49 89 F5 41 54 49 89 FC 53 89 CB");

    int rv = funchook_prepare(funchook, reinterpret_cast<void **>(&in_cond_original), (void *) in_cond_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&load_white_list_original),
                          (void *) load_white_list_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&enginevgui_paint_original),
                          (void *) enginevgui_paint_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&cl_move_original), (void *) cl_move_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&should_draw_local_player_original),
                          (void *) should_draw_local_player_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&should_draw_this_player_original),
                          (void *) should_draw_this_player_hook);
    if (rv != 0) {
    }

    rv = funchook_prepare(funchook, reinterpret_cast<void **>(&draw_view_models_original),
                          (void *) draw_view_models_hook);
    if (rv != 0) {
    }

    key_values_constructor_original = (KeyValues* (*)(void *, const char *)) sigscan_module(
        "client.so", "55 31 C0 66 0F EF C0 48 89 E5 53");

    rv = funchook_install(funchook, 0);
    if (rv != 0) {
        print("Non-VMT related hooks failed\n");
    } else {
        print("InCond hooked\n");
        print("LoadWhiteList hooked\n");
        print("CL_Move hooked\n");
        //print("vkQueuePresentKHR hooked\n");
    }

    // Bespoke SDL hooking
    void *lib_sdl_handle = dlopen("/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0", RTLD_LAZY | RTLD_NOLOAD);

    if (!lib_sdl_handle) {
        print("Failed to load SDL2\n");
        return;
    }

    print("SDL2 loaded at %p\n", lib_sdl_handle);

    if (!sdl_hook(lib_sdl_handle, "SDL_PollEvent", (void *) poll_event_hook, (void **) &poll_event_original)) {
        print("Failed to hook SDL_PollEvent\n");
        return;
    }

    if (!sdl_hook(lib_sdl_handle, "SDL_GL_SwapWindow", (void *) swap_window_hook, (void **) &swap_window_original)) {
        print("Failed to hook SDL_GL_SwapWindow\n");
        return;
    }

    dlclose(lib_sdl_handle);

    // TODO: Hook vulkan
    /*
    void* lib_vulkan_handle = dlopen("/run/host/usr/lib/libvulkan.so.1.4.313", RTLD_LAZY | RTLD_NOLOAD);

    if (!lib_vulkan_handle) {
      print("Failed to load vulkan\n");
      return;
    }

    queue_present_original = (VkResult (*)(VkQueue, const VkPresentInfoKHR*))dlsym(lib_vulkan_handle, "vkQueuePresentKHR");

    if (!queue_present_original) {
      print("Failed to locate vkQueuePresentKHR\n");
    } else {
      print("vkQueuePresentKHR located at %p\n", queue_present_original);
    }

    rv = funchook_prepare(funchook, (void**)&queue_present_original, (void*)queue_present_hook);
    if (rv != 0) {
    }
    */

    //dlclose(lib_vulkan_handle);


    // Misc static variables and hookable things
    const unsigned long func_address_2 = reinterpret_cast<unsigned long>(sigscan_module(
        "client.so", "48 8D 05 ? ? ? ? BA ? ? ? ? 89 10")); // credz: vannie / @clsendmove on github
    const unsigned int random_seed_eaddr = *reinterpret_cast<unsigned int *>(func_address_2 + 0x3);
    const unsigned long func_address_2_next_instruction = func_address_2 + 0x7;
    random_seed = static_cast<uint32_t *>(reinterpret_cast<void *>(
        func_address_2_next_instruction + random_seed_eaddr));
}

__attribute__((destructor))
void exit() {
    // Unhook VMT Functions
    if (!write_to_table(client_mode_vtable, 22, (void *) client_mode_create_move_original)) {
        print("ClientMode::CreateMove failed to restore hook\n");
    }

    if (!write_to_table(client_vtable, 21, (void *) client_create_move_original)) {
        print("Client::CreateMove failed to restore hook\n");
    }

    if (!write_to_table(client_mode_vtable, 17, (void *) override_view_original)) {
        print("OverrideView failed to restore hook\n");
    }

    if (!write_to_table(client_mode_vtable, 25, (void *) draw_view_model_original)) {
        print("ShouldDrawViewModel failed to restore hook\n");
    }

    if (!write_to_table(vgui_vtable, 42, (void *) paint_traverse_original)) {
        print("PaintTraverse failed to restore hook\n");
    }

    if (!write_to_table(model_render_vtable, 19, (void *) draw_model_execute_original)) {
        print("DrawModelExecute failed to restore hook\n");
    }

    // Unhook Non-VMT Functions
    funchook_uninstall(funchook, 0);

    // Unhook SDL
    void *lib_sdl_handle = dlopen("/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0", RTLD_LAZY | RTLD_NOLOAD);

    if (!restore_sdl_hook(lib_sdl_handle, "SDL_GL_SwapWindow", (void *) swap_window_original)) {
        print("Failed to restore SDL_GL_SwapWindow\n");
    }

    if (!restore_sdl_hook(lib_sdl_handle, "SDL_PollEvent", (void *) poll_event_original)) {
        print("Failed to restore SDL_PollEvent\n");
    }

    dlclose(lib_sdl_handle);

    // Fix thirdperson hack still being enabled when not injected
    {
        Player *localplayer = entity_list->get_localplayer();
        localplayer->set_thirdperson(false);
    }

    // Fix cursor visibility when we've removed/unhooked the menu
    if (menu_focused)
        surface->set_cursor_visible(!menu_focused);
}
