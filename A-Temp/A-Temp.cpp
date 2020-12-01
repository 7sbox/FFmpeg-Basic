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

struct dp_frame_t
{
	int        cx;          ///屏幕宽度
	int        cy;          ///屏幕高度
	int        line_bytes;  ///每个扫描行的实际数据长度
	int        line_stride; ///每个扫描行的4字节对齐的数据长度
	int        bitcount;    ///8.16.24.32 位深度, 8位是256调色板； 16位是555格式的图像

	char*      buffer;      ///屏幕数据
};

int jpeg_quality = 85;

struct jpeg_error_t {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
static void jpeg_error_exit(j_common_ptr cinfo)
{
	jpeg_error_t* myerr = (jpeg_error_t*)cinfo->err;
	////
	(*cinfo->err->output_message) (cinfo);

	longjmp(myerr->setjmp_buffer, 1);
}

void compressframe(struct dp_frame_t* frame)
{
	//if (frame->bitcount == 8)return; // 8浣嶈壊涓嶅鐞?

	unsigned char* line_data = NULL;
	struct jpeg_compress_struct cinfo; //
	memset(&cinfo, 0, sizeof(cinfo)); //鍏ㄩ儴鍒濆鍖栦负0锛?鍚﹀垯瑕佸嚭闂

	struct jpeg_error_t jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpeg_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		////
		jpeg_destroy_compress(&cinfo);
		if (line_data)free(line_data);
		return;
	}
	jpeg_create_compress(&cinfo);

	byte* out_ptr = NULL;
	unsigned long out_size = 0;

	jpeg_mem_dest(&cinfo, &out_ptr, (unsigned long*)&out_size);
	cinfo.image_width = frame->cx;
	cinfo.image_height = frame->cy;

	int bit = frame->bitcount;
	if (bit == 16) {
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_EXT_BGR;
		line_data = (byte*)malloc(cinfo.image_width * 3); /// BGR data
	}
	else {
		cinfo.input_components = bit / 8;
		if (bit == 32) cinfo.in_color_space = JCS_EXT_BGRA;
		else if (bit == 24) cinfo.in_color_space = JCS_EXT_BGR;
	}
	jpeg_set_defaults(&cinfo);

	int quality = jpeg_quality; ////鍘嬬缉璐ㄩ噺

	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	while (cinfo.next_scanline < cinfo.image_height) {
		if (bit == 16) {
			WORD* dat = (WORD*)(frame->buffer + cinfo.next_scanline * frame->line_stride);
			int k = 0;
			for (int i = 0; i < cinfo.image_width; ++i) {
				WORD bb = dat[i];
				line_data[k] = ((bb & 0x1F) << 3);          // b
				line_data[k + 1] = ((bb >> 5) & 0x1F) << 3; // g
				line_data[k + 2] = ((bb >> 10) & 0x1F) << 3; // r
															 ///
				k += 3; ///
			}
			jpeg_write_scanlines(&cinfo, &line_data, 1);
			////
		}
		else {
			byte* line = (byte*)frame->buffer + cinfo.next_scanline * frame->line_stride; ///
			jpeg_write_scanlines(&cinfo, &line, 1);
		}
	}

	jpeg_finish_compress(&cinfo);

	jpeg_destroy_compress(&cinfo);
	if (line_data)free(line_data);

	//------------------//
	// out_ptr out_size //
	//------------------//
	//place code to here...

	free(out_ptr);
}

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

// 		dp_frame_t a;
// 		a.bitcount = 32;
// 		a.buffer = (char*)sc.buffer;
// 		a.line_bytes = sc.width * sc.bitcount / 8;
// 		a.line_stride = (a.line_bytes + 3) / 4 * 4;
// 		a.cx = sc.width;
// 		a.cy = sc.height;
// 		compressframe(&a);

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
