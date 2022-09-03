#pragma once

#define CONFIG_FILE_NAME _T("bet.cfg")
#define MIN_AMOUNT 10

struct Config
{
	double cut;
	bool defaultClosing;
	int fastAddedAmount[4];
	double defaultProbError;
};

extern Config config;
extern float xScale;
extern float yScale;
extern int listItemHeight;
extern HFONT hFont;

extern HINSTANCE hInst;

void loadConfig(Config& cfg);