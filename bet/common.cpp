#include "framework.h"
#include "common.h"

#include <fstream>

using namespace std;

Config config;
float xScale = 1;
float yScale = 1;
int listItemHeight;
HFONT hFont;
HFONT hBoldFont;
UINT DRAGLISTMSG;