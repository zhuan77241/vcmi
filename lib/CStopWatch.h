#pragma once

#include <SDL_timer.h>

/*
 * CStopWatch.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CStopWatch
{
	si64 start, last, mem;

public:
	CStopWatch() : start(SDL_GetTicks())
	{
		last = SDL_GetTicks();
		mem = 0;
	}

	//get diff in milliseconds
	si64 getDiff() 
	{
		si64 ret = SDL_GetTicks() - last;
		last = SDL_GetTicks();
		return ret;
	}

	void update()
	{
		last = SDL_GetTicks();
	}

	void remember()
	{
		mem = SDL_GetTicks();
	}

	si64 memDif()
	{
		return SDL_GetTicks() - mem;
	}
};
