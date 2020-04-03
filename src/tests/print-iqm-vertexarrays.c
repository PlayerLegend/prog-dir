#include "precompiled.h"
#define FLAT_INCLUDES
#include "iqm_reader.h"

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
	log_error("usage: %s [iqm file]");
	exit(1);
    }

    const char * file_path = argv[1];

    log_normal("Reading file [%s]\n",file_path);

    vertex_ranges varrays = {0};

    iqm_header header;

    FILE * file;

    if( NULL == (file = fopen(file_path,"r")) )
    {
	perror(file_path);
	exit(1);
    }

    if( -1 == load_iqm_header(&header,file) )
    {
	log_error("Failed to load header for %s\n",file_path);
	fclose(file);
	exit(1);
    }

    if( -1 == load_vertex_ranges(&varrays,&header,file) )
    {
	log_error("Failed to load vertex ranges for %s\n",file_path);
	fclose(file);
	exit(1);
    }

    for_range(position,varrays.position)
    {
	printf( "position: %f %f %f\n",
		position->x,
		position->y,
		position->z );
    }
    
    for_range(texcoord,varrays.texcoord)
    {
	printf( "texcoord: %f %f\n",
		texcoord->x,
		texcoord->y );
    }
    
    return 0;
}
