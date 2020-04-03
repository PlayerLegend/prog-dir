#ifndef FLAT_INCLUDES

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FLAT_INCLUDES

#include "vec2.h"
#include "vec3.h"
#include "range.h"
#include "print.h"
#endif

typedef float model_vec;
typedef vec2(model_vec) model_vec2;
typedef vec3(model_vec) model_vec3;

typedef model_vec2 vertex_texcoord;
typedef model_vec3 vertex_position;

typedef struct {
    range(vertex_position) position;
    range(vertex_texcoord) texcoord;
}
    vertex_ranges;

typedef uint32_t uint;

//    from iqm.txt


enum // vertex array format
{
    IQM_BYTE   = 0,
    IQM_UBYTE  = 1,
    IQM_SHORT  = 2,
    IQM_USHORT = 3,
    IQM_INT    = 4,
    IQM_UINT   = 5,
    IQM_HALF   = 6,
    IQM_FLOAT  = 7,
    IQM_DOUBLE = 8,
};

enum // vertex array type
{
    IQM_POSITION     = 0,  // float, 3
    IQM_TEXCOORD     = 1,  // float, 2
    IQM_NORMAL       = 2,  // float, 3
    IQM_TANGENT      = 3,  // float, 4
    IQM_BLENDINDEXES = 4,  // ubyte, 4
    IQM_BLENDWEIGHTS = 5,  // ubyte, 4
    IQM_COLOR        = 6,  // ubyte, 4

    // all values up to IQM_CUSTOM are reserved for future use
    // any value >= IQM_CUSTOM is interpreted as CUSTOM type
    // the value then defines an offset into the string table, where offset = value - IQM_CUSTOM
    // this must be a valid string naming the type
    IQM_CUSTOM       = 0x10
};

typedef struct
{
    char magic[16]; // the string "INTERQUAKEMODEL\0", 0 terminated
    uint version; // must be version 2
    uint filesize;
    uint flags;
    uint num_text, ofs_text;
    uint num_meshes, ofs_meshes;
    uint num_vertexarrays, num_vertexes, ofs_vertexarrays;
    uint num_triangles, ofs_triangles, ofs_adjacency;
    uint num_joints, ofs_joints;
    uint num_poses, ofs_poses;
    uint num_anims, ofs_anims;
    uint num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint num_comment, ofs_comment;
    uint num_extensions, ofs_extensions; // these are stored as a linked list, not as a contiguous array
}
    iqm_header;

typedef struct 
{
    uint type;   // type or custom name
    uint flags;
    uint format; // component format
    uint size;   // number of components
    uint offset; // offset to array of tightly packed components, with num_vertexes * size total entries
                 // offset must be aligned to max(sizeof(format), 4)
}
    iqm_vertex_array;

//    end: from iqm.txt

typedef range(iqm_vertex_array) iqm_vertex_array_range;

static int load_range(char_range * range, ssize_t nmemb, ssize_t size, FILE * file)
{
    if( NULL == (range->begin = malloc(size * nmemb)) )
    {
	perror(__func__);
	return -1;
    }

    range->end = range->begin + size * nmemb;
        
    ssize_t got = fread(range->begin,size,nmemb,file);

    if(ferror(file))
    {
	perror(__func__);
	free(range->begin);
	return -1;
    }
    
    if(got != nmemb)
    {
	log_error("Could not read enough elements of size %zd, wanted %zd but got %zd\n", size, nmemb, got);
	free(range->begin);
	return -1;
    }

    return 0;
}

static int load_iqm_header(iqm_header * header, FILE * file)
{
    if( 1 == fread(header,sizeof(*header),1,file) )
    {
	return 0;
    }
    else
    {
	log_error("Failed to load model header");
	return -1;
    }
}

static int load_vertex_array_headers(iqm_vertex_array_range * headers, const iqm_header * iqm_header, FILE * file)
{
    if( -1 == fseek(file,iqm_header->ofs_vertexarrays,SEEK_SET) )
    {
	log_error("Failed to seek to vertex array headers");
	return -1;
    }

    return load_range((char_range*)headers,iqm_header->num_vertexarrays,sizeof(*headers->begin),file);
}

static int load_vertex_array(char_range * range, size_t size, const iqm_header * header, iqm_vertex_array_range * vertex_arrays, uint type, FILE * file)
{
    for_range(array,*vertex_arrays)
    {
	if(array->type == type)
	{
	    if( -1 == fseek(file,array->offset,SEEK_SET) )
	    {
		log_error("failed to seek to vertex array offset");
		return -1;
	    }
	    return load_range(range,header->num_vertexes,size,file);
	}
	else if(array->type > type)
	{
	    break;
	}
    }

    *range = (char_range){};
    
    return 0;
}

int load_vertex_ranges(vertex_ranges * vinfo, iqm_header * header, FILE * file)
{
    iqm_vertex_array_range va_headers;

    if( -1 == load_vertex_array_headers(&va_headers,header,file) )
    {
	return -1;
    }

    size_t size;
    char * name = NULL;
    char_range * range = NULL;

    *vinfo = (vertex_ranges){};

    for_range(vah,va_headers)
    {
	switch(vah->type)
	{
	case IQM_POSITION:
	    size = sizeof(vertex_position);
	    name = "position";
	    range = (void*)&vinfo->position;
	    break;
	    
	case IQM_TEXCOORD:
	    size = sizeof(vertex_texcoord);
	    name = "texcoord";
	    range = (void*)&vinfo->texcoord;
	    break;

	default:
	case IQM_NORMAL:
	case IQM_TANGENT:
	case IQM_BLENDINDEXES:
	case IQM_BLENDWEIGHTS:
	case IQM_COLOR:
	case IQM_CUSTOM:
	    size = 0;
	    break;
	}

	if( size == 0 )
	    continue;

	if( -1 == fseek(file,vah->offset,SEEK_SET) )
	{
	    printf("Failed to seek to vertex array for %s\n",name);
	    goto ERROR;
	}

	if( -1 == load_range(range,header->num_vertexes,size,file) )
	{
	    printf("Failed to read vertex array for %s\n",name);
	    goto ERROR;
	}
    }

    return 0;

ERROR:
    free(va_headers.begin);
    free(vinfo->position.begin);
    free(vinfo->texcoord.begin);
    return -1;
}

//
