#pragma once

#define _STR(x) #x
#define STR(x) _STR(x)

#define MAX_BET_AMOUNT_LEN 7
#define MAX_INITIAL_AMOUNT_LEN 13

#define CONFIG_FILE_NAME _T("bet.cfg")
#define DEFAULT_CONFIG {1,\
						false,\
						false,\
						1000,5000,10000,50000,\
						0}

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

constexpr long long maxNumLen(int len)
{
	long long num = 1;
	for (int i = 0; i < len; i++)
	{
		num *= 10;
	}
	return num - 1;
}