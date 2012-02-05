#include "StdInc.h"
#include "AnimationClasses.h"

#include "ImageClasses.h"
#include "../CGameInfo.h"
#include "../CResourceHandler.h"

TMutableAnimationPtr IAnimation::createAnimation(const CDefFile * defFile, size_t group)
{
	static const bool useImageBased = true;

	TMutableAnimationPtr anim;
	if (useImageBased)
	{
		anim = shared_ptr<IAnimation>(new CImageBasedAnimation);
	}
	else
	{
		// TODO: Direct based anim
	}

	if (group == -1)
		anim->load(defFile);
	else
		anim->load(defFile, group);

	return anim;
}

const GraphicsLocator & IAnimation::getLocator() const
{
	return locator;
}

std::map<size_t, size_t> IAnimation::getEntries() const
{
	return entries;
}

si8 IAnimation::getLoadedGroup() const
{
	return loadedGroup;
}

CImageBasedAnimation::CImageBasedAnimation() : IAnimation()
{
}

CImageBasedAnimation::CImageBasedAnimation(const CImageBasedAnimation & other)
{
	*this = other;
}

CImageBasedAnimation & CImageBasedAnimation::operator=(const CImageBasedAnimation & other)
{
	loadedGroup = other.loadedGroup;

	for(std::map<size_t, std::map<size_t, TMutableImagePtr> >::const_iterator i = other.images.begin();
		i != other.images.end(); ++i)
	{
		size_t group = (*i).first;
		std::map<size_t, TMutableImagePtr> frames = (*i).second;
		for(std::map<size_t, TMutableImagePtr>::iterator j = frames.begin(); 
			j != frames.end(); ++j)
		{
			size_t frame = (*j).first;
			shared_ptr<IImage> ptr((*j).second->clone());
			images[group][frame] = ptr;
		}
	}

	return *this;
}

IAnimation * CImageBasedAnimation::clone() const
{
	return new CImageBasedAnimation(*this);
}

void CImageBasedAnimation::load(const CDefFile * defFile)
{
	assert(loadedGroup == NO_GROUP_LOADED);

	images.clear();
	entries = defFile->getEntries();
	loadedGroup = -1;

	for(std::map<size_t, size_t>::iterator group = entries.begin(); group != entries.end(); ++group)
		for(size_t frame = 0; frame < group->second; frame++)
			images[group->first][frame] = IImage::createSpriteFromDEF(defFile, frame, group->first);
}

void CImageBasedAnimation::load(const CDefFile * defFile, size_t group)
{
	assert(loadedGroup == NO_GROUP_LOADED);

	images.clear();
	entries = defFile->getEntries();

	if(vstd::contains(entries, group))
	{
		loadedGroup = group;

		for(size_t frame = 0; frame < entries[group]; frame++)
			images[group][frame] = IImage::createSpriteFromDEF(defFile, frame, group);
	}
}

void CImageBasedAnimation::draw(TImagePtr where, size_t frame, size_t group, int posX, int posY) const
{
	std::map<size_t, std::map<size_t, TMutableImagePtr> >::const_iterator it = images.find(group);
	if (it != images.end())
	{
		std::map<size_t, TMutableImagePtr> frames = (*it).second;
		std::map<size_t, TMutableImagePtr>::const_iterator it2 = frames.find(frame);
		
		if (it2 != frames.end())
		{
				(*it2).second->draw(where, posX, posY);
		}
	}
}

void CImageBasedAnimation::recolorToPlayer(int player)
{
	// recolor all groups
	if(loadedGroup == -1)
	{
		for(size_t group = 0; group < images.size(); ++group)
		{
			for(size_t frame = 0; frame < images[group].size(); ++frame)
			{
				TMutableImagePtr img = images[group][frame];
				IGraphicsTasks * ptr = dynamic_cast<IGraphicsTasks *>(img.get());
				ptr->recolorToPlayer(player);
			}
		}
	}
	else
	{
		// recolor loaded group
		for(size_t frame = 0; frame < images[loadedGroup].size(); ++frame)
		{
			TMutableImagePtr img =  images[loadedGroup][frame];
			IGraphicsTasks * ptr = dynamic_cast<IGraphicsTasks *>(img.get());
			ptr->recolorToPlayer(player);
		}
	}
}

void CImageBasedAnimation::recolorToPlayerViaSelector(const GraphicsSelector & selector)
{
	recolorToPlayer(selector.playerColor);
}

TAnimationPtr CImageBasedAnimation::recolorToPlayer(int player) const
{
	GraphicsLocator newLoc(locator);
	newLoc.sel.playerColor = player;

	return CCS->resh->getTransformedAnimation(this, boost::bind(&CImageBasedAnimation::recolorToPlayerViaSelector, 
		const_cast<CImageBasedAnimation *>(this), newLoc.sel), newLoc);
}

TAnimationPtr CImageBasedAnimation::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha) const
{
	assert(0);
	TAnimationPtr ptr;
	return ptr;
}

TAnimationPtr CImageBasedAnimation::rotate(EImageRotation::EImageRotation rotation) const
{
	assert(0);
	TAnimationPtr ptr;
	return ptr;
}

CAnimation::CAnimation(TAnimationPtr animation) : currentGroup(0), currentFrame(0)
{
	anim = animation;
	setGroup(0);
}

CAnimation::CAnimation(const ResourceIdentifier & identifier) : currentGroup(0), currentFrame(0)
{
	anim = CCS->resh->getAnimation(identifier);
	setGroup(0);
}

CAnimation::CAnimation(const ResourceIdentifier & identifier, size_t group) : currentGroup(group), currentFrame(0)
{
	anim = CCS->resh->getAnimation(identifier, group);
	setGroup(group);
}

void CAnimation::setGroup(size_t group, bool repeat /*= false*/)
{
	this->repeat = repeat;
	std::map<size_t, size_t> entries = anim->getEntries();

	// check if the group is loaded
	if (anim->getLoadedGroup() != group && anim->getLoadedGroup() != IAnimation::ALL_GROUPS_LOADED)
	{
		// TODO: output resource name, resource source
		tlog2 << "Group Nr. " << group << " hasn't been loaded." << std::endl;
		return;
	}

	// check if the group nr is defined in the animation format
	if (vstd::contains(entries, group))
	{
		frameCount = entries[group];
		currentTime = 0.0;
		currentGroup = group;
	}
	else
	{
		// if all groups have been loaded and the specified group can't be accessed then
		// throw no exception
		// TODO: output warning/debugger warning?
	}
}

void CAnimation::update(double elapsedTime)
{
	// TODO: get frames per second default setting, affects animation playing speed
	// standing anim group shouldn't be faster, etc... -> setGroup should set framesSecond (?)
	static const double framesSecond = 1 / 6.;

	currentFrame = static_cast<size_t>(currentTime / framesSecond);
	if (currentFrame >= frameCount)
	{
		if (repeat == true)
		{
			currentTime -= static_cast<int>(currentTime / (frameCount * framesSecond)) * (frameCount * framesSecond);
			currentFrame = static_cast<size_t>(currentTime / framesSecond);
		}
		else
			currentFrame = frameCount - 1;
	}

	currentTime += elapsedTime;
}

void CAnimation::draw(TImagePtr where, int posX, int posY)
{
	anim->draw(where, currentFrame, currentGroup, posX, posY);
}

void CAnimation::recolorToPlayer(int player)
{
	anim = anim->recolorToPlayer(player);
}

void CAnimation::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{

}

void CAnimation::rotate(EImageRotation::EImageRotation rotation)
{

}