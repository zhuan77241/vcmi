#pragma once

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
class IImageTasks;
class SDLImage;
class CompImage;
struct SDL_Surface;
class CDefFile;

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

struct GraphicsLocator : public ResourceLocator
{
	size_t frame, group;

	// TODO: add imageTask queries, transformations,...

	GraphicsLocator() : ResourceLocator(), frame(0), group(0) { };
	GraphicsLocator(IResourceLoader * Loader, const std::string & ResourceName) 
		: ResourceLocator(Loader, ResourceName), frame(0), group(0) { };

	inline bool operator==(GraphicsLocator const & other) const
	{
		return loader == other.loader && resourceName == other.resourceName
			&& frame == other.frame && group == other.group;
	}

	inline friend std::size_t hash_value(GraphicsLocator const & p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.loader);
		boost::hash_combine(seed, p.resourceName);
		boost::hash_combine(seed, p.frame);
		boost::hash_combine(seed, p.group);

		return seed;
	}
};