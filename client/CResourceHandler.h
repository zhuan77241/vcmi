#pragma once

#include "../lib/CFileSystemHandlerFwd.h"

class IAnimation;
class IImage;
class CDefFile;

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
	IImage * loadImage(const ResourceLocator & loc, size_t frame = -1, size_t group = -1);
	IAnimation * loadAnimation(const ResourceLocator & loc, size_t group = -1);

	IImage * createImageFromFile(CMemoryStream * data, const std::string & imageType);

	IImage * createSpriteFromDEF(const CDefFile * defFile, size_t frame, size_t group);

	IAnimation * createAnimation(const CDefFile * defFile, size_t group = -1);
public:

	// Loads an image.
	IImage * getImage(const ResourceIdentifier & identifier, bool fromBegin = false);
	
	// Loads a frame/sprite.
	IImage * getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin = false);

	// Loads complete animation(all groups).
	IAnimation * getAnimation(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Loads a group of an animation.
	IAnimation * getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin = false);


	// Needs access to: createSpriteFromDEF
	friend class CImageBasedAnimation;
};