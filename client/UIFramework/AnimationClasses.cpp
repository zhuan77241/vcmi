#include "StdInc.h"
#include "AnimationClasses.h"

#include "ImageClasses.h"
#include "SDL_Extensions.h"
#include "../CGameInfo.h"
#include "../../lib/CFileSystemHandler.h"

IAnimation::IAnimation() : loadedGroup(0)
{

}

std::map<size_t, size_t> IAnimation::getEntries() const
{
	return entries;
}

si8 IAnimation::getLoadedGroup() const
{
	return loadedGroup;
}

void IAnimation::setPosition(const Point & pos)
{
	this->pos = pos;
}

Point IAnimation::getPosition() const
{
	return pos;
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

	IAnimation::operator=(other);
	return *this;
}

CImageBasedAnimation::~CImageBasedAnimation()
{
	forEach([](IImage * img) { 
		delete img;
	});
}

template<typename anim>
void CImageBasedAnimation::constructImageBasedAnimation(const CDefFile * defFile, size_t group /*= -1*/)
{
	entries = defFile->getEntries();
	loadedGroup = group;

	if (group == -1)
	{
		for(std::map<size_t, size_t>::iterator group = entries.begin(); group != entries.end(); ++group)
			for(size_t frame = 0; frame < group->second; frame++)
				images[group->first][frame] = new anim(defFile, frame, group->first);
	}
	else
	{
		if(vstd::contains(entries, group))
		{
			for(size_t frame = 0; frame < entries[group]; frame++)
				images[group][frame] = new anim(defFile, frame, group);
		}
	}

	delete defFile;
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
	group_itc it = images.find(group);
	if(it != images.end())
	{
		frame_itc it2 = it->second.find(frame);
		
		if(it2 != it->second.end())
		{
			IImage * img = it2->second;
			img->setPosition(pos);
			applyTransformations(img);
			img->draw();
		}
	}
}

void CImageBasedAnimation::recolorToPlayer(int player)
{
	forEach([player](IImage * img) { 
		img->recolorToPlayer(player);
	});
}

CCompAnimation::CCompAnimation(const CDefFile * defFile, size_t group /*= -1*/) : glowType(EGlowAnimationType::NONE),
		glowIntensity(0), alpha(255), rotateFlipType(ERotateFlipType::NONE)
{
	constructImageBasedAnimation<CCompImage>(defFile, group);
}

IAnimation * CCompAnimation::clone() const
{
	return new CCompAnimation(*this);
}

void CCompAnimation::applyTransformations(IImage * img) const
{
	img->rotateFlip(rotateFlipType);
	img->setGlowAnimation(glowType, glowIntensity);
	img->setAlpha(alpha);
}

void CCompAnimation::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity)
{
	this->glowIntensity = intensity;
	this->glowType = glowType;
}

void CCompAnimation::setAlpha(ui8 alpha)
{
	this->alpha = alpha;
}

void CCompAnimation::rotateFlip(ERotateFlipType::ERotateFlipType type)
{
	this->rotateFlipType = type;
}

CSDLAnimation::CSDLAnimation(const CDefFile * defFile, size_t group /*= -1*/)
{
	constructImageBasedAnimation<CSDLImage>(defFile, group);
}

IAnimation * CSDLAnimation::clone() const
{
	return new CSDLAnimation(*this);
}

void CSDLAnimation::rotateFlip(ERotateFlipType::ERotateFlipType type)
{
	forEach([type](IImage * img) {
		img->rotateFlip(type);
	});
}

CDefAnimation::CDefAnimation(const CDefFile * defFile)
: def(defFile), playerColor(-1), glowType(EGlowAnimationType::NONE), glowIntensity(0), rotateFlipType(ERotateFlipType::NONE)
{
	entries = defFile->getEntries();
	loadedGroup = -1;
}

CDefAnimation::~CDefAnimation()
{
	delete def;
}

IAnimation * CDefAnimation::clone() const
{
	return new CDefAnimation(*this);
}

void CDefAnimation::draw(size_t frame, size_t group) const
{
	switch(screen->format->BytesPerPixel)
	{
	case 2: drawT<2>(frame, group, screen, pos.x, pos.y); break;
	case 3: drawT<3>(frame, group, screen, pos.x, pos.y); break;
	case 4: drawT<4>(frame, group, screen, pos.x, pos.y); break;
	}
}

template<int bpp>
void CDefAnimation::drawT(size_t frame, size_t group, SDL_Surface * surf, int posX, int posY) const
{
	ui8 segmentType, segmentLength;

	size_t frameOffset = def->getOffset(group, frame);
	const ui8 * framePtr = def->getData()->getRawData(frameOffset);
	CDefFile::SpriteDef sprite = def->getSpriteDef(group, frame);
	
	const int rightMargin = sprite.fullWidth - sprite.width - sprite.leftMargin;
	const int bottomMargin = sprite.fullHeight - sprite.height - sprite.topMargin;
	const int spriteDefOffset = sizeof(CDefFile::SpriteDef);

	//as it should be always in creature animations
	assert(sprite.format == 1);
	
	int ftcp = 0;
	if(sprite.topMargin > 0)
	{
		ftcp += sprite.fullWidth * sprite.topMargin;
	}

	for(size_t i = 0; i < sprite.height; i++)
	{
		def->getData()->setSeekPos(frameOffset + spriteDefOffset + 4 * i);
		int baseOffset = spriteDefOffset + def->getData()->readInt32();
		
		// length of read segment
		int totalRowLength = 0;

		if(sprite.leftMargin > 0)
		{
			ftcp += sprite.leftMargin;
		}

		// Note: Bug fixed (Rev 2115): The implementation of omitting lines was false. 
		// We've to calculate several things so not showing/putting pixels should suffice.

		int yB = ftcp / sprite.fullWidth + posY;

		do
		{
			segmentType = framePtr[baseOffset++];
			segmentLength = framePtr[baseOffset++];

			const int remainder = ftcp % sprite.fullWidth;
			int xB = (rotateFlipType == ERotateFlipType::ROTATENONE_FLIPX
					? sprite.fullWidth - remainder - 1 : remainder) + posX;

			for(size_t k = 0; k <= segmentLength; k++)
			{
				if(xB >= 0 && xB < surf->w && yB >= 0 && yB < surf->h)
				{
					if(posX <= xB && posX + static_cast<int>(sprite.fullWidth) > xB 
						&& posY <= yB && posY + static_cast<int>(sprite.fullHeight) > yB)
					{
						const ui8 colorNr = segmentType == 0xff ? framePtr[baseOffset + k] : segmentType;
						putPixel<bpp>(surf, xB, yB, def->getColorFromPalette(colorNr), colorNr);
					}
				}

				ftcp++;
				if(rotateFlipType == ERotateFlipType::ROTATENONE_FLIPX)
					xB--;
				else
					xB++;

				if(segmentType == 0xFF && totalRowLength + k + 1 >= sprite.width)
					break;
			}
			if(segmentType == 0xFF)
			{
				baseOffset += segmentLength + 1;
			}

			totalRowLength += segmentLength + 1;
		} while(totalRowLength < static_cast<int>(sprite.width));
		
		if(rightMargin > 0)
		{
			ftcp += rightMargin;
		}
	}
}

template<int bpp>
inline void CDefAnimation::putPixel(SDL_Surface * surf, int posX, int posY, SDL_Color color, ui8 colorNr) const
{
	if(colorNr == 0)
		return;
	
	ui8 * p = reinterpret_cast<ui8 *>(surf->pixels) + posX * surf->format->BytesPerPixel 
		+ posY * surf->pitch;
	
	// normal color
	if(colorNr > 7)
	{
		ColorPutter<bpp, 0>::PutColor(p, color.r, color.g, color.b);
	}
	// selection highlight
	else if((glowType != EGlowAnimationType::NONE) && (colorNr == 6 || colorNr == 7))
	{
		if(glowType == EGlowAnimationType::BLUE)
			ColorPutter<bpp, 0>::PutColor(p, 0, glowIntensity, glowIntensity);
		else
			ColorPutter<bpp, 0>::PutColor(p, glowIntensity, glowIntensity, 0);
	}
	// selection highlight or transparent
	else if(colorNr == 5) 
	{
		if(glowType == EGlowAnimationType::BLUE)
			ColorPutter<bpp, 0>::PutColor(p, color.b, glowIntensity - color.g, glowIntensity - color.r);
		else if(glowType == EGlowAnimationType::YELLOW)
			ColorPutter<bpp, 0>::PutColor(p, glowIntensity - color.r, glowIntensity - color.g, color.b);
	}
	// shadow
	else 
	{
		// determining transparency value, 255 or 0 should be already filtered
		static ui8 colToAlpha[8] = { 255, 192, 128, 128, 128, 255, 128, 192 };
		ui8 alpha = colToAlpha[colorNr];

		if(bpp != 3 && bpp != 4)
		{
			ColorPutter<bpp, 0>::PutColor(p, 0, 0, 0, alpha);
		}
		else
		{
			p[0] = (p[0] * alpha) >> 8;
			p[1] = (p[1] * alpha) >> 8;
			p[2] = (p[2] * alpha) >> 8;
		}
	}
	
}

void CDefAnimation::rotateFlip(ERotateFlipType::ERotateFlipType type)
{
	rotateFlipType = type;
}

void CDefAnimation::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity)
{
	this->glowType = glowType;
	this->glowIntensity = intensity;
}

CAnimationHolder::CAnimationHolder(IAnimation * animation) 
	: anim(animation), currentGroup(0), currentFrame(0), glowType(EGlowAnimationType::NONE), 
	glowIntensity(MIN_GLOW_INTENSITY)
{
	size_t loadedGroup = animation->getLoadedGroup();
	setGroup(loadedGroup == -1 ? 0 : loadedGroup);
}

CAnimationHolder::~CAnimationHolder()
{
	delete anim;
}

void CAnimationHolder::setPosition(const Point & pos)
{
	anim->setPosition(pos);
}

Point CAnimationHolder::getPosition() const
{
	return anim->getPosition();
}

void CAnimationHolder::setGroup(size_t group, bool repeat /*= false*/)
{
	this->repeat = repeat;
	std::map<size_t, size_t> entries = anim->getEntries();

	// check if the group is loaded
	if(anim->getLoadedGroup() != group && anim->getLoadedGroup() != -1)
	{
		tlog2 << "Group Nr. " << group << " couldn't be loaded." << std::endl;
		return;
	}

	// check if the group nr is defined in the animation format
	if(vstd::contains(entries, group))
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
	updateFrame(elapsedTime);
	updateGlowAnimation(elapsedTime);
}

void CAnimationHolder::updateFrame(double elapsedTime)
{
	static const double framesSecond = 1 / 6.; // TODO

	currentFrame = static_cast<size_t>(currentTime / framesSecond);
	if(currentFrame >= frameCount)
	{
		if(repeat == true)
		{
			currentTime -= static_cast<int>(currentTime / (frameCount * framesSecond)) * (frameCount * framesSecond);
			currentFrame = static_cast<size_t>(currentTime / framesSecond);
		}
		else
			currentFrame = frameCount - 1;
	}
	currentTime += elapsedTime;
}

void CAnimationHolder::updateGlowAnimation(double elapsedTime)
{
	if(glowType != EGlowAnimationType::NONE)
	{
		const double glowDuration = 1.5;
		double currentGlow = ((glowTime - (static_cast<int>(glowTime / glowDuration) * glowDuration)) 
			/ glowDuration);
		const ui8 glowSpan = 255 - MIN_GLOW_INTENSITY;

		// Fade out
		if(currentGlow < 0.5)
		{
			ui8 deltaGlowIntensity = static_cast<ui8>(glowSpan * currentGlow * 2);
			anim->setGlowAnimation(glowType, 255 - deltaGlowIntensity);
		}
		// Fade in
		else
		{
			ui8 deltaGlowIntensity = static_cast<ui8>(glowSpan * (currentGlow - 0.5) * 2);
			anim->setGlowAnimation(glowType, MIN_GLOW_INTENSITY + deltaGlowIntensity);
		}
	}
	glowTime += elapsedTime;
}

void CAnimationHolder::draw()
{
	anim->draw(currentFrame, currentGroup);
}

void CAnimationHolder::recolorToPlayer(int player)
{
	anim->recolorToPlayer(player);
}

void CAnimationHolder::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType)
{
	glowTime = 0.0;
	this->glowType = glowType;
}

void CAnimationHolder::setAlpha(ui8 alpha)
{
	anim->setAlpha(alpha);
}

void CAnimationHolder::rotateFlip(ERotateFlipType::ERotateFlipType type)
{
	anim->rotateFlip(type);
}
