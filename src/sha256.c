#include "sha256.h"
#include <openssl/sha.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>

void sha256_makearmor(sha256_armor armor, const sha256 orig)
{
    const uint32_t * conv = (const void*)orig;
    size_t max = sizeof(sha256) / sizeof(*conv);
    char * spot;
    uint32_t flip;
    for(size_t i = 0; i < max; i++)
    {
	flip = htonl(conv[i]);
	spot = armor + 8 * i;
	sprintf(spot, "%08x", flip);
    }
}

int sha256_stream(sha256 sum, FILE * file, size_t buffer_size)
{
    SHA256_CTX ctx;

    SHA256_Init(&ctx);

    if( buffer_size == 0 )
	buffer_size = 1 << 17;

    unsigned char * buffer = malloc( buffer_size );

    size_t got;

    while( 0 < (got = fread(buffer,1,buffer_size,file)) )
    {
	SHA256_Update(&ctx,buffer,got);
	
	if( feof(file) || ferror(file) )
	    break;
    }

    free(buffer);
    
    SHA256_Final(sum, &ctx);

    if( ferror(file) )
    {
	perror("sha256_stream");
	return -1;
    }

    return 0;    
}

int sha256_path(sha256 sum, const char * path, size_t buffer_size)
{
    FILE * file = fopen(path,"r");
    if( !file )
    {
	perror(path);
	return -1;
    }

    int ret = sha256_stream(sum,file,buffer_size);

    fclose(file);

    return ret;
}

int sha256_buffer(sha256 sum, const void * buffer, size_t size)
{
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, buffer, size);
    SHA256_Final(sum,&ctx);

    return 0;
}

void sha256_halt_partial(sha256_job * job)
{
    SHA256_Final(job->sum,job->ctx);
    free(job->ctx);
    job->ctx = NULL;
}

int sha256_partial(sha256_job * job)
{
    if(!job->ctx)
    {
	job->ctx = malloc(sizeof(SHA256_CTX));
	SHA256_Init(job->ctx);
    }

    ssize_t get = fread(job->buffer,1,job->buffer_size,job->file);

    if(ferror(job->file))
    {
	sha256_halt_partial(job);
	job->success = false;
	return -1;
    }

    if(feof(job->file))
    {
	sha256_halt_partial(job);
	job->success = true;
	return 1;
    }

    return 0;
}
