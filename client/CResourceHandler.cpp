#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"
#include "UIFramework/AnimationClasses.h"


IImage * CResourceHandler::getImage(const std::string & name, size_t frame, size_t group)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(ResourceIdentifier(name, EResType::ANIMATION));
	return loadImage(loc, boost::make_optional(frame), boost::make_optional(group));
}

IImage * CResourceHandler::getImage(const std::string & name)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(ResourceIdentifier(name, EResType::GRAPHICS));
	return loadImage(loc);
}

CSDLImage * CResourceHandler::getSurface(const std::string & name)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(ResourceIdentifier(name, EResType::GRAPHICS));
	return dynamic_cast<CSDLImage *>(loadImage(loc, boost::none, boost::none, true));
}

CSDLImage * CResourceHandler::getSurface(const std::string & name, size_t frame, size_t group)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(ResourceIdentifier(name, EResType::ANIMATION));
	return dynamic_cast<CSDLImage *>(loadImage(loc, boost::make_optional(frame), boost::make_optional(group), true));
}

IImage * CResourceHandler::loadImage(const ResourceLocator & loc, boost::optional<size_t> frame /*= boost::none*/, boost::optional<size_t> group /*= boost::none*/, bool useSDL /*= false*/)
{
	// Load data stream
	CMemoryStream * data = CGI->filesystemh->getResource(loc);
	
	// Return NULL if the resource couldn't been loaded.
	if (!data)
		return NULL;

	// Get file info
	CFileInfo locInfo(loc.resourceName);

	// If the image should be used for image editing, then load it as SDL
	if(useSDL)
		return new CSDLImage(data, locInfo.getExtension());

	// Requested image(sprite) resides in a .DEF file
	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		// Construct the .DEF animation format
		CDefFile defFile(data);

		assert(frame && group);

		// always use CCompImage when loading from DEF file
		// we keep the possibility to load via SDL image
		static const bool useComp = true;

		if (useComp)
			return new CCompImage(&defFile, *frame, *group);

		else
			return new CSDLImage(&defFile, *frame, *group);
	}
	else
	{
		return new CSDLImage(data, locInfo.getExtension());
	}
}

CAnimationHolder * CResourceHandler::getAnimation(const std::string & name, boost::optional<size_t> group /*= boost::none*/)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(ResourceIdentifier(name, EResType::ANIMATION));
	return new CAnimationHolder(loadAnimation(loc, group));
}

IAnimation * CResourceHandler::loadAnimation(const ResourceLocator & loc, boost::optional<size_t> group /*= boost::none*/)
{
	CMemoryStream * data = CGI->filesystemh->getResource(loc);

	// Return NULL if the resource couldn't been loaded.
	if (!data)
		return NULL;

	// get file info of the locator
	CFileInfo locInfo(loc.resourceName);

	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile * defFile = new CDefFile(data);

		// always use image based animations as cdef animation is deprecated
		static const bool useImageBased = true;

		if (useImageBased)
		{
			//TODO add support for VCMI anim format

			// use ccomp animation for all def based animations
			return new CCompAnimation(defFile, group);
		}
		else
		{
			return new CDefAnimation(defFile);
		}
	}

	return NULL;
}
