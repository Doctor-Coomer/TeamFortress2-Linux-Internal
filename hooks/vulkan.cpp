#include <vulkan/vulkan.hpp>

#include "print.hpp"

VkResult (*queue_present_original)(VkQueue, const VkPresentInfoKHR *);

VkResult queue_present_hook(VkQueue queue, const VkPresentInfoKHR *present_info) {
    print("QueuePresent hooked :3\n");

    return queue_present_original(queue, present_info);
}
