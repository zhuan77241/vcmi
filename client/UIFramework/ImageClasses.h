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

/*
 * Interface for images
 */
class IImage
{
public:
	// Loads an image from memory.
	// Params: imageType		E.g.: PCX, TGA, BMP, DEF
	virtual void load(TMemoryStreamPtr data, const std::string & imageType) =0;
	
	// Loads an sprite image from DEF file.
	virtual void load(const CDefFile * defFile, size_t frame, size_t group) =0;

	// draws image on surface "where" at position
	virtual void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL, ui8 alpha = 255) const =0;

	virtual int width() const=0;
	virtual int height() const=0;
	virtual ~IImage() {};

	static TImagePtr createImageFromFile(TMemoryStreamPtr data, const std::string & imageType);
	static TImagePtr createSpriteFromDEF(const CDefFile * defFile, size_t frame, size_t group);
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

/// Class for def loading, methods are based on CDefHandler
/// After loading will store general info (palette and frame offsets) and pointer to file itself
class CDefFile
{
	//offset[group][frame] - offset of frame data in file
	std::map<size_t, std::vector <size_t> > offset;

	TMemoryStreamPtr data;
	SDL_Color * palette;

public:
	CDefFile(TMemoryStreamPtr Data);
	~CDefFile();

	//load frame as SDL_Surface
	template<class ImageLoader>
	void loadFrame(size_t frame, size_t group, ImageLoader &loader) const;

	std::map<size_t, size_t> getEntries() const;
};

class CSDLImageLoader
{
	CSDLImage * image;
	ui8 * lineStart;
	ui8 * position;

public:
	//load size raw pixels from data
	void load(size_t size, const ui8 * data);
	
	//set size pixels to color
	void load(size_t size, ui8 color = 0);
	void endLine();
	
	//init image with these sizes and palette
	void init(Point spriteSize, Point margins, Point fullSize, SDL_Color * pal);

	CSDLImageLoader(CSDLImage * Img);
	~CSDLImageLoader();
};

/*
 * Wrapper around SDL_Surface
 */
class CSDLImage : public IImage, public IImageTasks
{
	//Surface without empty borders
	SDL_Surface * surf;
	bool freeSurf;

	//size of left and top borders
	Point margins;
	
	//total size including borders
	Point fullSize;

public:
	//Load image by memory stream
	CSDLImage();
	~CSDLImage();

	void load(TMemoryStreamPtr data, const std::string & imageType);
	void load(const CDefFile * defFile, size_t frame, size_t group);
	void load(SDL_Surface * surf, bool freeSurf = true);
	void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL,  ui8 alpha = 255) const;
	int width() const;
	int height() const;
	
	// Get raw pointer to SDL surface. It's only needed for SDL related tasks.
	SDL_Surface * getSDL_Surface() const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);
	void rotate(EImageRotation::EImageRotation rotation);

	friend class CSDLImageLoader;
	friend class CSDLImage;
};

class CCompImageLoader
{
	CCompImage * image;
	ui8 * position;
	ui8 * entry;
	ui32 currentLine;

	ui8 typeOf(ui8 color);
	void newEntry(ui8 color, size_t size);
	void newEntry(const ui8 * & data, size_t size);

public:
	//load size raw pixels from data
	void load(size_t size, const ui8 * data);
	
	//set size pixels to color
	void load(size_t size, ui8 color = 0);
	void endLine();
	
	//init image with these sizes and palette
	void init(Point spriteSize, Point margins, Point fullSize, SDL_Color * pal);

	CCompImageLoader(CCompImage * img);
	~CCompImageLoader();
};

/*
 *  RLE-compressed image data for 8-bit images with alpha-channel, currently far from finished
 *  primary purpose is not high compression ratio but fast drawing.
 *  Consist of repeatable segments with format similar to H3 def compression:
 *  1st byte:
 *  if (byte == 0xff)
 *  	raw data, opaque and semi-transparent data always in separate blocks
 *  else
 *  	RLE-compressed image data with this color
 *  2nd byte = size of segment
 *  raw data (if any)
 */
class CCompImage : public IImage
{
	//x,y - margins, w,h - sprite size
	Rect sprite;

	//total size including borders
	Point fullSize;

	//RLE-d data
	ui8 * surf;

	//array of offsets for each line
	ui32 * line;

	//palette
	SDL_Color * palette;

	//Used internally to blit one block of data
	template<int bpp, int dir>
	void blitBlock(ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha) const;
	void blitBlockWithBpp(ui8 bpp, ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha, bool rotated) const;

public:
	CCompImage();
	~CCompImage();

	// TODO: Load image from file(SDL_Surface)
	void load(TMemoryStreamPtr data, const std::string & imageType);
	
	//Load image from def file
	void load(const CDefFile * defFile, size_t frame, size_t group);

	void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL, ui8 alpha = 255) const;
	int width() const;
	int height() const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);
	void rotate(EImageRotation::EImageRotation rotation);

	friend class CCompImageLoader;
};