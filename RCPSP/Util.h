#pragma once
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

namespace MyRCPSP
{
	namespace Timer
	{
		static clock_t timer1;
		static void  Tic(){ timer1 = clock();}
		static float Toc()
		{
			clock_t timer2 = clock();
			float  delta = (timer2 - timer1) / (float)(CLOCKS_PER_SEC) * 1000.00f;//ms
			return delta;
		}
	}
}