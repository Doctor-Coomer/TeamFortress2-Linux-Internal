#include "memory.hpp"
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
#include "interfaces/game_event_manager.hpp"

#include "hooks/hooks.cpp"
#include "libsigscan/libsigscan.h"
#include "funchook/funchook.h"

#include "hooks/sdl.cpp"
#include "hooks/vulkan.cpp"

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
#include "hooks/fire_event_client_side.cpp"
#include "hooks/frame_stage_notify.cpp"

#include "vec.hpp"

void** client_mode_vtable;
void** vgui_vtable;
void** client_vtable;
void** model_render_vtable;
void** game_event_manager_vtable;

funchook_t* funchook;

__attribute__((constructor))
void entry() {  
  // Interfaces
  client = (Client*)get_interface("./tf/bin/linux64/client.so", "VClient017");
  engine = (Engine*)get_interface("./bin/linux64/engine.so", "VEngineClient014");
  
  vgui = get_interface("./bin/linux64/vgui2.so", "VGUI_Panel009");
  surface = (Surface*)get_interface("./bin/linux64/vguimatsurface.so", "VGUI_Surface030");

  unsigned long func_address = (unsigned long)sigscan_module("client.so", "48 8D 05 ? ? ? ? 48 8B 38 48 8B 07 FF 90 ? ? ? ? 48 8D 15 ? ? ? ? 84 C0");
  unsigned int input_eaddr = *(unsigned int*)(func_address + 0x3);
  unsigned long next_instruction = (unsigned long)(func_address + 0x7);
  input = (Input*)(*(void**)(next_instruction + input_eaddr));
  
  unsigned long check_stuck_address = (unsigned long)sigscan_module("client.so", "48 8D 05 ? ? ? ? 48 89 85 ? ? ? ? 74 ? 48 8B 38");
  unsigned int move_helper_eaddr = *(unsigned int*)(check_stuck_address + 0x3);
  unsigned long check_stuck_next_instruction = (unsigned long)(check_stuck_address + 0x7);
  move_helper = (MoveHelper*)(*(void**)(check_stuck_next_instruction + move_helper_eaddr));

  unsigned long rcon_addr_change_address = (unsigned long)sigscan_module("engine.so", "48 8D 05 ? ? ? ? 4C 8B 40");
  unsigned int client_state_eaddr = *(unsigned int*)(rcon_addr_change_address + 0x3);
  unsigned long rcon_addr_change_next_instruction = (unsigned long)(rcon_addr_change_address + 0x7);
  client_state = (ClientState*)((void*)(rcon_addr_change_next_instruction + client_state_eaddr));
  
  prediction = (Prediction*)get_interface("./tf/bin/linux64/client.so", "VClientPrediction001");

  game_movement = (GameMovement*)get_interface("./tf/bin/linux64/client.so", "GameMovement001");
  
  overlay = (DebugOverlay*)get_interface("./bin/linux64/engine.so", "VDebugOverlay003");
  
  entity_list = (EntityList*)get_interface("./tf/bin/linux64/client.so", "VClientEntityList003");

  render_view = (RenderView*)get_interface("./bin/linux64/engine.so", "VEngineRenderView014");

  engine_trace = (EngineTrace*)get_interface("./bin/linux64/engine.so", "EngineTraceClient003");

  model_render = (ModelRender*)get_interface("./bin/linux64/engine.so", "VEngineModel016");

  material_system = (MaterialSystem*)get_interface("./bin/linux64/materialsystem.so", "VMaterialSystem082");

  convar_system = (ConvarSystem*)get_interface("./bin/linux64/libvstdlib.so", "VEngineCvar004");

  game_event_manager = (GameEventManager*)get_interface("./bin/linux64/engine.so", "GAMEEVENTSMANAGER002");
  {
    std::vector<const char*> events = {	"client_beginconnect", "client_connected", "client_disconnect", "game_newmap",
					"teamplay_round_start", "scorestats_accumulated_update", "mvm_reset_stats",
					"player_connect_client", "player_spawn", "player_changeclass", "player_hurt",
					"vote_cast", "item_pickup", "revive_player_notify", "localplayer_respawn"};
    
    for (const char* event : events) {
      game_event_manager->add_listener((IGameEventListener*)game_event_manager, event, false);
      
      if (!game_event_manager->find_listener((IGameEventListener*)game_event_manager, event)) {
	print("Failed to add event listener: %s\n", event);
      }
    }
  }
  
  {
    steam_client = (SteamClient*)get_interface("../../../linux64/steamclient.so", "SteamClient020");
    int steam_pipe = steam_client->create_steam_pipe();
    int steam_user = steam_client->connect_to_global_user(steam_pipe);
    steam_friends = (SteamFriends*)steam_client->get_steam_friends_interface(steam_user, steam_pipe, "SteamFriends017");
  }

  client_vtable = *(void ***)client;
  void* hud_process_input_addr = client_vtable[10];
  __uint32_t client_mode_eaddr = *(__uint32_t *)((__uint64_t)(hud_process_input_addr) + 0x3);
  void* client_mode_next_instruction = (void *)((__uint64_t)(hud_process_input_addr) + 0x7);
  void* client_mode_interface = *(void **)((__uint64_t)(client_mode_next_instruction) + client_mode_eaddr);
  
  unsigned long hud_update = (unsigned long)client_vtable[11];
  unsigned int global_vars_eaddr = *(unsigned int *)(hud_update + 0x16);
  unsigned long global_vars_next_instruction = (unsigned long)(hud_update + 0x1A);
  global_vars = (GlobalVars*)(*(void **)(global_vars_next_instruction + global_vars_eaddr));

  // VMT Function Hooks
  client_mode_vtable = *(void***)client_mode_interface;  
  client_mode_create_move_original = (bool (*)(void*, float, user_cmd*))client_mode_vtable[22];
  if (!write_to_table(client_mode_vtable, 22, (void*)client_mode_create_move_hook)) {
    print("ClientModeShared::CreateMove hook failed\n");
  } else {
    print("ClientModeShared::CreateMove hooked\n");
  }

  client_create_move_original = (void (*)(void*, int, float, bool))client_vtable[21];
  if (!write_to_table(client_vtable, 21, (void*)client_create_move_hook)) {
    print("Client::CreateMove hook failed\n");
  } else {
    print("Client::CreateMove hooked\n");
  }
  
  override_view_original = (void (*)(void*, view_setup*))client_mode_vtable[17];  
  if (!write_to_table(client_mode_vtable, 17, (void*)override_view_hook)) {
    print("OverrideView hook failed\n");
  } else {
    print("OverrideView hooked\n");
  }

  draw_view_model_original = (bool (*)(void*))client_mode_vtable[25];  
  if (!write_to_table(client_mode_vtable, 25, (void*)draw_view_model_hook)) {
    print("ShouldDrawViewModel hook failed\n");
  } else {
    print("ShouldDrawViewModel hooked\n");
  }
  
  vgui_vtable = *(void ***)vgui;
  paint_traverse_original = (void (*)(void*, void*, __int8_t, __int8_t))vgui_vtable[42];  
  if (!write_to_table(vgui_vtable, 42, (void*)paint_traverse_hook)) {
    print("PaintTraverse hook failed\n");
  } else {
    print("PaintTraverse hooked\n");
  }

  model_render_vtable = *(void ***)model_render;    
  draw_model_execute_original = (void (*)(void*, void*, ModelRenderInfo_t*, VMatrix*))model_render_vtable[19];  
  if (!write_to_table(model_render_vtable, 19, (void*)draw_model_execute_hook)) {
    print("DrawModelExecute hook failed\n");
  } else {
    print("DrawModelExecute hooked\n");
  }
  
  game_event_manager_vtable = *(void***)game_event_manager;
  fire_event_client_side_original = (bool (*)(void*, GameEvent*))game_event_manager_vtable[9];
  if (!write_to_table(game_event_manager_vtable, 9, (void*)fire_event_client_side_hook)) {
    print("FireEventClientSide hook failed\n");
  } else {
    print("FireEventClientSide hooked\n");
  }  

  
  frame_stage_notify_original = (void (*)(void*, ClientFrameStage))client_vtable[35];
  if (!write_to_table(client_vtable, 35, (void*)frame_stage_notify_hook)) {
    print("FrameStageNotify hook failed\n");
  } else {
    print("FrameStageNotify hooked\n");
  }  
  
  
  // Non-VMT Function hooks
  funchook = funchook_create();
  
  in_cond_original = (bool (*)(void*, int))sigscan_module("client.so", "55 83 FE ? 48 89 E5 41 54 41 89 F4");
  
  load_white_list_original = (void* (*)(void*, const char*))sigscan_module("engine.so", "55 48 89 E5 41 55 41 54 49 89 FC 48 83 EC ? 48 8B 07 FF 50");

  cl_move_original = (void (*)(float, bool))sigscan_module("engine.so", "55 48 89 E5 41 57 41 56 41 55 41 54 53 48 83 EC ? 83 3D ? ? ? ? ? F3 0F 11 85");

  should_draw_local_player_original = (bool (*)(void*))sigscan_module("client.so", "55 48 89 E5 41 54 48 83 EC ? 48 8D 05 ? ? ? ? 48 8B 38 48 85 FF 74 ? 48 8B 07 FF 50");

  should_draw_this_player_original = (bool (*)(void*))sigscan_module("client.so", "55 48 89 E5 41 54 53 E8 ? ? ? ? 84 C0 75");

  draw_view_models_original = (void (*)(void*, view_setup*, bool))sigscan_module("client.so", "55 31 C0 48 89 E5 41 57 41 56 41 55 41 89 D5 41 54 49 89 FC 53 48 89 F3 48 81 EC");

  attribute_hook_value_float_original = (float (*)(float, const char*, Entity*, void*, bool))sigscan_module("client.so", "55 31 C0 48 89 E5 41 57 41 56 41 55 49 89 F5 41 54 49 89 FC 53 89 CB");
  
  int rv;
  
  rv = funchook_prepare(funchook, (void**)&in_cond_original, (void*)in_cond_hook);
  if (rv != 0) {
  }

  rv = funchook_prepare(funchook, (void**)&load_white_list_original, (void*)load_white_list_hook);
  if (rv != 0) {
  }

  rv = funchook_prepare(funchook, (void**)&cl_move_original, (void*)cl_move_hook);
  if (rv != 0) {
  }  

  rv = funchook_prepare(funchook, (void**)&should_draw_local_player_original, (void*)should_draw_local_player_hook);
  if (rv != 0) {
  }  

  rv = funchook_prepare(funchook, (void**)&should_draw_this_player_original, (void*)should_draw_this_player_hook);
  if (rv != 0) {
  }  

  rv = funchook_prepare(funchook, (void**)&draw_view_models_original, (void*)draw_view_models_hook);
  if (rv != 0) {
  }  

  key_values_constructor_original = (KeyValues* (*)(void*, const char*))sigscan_module("client.so", "55 31 C0 66 0F EF C0 48 89 E5 53");

  
  // Hook Vulkan if present
  // Determine if we're in Vulkan mode
  void* lib_dxvk_base_address = get_module_base_address("libdxvk_d3d9.so");
  if (lib_dxvk_base_address != nullptr) {  
    void* lib_vulkan_handle = dlopen("/run/host/usr/lib/libvulkan.so.1", RTLD_LAZY | RTLD_NOLOAD);
    if (lib_vulkan_handle == nullptr)
      lib_vulkan_handle = dlopen("/run/host/usr/lib64/libvulkan.so.1", RTLD_LAZY | RTLD_NOLOAD); // Some distributions have a lib64 directory instead
    
    if (lib_vulkan_handle != nullptr) {
      print("Vulkan loaded at %p\n", lib_vulkan_handle);

      // https://github.com/bruhmoment21/UniversalHookX/blob/main/UniversalHookX/src/hooks/backend/vulkan/hook_vulkan.cpp#L47
      VkInstanceCreateInfo create_info = {};
      constexpr const char* instance_extension = "VK_KHR_surface";

      create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      create_info.enabledExtensionCount = 1;
      create_info.ppEnabledExtensionNames = &instance_extension;
  
      // Create Vulkan Instance without any debug feature
      vkCreateInstance(&create_info, vk_allocator, &vk_instance);
  
      uint32_t gpu_count;
      vkEnumeratePhysicalDevices(vk_instance, &gpu_count, NULL);
      IM_ASSERT(gpu_count > 0);

      VkPhysicalDevice* gpus = new VkPhysicalDevice[sizeof(VkPhysicalDevice) * gpu_count];
      vkEnumeratePhysicalDevices(vk_instance, &gpu_count, gpus);

      // If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
      // most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
      // dedicated GPUs) is out of scope of this sample.
      int use_gpu = 0;
      for (int i = 0; i < (int)gpu_count; ++i) {
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(gpus[i], &properties);
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
	  use_gpu = i;
	  break;
	}
      }
	
      vk_physical_device = gpus[use_gpu];

      delete[] gpus;

      vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, NULL);

      queue_families = (VkQueueFamilyProperties*)malloc(count*sizeof(VkQueueFamilyProperties));

      vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device, &count, queue_families);

      for (uint32_t i = 0; i < count; ++i) {
	if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
	  queue_family = i;
	  break;
	}
      }
  
      if (queue_family == (uint32_t)-1) {
	print("queue_family fail\n");
      }
  
      constexpr const char* device_extension = "VK_KHR_swapchain";
      constexpr const float queue_priority = 1.0f;

      VkDeviceQueueCreateInfo queue_info = { };
      queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_info.queueFamilyIndex = queue_family;
      queue_info.queueCount = 1;
      queue_info.pQueuePriorities = &queue_priority;

      VkDeviceCreateInfo create_info2 = { };
      create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      create_info2.queueCreateInfoCount = 1;
      create_info2.pQueueCreateInfos = &queue_info;
      create_info.enabledExtensionCount = 1;
      create_info.ppEnabledExtensionNames = &device_extension;

      VkDevice vk_fake_device = VK_NULL_HANDLE;

      vkCreateDevice(vk_physical_device, (const VkDeviceCreateInfo*)&create_info, vk_allocator, &vk_fake_device);
      if (vk_fake_device == nullptr) {
	print("Failed to create Vulkan dummy device\n");
	return;
      }
      
      queue_present_original = (VkResult (*)(VkQueue, const VkPresentInfoKHR*))vkGetDeviceProcAddr(vk_fake_device, "vkQueuePresentKHR");
      acquire_next_image_original = (VkResult (*)(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*))vkGetDeviceProcAddr(vk_fake_device, "vkAcquireNextImageKHR");
      acquire_next_image2_original = (VkResult (*)(VkDevice, const VkAcquireNextImageInfoKHR*,  uint32_t*))vkGetDeviceProcAddr(vk_fake_device, "vkAcquireNextImage2KHR");
      create_swapchain_original = (VkResult (*)(VkDevice, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*))vkGetDeviceProcAddr(vk_fake_device, "vkCreateSwapchainKHR");

      vkDestroyDevice(vk_fake_device, vk_allocator);

      // Hook the functions
      rv = funchook_prepare(funchook, (void**)&queue_present_original, (void*)queue_present_hook);
      if (rv != 0) {
      }  

      rv = funchook_prepare(funchook, (void**)&acquire_next_image_original, (void*)acquire_next_image_hook);
      if (rv != 0) {
      }  

      rv = funchook_prepare(funchook, (void**)&acquire_next_image2_original, (void*)acquire_next_image2_hook);
      if (rv != 0) {
      }  

      rv = funchook_prepare(funchook, (void**)&create_swapchain_original, (void*)create_swapchain_hook);
      if (rv != 0) {
      }  

      dlclose(lib_vulkan_handle);
    }
  }
  
  rv = funchook_install(funchook, 0);
  if (rv != 0) {
    print("Non-VMT related hooks failed\n");
  }

  // Bespoke SDL hooking
  void* lib_sdl_handle = dlopen("/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0", RTLD_LAZY | RTLD_NOLOAD);

  if (!lib_sdl_handle) {
    print("Failed to load SDL2\n");
    return;
  }
 
  print("SDL2 loaded at %p\n", lib_sdl_handle);

  if (!sdl_hook(lib_sdl_handle, "SDL_PollEvent", (void*)poll_event_hook, (void **)&poll_event_original)) {
    print("Failed to hook SDL_PollEvent\n");
    return;
  }

  if (!sdl_hook(lib_sdl_handle, "SDL_GL_SwapWindow", (void*)swap_window_hook, (void **)&swap_window_original)) {
    print("Failed to hook SDL_GL_SwapWindow\n");
    return;
  }

  
  if (!sdl_hook(lib_sdl_handle, "SDL_GetWindowFlags", (void*)get_window_flags_hook, (void **)&get_window_flags_original)) {
    print("Failed to hook SDL_GetWindowFlags\n");
    return;
  }

  if (!sdl_hook(lib_sdl_handle, "SDL_GetWindowWMInfo", (void*)get_window_WM_info_hook, (void **)&get_window_WM_info_original)) {
    print("Failed to hook SDL_GetWindowWMInfo\n");
    return;
  }
  
  if (!sdl_hook(lib_sdl_handle, "SDL_GetWindowSize", (void*)get_window_size_hook, (void **)&get_window_size_original)) {
    print("Failed to hook SDL_GetWindowSize\n");
    return;
  }

  dlclose(lib_sdl_handle);
  
  // Misc static variables and hookable things
  unsigned long func_address_2 = (unsigned long)sigscan_module("client.so", "48 8D 05 ? ? ? ? BA ? ? ? ? 89 10"); // credz: vannie / @clsendmove on github
  unsigned int random_seed_eaddr = *(unsigned int*)(func_address_2 + 0x3);
  unsigned long func_address_2_next_instruction = (unsigned long)(func_address_2 + 0x7);
  random_seed = (uint32_t*)((void*)(func_address_2_next_instruction + random_seed_eaddr));
  
  return;
}

__attribute__((destructor))
void exit() {

  // Unhook VMT Functions
  if (!write_to_table(client_mode_vtable, 22, (void*)client_mode_create_move_original)) {
    print("ClientMode::CreateMove failed to restore hook\n");
  }

  if (!write_to_table(client_vtable, 21, (void*)client_create_move_original)) {
    print("Client::CreateMove failed to restore hook\n");
  }

  if (!write_to_table(client_mode_vtable, 17, (void*)override_view_original)) {
    print("OverrideView failed to restore hook\n");
  }

  if (!write_to_table(client_mode_vtable, 25, (void*)draw_view_model_original)) {
    print("ShouldDrawViewModel failed to restore hook\n");
  }
  
  if (!write_to_table(vgui_vtable, 42, (void*)paint_traverse_original)) {
    print("PaintTraverse failed to restore hook\n");
  }   

  if (!write_to_table(model_render_vtable, 19, (void*)draw_model_execute_original)) {
    print("DrawModelExecute failed to restore hook\n");
  }
  
  if (!write_to_table(game_event_manager_vtable, 9, (void*)fire_event_client_side_original)) {
    print("FireEventClientSide failed to restore hook\n");
  }

  if (!write_to_table(client_vtable, 35, (void*)frame_stage_notify_original)) {
    print("FrameStageNotify failed to restore hook\n");
  }

  
  // Unhook Non-VMT Functions
  funchook_uninstall(funchook, 0);

  // Unhook SDL
  void* lib_sdl_handle = dlopen("/usr/lib/x86_64-linux-gnu/libSDL2-2.0.so.0", RTLD_LAZY | RTLD_NOLOAD);

  if (!restore_sdl_hook(lib_sdl_handle, "SDL_GL_SwapWindow", (void*)swap_window_original)) {
    print("Failed to restore SDL_GL_SwapWindow\n");
  }
    
  if (!restore_sdl_hook(lib_sdl_handle, "SDL_PollEvent", (void*)poll_event_original)) {
    print("Failed to restore SDL_PollEvent\n");
  }

  if (!restore_sdl_hook(lib_sdl_handle, "SDL_GetWindowFlags", (void*)get_window_flags_original)) {
    print("Failed to restore SDL_GetWindowFlags\n");
  }

  if (!restore_sdl_hook(lib_sdl_handle, "SDL_GetWindowWMInfo", (void*)get_window_WM_info_original)) {
    print("Failed to restore SDL_GetWindowWMInfo\n");
  }

  if (!restore_sdl_hook(lib_sdl_handle, "SDL_GetWindowSize", (void*)get_window_size_original)) {
    print("Failed to restore SDL_GetWindowSize\n");
  }

  dlclose(lib_sdl_handle);

  // Fix thirdperson hack still being enabled when uninjecting
  {
    Player* localplayer = entity_list->get_localplayer();
    if (localplayer != nullptr)
      localplayer->set_thirdperson(false);
  }
  
  // Fix cursor visibility when we've removed/unhooked the menu
  if (menu_focused == true)
    surface->set_cursor_visible(!menu_focused);
}

