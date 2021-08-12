#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../log/log.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../file/file.h"
#include "../json/json.h"
#include "gltf.h"

// misc helpers

#define fatal(...) { log_error(__VA_ARGS__); return false; }
#define bound_index(index, range) (index <= (size_t)range_count(range))

static void * _reference_array(range_void * array, size_t item_size, const json_object * parent, const char * name)
{
    bool success = true;
    size_t count = ((char*)array->end - (char*)array->begin) / item_size;
    size_t index = json_get_number(.success = &success, .parent = parent, .key = name);

    if (index >= count)
    {
	fatal ("Index out of range for %s, (index)%zd >= (count)%zd", name, index, count);
    }

    return (char*)array->begin + index * item_size;
    
}

#define reference_array(array, parent, name) _reference_array((range_void*)&(array), sizeof(*(array).begin), parent, name)

struct read_array_arg {
    range_void * output;
    const json_array * input;
    gltf * gltf;
    size_t item_size;
    bool (*function)(void * output, gltf * gltf, const json_object * input);
};

static bool _read_array (struct read_array_arg arg)
{
    if (!arg.input)
    {
	return false;
    }

    assert (arg.item_size);
    assert (arg.output);
    assert (arg.gltf);
    
    size_t count = range_count (*arg.input);

    arg.output->begin = calloc (count, arg.item_size);
    arg.output->end = (char*)arg.output->begin + (count * arg.item_size);

    json_value * value;
    
    for (size_t i = 0; i < count; i++)
    {
	value = arg.input->begin + i;

	if (value->type != JSON_OBJECT)
	{
	    fatal ("JSON Array member is not an object");
	}

	if (!arg.function ((char*)arg.output->begin + (i * arg.item_size), arg.gltf, value->object))
	{
	    fatal ("Failed to read array element");
	}
    }

    return true;
}

#define read_array(...) _read_array((struct read_array_arg){__VA_ARGS__})

static char * _dupe_string (const char * input)
{
    return input ? strcpy(malloc(strlen(input) + 1), input) : NULL;
}

static bool _set_accessor_type (gltf_accessor_type * type_int, const char * type_string)
{
    struct { const char * key; gltf_accessor_type value; }
    map[] =
	{
	    { "SCALAR", GLTF_ACCESSOR_SCALAR },
	    { "VEC2", GLTF_ACCESSOR_VEC2 },
	    { "VEC3", GLTF_ACCESSOR_VEC3 },
	    { "VEC4", GLTF_ACCESSOR_VEC4 },
	    { "MAT2", GLTF_ACCESSOR_MAT2 },
	    { "MAT3", GLTF_ACCESSOR_MAT3 },
	    { "MAT4", GLTF_ACCESSOR_MAT4 },
	};

    for (size_t i = 0; i < sizeof (map) / sizeof (*map); i++)
    {
	if (0 == strcmp (type_string, map[i].key))
	{
	    *type_int = map[i].value;
	    return true;
	}
    }

    *type_int = GLTF_ACCESSOR_BADTYPE;

    return false;
}

static bool _validate_component_type (size_t type)
{
    switch (type)
    {
    case GLTF_ACCESSOR_COMPONENT_BYTE:
    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_BYTE:
    case GLTF_ACCESSOR_COMPONENT_SHORT:
    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_SHORT:
    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_INT:
    case GLTF_ACCESSOR_COMPONENT_FLOAT:
	return true;

    default:
	return false;
    }

    return false;
}

static size_t _get_accessor_component_size (gltf_accessor_component_type component_type)
{
    switch (component_type)
    {
    case GLTF_ACCESSOR_COMPONENT_BYTE:
	return 1;

    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_BYTE:
	return 1;
	
    case GLTF_ACCESSOR_COMPONENT_SHORT:
	return 2;

    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_SHORT:
	return 2;

    case GLTF_ACCESSOR_COMPONENT_UNSIGNED_INT:
	return 4;
	
    case GLTF_ACCESSOR_COMPONENT_FLOAT:
	return 4;
	
    default:
	return 0;
    }

    return 0;
}

// glb_toc_*

bool glb_toc_load_file (glb_toc * toc, FILE * file)
{
    buffer_char file_contents = {0};
    
    if (!load_file (&file_contents, file))
    {
	free(file_contents.begin);
	return false;
    }
    
    if (!glb_toc_load_memory(toc, file_contents.begin, range_count(file_contents)))
    {
	//log_error ("glb file failed to load");
	free (file_contents.begin);
	return false;
    }

    return true;
}

bool glb_toc_load_memory (glb_toc * toc, void * start, size_t size)
{
    toc->header = start;
    toc->json = (void*)((char*)start + sizeof(*toc->header));

    if ((void*)((char*)start + size) < (void*)(toc->json + 1))
    {
	log_error ("glb file is truncated");
	return false;
    }
    
    if (toc->header->magic != GLB_MAGIC)
    {
	log_error ("glb file has wrong magic");
	return false;
    }

    if (toc->json->type != GLB_CHUNKTYPE_JSON)
    {
	log_error ("glb json chunk is malformed");
	return false;
    }

    toc->bin = (void*)((char*)toc->json + toc->json->length + sizeof (*toc->json));

    if ((void*)((char*)start + size) < (void*)(toc->bin + 1))
    {
	log_error ("glb binary data header is truncated");
	return false;
    }

    if (toc->bin->type != GLB_CHUNKTYPE_BIN)
    {
	log_error ("glb bin chunk is malformed");
	return false;
    }

    if ((void*)((char*)start + size) < (void*)((char*)toc->bin + toc->bin->length))
    {
	log_error ("glb binary data is truncated");
	return false;
    }

    return true;
}

// _get_json_* functions

typedef struct number_arg number_arg;
struct number_arg
{
    const char * name;
    size_t default_value;
    bool optional;
};

typedef struct object_arg object_arg;
struct object_arg
{
    const char * name;
    bool optional;
};

// _gltf_read_* single elements

static bool _gltf_read_asset (gltf_asset * gltf_asset, const json_object * json_asset)
{
    if (!json_asset)
    {
	return false;
    }
    
    bool success = true;
    
    gltf_asset->generator = _dupe_string(json_get_string(.success = &success, .parent = json_asset, .key = "generator"));
    gltf_asset->version	  = _dupe_string(json_get_string(.success = &success, .parent = json_asset, .key = "version"));

    return success;
}

static bool _gltf_read_buffer (void * output, gltf * gltf, const json_object * json_buffer)
{
    gltf_buffer * gltf_buffer = output;
    
    bool success = true;

    gltf_buffer->byte_length = json_get_number(.parent = json_buffer, .key = "byteLength", .success = &success);

    if (!success)
    {
	return false;
    }
    
    json_value * value = json_lookup (json_buffer, "uri");

    gltf_buffer->data = NULL;
    
    if (value->type == JSON_STRING)
    {
	gltf_buffer->uri = strcpy (malloc (strlen (value->string) + 1), value->string);
    }
    else if (value->type == JSON_NULL)
    {
	gltf_buffer->uri = NULL;
    }
    else 
    {
	log_error ("GLTF JSON buffer uri is present but is not a string");
	return false;
    }

    return true;
}

static bool _gltf_read_buffer_view (void * output, gltf * gltf, const json_object * json_buffer_view)
{
    gltf_buffer_view * gltf_buffer_view = output;

    bool success = true;

    gltf_buffer_view->buffer = reference_array(gltf->buffers, json_buffer_view, "buffer");
    gltf_buffer_view->byte_length = json_get_number(.success = &success, .parent = json_buffer_view, .key = "byteLength");
    gltf_buffer_view->byte_offset = json_get_number(.success = &success, .parent = json_buffer_view, .key = "byteOffset", .optional = true);
    gltf_buffer_view->byte_stride = json_get_number(.success = &success, .parent = json_buffer_view, .key = "byteStride", .optional = true, .default_value = 1);
    
    if (!success || !gltf_buffer_view->buffer)
    {
	return false;
    }

    return true;
}

static bool _gltf_read_accessor_sparse (gltf_accessor * gltf_accessor, gltf * gltf, const json_object * json_sparse)
{
    const json_object * sparse_indices;
    const json_object * sparse_values;

    sparse_indices = json_get_object (.parent = json_sparse, .key = "indices");
    sparse_values = json_get_object (.parent = json_sparse, .key = "values");
        
    if (!sparse_indices || !sparse_values)
    {
	log_error ("GLTF JSON failed to read sparse accessor");
	return false;
    }

    bool success = true;

    gltf_accessor->sparse.count			 = json_get_number(.success = &success, .parent = json_sparse, .key = "count");
    gltf_accessor->sparse.indices.byte_offset	 = json_get_number(.success = &success, .parent = sparse_values, .key = "byteOffset");
    gltf_accessor->sparse.indices.component_type = json_get_number(.success = &success, .parent = sparse_indices, .key = "componentType");

    gltf_accessor->sparse.indices.buffer_view = reference_array (gltf->buffer_views, sparse_indices, "bufferView");
    gltf_accessor->sparse.values.buffer_view  = reference_array (gltf->buffer_views, sparse_values, "bufferView");
        
    if (!success || !gltf_accessor->sparse.indices.buffer_view || !gltf_accessor->sparse.values.buffer_view)
    {
	log_error ("GLTF JSON failed to read sparse accessor members");
	return false;	
    }

    if (!_validate_component_type(gltf_accessor->sparse.indices.component_type))
    {
	log_error ("GLTF JSON sparse accessor has invalid indices component type");
	return false;
    }

    size_t component_size = _get_accessor_component_size(gltf_accessor->component_type);

    if ((gltf_accessor->byte_offset + gltf_accessor->buffer_view->byte_offset) % component_size)
    {
	log_error ("GLTF JSON sparse accessor is not aligned properly to size %zd", component_size);
	return false;
    }

    return true;
}

static bool _gltf_read_accessor (void * output, gltf * gltf, const json_object * json_accessor)
{
    gltf_accessor * gltf_accessor = output;
    
    const char * accessor_type;
    bool success;

    const json_object * sparse = json_get_object (.parent = json_accessor, .key = "sparse", .optional = true);
        
    if (sparse)
    {
	gltf_accessor->sparse.present = true;
	if (!_gltf_read_accessor_sparse(gltf_accessor, gltf, sparse))
	{
	    log_error ("GLTF JSON failed to read sparse accessor");
	    return false;
	}
    }

    success = true;

    gltf_accessor->byte_offset	  = json_get_number(.success = &success, .parent = json_accessor, .key = "byteOffset", .optional = true, .default_value = 0);
    gltf_accessor->component_type = json_get_number(.success = &success, .parent = json_accessor, .key = "componentType");
    gltf_accessor->count	  = json_get_number(.success = &success, .parent = json_accessor, .key = "count");
    accessor_type		  = json_get_string(.success = &success, .parent = json_accessor, .key = "type");
    
    if (!success)
    {
	log_error ("GLTF JSON failed to read component of accessor");
	return false;	
    }

    if (!_validate_component_type (gltf_accessor->component_type))
    {
	log_error ("GLTF JSON accessor has invalid component type");
	return false;
    }
    
    if (!_set_accessor_type (&gltf_accessor->type, accessor_type))
    {
	log_error ("GLTF JSON accessor has invalid type");
	return false;
    }

    gltf_accessor->buffer_view = reference_array (gltf->buffer_views, json_accessor, "bufferView");

    if (!gltf_accessor->buffer_view && !gltf_accessor->sparse.present)
    {
	fatal("GLTF JSON accessor has neither a bufferView index nor sparse accessor");
    }
    
    return true;
}

static bool _gltf_read_material (void * output, gltf * gltf, const json_object * json_mesh)
{
    
}

static bool _gltf_read_mesh_primitive_target (void * output, gltf * gltf, const json_object * json_target)
{
    gltf_mesh_primitive_target * gltf_target = output;
    
    gltf_target->normal	  = reference_array (gltf->accessors, json_target, "NORMAL");
    gltf_target->position = reference_array (gltf->accessors, json_target, "POSITION");
    gltf_target->tangent  = reference_array (gltf->accessors, json_target, "TANGENT");

    return gltf_target->normal && gltf_target->position && gltf_target->tangent;
}

static bool _gltf_read_mesh_primitive (void * output, gltf * gltf, const json_object * json_primitive)
{
    gltf_mesh_primitive * primitive = output;
    
    primitive->indices	= reference_array (gltf->accessors, json_primitive, "indices");
    primitive->material = reference_array (gltf->materials, json_primitive, "material");

    if (!read_array (.output = (range_void*)&primitive->targets, .gltf = gltf, .input = json_get_array (.parent = json_primitive, .key = "targets"), .function = _gltf_read_mesh_primitive_target, .item_size = sizeof(*primitive->targets.begin)))
    {
	fatal ("Failed to read mesh primitive targets array");
    }
    
    if (!primitive->indices)
    {
	fatal ("No indices specified in mesh primitive");
    }

    if (!primitive->material)
    {
	fatal ("No materials specified in mesh primitive");
    }

    return true;
}

static bool _gltf_read_mesh (void * output, gltf * gltf, const json_object * json_mesh)
{
    gltf_mesh * gltf_mesh = output;
    if (!read_array (.output = (range_void*)&gltf_mesh->primitives, .gltf = gltf, .input = json_get_array (.parent = json_mesh, .key = "primitives"), .function = _gltf_read_mesh_primitive, .item_size = sizeof(*gltf_mesh->primitives.begin)))
    {
	fatal ("Could not read mesh primitives");
    }

    return true;
}

//

bool gltf_from_json (gltf * gltf, json_value * json_root_value)
{
    memset (gltf, 0, sizeof (*gltf));

    if (json_root_value->type != JSON_OBJECT)
    {
	log_error ("GLTF JSON data is not an object");
	goto error;
    }
    
    json_object * json_root_object = json_root_value->object;

    // reading gltf members:
    
    //    asset
    if (!_gltf_read_asset (&gltf->asset, json_get_object(.parent = json_root_object, .key = "asset")))
    {
	log_error ("GLTF JSON failed to load asset");
	goto error;
    }

    //    buffers
    if (!read_array (.output = (range_void*)&gltf->buffers, .input = json_get_array (.parent = json_root_object, .key = "buffers"), .function = _gltf_read_buffer, .gltf = gltf, .item_size = sizeof(*gltf->buffers.begin)))
    {
	log_error ("GLTF JSON failed to load buffers");
	goto error;
    }

    //    buffer views
    if (!read_array (.output = (range_void*)&gltf->buffer_views, .input = json_get_array (.parent = json_root_object, .key = "bufferViews"), .function = _gltf_read_buffer_view, .gltf = gltf, .item_size = sizeof(*gltf->buffer_views.begin)))
    {
	log_error ("GLTF JSON failed to load bufferViews");
	goto error;
    }

    //    accessors
    if (!read_array (.output = (range_void*)&gltf->accessors, .input = json_get_array (.parent = json_root_object, .key = "accessors"), .function = _gltf_read_accessor, .gltf = gltf, .item_size = sizeof(*gltf->accessors.begin)))
    {
	log_error ("GLTF JSON failed to load accessors");
	goto error;
    }

    //    materials
    // ...todo

    //    meshes
    /*if (!read_array (.output = (range_void*)&gltf->meshes, .input = json_get_array (.parent = json_root_object, .key = "meshes"), .function = _gltf_read_mesh, .gltf = gltf, .item_size = sizeof(*gltf->meshes.begin)))
    {
	log_error ("GLTF JSON failed to load meshes");
	goto error;
	}*/
    
    return true;
    
error:
    return false;
}
