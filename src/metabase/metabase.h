#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef enum metabase_id metabase_id;
enum metabase_id
{
    METABASE_IDENTITY = 0x00,
    METABASE_BASE2 = '0',
    METABASE_BASE8 = '7',
    METABASE_BASE10 = '9',
    METABASE_BASE16 = 'f',
    METABASE_BASE16UPPER = 'F',
    METABASE_BASE32HEX = 'v',
    METABASE_BASE32HEXUPPER = 'V',
    METABASE_BASE32HEXPAD = 't',
    METABASE_BASE32HEXPADUPPER = 'T',
    METABASE_BASE32 = 'b',
    METABASE_BASE32UPPER = 'B',
    METABASE_BASE32PAD = 'c',
    METABASE_BASE32PADUPPER = 'C',
    METABASE_BASE32Z = 'h',
    METABASE_BASE58FLICKR = 'Z',
    METABASE_BASE58BTC = 'z',
    METABASE_BASE64 = 'm',
    METABASE_BASE64PAD = 'M',
    METABASE_BASE64URL = 'u',
    METABASE_BASE64URLPAD = 'U',
    METABASE_MBTI = 'P', // this is a joke
    METABASE_DEFAULT = METABASE_BASE16
};

typedef void (*metabase_encoder)(buffer_char * output, const range_const_unsigned_char * input);

void metabase_encode_base16(buffer_char * output, const range_const_unsigned_char * input);
bool metabase_decode_base16 (buffer_unsigned_char * output, const range_const_char * input);

void metabase_encode_base2(buffer_char * output, const range_const_unsigned_char * input);
bool metabase_decode_base2 (buffer_unsigned_char * output, const range_const_char * input);

void metabase_encode_mbti(buffer_char * output, const range_const_unsigned_char * input);

#define metabase_encode metabase_encode_base16
bool metabase_decode (buffer_unsigned_char * output, const range_const_char * input);

metabase_id metabase_identify_name (const char * name);
metabase_encoder metabase_select_encoder (metabase_id id);
