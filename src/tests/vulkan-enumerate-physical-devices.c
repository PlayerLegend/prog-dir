#include "vk_precompiled.h"
#define FLAT_INCLUDES

typedef struct {
    VkInstance instance;
}
    vulkan_state;

typedef range(VkPhysicalDevice) range_vk_physical_device;
typedef range(VkQueueFamilyProperties) range_vk_queue_family_properties;

int init_vulkan(vulkan_state * state)
{
    VkApplicationInfo app =
    {
	.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	
	.pApplicationName = "vulkan-enumerate-devices",
	.applicationVersion = 0x000001,
	.pEngineName = "vulkan-enumerate-devices",
	.engineVersion = 0x000001,

	.apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo instance_create =
    {
	.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	
	.pApplicationInfo = &app,
    };

    VkResult result;

    if( VK_SUCCESS != (result = vkCreateInstance(&instance_create,NULL,&state->instance)) )
    {
	log_error("Failed to create vulkan instance");
	return -1;
    }

    return 0;
}

void halt_vulkan(vulkan_state * state)
{
    vkDestroyInstance(state->instance,NULL);
}

int vulkan_get_physical_devices(range_vk_physical_device * devices, VkInstance instance)
{
    uint32_t count = -1;

    VkResult result;

    if( VK_SUCCESS != (result = vkEnumeratePhysicalDevices(instance,&count,NULL)) )
    {
	log_error("Failed to count physical devices");
	return -1;
    }

    devices->begin = malloc( count * sizeof(*devices->begin) );
    devices->end = devices->begin + count;
    
    if( VK_SUCCESS != (result = vkEnumeratePhysicalDevices(instance,&count,devices->begin)) )
    {
	log_error("Failed to enumerate physical devices");
	free(devices->begin);
	return -1;
    }

    return 0;
}

void vulkan_get_queue_family_properties(range_vk_queue_family_properties * properties, VkPhysicalDevice device)
{
    uint32_t count = -1;

    vkGetPhysicalDeviceQueueFamilyProperties(device,&count,NULL);
    
    properties->begin = malloc( count * sizeof(*properties->begin) );
    properties->end = properties->begin + count;

    vkGetPhysicalDeviceQueueFamilyProperties(device,&count,properties->begin);
}

const char * name_vulkan_physical_device_type(VkPhysicalDeviceType type)
{
    switch (type)
    {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
	return "Other";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
	return "iGPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
	return "GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
	return "vGPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
	return "Software";
    default:
	return "Unrecognized";
    }
}

void print_device_properties(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory_properties;
    range_vk_queue_family_properties qfp_range;

    vkGetPhysicalDeviceProperties(device,&properties);
    vkGetPhysicalDeviceFeatures(device,&features);
    vkGetPhysicalDeviceMemoryProperties(device,&memory_properties);

    vulkan_get_queue_family_properties(&qfp_range,device);

    //    printing
    {
	printf("Device: %s\n\t%s (id: 0x%04X) from vendor 0x%04X [driver version: 0x%04X, API version 0x%04X]\n",
	       name_vulkan_physical_device_type(properties.deviceType),
	       properties.deviceName,
	       properties.deviceID,
	       properties.vendorID,
	       properties.driverVersion,
	       properties.apiVersion);
    }

    printf("\tThe device supports the following queue families:\n");

    for_range(qfp,qfp_range)
    {
	printf("\t* %zd queues with the following capabilities:\n",
	       (ssize_t)qfp->queueCount);

	if( qfp->queueFlags == 0 )
	    printf("\t\tNone\n");
	else
	{
	    if( qfp->queueFlags & VK_QUEUE_GRAPHICS_BIT )
		printf("\t\tGraphics\n");
	    
	    if( qfp->queueFlags & VK_QUEUE_COMPUTE_BIT )
		printf("\t\tCompute\n");

	    if( qfp->queueFlags & VK_QUEUE_TRANSFER_BIT )
		printf("\t\tTransfer\n");

	    if( qfp->queueFlags & VK_QUEUE_SPARSE_BINDING_BIT )
		printf("\t\tSparse binding\n");
	}
    }

    free(qfp_range.begin);
}

int main()
{
    vulkan_state state;

    if( -1 == init_vulkan(&state) )
    {
	log_error("Failed to init vulkan");
	exit(1);
    }
    
    range_vk_physical_device devices_range;

    if( -1 == vulkan_get_physical_devices(&devices_range,state.instance) )
    {
	log_error("Failed to get physical devices");
	exit(1);
    }

    for_range(device,devices_range)
	print_device_properties(*device);

    free(devices_range.begin);

    halt_vulkan(&state);

    return 0;
}
