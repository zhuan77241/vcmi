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

	void loadResource(const GraphicsLocator & gloc, TImagePtr & img, bool unshared = false);
	void loadResource(const GraphicsLocator & gloc, TAnimationPtr & anim, bool unshared = false);

	// Gets a resource by a exact location.
	template <typename IResource, typename Storage>
	shared_ptr<IResource> getResource(Storage & storage, const GraphicsLocator & gloc);

	// Gets a resource by an identifier.
	template <typename IResource, typename Storage>
	shared_ptr<IResource> getResource(Storage &storage, const ResourceIdentifier & identifier, const GraphicsSelector & sel, bool fromBegin = false, bool unshared = false);

	// Sets a resource to a new location.
	template <typename IResource, typename Storage>
	shared_ptr<const IResource> setResource(IResource * res, Storage & storage, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc);

	template <typename BaseResource, typename Storage, typename Resource>
	shared_ptr<const BaseResource> getTransformedResource(Storage & storage, const Resource * resource, 
		boost::function<void (GraphicsSelector sel)> func, const GraphicsLocator & newLoc);

	// Applies the specified transformation to the specified resource and updates the locator object.
	template <typename Resource>
	TImagePtr getTransformedImage(const Resource * resource, boost::function<void (GraphicsSelector sel)> func, const GraphicsLocator & newLoc);

	// Applies the specified transformation to the specified resource and updates the locator object.
	template <typename Resource>
	TAnimationPtr getTransformedAnimation(const Resource * resource, boost::function<void (GraphicsSelector sel)> func, const GraphicsLocator & newLoc);

	// Checks if the resource is used only once.
	template <typename IResource, typename Storage>
	bool isResourceUnique(Storage & storage, const GraphicsLocator & locator);

	TMutableImagePtr createImageFromFile(TMemoryStreamPtr data, const std::string & imageType, const GraphicsLocator & locator = GraphicsLocator());

	TMutableImagePtr createSpriteFromDEF(const CDefFile * defFile, size_t frame, size_t group, const GraphicsLocator & locator = GraphicsLocator());

	TMutableAnimationPtr createAnimation(const CDefFile * defFile, size_t group = -1, const GraphicsLocator & locator = GraphicsLocator());
public:

	// Loads an image.
	TImagePtr getImage(const ResourceIdentifier & identifier, bool fromBegin = false);
	
	// Loads a frame/sprite.
	TImagePtr getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin = false);
	
	// Loads an unshared image.
	shared_ptr<CSDLImage> getUnsharedImage(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Loads complete animation(all groups).
	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Loads a group of an animation.
	TAnimationPtr getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin = false);


	// Those classes need access to the following methods:
	// createAnimation, createSpriteFromDEF(for animations), getTransformedAnimation, getTransformedImage
	friend class CSDLImage;
	friend class CCompImage;
	friend class CImageBasedAnimation;
};