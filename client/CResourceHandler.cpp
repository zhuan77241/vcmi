#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"
#include "UIFramework/AnimationClasses.h"


IImage * CResourceHandler::getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin /*= false*/)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier, fromBegin);
	return loadImage(loc, frame, group);
}

IImage * CResourceHandler::getImage(const ResourceIdentifier & identifier, bool fromBegin /*= false*/)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier, fromBegin);
	return loadImage(loc);
}

IImage * CResourceHandler::createImageFromFile(CMemoryStream * data, const std::string & imageType)
{
	// always use SDL when loading image from file 
	return new CSDLImage(data, imageType);
}

IImage * CResourceHandler::createSpriteFromDEF(const CDefFile * defFile, size_t frame, size_t group)
{
	// always use CCompImage when loading from DEF file
	// we keep the possibility to load via SDL image
	static const bool useComp = true;

	if (useComp)
		return new CCompImage(defFile, frame, group);

	else
		return new CSDLImage(defFile, frame, group);
}

IImage * CResourceHandler::loadImage(const ResourceLocator & loc, size_t frame /*= -1*/, size_t group /*= -1*/)
{
	CMemoryStream * data = CGI->filesystemh->getResource(loc);
	
	// get file info of the locator
	CFileInfo locInfo(loc.resourceName);

	IImage * img = NULL;
	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile defFile(data);
		return createSpriteFromDEF(&defFile, frame, group);
	}
	else
	{
		return createImageFromFile(data, locInfo.getExtension());
	}
}

IAnimation * CResourceHandler::getAnimation(const ResourceIdentifier & identifier, bool fromBegin /*= false*/)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier, fromBegin);
	return loadAnimation(loc);
}

IAnimation * CResourceHandler::getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin /*= false*/)
{
	ResourceLocator loc = CGI->filesystemh->getResourceLocator(identifier, fromBegin);
	return loadAnimation(loc, group);
}

IAnimation * CResourceHandler::createAnimation(const CDefFile * defFile, size_t group /*= -1*/)
{
	// use always image based animations for the moment;
	static const bool useImageBased = true;

	if (useImageBased)
	{
		return new CImageBasedAnimation(defFile, group);
	}
	else
	{
		return new CDefAnimation(defFile);
	}
}

IAnimation * CResourceHandler::loadAnimation(const ResourceLocator & loc, size_t group /*= -1*/)
{
	CMemoryStream * data = CGI->filesystemh->getResource(loc);

	// get file info of the locator
	CFileInfo locInfo(loc.resourceName);

	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile * defFile = new CDefFile(data);
		return createAnimation(defFile, group);
	}

	return NULL;
}