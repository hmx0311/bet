#pragma once

#define CONFIG_FILE_NAME _T("bet.cfg")

struct Config
{
	double defaultCut;
	bool useDefaultCut;
	bool defaultClosing;
	int fastAddedAmount[4];
	double defaultProbError;
};

extern Config config;
extern float xScale;
extern float yScale;
extern int listItemHeight;
extern HFONT hFont;
extern HFONT hBoldFont;

extern HINSTANCE hInst;

void loadConfig(Config& cfg);