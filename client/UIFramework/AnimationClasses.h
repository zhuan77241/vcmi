#pragma once

#include "ImageClassesFwd.h"
#include "Geometries.h"

/*
 * AnimationClasses.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class IAnimation;
class CImageBasedAnimation;
class CAnimationHolder;

class IAnimation : public ITransformational
{
protected:
	std::map<size_t, size_t> entries;
	Point pos;

	// Positive value for the loaded group. -1 if all groups are loaded.
	size_t loadedGroup;

public:
	virtual ~IAnimation() { };
	virtual IAnimation * clone() const =0;

	// Gets the information how many groups/frames exist
	std::map<size_t, size_t> getEntries() const;

	// Gets the index of the loaded group. -1 if all groups are loaded.
	si8 getLoadedGroup() const;

	virtual void draw(size_t frame, size_t group) const =0;

	void setPosition(const Point & pos);
	Point getPosition() const;
};

class CImageBasedAnimation : public IAnimation
{
	// images[group][frame], store objects with loaded images
	std::map<size_t, std::map<size_t, IImage *> > images;

	typedef std::map<size_t, std::map<size_t, IImage *> >::iterator group_it;
	typedef std::map<size_t, IImage *>::iterator frame_it;
	typedef std::map<size_t, std::map<size_t, IImage *> >::const_iterator group_itc;
	typedef std::map<size_t, IImage *>::const_iterator frame_itc;

public:
	// Loads frames of the specified group of an animation. Assign -1 to the second parameter 'group' to load all groups.
	CImageBasedAnimation(const CDefFile * defFile, size_t group = -1);
	
	CImageBasedAnimation(const CImageBasedAnimation & other);
	CImageBasedAnimation & operator=(const CImageBasedAnimation & other); 
	~CImageBasedAnimation();

	IAnimation * clone() const;
	void forEach(std::function<void(IImage *)> func);

	void draw(size_t frame, size_t group) const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);
	void setAlpha(ui8 alpha);
};

class CDEFAnimation : public IAnimation
{
	const CDefFile * def;

public:
	CDEFAnimation(const CDefFile * defFile);

	void draw() const;
};

class CAnimationHolder
{
	IAnimation * anim;
	size_t currentGroup, currentFrame, frameCount;
	double currentTime;
	bool repeat;

public:
	CAnimationHolder(IAnimation * animation);
	CAnimationHolder(const ResourceIdentifier & identifier);
	CAnimationHolder(const ResourceIdentifier & identifier, size_t group, bool repeat = false);
	~CAnimationHolder();

	void setGroup(size_t group, bool repeat = false);

	void update(double elapsedTime);
	void draw();

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);
};
