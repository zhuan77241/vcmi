#include "StdInc.h"
#include "AnimationClasses.h"

#include "ImageClasses.h"
#include "../CGameInfo.h"
#include "../CResourceHandler.h"

TAnimationPtr IAnimation::createAnimation(const CDefFile * defFile, size_t group)
{
	static const bool useImageBased = true;

	TAnimationPtr anim;
	if (useImageBased)
	{
		anim = shared_ptr<IAnimation>(new ImageBasedAnimation);
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

std::map<size_t, size_t> IAnimation::getEntries() const
{
	return entries;
}

void ImageBasedAnimation::load(const CDefFile * defFile)
{
	images.clear();
	entries = defFile->getEntries();

	for(std::map<size_t, size_t>::iterator group = entries.begin(); group != entries.end(); ++group)
		for(size_t frame = 0; frame < group->second; frame++)
			images[group->first][frame] = IImage::createSpriteFromDEF(defFile, frame, group->first);
}

void ImageBasedAnimation::load(const CDefFile * defFile, size_t group)
{
	images.clear();
	entries = defFile->getEntries();

	if(vstd::contains(entries, group))
		for(size_t frame = 0; frame < entries[group]; frame++)
			images[group][frame] = IImage::createSpriteFromDEF(defFile, frame, group);
}

void ImageBasedAnimation::draw(TImagePtr where, size_t frame, size_t group, int posX, int posY)
{
	if(vstd::contains(images, group))
	{
		if(vstd::contains(images[group], frame))
			images[group][frame]->draw(where, posX, posY);
	}
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

	if (vstd::contains(entries, group))
	{
		frameCount = entries[group];
		currentTime = 0.0;
	}
	else
	{
		// TODO: Group not available, throw exception?
	}
}

void CAnimation::update(double elapsedTime)
{
	// TODO: get frames per second default setting, affects animation playing speed
	static const double framesSecond = 1 / 24.;


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