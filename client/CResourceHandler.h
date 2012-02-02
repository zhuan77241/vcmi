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

typedef boost::unordered_map<const GraphicsLocator, weak_ptr<const IImage> > TImageMap;
typedef boost::unordered_map<const GraphicsLocator, weak_ptr<const IAnimation> > TAnimationMap;

class CResourceHandler
{
	TImageMap images;
	TAnimationMap animations;

	void loadResource(const GraphicsLocator & gloc, TImagePtr & img);
	void loadResource(const GraphicsLocator & gloc, TAnimationPtr & anim);

	// Gets a resource by a exact location.
	template <typename IResource, typename Storage>
	shared_ptr<IResource> getResource(Storage & storage, const GraphicsLocator & gloc);

	// Gets a resource by an identifier.
	template <typename IResource, typename Storage>
	shared_ptr<IResource> getResource(Storage &storage, const ResourceIdentifier & identifier, const GraphicsSelector & sel, bool fromBegin = false);

	template <typename IResource, typename Storage>
	shared_ptr<const IResource> setResource(IResource * res, Storage & storage, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc);

	TImagePtr getImage(const GraphicsLocator & gloc);
	TImagePtr setImage(IImage * img, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc);

	TAnimationPtr getAnimation(const GraphicsLocator & gloc);
	TAnimationPtr setAnimation(IAnimation * anim, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc);

	template <typename IResource, typename Storage>
	bool isResourceUnique(Storage & storage, const GraphicsLocator & locator);

	// Checks if the image used is unique. Deletes the weak ptr 
	bool isImageUnique(const GraphicsLocator & locator);
	bool isAnimationUnique(const GraphicsLocator & locator);
public:

	// Loads an image.
	TImagePtr getImage(const ResourceIdentifier & identifier, bool fromBegin = false);
	
	// Loads a frame/sprite.
	TImagePtr getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin = false);

	// Loads complete animation(all groups).
	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Loads a group of an animation.
	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin = false);

	// This is needed because get/setImage/Animation via locator directly from "outside"
	friend class CSDLImage;
	friend class CCompImage;
	friend class CImageBasedAnimation;
};