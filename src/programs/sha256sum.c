#include "sha256.h"

int sum_file(FILE * stream, const char * name)
{
    if( ! stream )
    {
	perror(name);
	return -1;
    }
    
    static sha256 sum;
    static sha256_armor armor;

    if( -1 == sha256_stream(sum,stream,1 << 20) )
    {
	fprintf(stderr,"error: sha256sum function failed");
	return -1;
    }
    
    fclose(stream);

    sha256_makearmor(armor,sum);

    printf("%s  %s\n",armor,name);

    return 0;
}

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
	return sum_file(stdin,"-");
    }
    else
    {
	for( char ** name = argv + 1, **max = argv + argc; name < max; name++)
	{
	    if( -1 == sum_file(fopen(*name,"r"),*name) )
		return -1;
	}

	return 0;
    }

    return -1;
}
