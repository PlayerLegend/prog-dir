#include "vk_precompiled.h"
#define FLAT_INCLUDES

typedef range(VkPhysicalDevice) range_VkPhysicalDevice;
typedef range(VkQueueFamilyProperties) range_VkQueueFamilyProperties;

int init_vulkan(VkInstance * instance)
{
    VkApplicationInfo application_info =
    {
	.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
	
	.pApplicationName = "vulkan-create-logical-device",
	.applicationVersion = 0x000001,
	.pEngineName = "vulkan-create-logical-device",
	.engineVersion = 0x000001,

	.apiVersion = VK_API_VERSION_1_0,
    };

    VkInstanceCreateInfo instance_create_info =
    {
	.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
	
	.pApplicationInfo = &application_info,
    };

    VkResult result;

    if( VK_SUCCESS != (result = vkCreateInstance(&instance_create_info,NULL,instance)) )
    {
	log_error("Failed to create vulkan instance");
	return -1;
    }

    return 0;
}

void halt_vulkan(VkInstance instance)
{
    vkDestroyInstance(instance,NULL);
}

int vulkan_get_physical_devices(range_VkPhysicalDevice * devices, VkInstance instance)
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

void vulkan_get_queue_family_properties(range_VkQueueFamilyProperties * properties, VkPhysicalDevice device)
{
    uint32_t count = -1;

    vkGetPhysicalDeviceQueueFamilyProperties(device,&count,NULL);
    
    properties->begin = malloc( count * sizeof(*properties->begin) );
    properties->end = properties->begin + count;

    vkGetPhysicalDeviceQueueFamilyProperties(device,&count,properties->begin);
}

int vulkan_create_logical(VkDevice * logical, VkPhysicalDevice physical, VkQueueFlags select_queue_flags)
{
    range_VkQueueFamilyProperties all_properties;

    vulkan_get_queue_family_properties(&all_properties,physical);

    range(VkDeviceQueueCreateInfo) queue_create_infos;
    queue_create_infos.begin = queue_create_infos.end = malloc( sizeof(*queue_create_infos.begin) * count_range(all_properties) );

    uint32_t max_queue_count = 0;
    for_range(prop,all_properties)
	if(max_queue_count < prop->queueCount)
	    max_queue_count = prop->queueCount;

    float * queue_priorities = calloc(max_queue_count,sizeof(*queue_priorities));
	
    for_range(prop,all_properties)
    {
	*queue_create_infos.end = (VkDeviceQueueCreateInfo){
	    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
	    .queueFamilyIndex = prop - all_properties.begin,
	    .queueCount = prop->queueCount,
	    .pQueuePriorities = queue_priorities,
	};
	
	queue_create_infos.end++;
    }

    if(is_range_empty(queue_create_infos))
    {
	//log_error("Could not locate any physical device queues matching the requested queue flags %x",select_queue_flags);
	free(queue_create_infos.begin);
	free(queue_priorities);
	return -1;
    }

    VkPhysicalDeviceFeatures features;

    vkGetPhysicalDeviceFeatures(physical,&features);

//#warning might be a segfault, since this is referenced as a pointer

    VkDeviceCreateInfo logical_create_info = {
	.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
	.queueCreateInfoCount = count_range(queue_create_infos),
	.pQueueCreateInfos = queue_create_infos.begin,
	.pEnabledFeatures = &features,
    };

    VkResult result = vkCreateDevice(physical,&logical_create_info,NULL,logical);

    free(queue_create_infos.begin);
    free(queue_priorities);

    return (result == VK_SUCCESS) ? 0 : -1;
}

int vulkan_get_logical_from_first_physical(VkDevice * logical, VkInstance instance, VkQueueFlags select_queue_flags)
{
    range_VkPhysicalDevice physicals;
    if( -1 == vulkan_get_physical_devices(&physicals, instance) )
    {
	log_error("Failed to enumerate vulkan physical devices");
	return -1;
    }

    for_range(physical,physicals)
    {
	if( 0 == vulkan_create_logical(logical,*physical,select_queue_flags) )
	    return 0;
    }

    log_error("No vulkan device found matching the select queue flags %x",select_queue_flags);
    
    return -1;
}

int main()
{
    VkInstance instance;

    if( -1 == init_vulkan(&instance) )
    {
	exit(1);
    }

    VkDevice logical;
    if( -1 == vulkan_get_logical_from_first_physical(&logical, instance, VK_QUEUE_COMPUTE_BIT) )
    {
	exit(1);
    }
    else
    {
	log_normal("Success!!!");
    }

    halt_vulkan(instance);

    return 0;
}
