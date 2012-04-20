#pragma once

#include "../lib/CFileSystemHandlerFwd.h"

class IAnimation;
class IImage;
class CDefFile;
class CAnimationHolder;
class CSDLImage;

/*
 * CResourceHandler.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CResourceHandler
{
	IImage * loadImage(const ResourceLocator & loc, size_t frame = -1, size_t group = -1, bool useSDL = false);
	IAnimation * loadAnimation(const ResourceLocator & loc, size_t group = -1);

public:

	// Loads an image.
	IImage * getImage(const ResourceIdentifier & identifier);
	
	// Loads a frame/sprite.
	IImage * getImage(const ResourceIdentifier & identifier, size_t frame, size_t group);

	CSDLImage * getSurface(const ResourceIdentifier & identifier);

	CSDLImage * getSurface(const ResourceIdentifier & identifier, size_t frame, size_t group);

	// Loads complete animation(all groups).
	CAnimationHolder * getAnimation(const ResourceIdentifier & identifier);

	// Loads a group of an animation.
	CAnimationHolder * getAnimation(const ResourceIdentifier & identifier, size_t group);
};
