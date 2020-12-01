#pragma once
#include <atlstr.h>
#include <vector>
#include <WinDef.h>
#include <tchar.h>

struct MonitorInfo {
	WCHAR       szDevice[CCHDEVICENAME];				// ��ʾ������
	RECT area;											// ��ʾ����������
	DWORD nWidth, nHeight, nFreq, nBits;
};
typedef std::vector<MonitorInfo*> VEC_MONITOR_INFO;		// ���е���ʾ����Ϣ

extern VEC_MONITOR_INFO vecMonitorListInfo;

struct cap_screen_t
{
	HDC memdc;
	HBITMAP hbmp;
	unsigned char* buffer;
	int            length;

	int width;
	int height;
	int bitcount;
	int left, top;
};

class VS_SrcScreen
{
public:
	static void GetAllMonitorInfo();
	static HCURSOR FetchCursorHandle();
	static void AddCursor(HDC hMemDC, POINT origin);
public:
	static int init_cap_screen(struct cap_screen_t* sc, int indexOfMonitor = 0);
	static int blt_cap_screen(struct cap_screen_t* sc);
	static void des_cap_screen(struct cap_screen_t* sc);
};

