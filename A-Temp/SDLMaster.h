#pragma once
/*
实现一个播放器
*/
#include "ffmpeg.h"
#define _SDL_main_h
#define SDL_main_h_
extern "C"
{
	#include "SDL2/SDL.h"
}

//Refresh Event
#define REFRESH_EVENT  (SDL_USEREVENT + 1)
#define BREAK_EVENT  (SDL_USEREVENT + 2)

namespace SDLMaster
{
	SDL_Rect rect;
	Uint32 pixformat;

	//for render
	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	SDL_Thread *video_thread;
	SDL_Event event;

	int thread_exit = 0;
	int thread_pause = 0;

	int video_refresh_thread(void *data) {
		thread_exit = 0;
		thread_pause = 0;

		while (!thread_exit) {
			if (!thread_pause) {
				SDL_Event event;
				event.type = REFRESH_EVENT;
				SDL_PushEvent(&event);
			}
			SDL_Delay(40);
		}
		thread_exit = 0;
		thread_pause = 0;
		//Break
		SDL_Event event;
		event.type = BREAK_EVENT;
		SDL_PushEvent(&event);

		return 0;
	}

	void static init(int width, int height)
	{
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) 
		{
			printf("Could not initialize SDL - %s\n", SDL_GetError());
			return;
		}

 		int screen_w = width, screen_h = height;
		win = SDL_CreateWindow("Media Player",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			800, 600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		
		renderer = SDL_CreateRenderer(win, -1, 0);
		if (!renderer) {
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Renderer by SDL");
			return;
		}

		pixformat = SDL_PIXELFORMAT_IYUV;
		texture = SDL_CreateTexture(renderer,
			pixformat,
			SDL_TEXTUREACCESS_STREAMING,
			screen_w,
			screen_h);

		rect.x = 0;
		rect.y = 0;
		rect.w = screen_w;
		rect.h = screen_h;

		video_thread = SDL_CreateThread(video_refresh_thread, "Video Thread", NULL);

	}

	void static updateScreen(AVFrame *pFrameYUV)
	{
		SDL_WaitEvent(&event);
		if (event.type == REFRESH_EVENT)
		{
			SDL_UpdateYUVTexture(texture, NULL,
				pFrameYUV->data[0], pFrameYUV->linesize[0],
				pFrameYUV->data[1], pFrameYUV->linesize[1],
				pFrameYUV->data[2], pFrameYUV->linesize[2]);

			// Set Size of Window
// 			rect.x = 0;
// 			rect.y = 0;
// 			rect.w = pCodecCtx->width;
// 			rect.h = pCodecCtx->height;

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_RenderPresent(renderer);
		}
		else if (event.type == SDL_QUIT)
		{
			printf("quit\n");
			thread_exit = 1;
		}
	}
};

