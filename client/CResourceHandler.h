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
	IImage * loadImage(const ResourceLocator & loc, boost::optional<size_t> frame = boost::none, boost::optional<size_t> group = boost::none, bool useSDL = false);
	IAnimation * loadAnimation(const ResourceLocator & loc, boost::optional<size_t> group = boost::none);

public:

	// Loads an image.
	IImage * getImage(const std::string & name);
	
	// Loads a frame/sprite.
	IImage * getImage(const std::string & name, size_t frame, size_t group);

	CSDLImage * getSurface(const std::string & name);

	CSDLImage * getSurface(const std::string & name, size_t frame, size_t group);

	/**
	 * Gets and loads an animation.
	 *
	 * @param name The resource name of the animation(no path or file extensions, only name)
	 * @param group If you wan't to load one animation group only then pass the number. Leave empty to load all groups.
	 * @return An object of type CAnimationHolder which provides functionality to show a time-based animation.
	 */
	CAnimationHolder * getAnimation(const std::string & name, boost::optional<size_t> group = boost::none);
};
