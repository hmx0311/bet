#include "framework.h"
#include "common.h"

#include <iostream>
#include <fstream>

using namespace std;

#define DEFAULT_CONFIG {1,\
						false,\
						1000,5000,10000,50000,\
						0}

Config config;
float xScale = 1;
float yScale = 1;
int listItemHeight;
HFONT hFont;

void loadConfig(Config& cfg)
{
	fstream file(CONFIG_FILE_NAME, ios::_Nocreate | ios::in | ios::binary);
	if (file.good())
	{
		if (file.read((char*)&cfg, sizeof(Config)).good() &&
			cfg.cut > 0 && cfg.cut <= 1 &&
			cfg.fastAddedAmount[0] > 0 && cfg.fastAddedAmount[0] < 100000 &&
			cfg.fastAddedAmount[1] > 0 && cfg.fastAddedAmount[1] < 100000 &&
			cfg.fastAddedAmount[2] > 0 && cfg.fastAddedAmount[2] < 100000 &&
			cfg.fastAddedAmount[3] > 0 && cfg.fastAddedAmount[3] < 100000 &&
			cfg.defaultProbError >= 0 && cfg.defaultProbError < 1)
		{
			file.close();
			return;
		}
		file.close();
	}
	cfg = DEFAULT_CONFIG;
	file.open(CONFIG_FILE_NAME, ios::out | ios::binary);
	if (file.good())
	{
		file.write((char*)&cfg, sizeof(Config));
		file.close();
	}
}