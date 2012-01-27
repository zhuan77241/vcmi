#pragma once

#include "../lib/CFileSystemHandlerFwd.h"
#include "UIFramework/ImageClassesFwd.h"

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
	boost::unordered_map<const GraphicsLocator, weak_ptr<IImage> > images;

	TImagePtr loadImage(const GraphicsLocator & gloc);

public:

	// Loads an image.
	TImagePtr getImage(ResourceIdentifier identifier, bool fromBegin = false);
	
	// Loads a frame/sprite.
	TImagePtr getImage(ResourceIdentifier identifier, size_t frame, size_t group, bool fromBegin = false);
};