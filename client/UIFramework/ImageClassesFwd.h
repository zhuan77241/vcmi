#pragma once

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
class IImageTasks;
class SDLImage;

typedef shared_ptr<IImage> TImagePtr;


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

namespace EImageRotation
{
	enum EImageRotation
	{
		Flip180X,
		Flip180Y
	};
}