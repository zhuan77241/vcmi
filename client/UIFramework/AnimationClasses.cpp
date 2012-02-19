#include "StdInc.h"
#include "AnimationClasses.h"

#include "ImageClasses.h"
#include "../CGameInfo.h"
#include "../CResourceHandler.h"

std::map<size_t, size_t> IAnimation::getEntries() const
{
	return entries;
}

si8 IAnimation::getLoadedGroup() const
{
	return loadedGroup;
}

CImageBasedAnimation::CImageBasedAnimation(const CDefFile * defFile, size_t group /*= -1*/)
{
	images.clear();
	entries = defFile->getEntries();
	loadedGroup = group;
	
	if (group == -1)
	{
		for(std::map<size_t, size_t>::iterator group = entries.begin(); group != entries.end(); ++group)
			for(size_t frame = 0; frame < group->second; frame++)
				images[group->first][frame] = CCS->resh->createSpriteFromDEF(defFile, frame, group->first);
	}
	else
	{
		if(vstd::contains(entries, group))
		{
			for(size_t frame = 0; frame < entries[group]; frame++)
				images[group][frame] = CCS->resh->createSpriteFromDEF(defFile, frame, group);
		}
	}
}

CImageBasedAnimation::CImageBasedAnimation(const CImageBasedAnimation & other)
{
	*this = other;
}

CImageBasedAnimation & CImageBasedAnimation::operator=(const CImageBasedAnimation & other)
{
	for(group_itc it = other.images.begin(); it != other.images.end(); ++it)
	{
		size_t group = it->first;
		for(frame_itc it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			size_t frame = it2->first;
			IImage * img = it2->second->clone();
			images[group][frame] = img;
		}
	}

	return *this;
}

IAnimation * CImageBasedAnimation::clone() const
{
	return new CImageBasedAnimation(*this);
}

CImageBasedAnimation::~CImageBasedAnimation()
{
	forEach([](IImage * img) { 
		delete img;
	});
}

void CImageBasedAnimation::forEach(std::function<void(IImage *)> func)
{
	// recolor all groups
	if(loadedGroup == -1)
	{
		for(group_it it = images.begin(); it != images.end(); ++it)
		{
			for(frame_it it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			{
				IImage * img = it2->second;
				func(img);
			}
		}
	}
	else
	{
		// recolor loaded group
		group_it it = images.find(loadedGroup);
		assert(it != images.end());

		for(frame_it it2 = it->second.begin(); it2 != it->second.end(); ++it2)
		{
			IImage * img = it2->second;
			func(img);
		}
	}
}

void CImageBasedAnimation::draw(size_t frame, size_t group) const
{
	std::map<size_t, std::map<size_t, IImage *> >::const_iterator it = images.find(group);
	if(it != images.end())
	{
		std::map<size_t, IImage *> frames = it->second;
		std::map<size_t, IImage *>::const_iterator it2 = frames.find(frame);
		
		if(it2 != frames.end())
		{
			it2->second->draw();
		}
	}
}

void CImageBasedAnimation::recolorToPlayer(int player)
{
	forEach([player](IImage * img) { 
		img->recolorToPlayer(player);
	});
}

void CImageBasedAnimation::setAlpha(ui8 alpha)
{
	assert(0);
}

void CImageBasedAnimation::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{
	assert(0);
}

CDEFAnimation::CDEFAnimation(const CDefFile * defFile)
	: def(defFile)
{
}

void CDEFAnimation::draw() const
{
	/*switch(dest->format->BytesPerPixel)
	{
	case 2: return drawT<2>(dest, x, y,  destRect);
	case 3: return drawT<3>(dest, x, y,  destRect);
	case 4: return drawT<4>(dest, x, y,  destRect);
	default:
		tlog1 << (int)dest->format->BitsPerPixel << " bpp is not supported!!!\n";
		return -1;
	}*/
}

CAnimationHolder::CAnimationHolder(IAnimation * animation) : anim(animation), currentGroup(0), currentFrame(0)
{
	setGroup(0);
}

CAnimationHolder::CAnimationHolder(const ResourceIdentifier & identifier) : currentGroup(0), currentFrame(0)
{
	anim = CCS->resh->getAnimation(identifier);
	setGroup(0);
}

CAnimationHolder::CAnimationHolder(const ResourceIdentifier & identifier, size_t group, bool repeat /*= false*/) : currentGroup(group), currentFrame(0)
{
	anim = CCS->resh->getAnimation(identifier, group);
	setGroup(group, repeat);
}

CAnimationHolder::~CAnimationHolder()
{
	delete anim;
}

void CAnimationHolder::setGroup(size_t group, bool repeat /*= false*/)
{
	this->repeat = repeat;
	std::map<size_t, size_t> entries = anim->getEntries();

	// check if the group is loaded
	if (anim->getLoadedGroup() != group && anim->getLoadedGroup() != -1)
	{
		tlog2 << "Group Nr. " << group << " couldn't be loaded." << std::endl;
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
		tlog3 << "Group Nr. " << group << " isn't available." << std::endl;
	}
}

void CAnimationHolder::update(double elapsedTime)
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

void CAnimationHolder::draw()
{
	anim->draw(currentFrame, currentGroup);
}

void CAnimationHolder::recolorToPlayer(int player)
{
	anim->recolorToPlayer(player);
}

void CAnimationHolder::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{

}
