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

namespace EImageRotation
{
	enum EImageRotation
	{
		Flip180X,
		Flip180Y
	};
}

struct GraphicsSelector
{
	si8 frame, group;

	// TODO: add imageTask queries, transformations,...
	si8 playerColor;

	GraphicsSelector(si8 Group = -1, si8 Frame = 0) : frame(Frame), group(Group), playerColor(-1) { };

	inline bool operator==(GraphicsSelector const & other) const
	{
		return frame == other.frame && group == other.group && playerColor == other.playerColor;
	}

	inline friend std::size_t hash_value(GraphicsSelector const & p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.frame);
		boost::hash_combine(seed, p.group);
		boost::hash_combine(seed, p.playerColor);

		return seed;
	}
};

struct GraphicsLocator : public ResourceLocator
{
	GraphicsSelector sel;

	GraphicsLocator() : ResourceLocator() { };
	GraphicsLocator(IResourceLoader * Loader, const std::string & ResourceName, const GraphicsSelector & Sel) 
		: ResourceLocator(Loader, ResourceName), sel(Sel) { };

	inline bool operator==(GraphicsLocator const & other) const
	{
		return loader == other.loader && resourceName == other.resourceName
			&& sel == other.sel;
	}

	inline friend std::size_t hash_value(GraphicsLocator const & p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.loader);
		boost::hash_combine(seed, p.resourceName);
		boost::hash_combine(seed, p.sel);

		return seed;
	}
};

/*
 * Interface for VCMI related graphics tasks like player coloring
 */
class ITransformational
{
public:
	// Change palette to specific player.
	virtual void recolorToPlayer(int player) =0;

	// Sets/Unsets the yellow or blue glow animation effect.
	virtual void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity) =0;

	virtual void setAlpha(ui8 alpha) =0;

	virtual void flipHorizontal(bool flipped) =0;
};
