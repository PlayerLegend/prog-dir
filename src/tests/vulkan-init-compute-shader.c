#include "vk_precompiled.h"
#define FLAT_INCLUDES

int load_file_name(char_range * content, const char * file_name)
{
    FILE * file = fopen(file_name,"r");

    fseek(file,0,SEEK_END);
    
    size_t file_size = ftell(file);

    fseek(file,0,SEEK_SET);

    content->begin = malloc(file_size + 1);
    content->end = content->begin + file_size;

    if( file_size != fread(content->begin,1,file_size,file) || ferror(file) )
    {
	perror(file_name);
	free(content->begin);
	return -1;
    }

    *content->end = '\0';

    return 0;
}

int create_shader_module_from_file(VkShaderModule * shader, VkDevice logical, const char * file_name)
{
    char_range file_bytes = {0};
    if( -1 == load_file_name(&file_bytes,file_name) )
    {
	return -1;
    }

    VkShaderModuleCreateInfo shader_module_create_info = {
	.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
	.codeSize = file_bytes.end - file_bytes.begin,
	.pCode = (void*)file_bytes.begin,
    };

    VkResult res = vkCreateShaderModule(logical,&shader_module_create_info,NULL,shader);

    free(file_bytes.begin);

    if( res == VK_SUCCESS )
    {
	return 0;
    }
    else
    {
	log_error("Failed to create shader [%s]\n",file_name);
	return -1;
    }
}

void destroy_shader_module(VkShaderModule * shader, VkDevice logical)
{
    vkDestroyShaderModule(logical,*shader,NULL);
}

