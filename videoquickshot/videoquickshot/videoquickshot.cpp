// videoquickshot.cpp : Defines the entry point for the console application.
// Get VideoFrame from video file,then dump the snapshot image to JPG or Png format
//
#include "stdafx.h"
#include <stdio.h>

extern "C"
{
	#include  "libavcodec/avcodec.h" 
	#include  "libavformat/avformat.h" 
	#include  "libswscale/swscale.h" 
	#include  "image.h"
}

#ifndef HAVE_COMMANDLINETOARGVW
#define HAVE_COMMANDLINETOARGVW
#endif

#ifdef HAVE_COMMANDLINETOARGVW 
#include <windows.h>
#include <shellapi.h>

static char** win32_argv_utf8 = NULL;
static int win32_argc = 0;

static void prepare_app_arguments(int *argc_ptr, char ***argv_ptr)
{
    char *argstr_flat;
    wchar_t **argv_w;
    int i, buffsize = 0, offset = 0;

    if (win32_argv_utf8) {
        *argc_ptr = win32_argc;
        *argv_ptr = win32_argv_utf8;
        return;
    }

    win32_argc = 0;
    argv_w = CommandLineToArgvW(GetCommandLineW(), &win32_argc);
    if (win32_argc <= 0 || !argv_w)
        return;
    
    for (i = 0; i < win32_argc; i++)
        buffsize += WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1,
                                        NULL, 0, NULL, NULL);

    win32_argv_utf8 = (char **)av_mallocz(sizeof(char *) * (win32_argc + 1) + buffsize);
    argstr_flat     = (char *)win32_argv_utf8 + sizeof(char *) * (win32_argc + 1);
    if (win32_argv_utf8 == NULL) {
        LocalFree(argv_w);
        return;
    }

    for (i = 0; i < win32_argc; i++) {


        win32_argv_utf8[i] = &argstr_flat[offset];
        offset += WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1,
                                      &argstr_flat[offset],
                                      buffsize - offset, NULL, NULL);
    }
    win32_argv_utf8[i] = NULL;
    LocalFree(argv_w);

    *argc_ptr = win32_argc;
    *argv_ptr = win32_argv_utf8;
}
#endif

int SnapShotInstance(const char* Src,const char* dst, uint64_t iFrameSec,int output_w,int output_h)
{
    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL; 
    AVFrame         *pFrameRGB = NULL;
    AVPacket        packet;
    int             frameFinished;
    int             numBytes;
	int				numMaxSeekCnt = 5;
	int				numMaxReadCnt = 10;
    uint8_t         *buffer = NULL;
	uint64_t		timestamp = 0;
	uint64_t		timeBase = 0;
	int				flags;

    AVDictionary    *optionsDict = NULL;
    struct SwsContext      *sws_ctx = NULL;
  
    av_register_all();

   
	if(avformat_open_input(&pFormatCtx, Src, NULL, NULL)!=0)
	{
		printf("Error %s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
        return -1; 
	}

  
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
	{
		printf("Error %s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
        return -1; 
	}
   
    av_dump_format(pFormatCtx, 0, Src, 0);
    
    videoStream = -1;
    for(i=0;i<pFormatCtx->nb_streams;i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
        {
            videoStream=i;
            break;
        }
    }
        
    if(videoStream==-1)
	{
		printf("Error %s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
        return -1; 
	}
   
    pCodecCtx= pFormatCtx->streams[videoStream]->codec;  
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        fprintf(stderr,"Unsupported codec!\n");
        return -1;
    }
    
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
	{
		printf("Error %s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
	}

   
    pFrame = avcodec_alloc_frame();  
    pFrameRGB=avcodec_alloc_frame();
    
    if(pFrameRGB==NULL)
        return -1;
   
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
          pCodecCtx->height);
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    sws_ctx =
    sws_getContext
    (
        pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
    );

 
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
         pCodecCtx->width, pCodecCtx->height);

  // using iFrameSec to seek video pos and capture the video frame 
  // if iFrameSec Pos In Video Frame Meet a black  or miss info ,will retry to snapshot

  // caculate the timestamp by using TIMEBASE
  /*
  timeBase = (int64_t(pCodecCtx->time_base.num) * AV_TIME_BASE) / int64_t(pCodecCtx->time_base.den);
  timestamp = iFrameSec/timeBase; */

  

  timestamp= iFrameSec*1000/pCodecCtx->time_base.num/pCodecCtx->time_base.den;

  


  while( (av_seek_frame(pFormatCtx,videoStream,timestamp,AVSEEK_FLAG_BACKWARD) <  0)  &&  numMaxSeekCnt   )
  {
	  numMaxSeekCnt--;
	  timestamp += 5;
  }  
 
  while(av_read_frame(pFormatCtx, &packet)>=0  && numMaxReadCnt ) 
  {
    
    if(packet.stream_index==videoStream) 
    {   
		numMaxReadCnt--;
		avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, 
			   &packet);
   
		if(frameFinished) 
		{       
			sws_scale
			(
				sws_ctx,
				(uint8_t const * const *)pFrame->data,
				pFrame->linesize,
				0,
				pCodecCtx->height,
				pFrameRGB->data,
				pFrameRGB->linesize
			);

			if( image_save(dst, pFrameRGB->data[0],
			pCodecCtx->width, pCodecCtx->height,
			IMAGE_FORMAT_JPEG)  ==  -1 )
			{
				printf("Error %s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
			}
			break;
		}

		av_free_packet(&packet);
    }
  } 
  av_free(buffer);
  av_free(pFrameRGB);  
  av_free(pFrame); 
  avcodec_close(pCodecCtx); 
  avformat_close_input(&pFormatCtx);
  
  return 0;
}


int main(int argc, char** argv)
{
    if( argc < 5 )
    {
        printf("Usage: cmd inputvideo outputimage time(ms) width height \n   sample: cmd d:\video\1.mp4  d:\snapshot.jpg  10  320 240");
        return -1;
    }
	
#ifdef HAVE_COMMANDLINETOARGVW	&& defined(WIN32)

	//Why do this:
	// ffmpeg api only support UTF8
	// wfopen only support unicode
    prepare_app_arguments(&argc,&argv);
	
	wchar_t **argv_w;
	argv_w = CommandLineToArgvW(GetCommandLineW(), &win32_argc);
	return SnapShotInstance( argv[1],(char*)argv_w[2],atoi(argv[3]));
#else
	return SnapShotInstance( argv[1],argv[2],atoi(argv[3]));
#endif	
	
    
}
