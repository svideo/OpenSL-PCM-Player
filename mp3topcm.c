#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

AVFrame * aFrame;
AVFrame * pFrame;
AVCodecContext * aCodecCtx;
AVCodecContext * pCodecCtx;
AVCodec * aCodec;
AVCodec * pCodec;
AVFormatContext * input_context;
AVPacket packet;
int videoStream = -1;
int audioStream = -1;

int main (int argc, char* const argv[])
{
	const char * filename = argv[1];
	printf("filename:%s", filename);

	FILE * fd;
	fd = fopen("gogo.pcm", "w");
	// fseek(fd, 0L, 2);
	// long filesize = ftell(fd);

	av_register_all();
	input_context = NULL;
	int ret = avformat_open_input(&input_context, filename, NULL, NULL);

	ret = avformat_find_stream_info(input_context, NULL);
	// int streamNum = input_context->nb_streams;
	int i;
	for (i=0; i<input_context->nb_streams; i++) {
		if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
		}
		else if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
			audioStream = i;
		}
	}

	aCodecCtx = input_context->streams[audioStream]->codec;
	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	avcodec_open2(aCodecCtx, aCodec, NULL);

	aFrame = av_frame_alloc();

	while(av_read_frame(input_context,&packet)>=0){
		if(packet.stream_index==audioStream){
			int len, got_frame, data_size;
			len = avcodec_decode_audio4(aCodecCtx, aFrame, &got_frame, &packet);
			data_size = av_samples_get_buffer_size(aFrame->linesize,aCodecCtx->channels, aFrame->nb_samples,aCodecCtx->sample_fmt, 0);
			printf("samples=%d,channels=%d,sample_fmt=%d\n", aFrame->nb_samples, aCodecCtx->channels, aCodecCtx->sample_fmt);
			fwrite(aFrame->data[0], data_size, 1, fd);
		}else {
		}
	}

	av_free(pFrame);
	av_free(aFrame);
	av_free_packet(&packet);
	avcodec_close(pCodecCtx);
	avcodec_close(aCodecCtx);
	avformat_close_input(&input_context);

	return 0;
}
