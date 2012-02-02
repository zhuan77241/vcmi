#include "StdInc.h"
#include "CResourceHandler.h"

#include "CGameInfo.h"
#include "../lib/CFileSystemHandler.h"
#include "UIFramework/ImageClasses.h"
#include "UIFramework/AnimationClasses.h"


template <typename IResource, typename Storage>
shared_ptr<IResource> CResourceHandler::getResource(Storage & storage, const ResourceIdentifier & identifier, const GraphicsSelector & sel, bool fromBegin /*= false*/)
{
	TResourcesMap resources = CGI->filesystemh->getResourcesMap();
	shared_ptr<IResource> rslt;

	// check if resource is registered
	if(resources.find(identifier) == resources.end())
	{
		tlog2 << "Resource with name " << identifier.name << " and type "
			<< identifier.type << " wasn't found." << std::endl;
		return rslt;
	}

	// get former/origin resource e.g. from lod with fromBegin=true
	// and get the latest inserted resource with fromBegin=false
	std::list<ResourceLocator> locators = resources.at(identifier);
	ResourceLocator loc;
	if(!fromBegin)
		loc = locators.back();
	else
		loc = locators.front();

	// check if resource is already loaded
	GraphicsLocator gloc(loc.loader, loc.resourceName, sel);

	if(storage.find(gloc) == storage.end())
	{
		// load it
		loadResource(gloc, rslt);
		return rslt;
	}

	weak_ptr<IResource> ptr = storage.at(gloc);
	if(ptr.expired())
	{
		// load it
		loadResource(gloc, rslt);
		return rslt;
	}

	// already loaded, just return resource
	rslt = shared_ptr<IResource>(ptr);
	return rslt;
}

TImagePtr CResourceHandler::getImage(const ResourceIdentifier & identifier, size_t frame, size_t group, bool fromBegin /*= false*/)
{
	return getResource<const IImage, TImageMap>(images, identifier, GraphicsSelector(group, frame), fromBegin);
}

TImagePtr CResourceHandler::getImage(const ResourceIdentifier & identifier, bool fromBegin /*= false*/)
{
	return getResource<const IImage, TImageMap>(images, identifier, GraphicsSelector(), fromBegin);
}

TImagePtr CResourceHandler::getImage(const GraphicsLocator & gloc)
{
	return getResource<const IImage, TImageMap>(images, gloc);
}

TImagePtr CResourceHandler::setImage(IImage * img, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc)
{
	return setResource<IImage, TImageMap>(img, images, newLoc, oldLoc);
}

template <typename IResource, typename Storage>
shared_ptr<const IResource> CResourceHandler::setResource(IResource * res, Storage & storage, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc)
{
	assert(res != NULL);

	Storage::iterator i = storage.find(oldLoc);
	weak_ptr<const IResource> wptr((*i).second);
	shared_ptr<const IResource> sptr;

	if (wptr.use_count() == 1)
	{
		// Remove old locator
		sptr = shared_ptr<const IResource>(wptr);
		storage.erase(i);
	}
	else
	{
		sptr = shared_ptr<const IResource>(res);
	}

	const_cast<IResource *>(sptr.get())->locator = newLoc;
	wptr = weak_ptr<const IResource>(sptr);
	storage[newLoc] = wptr;
	return sptr;
}

template <typename IResource, typename Storage>
shared_ptr<IResource> CResourceHandler::getResource(Storage & storage, const GraphicsLocator & gloc)
{
	shared_ptr<IResource> res;
	if (storage.find(gloc) != storage.end())
	{
		weak_ptr<IResource> ptr = storage.at(gloc);
		if (!ptr.expired())
			res = shared_ptr<IResource>(ptr);
	}
	return res;
}

void CResourceHandler::loadResource(const GraphicsLocator & gloc, TImagePtr & img)
{
	ResourceLocator resLoc(gloc.loader, gloc.resourceName);
	TMemoryStreamPtr data = CGI->filesystemh->getResource(resLoc);
	
	// get file info of the locator
	CFileInfo locInfo(gloc.resourceName);

	TMutableImagePtr mutableImg;
	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile defFile(data);
		mutableImg = IImage::createSpriteFromDEF(&defFile, gloc.sel.frame, gloc.sel.group);
	}
	else
	{
		mutableImg = IImage::createImageFromFile(data, locInfo.getExtension());
	}

	mutableImg->locator = gloc;
	img = mutableImg;

	weak_ptr<const IImage> wPtr(img);
	images[gloc] = wPtr;
}

TAnimationPtr CResourceHandler::getAnimation(const ResourceIdentifier & identifier, bool fromBegin /*= false*/)
{
	return getResource<const IAnimation, TAnimationMap>(animations, identifier, GraphicsSelector(), fromBegin); 
}

TAnimationPtr CResourceHandler::getAnimation(const ResourceIdentifier & identifier, size_t group, bool fromBegin /*= false*/)
{
	return getResource<const IAnimation, TAnimationMap>(animations, identifier, GraphicsSelector(group), fromBegin);
}

void CResourceHandler::loadResource(const GraphicsLocator & gloc, TAnimationPtr & anim)
{
	ResourceLocator resLoc(gloc.loader, gloc.resourceName);
	TMemoryStreamPtr data = CGI->filesystemh->getResource(resLoc);

	// get file info of the locator
	CFileInfo locInfo(gloc.resourceName);

	TMutableAnimationPtr mutableAnim;
	if(boost::iequals(locInfo.getExtension(), ".DEF"))
	{
		CDefFile defFile(data);
		mutableAnim = IAnimation::createAnimation(&defFile, gloc.sel.group);
	}

	mutableAnim->locator = gloc;
	anim = mutableAnim;

	weak_ptr<const IAnimation> wPtr(anim);
	animations[gloc] = wPtr;
}

TAnimationPtr CResourceHandler::getAnimation(const GraphicsLocator & gloc)
{
	return getResource<const IAnimation, TAnimationMap>(animations, gloc);
}

TAnimationPtr CResourceHandler::setAnimation(IAnimation * anim, const GraphicsLocator & newLoc, const GraphicsLocator & oldLoc)
{
	return setResource<IAnimation, TAnimationMap>(anim, animations, newLoc, oldLoc);
}

template <typename IResource, typename Storage>
bool CResourceHandler::isResourceUnique(Storage & storage, const GraphicsLocator & locator)
{
	// Resource has to exist, otherwise you're not allowed to call this function
	Storage::iterator it = storage.find(locator);
	assert(it != storage.end());

	weak_ptr<IResource> wptr = (*it).second;
	assert(wptr.expired() == false);
	
	return wptr.use_count() == 1;
}

bool CResourceHandler::isImageUnique(const GraphicsLocator & locator)
{
	return isResourceUnique<const IImage, TImageMap>(images, locator);
}

bool CResourceHandler::isAnimationUnique(const GraphicsLocator & locator)
{
	return isResourceUnique<const IAnimation, TAnimationMap>(animations, locator);
}

template TImagePtr CResourceHandler::getResource<const IImage, TImageMap>(TImageMap &, const GraphicsLocator &);

template TAnimationPtr CResourceHandler::getResource<const IAnimation, TAnimationMap>(TAnimationMap &, const GraphicsLocator &);

template TImagePtr CResourceHandler::setResource<IImage, TImageMap>(IImage *, TImageMap &, const GraphicsLocator &, const GraphicsLocator &);

template TAnimationPtr CResourceHandler::setResource<IAnimation, TAnimationMap>(IAnimation *, TAnimationMap &, const GraphicsLocator &, const GraphicsLocator &);

template TImagePtr CResourceHandler::getResource<const IImage, TImageMap>(TImageMap &, 
	const ResourceIdentifier &, const GraphicsSelector &, bool);

template TAnimationPtr CResourceHandler::getResource<const IAnimation, TAnimationMap>(TAnimationMap &, 
	const ResourceIdentifier &, const GraphicsSelector &, bool);

template bool CResourceHandler::isResourceUnique<const IImage, TImageMap>(TImageMap &, const GraphicsLocator &);

template bool CResourceHandler::isResourceUnique<const IAnimation, TAnimationMap>(TAnimationMap &, const GraphicsLocator &);