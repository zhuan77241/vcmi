#pragma once

#include "ImageClassesFwd.h"
#include "AnimationClassesFwd.h"

/*
 * AnimationClasses.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class IAnimation
{
protected:
	std::map<size_t, size_t> entries;

public:
	virtual void load(const CDefFile * defFile) =0;
	virtual void load(const CDefFile * defFile, size_t group) =0;
	
	std::map<size_t, size_t> getEntries() const;

	virtual void draw(TImagePtr where, size_t frame, size_t group, int posX, int posY) =0;

	static TAnimationPtr createAnimation(const CDefFile * defFile, size_t group = -1);
};

class ImageBasedAnimation : public IAnimation
{
	// images[group][frame], store objects with loaded images
	std::map<size_t, std::map<size_t, TImagePtr> > images;

public:
	void load(const CDefFile * defFile);
	void load(const CDefFile * defFile, size_t group);

	void draw(TImagePtr where, size_t frame, size_t group, int posX, int posY);
};

class CAnimation
{
	TAnimationPtr anim;
	size_t currentGroup, currentFrame, frameCount;
	double currentTime;
	bool repeat;

public:
	CAnimation(TAnimationPtr animation);
	CAnimation(const ResourceIdentifier & identifier);
	CAnimation(const ResourceIdentifier & identifier, size_t group);

	void setGroup(size_t group, bool repeat = false);

	void update(double elapsedTime);
	void draw(TImagePtr where, int posX, int posY);
};