#include <stdint.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "blkd.h"

#include <stdio.h>
#include <unistd.h>
#include "../log/log.h"
#include <string.h>

blkd_size min_blkdfs_size =
    +BLKD_SECTOR_SIZE // superblock header
    +BLKD_SECTOR_SIZE // superblock
    +BLKD_SECTOR_SIZE // root dir header
    +BLKD_SECTOR_SIZE // root dir
    ;

bool blkd_mkfs (int fd)
{
    long size = lseek (fd, 0, SEEK_END);

    if (size < 0)
    {
	perror ("lseek");
	log_fatal ("Failed to determine device size");
    }

    if ((size_t)size < min_blkdfs_size)
    {
	log_fatal ("Size %zu is too small to create a blkd filesystem in", size);
    }

    unsigned char write_block[BLKD_SECTOR_SIZE];

    {
	memset (write_block, 0, sizeof (write_block));
	blkd_header * header = (void*) write_block;
	header->magic = BLKD_MAGIC;
	header->version = BLKD_HEADER_VERSION;
	
    
fail:
    return false;
}
