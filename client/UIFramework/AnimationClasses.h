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
class CCompAnimation;
class CSDLAnimation;
class CAnimationHolder;

class IAnimation : public ITransformational
{
protected:
	std::map<size_t, size_t> entries;
	Point pos;

	/** Contains the number of the loaded group. */
	boost::optional<size_t> loadedGroup;

public:
	IAnimation();
	virtual ~IAnimation() { };
	virtual IAnimation * clone() const =0;

	// Gets the information how many groups/frames exist
	std::map<size_t, size_t> getEntries() const;

	// Gets the index of the loaded group. -1 if all groups are loaded.
	boost::optional<size_t> getLoadedGroup() const;

	virtual void draw(size_t frame, size_t group) const =0;

	void setPosition(const Point & pos);
	Point getPosition() const;
};

class CImageBasedAnimation : public IAnimation
{
protected:
	// images[group][frame], store objects with loaded images
	std::map<size_t, std::map<size_t, IImage *> > images;

	typedef std::map<size_t, std::map<size_t, IImage *> >::iterator group_it;
	typedef std::map<size_t, IImage *>::iterator frame_it;
	typedef std::map<size_t, std::map<size_t, IImage *> >::const_iterator group_itc;
	typedef std::map<size_t, IImage *>::const_iterator frame_itc;

	CImageBasedAnimation() { };
	CImageBasedAnimation(const CImageBasedAnimation & other);
	CImageBasedAnimation & operator=(const CImageBasedAnimation & other);
	~CImageBasedAnimation();

	template<typename anim>
	void constructImageBasedAnimation(const CDefFile * defFile, boost::optional<size_t> group = boost::none);

public:
	void forEach(std::function<void(IImage *)> func);

	virtual void applyTransformations(IImage * img) const { };
	void draw(size_t frame, size_t group) const;

	void recolorToPlayer(int player);
};

class CCompAnimation : public CImageBasedAnimation
{
	EGlowAnimationType::EGlowAnimationType glowType;
	ui8 glowIntensity;
	ui8 alpha;
	ERotateFlipType::ERotateFlipType rotateFlipType;

public:
	CCompAnimation(const CDefFile * defFile, boost::optional<size_t> group = boost::none);
	IAnimation * clone() const;

	void applyTransformations(IImage * img) const;
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity);
	void setAlpha(ui8 alpha);
	void rotateFlip(ERotateFlipType::ERotateFlipType type);
};

class CSDLAnimation : public CImageBasedAnimation
{
public:
	CSDLAnimation(const CDefFile * defFile, boost::optional<size_t> group = boost::none);
	IAnimation * clone() const;

	void rotateFlip(ERotateFlipType::ERotateFlipType type);
};

class CDefAnimation : public IAnimation
{
	const CDefFile * def;
	int playerColor;
	EGlowAnimationType::EGlowAnimationType glowType;
	ui8 glowIntensity;
	ERotateFlipType::ERotateFlipType rotateFlipType;

	template<int bpp>
	inline void putPixel(SDL_Surface * surf, int posX, int posY, SDL_Color color, ui8 colorNr) const;

	template<int bpp>
	void drawT(size_t frame, size_t group, SDL_Surface * surf, int posX, int posY) const;

public:
	explicit CDefAnimation(const CDefFile * defFile);
	~CDefAnimation();

	IAnimation * clone() const;

	void draw(size_t frame, size_t group) const;

	void rotateFlip(ERotateFlipType::ERotateFlipType type);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity);
};

class CAnimationHolder
{
	static const ui8 MIN_GLOW_INTENSITY = 50;

	IAnimation * anim;
	size_t currentGroup, currentFrame, frameCount;
	double currentTime, glowTime;
	bool repeat;
	EGlowAnimationType::EGlowAnimationType glowType;
	ui8 glowIntensity;

	void updateFrame(double elapsedTime);
	void updateGlowAnimation(double elapsedTime);

public:
	explicit CAnimationHolder(IAnimation * animation);
	CAnimationHolder(const CAnimationHolder & cpy);
	CAnimationHolder & operator=(const CAnimationHolder & cpy);
	~CAnimationHolder();

	void setGroup(size_t group, bool repeat = false);

	void setPosition(const Point & pos);
	Point getPosition() const;

	void update(double elapsedTime);
	void draw();

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType);
	void setAlpha(ui8 alpha);
	void rotateFlip(ERotateFlipType::ERotateFlipType type);
};
