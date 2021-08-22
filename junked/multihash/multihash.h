#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef enum multihash_id multihash_id;
enum multihash_id {
    MULTIHASH_IDENTITY = 0x0,
    MULTIHASH_SHA1 = 0x11,
    MULTIHASH_SHA2_256,
    MULTIHASH_SHA2_512,
    MULTIHASH_SHA3_512 = 0x14,
    MULTIHASH_SHA3_384,
    MULTIHASH_SHA3_256,
    MULTIHASH_SHA3_224,
    MULTIHASH_SHAKE_128 = 0x18,
    MULTIHASH_SHAKE_256,
    MULTIHASH_KECCAK_224 = 0x1a,
    MULTIHASH_KECCAK_256,
    MULTIHASH_KECCAK_384,
    MULTIHASH_MURMUR3_128 = 0x22,
    MULTIHASH_MURMUR3_32,
};

typedef enum multibase_id multibase_id;
enum multibase_id
{
    MULTIBASE_IDENTITY = 0x00,
    MULTIBASE_BASE2 = '0',
    MULTIBASE_BASE8 = '7',
    MULTIBASE_BASE10 = '9',
    MULTIBASE_BASE16 = 'f',
    MULTIBASE_BASE16UPPER = 'F',
    MULTIBASE_BASE32HEX = 'v',
    MULTIBASE_BASE32HEXUPPER = 'V',
    MULTIBASE_BASE32HEXPAD = 't',
    MULTIBASE_BASE32HEXPADUPPER = 'T',
    MULTIBASE_BASE32 = 'b',
    MULTIBASE_BASE32UPPER = 'B',
    MULTIBASE_BASE32PAD = 'c',
    MULTIBASE_BASE32PADUPPER = 'C',
    MULTIBASE_BASE32Z = 'h',
    MULTIBASE_BASE58FLICKR = 'Z',
    MULTIBASE_BASE58BTC = 'z',
    MULTIBASE_BASE64 = 'm',
    MULTIBASE_BASE64PAD = 'M',
    MULTIBASE_BASE64URL = 'u',
    MULTIBASE_BASE64URLPAD = 'U',
};