/*
 * Copyright (c) 2017 Liming Shao <lmshao@163.com>
 */

#include "Transcoding.h"
#include <stdio.h>

// unfinished
// ���������е�󣬹�����������Ϥ��������Filter��������ת�롣
int main()
{
    const char *srcFile = "../assets/Sample.mkv";
    const char *dstFile = "../assets/Sample.ts";

    Transcoding tc;
    
    tc.initSys();

    if (tc.initDecCtx(srcFile)) {
        printf("Failed to init decodec ctx");
        return 1;
    }

    if (tc.initEncCtx(dstFile)) {
        printf("Failed to init encodec ctx");
        return 1;
    }

    if (tc.initFilters()) {
        printf("Failed to init filters");
        return 1;
    }

    if (tc.transcode()) {
        printf("Err.");
        return 1;
    }

    return 0;
}