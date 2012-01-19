#pragma once

#include "ImageClassesFwd.h"
#include "../../lib/CFileSystemHandlerFwd.h"
#include "Geometries.h"

/*
 * ImageClasses.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

struct SDL_Surface;

/*
 * Interface for images
 */
class IImage
{
public:
	// Loads an image from memory.
	// Params: imageType		E.g.: PCX, TGA, BMP, DEF
	virtual void load(TMemoryStreamPtr data, const std::string &imageType) =0;
	
	// draws image on surface "where" at position
	virtual void draw(IImage *where, int posX = 0, int posY = 0, Rect * src = NULL, ui8 alpha = 255) const =0;

	virtual int width() const=0;
	virtual int height() const=0;
	virtual ~IImage() {};

	static IImage * createInstance(TMemoryStreamPtr data, const std::string &imageType, bool useComp = false);
};

/*
 * Interface for VCMI related image tasks like player coloring
 */
class IImageTasks
{
public:
	// Change palette to specific player.
	virtual void recolorToPlayer(int player) =0;

	// Sets/Unsets the yellow or blue glow animation effect.
	virtual void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha) =0;

	virtual void rotate(EImageRotation::EImageRotation rotation) =0;
};
/*
class SDLImageLoader
{
	SDLImage * image;
	ui8 * lineStart;
	ui8 * position;

public:
	//load size raw pixels from data
	inline void load(size_t size, const ui8 * data);
	
	//set size pixels to color
	inline void load(size_t size, ui8 color = 0);
	inline void endLine();
	
	//init image with these sizes and palette
	inline void init(Point SpriteSize, Point Margins, Point FullSize, SDL_Color * pal);

	SDLImageLoader(SDLImage * Img);
	~SDLImageLoader();
};*/

/*
 * Wrapper around SDL_Surface
 */
class SDLImage : public IImage, public IImageTasks
{
public:
	//Surface without empty borders
	SDL_Surface * surf;
	
	//size of left and top borders
	Point margins;
	
	//total size including borders
	Point fullSize;

public:
	//Load image by memory stream
	SDLImage();
	~SDLImage();

	void load(TMemoryStreamPtr data, const std::string &imageType);
	void draw(IImage * where, int posX = 0, int posY = 0, Rect * src = NULL,  ui8 alpha = 255) const;
	int width() const;
	int height() const;
	
	// Get raw pointer to SDL surface. It's only needed for SDL related tasks.
	SDL_Surface * getSDL_Surface() const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);
	void rotate(EImageRotation::EImageRotation rotation);

	friend class SDLImageLoader;
	friend class SDLImage;
};