#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"
#include "UIFramework/AnimationClasses.h"


IImage * CResourceHandler::getImage(const ResourceIdentifier & identifier, size_t frame, size_t group)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return loadImage(loc, frame, group);
}

IImage * CResourceHandler::getImage(const ResourceIdentifier & identifier)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return loadImage(loc);
}

CSDLImage * CResourceHandler::getSurface(const ResourceIdentifier & identifier)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return dynamic_cast<CSDLImage *>(loadImage(loc, -1, -1, true));
}

CSDLImage * CResourceHandler::getSurface(const ResourceIdentifier & identifier, size_t frame, size_t group)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return dynamic_cast<CSDLImage *>(loadImage(loc, frame, group, true));
}

IImage * CResourceHandler::loadImage(const ResourceLocator & loc, size_t frame /*= -1*/, size_t group /*= -1*/, bool useSDL /*= false*/)
{
	// Load data stream
	CMemoryStream * data = CGI->filesystemh->getResource(loc);
	
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

		// always use CCompImage when loading from DEF file
		// we keep the possibility to load via SDL image
		static const bool useComp = true;

		if (useComp)
			return new CCompImage(&defFile, frame, group);

		else
			return new CSDLImage(&defFile, frame, group);
	}
	else
	{
		return new CSDLImage(data, locInfo.getExtension());
	}
}

CAnimationHolder * CResourceHandler::getAnimation(const ResourceIdentifier & identifier)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return new CAnimationHolder(loadAnimation(loc));
}

CAnimationHolder * CResourceHandler::getAnimation(const ResourceIdentifier & identifier, size_t group)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier);
	return new CAnimationHolder(loadAnimation(loc, group));
}

IAnimation * CResourceHandler::loadAnimation(const ResourceLocator & loc, size_t group /*= -1*/)
{
	CMemoryStream * data = CGI->filesystemh->getResource(loc);

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
