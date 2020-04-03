#include "vk_precompiled.h"
#define FLAT_INCLUDES

typedef struct {
    VkDescriptorSetLayout vk_set_layout;
    VkPipelineLayout vk_pipeline_layout;
    VkPipeline vk_pipeline;
}
    vulkan_pipeline;

typedef range(vulkan_pipeline) range_vulkan_pipeline;

typedef struct {
    VkInstance vk_instance;
}
    vulkan_instance;

