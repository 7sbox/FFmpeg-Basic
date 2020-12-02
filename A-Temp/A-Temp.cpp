// A-Temp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define    WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include "rgb2yuv.h"
#include "SDLMaster.h"
#include "ffmpegEncoder.h"
#include <setjmp.h>
#include "jpeglib.h"

#include "VS_SrcScreen.h"
#include "VS_Center.h"

extern "C"{ FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

int main()
{
	struct cap_screen_t sc;
	VS_SrcScreen::init_cap_screen(&sc, 1);

	BYTE* out;
	out = (BYTE*)malloc(sc.length);

	ffmpeg_init();
	AVFrame* frame;
	
	SDLMaster::init(sc.width, sc.height); // SDL播放器，可以用于播放测试
	YUVencoder enc(sc.width, sc.height);

	// 以下为不同的编码器，任选一个即可
	x264Encoder ffmpeg264(sc.width, sc.height, "save.h264"); // save.h264为本地录屏文件 可以使用vlc播放器播放
	//mpeg1Encoder ffmpegmpeg1(sc.width, sc.height, "save.mpg");
	
	time_t start = clock();
	size_t icount = 1000;
	for (size_t i = 0; i < icount; i++)
	{
		VS_SrcScreen::blt_cap_screen(&sc);

		frame = enc.encode(sc.buffer, sc.length, sc.width, sc.height, out, sc.length);

		if (frame == NULL)
		{
			printf("Encoder error!!\n"); Sleep(1000); continue;
		}

		// 一下是3种编码方式，任选一种均可
		ffmpeg264.encode(frame);
		// ffmpegmpeg1.encode(frame); 
		printf("encode a frame %d ................\n",i);

		// SDL播放器播放视频，取消注释即可播放
		SDLMaster::updateScreen(frame);

	}
	time_t end = clock();
	printf("运行%d帧耗时:%fs秒\n", icount, double(end - start) / CLOCKS_PER_SEC); //秒

	ffmpeg264.flush(frame);

	SDL_Quit();

	VS_SrcScreen::des_cap_screen(&sc);

	getchar();

	return 0;
}
