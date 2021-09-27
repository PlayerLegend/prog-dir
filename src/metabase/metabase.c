#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#define FLAT_INCLUDES

#include "../array/range.h"
#include "../array/buffer.h"

#include "metabase.h"

#include "../log/log.h"
#include "../base16/base16.h"
#include "../base2/base2.h"

#define fatal(mesg,...) { log_error(mesg,#__VA_ARGS__); goto fail; }

#define BASE16_LITTLE_ENDIAN false

// base16

bool metabase_decode_base16 (buffer_unsigned_char * output, const range_const_char * input)
{
    if (*input->begin != METABASE_BASE16 && *input->begin != METABASE_BASE16UPPER)
    {
	fatal("Base16 decoder used on non-base16 string");
    }
    
    range_const_char decode = { .begin = input->begin + 1, .end = input->end };

    return base16_decode (output, &decode, BASE16_LITTLE_ENDIAN);
    
fail:
    return false;
}

void metabase_encode_base16(buffer_char * output, const range_const_unsigned_char * input)
{
    *buffer_push(*output) = METABASE_BASE16;
    base16_encode(output,input, BASE16_LITTLE_ENDIAN);
}

// mbti

static void print_mbti_personality (buffer_char * output, unsigned char input)
{
    assert (input < 16);

    *buffer_push(*output) = input & 1 ? 'E' : 'I';
    *buffer_push(*output) = input & 2 ? 'S' : 'N';
    *buffer_push(*output) = input & 4 ? 'F' : 'T';
    *buffer_push(*output) = input & 8 ? 'J' : 'P';
}

void metabase_encode_mbti(buffer_char * output, const range_const_unsigned_char * input)
{
    *buffer_push(*output) = METABASE_MBTI;

    const unsigned char * c;

    for_range (c, *input)
    {
	print_mbti_personality(output, *c % 16);
	print_mbti_personality(output, *c / 16);
    }
}

// base2

void metabase_encode_base2(buffer_char * output, const range_const_unsigned_char * input)
{
    *buffer_push (*output) = METABASE_BASE2;
    base2_encode(output,input);
}

bool metabase_decode_base2 (buffer_unsigned_char * output, const range_const_char * input)
{
    if (*input->begin != METABASE_BASE2)
    {
	fatal("base2 decoder used on non-base2 string");
    }
    
    range_const_char decode = { .begin = input->begin + 1, .end = input->end };

    return base2_decode (output, &decode);
    
fail:
    return false;
}

// name
const char * metabase_name (metabase_id id)
{
    switch (id)
    {
    default:
	return "invalid";
	
    case METABASE_IDENTITY:
	return "NIL";
	
    case METABASE_BASE2:
	return "base2";
	
    case METABASE_BASE8:
	return "base8";
	
    case METABASE_BASE10:
	return "base10";
	
    case METABASE_BASE16:
	return "base16";
	
    case METABASE_BASE16UPPER:
	return "base16-upper";
	
    case METABASE_BASE32HEX:
	return "base32-hex";
	
    case METABASE_BASE32HEXUPPER:
	return "base32-hex-upper";
	
    case METABASE_BASE32HEXPAD:
	return "base32-hex-pad";
	
    case METABASE_BASE32HEXPADUPPER:
	return "base32-hex-pad-upper";
	
    case METABASE_BASE32:
	return "base32";
	
    case METABASE_BASE32UPPER:
	return "base32-upper";
	
    case METABASE_BASE32PAD:
	return "base32-pad";
	
    case METABASE_BASE32PADUPPER:
	return "base32-pad-upper";
	
    case METABASE_BASE32Z:
	return "base32-z";
	
    case METABASE_BASE58FLICKR:
	return "base58-flickr";
	
    case METABASE_BASE58BTC:
	return "base58-btc";
	
    case METABASE_BASE64:
	return "base64";
	
    case METABASE_BASE64PAD:
	return "base64-pad";
	
    case METABASE_BASE64URL:
	return "base64-url";
	
    case METABASE_BASE64URLPAD:
	return "base64-url-pad";
	
    case METABASE_MBTI:
	return "MBTI";
    }
}

// generic decode

bool metabase_decode (buffer_unsigned_char * output, const range_const_char * input)
{
    switch ((metabase_id) *input->begin)
    {
    case METABASE_BASE16:
	return metabase_decode_base16(output, input);
	
    case METABASE_BASE2:
	return metabase_decode_base2(output, input);
	
    case METABASE_IDENTITY:
    case METABASE_BASE8:
    case METABASE_BASE10:
    case METABASE_BASE16UPPER:
    case METABASE_BASE32HEX:
    case METABASE_BASE32HEXUPPER:
    case METABASE_BASE32HEXPAD:
    case METABASE_BASE32HEXPADUPPER:
    case METABASE_BASE32:
    case METABASE_BASE32UPPER:
    case METABASE_BASE32PAD:
    case METABASE_BASE32PADUPPER:
    case METABASE_BASE32Z:
    case METABASE_BASE58FLICKR:
    case METABASE_BASE58BTC:
    case METABASE_BASE64:
    case METABASE_BASE64PAD:
    case METABASE_BASE64URL:
    case METABASE_BASE64URLPAD:
    case METABASE_MBTI:
	log_error ("Base %s decoding is not implemented yet", metabase_name (*input->begin));
	return false;

    default:
	log_error ("Invalid prefix, cannot determine decoder");
	return false;
    }
}

// identify name
metabase_id metabase_identify_name (const char * name)
{
    struct map { const char * name; metabase_id id; }
    *max, *i,
	map[] =
	{
	    { "base16", METABASE_BASE16 },
	    { "base2", METABASE_BASE2 },
	};

    for (i = map, max = map + sizeof (map) / sizeof (map[0]); i < max; i++)
    {
	if (0 == strcmp (i->name, name))
	{
	    return i->id;
	}
    }

    return METABASE_IDENTITY;
}

// select encoder
metabase_encoder metabase_select_encoder (metabase_id id)
{
    
    switch (id)
    {
    case METABASE_BASE16:
	return metabase_encode_base16;
	
    case METABASE_BASE2:
	return metabase_encode_base2;
	
    case METABASE_IDENTITY:
    case METABASE_BASE8:
    case METABASE_BASE10:
    case METABASE_BASE16UPPER:
    case METABASE_BASE32HEX:
    case METABASE_BASE32HEXUPPER:
    case METABASE_BASE32HEXPAD:
    case METABASE_BASE32HEXPADUPPER:
    case METABASE_BASE32:
    case METABASE_BASE32UPPER:
    case METABASE_BASE32PAD:
    case METABASE_BASE32PADUPPER:
    case METABASE_BASE32Z:
    case METABASE_BASE58FLICKR:
    case METABASE_BASE58BTC:
    case METABASE_BASE64:
    case METABASE_BASE64PAD:
    case METABASE_BASE64URL:
    case METABASE_BASE64URLPAD:
    case METABASE_MBTI:
	log_fatal ("Base %s decoding is not implemented yet", metabase_name (id));

    default:
	log_fatal ("Invalid prefix, cannot determine decoder");
    }

fail:
    return false;
}
