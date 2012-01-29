#pragma once

#include "../lib/CFileSystemHandlerFwd.h"
#include "UIFramework/ImageClassesFwd.h"
#include "UIFramework/AnimationClassesFwd.h"

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
	boost::unordered_map<const GraphicsLocator, weak_ptr<IAnimation> > animations;

	TImagePtr loadImage(const GraphicsLocator & gloc);
	TAnimationPtr loadAnimation(const GraphicsLocator & gloc);

public:

	// Loads an image.
	TImagePtr getImage(const ResourceIdentifier & identifier, bool fromBegin = false);
	
	// Loads a frame/sprite.
	TImagePtr getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin = false);

	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, bool fromBegin = false);

	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin = false);
};