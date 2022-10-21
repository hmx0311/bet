#pragma once

#define CONFIG_FILE_NAME _T("bet.cfg")

struct Config
{
	double defCut;
	bool useDefCut;
	bool defClosing;
	int fastAddedAmount[4];
	double defProbError;
};

extern HINSTANCE hInst;

extern Config config;
extern float xScale;
extern float yScale;
extern int listItemHeight;
extern HFONT hFont;
extern HFONT hBoldFont;
extern UINT DRAGLISTMSG;


void loadConfig(Config& cfg);