#include <assert.h>
#include <stdlib.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/pixdesc.h>
#define FLAT_INCLUDES
#include "../../log/log.h"

/*int avpicture_fill_wrapper (AVPicture * picture,
			    const uint8_t * ptr,
			    enum AVPixelFormat pix_fmt,
			    int width,
			    int height)
{
    return av_image_fill_arrays (picture->data, picture->linesize, ptr, pix_fmt, width, height, 1);
    }*/

int main(int argc, char * argv[])
{

    assert (argc == 2);
    AVFormatContext * format_ctx = NULL;
    if (0 != avformat_open_input (&format_ctx, argv[1], NULL, 0))
    {
	log_fatal ("Couldn't open file");
    }

    if (0 > avformat_find_stream_info (format_ctx, NULL))
    {
	log_fatal ("Couldn't find stream information");
    }

    av_dump_format (format_ctx, 0, argv[1], 0); // dump information about the file into stderr

    int video_stream = -1;
    
    {
	unsigned int i;
	for (i = 0; i < format_ctx->nb_streams; i++)
	{
	    if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
	    {
		video_stream = i;
		break;
	    }
	}

	if (i == format_ctx->nb_streams)
	{
	    log_fatal ("Couldn't find a video stream in the file");
	}

	assert (video_stream != -1);
    }

    AVCodecParameters * codec_parameters = format_ctx->streams[video_stream]->codecpar;

    AVCodec * codec = avcodec_find_decoder (codec_parameters->codec_id);

    if (!codec)
    {
	log_fatal ("Unsupported codec!");
    }

    AVCodecContext * codec_context = avcodec_alloc_context3 (codec);

    if (0 > avcodec_parameters_to_context (codec_context, codec_parameters))
    {
	log_fatal ("Could not convert codec parameters to codec context");
    }

    if (0 > avcodec_open2 (codec_context, codec, NULL))
    {
	log_fatal ("Could not open avcodec");
    }

    AVFrame * frame_rgb = av_frame_alloc();

    if (!frame_rgb)
    {
	log_fatal ("Could not allocate frame_rgb");
    }

    int buffer_size = av_image_get_buffer_size (AV_PIX_FMT_RGB24, codec_context->width, codec_context->height, codec_context->block_align);

    unsigned char * buffer = av_malloc (buffer_size);

    int buffer_linesize[4];

    if (0 > av_image_fill_arrays (frame_rgb->data,
				  buffer_linesize,
				  buffer,
				  AV_PIX_FMT_RGB24,
				  codec_context->width,
				  codec_context->height,
				  1))
    {
	log_fatal ("av_image_fill_arrays failed");
    }

    struct SwsContext * sws_context = sws_getContext (codec_context->width,
						      codec_context->height,
						      codec_context->pix_fmt,
						      codec_context->width,
						      codec_context->height,
						      AV_PIX_FMT_RGB24,
						      SWS_BILINEAR,
						      NULL,
						      NULL,
						      NULL);

    if (!sws_context)
    {
	log_fatal ("Could not get sws_context");
    }

    AVPacket packet;
    
    AVFrame * frame = av_frame_alloc();

    if (!frame)
    {
	log_fatal ("Could not allocate frame_rgb");
    }
    
    while (0 <= av_read_frame (format_ctx, &packet))
    {
	if (packet.stream_index == video_stream)
	{
	    if (0 != avcodec_receive_frame (codec_context, frame))
	    {
		log_fatal ("avcodec_receive_frame failed");
	    }

	    
	}
    }
    
    return 0;
	
fail:
    return 1;
}
