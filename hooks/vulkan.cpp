#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>

#include "../imgui/imgui_impl_vulkan.h"
#include "../imgui/imgui_impl_sdl2.h"

#include "../interfaces/surface.hpp"
#include "../interfaces/engine.hpp"

#include "../hacks/esp/esp.cpp"

#include "../print.hpp"

#include "../gui/menu.hpp"

static VkDevice vk_device;
static VkAllocationCallbacks* vk_allocator = NULL;
static VkQueueFamilyProperties* queue_families;
static uint32_t queue_family = (uint32_t)-1;
static VkRenderPass vk_render_pass = VK_NULL_HANDLE;
static VkDescriptorPool vk_descriptor_pool = VK_NULL_HANDLE;
static VkExtent2D vk_image_extent = { };
static VkPhysicalDevice vk_physical_device = VK_NULL_HANDLE;
static VkInstance vk_instance = VK_NULL_HANDLE;
static VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
static uint32_t min_image_count = 3;
static ImGui_ImplVulkanH_Frame g_Frames[8] = { };
static ImGui_ImplVulkanH_FrameSemaphores g_FrameSemaphores[8] = { };
static uint32_t count;

VkResult (*queue_present_original)(VkQueue, const VkPresentInfoKHR*) = NULL;
VkResult (*create_swapchain_original)(VkDevice, const VkSwapchainCreateInfoKHR*, const VkAllocationCallbacks*, VkSwapchainKHR*) = NULL;
VkResult (*acquire_next_image_original)(VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t*) = NULL;
VkResult (*acquire_next_image2_original)(VkDevice, const VkAcquireNextImageInfoKHR*,  uint32_t*) = NULL;

VkResult queue_present_hook(VkQueue queue, const VkPresentInfoKHR* present_info) {

  // We haven't gotten the device yet
  if (!vk_device) {
    return queue_present_original(queue, present_info);
  }

  // We havn't gotten SDL's window context yet
  if (sdl_window == nullptr) {
    return queue_present_original(queue, present_info);    
  }
  

  // Initialize ImGui context if we haven't already
  if (ImGui::GetCurrentContext() == nullptr) {
    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForVulkan(sdl_window);
  }

  // https://github.com/bruhmoment21/UniversalHookX/blob/main/UniversalHookX/src/hooks/backend/vulkan/hook_vulkan.cpp#L418
  VkQueue graphicQueue = VK_NULL_HANDLE;  
  for (uint32_t i = 0; i < count; ++i) {
    const VkQueueFamilyProperties& family = queue_families[i];
    for (uint32_t j = 0; j < family.queueCount; ++j) {
      VkQueue it = VK_NULL_HANDLE;
      vkGetDeviceQueue(vk_device, i, j, &it);

      if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {     	
	if (graphicQueue == VK_NULL_HANDLE) {
	  graphicQueue = it;
	}
      }
    }
  }
  
  for (uint32_t i = 0; i < present_info->swapchainCount; ++i) {
    VkSwapchainKHR swapchain = present_info->pSwapchains[i];
    if (g_Frames[0].Framebuffer == VK_NULL_HANDLE) {
    // begin creating render target
      uint32_t uImageCount;
      vkGetSwapchainImagesKHR(vk_device, swapchain, &uImageCount, NULL);

      VkImage backbuffers[8] = { };
      vkGetSwapchainImagesKHR(vk_device, swapchain, &uImageCount, backbuffers);
      
      for (uint32_t i = 0; i < uImageCount; ++i) {
	g_Frames[i].Backbuffer = backbuffers[i];

	ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
	ImGui_ImplVulkanH_FrameSemaphores* fsd = &g_FrameSemaphores[i];
	{
	  VkCommandPoolCreateInfo info = { };
	  info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	  info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	  info.queueFamilyIndex = queue_family;

	  vkCreateCommandPool(vk_device, &info, vk_allocator, &fd->CommandPool);
	}
	{
	  VkCommandBufferAllocateInfo info = { };
	  info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	  info.commandPool = fd->CommandPool;
	  info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	  info.commandBufferCount = 1;

	  vkAllocateCommandBuffers(vk_device, &info, &fd->CommandBuffer);
	}
	{
	  VkFenceCreateInfo info = { };
	  info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	  info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	  vkCreateFence(vk_device, &info, vk_allocator, &fd->Fence);
	}

	{
	  VkSemaphoreCreateInfo info = { };
	  info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	  vkCreateSemaphore(vk_device, &info, vk_allocator, &fsd->ImageAcquiredSemaphore);
	  vkCreateSemaphore(vk_device, &info, vk_allocator, &fsd->RenderCompleteSemaphore);
	}
      }

      // Create the Render Pass
      {
	VkAttachmentDescription attachment = { };
	attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment = { };
	color_attachment.attachment = 0;
	color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	VkRenderPassCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.attachmentCount = 1;
	info.pAttachments = &attachment;
	info.subpassCount = 1;
	info.pSubpasses = &subpass;

	if (vkCreateRenderPass(vk_device, &info, vk_allocator, &vk_render_pass) != VK_SUCCESS) {
	  print("renderpass err\n");
	  return queue_present_original(queue, present_info);
	}
      }

      // Create The Image Views
      {
	VkImageViewCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	info.format = VK_FORMAT_B8G8R8A8_UNORM;

	info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	info.subresourceRange.baseMipLevel = 0;
	info.subresourceRange.levelCount = 1;
	info.subresourceRange.baseArrayLayer = 0;
	info.subresourceRange.layerCount = 1;

	for (uint32_t i = 0; i < uImageCount; ++i) {
	  ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
	  info.image = fd->Backbuffer;

	  vkCreateImageView(vk_device, &info, vk_allocator, &fd->BackbufferView);
	}
      }
      
      // Create Framebuffer
      {
	VkImageView attachment[1];
	VkFramebufferCreateInfo info = { };
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.renderPass = vk_render_pass;
	info.attachmentCount = 1;
	info.pAttachments = attachment;
	info.layers = 1;

	for (uint32_t i = 0; i < uImageCount; ++i) {
	  ImGui_ImplVulkanH_Frame* fd = &g_Frames[i];
	  attachment[0] = fd->BackbufferView;

	  if (vkCreateFramebuffer(vk_device, &info, vk_allocator, &fd->Framebuffer) != VK_SUCCESS) {
	    print("framebuffer err\n");
	    return queue_present_original(queue, present_info);
	  }
	}
      }

      if (!vk_descriptor_pool) // Create Descriptor Pool.
	{
	  constexpr VkDescriptorPoolSize pool_sizes[] =
	    {
	      {VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
	      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
	      {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
	      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
	      {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
	      {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
	      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
	      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
	      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
	      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
	      {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

	  VkDescriptorPoolCreateInfo pool_info = { };
	  pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	  pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	  pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
	  pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
	  pool_info.pPoolSizes = pool_sizes;

	  vkCreateDescriptorPool(vk_device, &pool_info, vk_allocator, &vk_descriptor_pool);
	}
	//end creating render target
    }
    
    ImGui_ImplVulkanH_Frame* fd = &g_Frames[present_info->pImageIndices[i]];
    ImGui_ImplVulkanH_FrameSemaphores* fsd = &g_FrameSemaphores[present_info->pImageIndices[i]];
    {
      vkWaitForFences(vk_device, 1, &fd->Fence, VK_TRUE, ~0ull);
      //vkResetFences(vk_device, 1, &fd->Fence);
    }

    {
      vkResetCommandBuffer(fd->CommandBuffer, 0);

      VkCommandBufferBeginInfo info = { };
      info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      vkBeginCommandBuffer(fd->CommandBuffer, &info);
    }
    
    {
      VkRenderPassBeginInfo info = { };
      info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass = vk_render_pass;
      info.framebuffer = fd->Framebuffer;
      if (vk_image_extent.width == 0 || vk_image_extent.height == 0) {
	Vec2 resolution = engine->get_screen_size();

	// We don't know the window size the first time. So we just set it to 4K.

	info.renderArea.extent.width = resolution.x;
	info.renderArea.extent.height = resolution.y;
      } else {
	info.renderArea.extent = vk_image_extent;
      }

      vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
    }
    
    if (!ImGui::GetIO().BackendRendererUserData) {
      ImGui_ImplVulkan_InitInfo init_info = { };
      init_info.Instance = vk_instance;
      init_info.PhysicalDevice = vk_physical_device;
      init_info.Device = vk_device;
      init_info.ApiVersion = VK_API_VERSION_1_4;
      init_info.QueueFamily = queue_family;
      init_info.Queue = graphicQueue;
      init_info.PipelineCache = vk_pipeline_cache;
      init_info.DescriptorPool = vk_descriptor_pool;
      init_info.Subpass = 0;
      init_info.MinImageCount = min_image_count;
      init_info.ImageCount = min_image_count;
      init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
      init_info.Allocator = vk_allocator;
      init_info.RenderPass = vk_render_pass;
      
      Vec2 resolution = engine->get_screen_size();
      
      ImGui::GetIO().DisplaySize.x = resolution.x;
      ImGui::GetIO().DisplaySize.y = resolution.y;
      
      ImGui_ImplVulkan_Init(&init_info);

      ImGuiStyle* style = &ImGui::GetStyle();

      style->Colors[ImGuiCol_WindowBg]         = ImVec4(0.1, 0.1, 0.1, 1);

      style->Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.05, 0.05, 0.05, 1);
      style->Colors[ImGuiCol_TitleBg]          = ImVec4(0.05, 0.05, 0.05, 1);

      style->Colors[ImGuiCol_CheckMark]        = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);

      style->Colors[ImGuiCol_FrameBg]          = ImVec4(0.15, 0.15, 0.15, 1);
      style->Colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
      style->Colors[ImGuiCol_FrameBgActive]    = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

      style->Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
      style->Colors[ImGuiCol_ButtonActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

      style->Colors[ImGuiCol_SliderGrab]       = ImVec4(0.869346734, 0.450980392, 0.211764706, 1);
      style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.899346734, 0.480980392, 0.241764706, 1);
      style->GrabMinSize = 2;

      style->Colors[ImGuiCol_Header]           = ImVec4(0.18, 0.18, 0.18, 1);
      style->Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.869346734, 0.450980392, 0.211764706, 0.5);
      style->Colors[ImGuiCol_HeaderActive]     = ImVec4(0.919346734, 0.500980392, 0.261764706, 0.6);

    }

    /* Do our overlay drawing */
    ImGui_ImplVulkan_NewFrame( );
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame( );

    draw_players_imgui();
    
    if (ImGui::IsKeyPressed(ImGuiKey_Insert, false)) {
      menu_focused = !menu_focused;
      surface->set_cursor_visible(menu_focused);
    }
    
    if (menu_focused) {
      draw_menu();
    }  

    draw_watermark();
    
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), fd->CommandBuffer);
    /* End of our overlay drawing */
    
    vkCmdEndRenderPass(fd->CommandBuffer);
    vkEndCommandBuffer(fd->CommandBuffer);

    constexpr VkPipelineStageFlags stages_wait = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    {
      VkSubmitInfo info = {};
      info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

      info.pWaitDstStageMask = &stages_wait;

      info.signalSemaphoreCount = 1;
      info.pSignalSemaphores = &fsd->RenderCompleteSemaphore;

      vkQueueSubmit(queue, 1, &info, VK_NULL_HANDLE);
    }

    {
      VkSubmitInfo info = { };
      info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      info.commandBufferCount = 1;
      info.pCommandBuffers = &fd->CommandBuffer;

      info.pWaitDstStageMask = &stages_wait;
      info.waitSemaphoreCount = 1;
      info.pWaitSemaphores = &fsd->RenderCompleteSemaphore;

      info.signalSemaphoreCount = 1;
      info.pSignalSemaphores = &fsd->ImageAcquiredSemaphore;

      vkQueueSubmit(graphicQueue, 1, &info, fd->Fence);
    }
  }

  return queue_present_original(queue, present_info);
}

VkResult create_swapchain_hook(VkDevice device, const VkSwapchainCreateInfoKHR* create_info, const VkAllocationCallbacks* allocator, VkSwapchainKHR* swapchain) {

  for (uint32_t i = 0; i < 8; ++i) {
    if (g_Frames[i].Fence) {
      vkDestroyFence(vk_device, g_Frames[i].Fence, vk_allocator);
      g_Frames[i].Fence = VK_NULL_HANDLE;
    }
    if (g_Frames[i].CommandBuffer) {
      vkFreeCommandBuffers(vk_device, g_Frames[i].CommandPool, 1, &g_Frames[i].CommandBuffer);
      g_Frames[i].CommandBuffer = VK_NULL_HANDLE;
    }
    if (g_Frames[i].CommandPool) {
      vkDestroyCommandPool(vk_device, g_Frames[i].CommandPool, vk_allocator);
      g_Frames[i].CommandPool = VK_NULL_HANDLE;
    }
    if (g_Frames[i].BackbufferView) {
      vkDestroyImageView(vk_device, g_Frames[i].BackbufferView, vk_allocator);
      g_Frames[i].BackbufferView = VK_NULL_HANDLE;
    }
    if (g_Frames[i].Framebuffer) {
      vkDestroyFramebuffer(vk_device, g_Frames[i].Framebuffer, vk_allocator);
      g_Frames[i].Framebuffer = VK_NULL_HANDLE;
    }
  }

  for (uint32_t i = 0; i < 8; ++i) {
    if (g_FrameSemaphores[i].ImageAcquiredSemaphore) {
      vkDestroySemaphore(vk_device, g_FrameSemaphores[i].ImageAcquiredSemaphore, vk_allocator);
      g_FrameSemaphores[i].ImageAcquiredSemaphore = VK_NULL_HANDLE;
    }
    if (g_FrameSemaphores[i].RenderCompleteSemaphore) {
      vkDestroySemaphore(vk_device, g_FrameSemaphores[i].RenderCompleteSemaphore, vk_allocator);
      g_FrameSemaphores[i].RenderCompleteSemaphore = VK_NULL_HANDLE;
    }
  }
  
  vk_image_extent = create_info->imageExtent;
  
  return create_swapchain_original(device, create_info, allocator, swapchain);
}


// These two hooks populate our local VkDevice reference
VkResult acquire_next_image_hook(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* image_index) {
  
  vk_device = device;
  
  return acquire_next_image_original(device, swapchain, timeout, semaphore, fence, image_index);
}

VkResult acquire_next_image2_hook(VkDevice device, const VkAcquireNextImageInfoKHR* acquire_info, uint32_t* image_index) {

  vk_device = device;
  
  return acquire_next_image2_original(device, acquire_info, image_index);
}
