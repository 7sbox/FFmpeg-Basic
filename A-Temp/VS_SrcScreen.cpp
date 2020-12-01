#include "VS_SrcScreen.h"


static std::vector<HMONITOR> g_hMonitorGroup;
static VEC_MONITOR_INFO vecMonitorListInfo;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor,
	HDC hdc,
	LPRECT lpRMonitor,
	LPARAM dwData)
{
	g_hMonitorGroup.push_back(hMonitor);
	MonitorInfo* monitorinfo = new MonitorInfo();
	monitorinfo->area.left = lpRMonitor->left;
	monitorinfo->area.right = lpRMonitor->right;
	monitorinfo->area.top = lpRMonitor->top;
	monitorinfo->area.bottom = lpRMonitor->bottom;
	monitorinfo->nWidth = lpRMonitor->right - lpRMonitor->left;
	monitorinfo->nHeight = lpRMonitor->bottom - lpRMonitor->top;
	vecMonitorListInfo.push_back(monitorinfo);
	return 1;
}

void VS_SrcScreen::GetAllMonitorInfo()
{
	g_hMonitorGroup.clear();
	vecMonitorListInfo.clear();

	::EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
	for (int i = 0; i < g_hMonitorGroup.size(); i++)
	{
		MONITORINFOEX mixTemp;
		memset(&mixTemp, 0, sizeof(MONITORINFOEX));
		mixTemp.cbSize = sizeof(MONITORINFOEX);

		GetMonitorInfo(g_hMonitorGroup[i], &mixTemp);
		wcscpy_s((*vecMonitorListInfo[i]).szDevice, mixTemp.szDevice);
		DEVMODE DeviceMode;
		EnumDisplaySettings(mixTemp.szDevice, ENUM_CURRENT_SETTINGS, &DeviceMode);
		(*vecMonitorListInfo[i]).nFreq = DeviceMode.dmDisplayFrequency;
		(*vecMonitorListInfo[i]).nBits = DeviceMode.dmBitsPerPel;
	}
}

int VS_SrcScreen::init_cap_screen(struct cap_screen_t* sc, int indexOfMonitor)
{
	GetAllMonitorInfo();
	if (indexOfMonitor >= vecMonitorListInfo.size())
		indexOfMonitor = 0;
	BITMAPINFOHEADER bi;
	sc->width = vecMonitorListInfo[indexOfMonitor]->nWidth;
	sc->height = vecMonitorListInfo[indexOfMonitor]->nHeight;
	sc->bitcount = vecMonitorListInfo[indexOfMonitor]->nBits;
	sc->left = vecMonitorListInfo[indexOfMonitor]->area.left;
	sc->top = vecMonitorListInfo[indexOfMonitor]->area.top;
	memset(&bi, 0, sizeof(bi));
	bi.biSize = sizeof(bi);
	bi.biWidth = sc->width;
	bi.biHeight = -sc->height; //从上朝下扫描
	bi.biPlanes = 1;
	bi.biBitCount = sc->bitcount; //RGB
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	HDC hdc = GetDC(NULL); //屏幕DC
	sc->memdc = CreateCompatibleDC(hdc);
	sc->buffer = NULL;
	sc->hbmp = CreateDIBSection(hdc, (BITMAPINFO*)&bi, DIB_RGB_COLORS, (void**)&sc->buffer, NULL, 0);
	ReleaseDC(NULL, hdc);
	SelectObject(sc->memdc, sc->hbmp); ///
	sc->length = sc->height* (((sc->width*sc->bitcount / 8) + 3) / 4 * 4);
	return 0;
}

void VS_SrcScreen::des_cap_screen(struct cap_screen_t* sc)
{
	DeleteObject(sc->hbmp);
	DeleteDC(sc->memdc);
}

HCURSOR VS_SrcScreen::FetchCursorHandle()
{
	CURSORINFO hCur;
	ZeroMemory(&hCur, sizeof(hCur));
	hCur.cbSize = sizeof(hCur);
	GetCursorInfo(&hCur);
	return hCur.hCursor;
}

void VS_SrcScreen::AddCursor(HDC hMemDC, POINT origin)
{
	POINT xPoint;
	GetCursorPos(&xPoint);
	xPoint.x -= origin.x;
	xPoint.y -= origin.y;
	if (xPoint.x < 0 || xPoint.y < 0)
		return;
	HCURSOR hcur = FetchCursorHandle();
	ICONINFO iconinfo;
	BOOL ret;
	ret = GetIconInfo(hcur, &iconinfo);
	if (ret)
	{
		xPoint.x -= iconinfo.xHotspot;
		xPoint.y -= iconinfo.yHotspot;
		if (iconinfo.hbmMask) DeleteObject(iconinfo.hbmMask);
		if (iconinfo.hbmColor) DeleteObject(iconinfo.hbmColor);
	}
	DrawIcon(hMemDC, xPoint.x, xPoint.y, hcur);
}


int VS_SrcScreen::blt_cap_screen(struct cap_screen_t* sc)
{
	HDC hdc = GetDC(NULL);
	BitBlt(sc->memdc, 0, 0, sc->width, sc->height, hdc, sc->left, sc->top, SRCCOPY); // 截屏
	AddCursor(sc->memdc, POINT{ sc->left, sc->top }); // 增加鼠标进去
	ReleaseDC(NULL, hdc);
	return 0;
}