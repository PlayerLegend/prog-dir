#ifndef FLAT_INCLUDES
#include "precompiled.h"

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define FLAT_INCLUDES

//#include "range.h"
//#include "print.h"

#endif

typedef struct {
    GLFWwindow * window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
}
    vulkan_state;

static void halt_vulkan(vulkan_state * state)
{
    {
	glfwTerminate();
    }

#ifndef NDEBUG
    {
	PFN_vkDestroyDebugUtilsMessengerEXT destroy = (void*)vkGetInstanceProcAddr(state->instance, "vkDestroyDebugUtilsMessengerEXT");
	//if(destroy)
	//  destroy(state->instance,state->debug_messenger,NULL);
    }
#endif

    {
	vkDestroyInstance(state->instance, NULL);
    }
}
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
	// Message is important enough to show
	log_normal("%s\n",pCallbackData->pMessage);
    }
    return VK_FALSE;
}

#ifndef NDEBUG
static int create_messenger(vulkan_state * state)
{
    VkDebugUtilsMessengerCreateInfoEXT create = {};
    create.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create.pfnUserCallback = debug_callback;
    create.pUserData = NULL; // Optional

    PFN_vkCreateDebugUtilsMessengerEXT func = (void*)vkGetInstanceProcAddr(state->instance, "vkCreateDebugUtilsMessengerEXT");
    if(func)
    {
	func(state->instance,&create,NULL,&state->debug_messenger);
	return VK_SUCCESS;
    }
    else
    {
	return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

#endif

static int init_vulkan(vulkan_state * state)
{
    glfwInit();
    
#ifndef NDEBUG
    const char * validation_names[] = {
	//"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_LUNARG_core_validation",
	"VK_LAYER_LUNARG_standard_validation",
	"VK_LAYER_LUNARG_parameter_validation",
    };

    size_t validation_count = sizeof(validation_names) / sizeof(validation_names[0]);
#endif
    
	
    {
	VkApplicationInfo application = {};
	application.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	application.pApplicationName = "Hello Triangle";
	application.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application.pEngineName = "No Engine";
	application.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application.apiVersion = VK_API_VERSION_1_0;

	

	VkInstanceCreateInfo create = {};
	create.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create.pApplicationInfo = &application;

#ifndef NDEBUG
	create.ppEnabledLayerNames = validation_names;
	create.enabledLayerCount = validation_count;
#endif
	
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	create.enabledExtensionCount = glfwExtensionCount;
	create.ppEnabledExtensionNames = glfwExtensions;

	{ // blank for now
	    create.enabledLayerCount = 0;
	}

	if( VK_SUCCESS != vkCreateInstance(&create, NULL, &state->instance) )
	{
	    log_error("Failed to create vulkan instance");
	    return -1;
	}
    }

#ifndef NDEBUG
    {	
	uint32_t layercount;
	vkEnumerateInstanceLayerProperties(&layercount, NULL);
	range(VkLayerProperties) available_layers = { .begin = malloc(layercount * sizeof(*available_layers.begin)), .end = available_layers.begin + layercount };
	vkEnumerateInstanceLayerProperties(&layercount, available_layers.begin);

	bool layer_is_available;
	
	for(const char **requested = validation_names, **max = validation_names + validation_count; requested < max; requested++ )
	{
	    log_normal("Checking validation layer %s",*requested);
	    layer_is_available = false;	    
	    for_range(available,available_layers)
	    {
		//log_debug("Testing %s == %s\n",*requested,available->layerName);
		if( 0 == strcmp(*requested,available->layerName) )
		{
		    layer_is_available = true;
		    break;
		}
	    }

	    if(!layer_is_available)
	    {
		log_debug("Validation layer unavailable: %s",*requested);
		free(available_layers.begin);
		return -1;
	    }
	}
	
	free(available_layers.begin);

	log_normal("All validation layers are available");
    }
    
    create_messenger(state);
#endif
    
    return 0;
}

static inline GLFWwindow * create_window(int width, int height, const char * name)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    return glfwCreateWindow(width,height,name,NULL,NULL);
}

void destroy_window(GLFWwindow * window)
{
    glfwDestroyWindow(window);
}

static inline void vulkan_frame()
{
    glfwPollEvents();
}

bool vulkan_should_exit(vulkan_state * state)
{
    return glfwWindowShouldClose(state->window);
}
