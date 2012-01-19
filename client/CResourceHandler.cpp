#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"

TImagePtr CResourceHandler::getImage(ResourceIdentifier identifier, bool fromBegin /*= false*/)
{
	TResourcesMap resources = CGI->filesystemh->getResourcesMap();
	TImagePtr rslt;

	// check if resource is registered
	if(resources.find(identifier) == resources.end())
	{
		tlog2 << "Image with name " << identifier.name << " and type " 
			<< identifier.type << " wasn't found." << std::endl;
		return rslt;
	}

	// get former/origin resource e.g. from lod with fromBegin=true 
	// and get the latest inserted resource with fromBegin=false
	std::list<ResourceLocator> locators = resources.at(identifier);
	ResourceLocator loc;
	if (!fromBegin)
		loc = locators.back();
	else 
		loc = locators.front();

	// get file info of the locator
	CFileInfo locInfo(loc.resourceName);

	// check if resource is already loaded
	if(images.find(loc) == images.end())
	{
		// load it
		return loadImage(identifier, locInfo.getExtension(), fromBegin);
	}

	weak_ptr<IImage> ptr = images.at(loc);
	if (ptr.expired())
	{
		// load it
		return loadImage(identifier, locInfo.getExtension(), fromBegin);
	}

	// already loaded, just return resource
	rslt = shared_ptr<IImage>(ptr);
	return rslt;
}

TImagePtr CResourceHandler::loadImage(ResourceIdentifier identifier, const std::string & fileExt, bool fromBegin /*= false*/)
{
	TMemoryStreamPtr data = CGI->filesystemh->getResource(identifier, fromBegin);
	shared_ptr<IImage> rslt(IImage::createInstance(data, fileExt));
	return rslt;
}