#include "framework.h"
#include "common.h"

#include<Windowsx.h>

using namespace std;

Config config;
float xScale = 1;
float yScale = 1;
int listItemHeight;
HFONT hFont;
HFONT hBoldFont;
UINT DRAGLISTMSG;

HFONT createBoldFont(HFONT hFont)
{
	LOGFONT logFont;
	GetObject(hFont, sizeof(LOGFONT), &logFont);
	listItemHeight = abs(logFont.lfHeight);
	logFont.lfWeight = FW_BOLD;
	return CreateFontIndirect(&logFont);
}