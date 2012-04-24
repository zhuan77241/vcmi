#pragma once

#include <typeinfo>
#include "../../lib/CFileSystemHandlerFwd.h"


/*
 * ImageClassesFwd.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class IImage;
class ITransformational;
class CSDLImage;
class CCompImage;
struct SDL_Surface;
class CDefFile;

/*
 * Enumeration for the glow effect
 */
namespace EGlowAnimationType
{
	enum EGlowAnimationType
	{
		NONE,
		YELLOW,
		BLUE
	};
}

/** Enumeration for the rotation/flip type. May be extended to support more rotations. */
namespace ERotateFlipType
{
	enum ERotateFlipType
	{
		NONE,
		ROTATENONE_FLIPX,
		ROTATENONE_FLIPY
	};
}

/*
 * Interface for VCMI related graphics tasks like player coloring
 */
class ITransformational
{
public:
	inline void printNotImplMsg(const std::string & methodName)
	{
		tlog1 << "Method " << methodName << " in class " << typeid(*this).name() << " not implemented." << std::endl;
	}

	// Change palette to specific player.
	virtual void recolorToPlayer(int player) { printNotImplMsg("recolorToPlayer"); }

	// Sets/Unsets the yellow or blue glow animation effect.
	virtual void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity) { printNotImplMsg("setGlowAnimation"); }

	virtual void setAlpha(ui8 alpha) { printNotImplMsg("setAlpha"); }

	virtual void rotateFlip(ERotateFlipType::ERotateFlipType type) { printNotImplMsg("rotateFlip"); }
};
