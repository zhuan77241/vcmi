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
 * Interface for VCMI related image tasks like player coloring
 */
class IImageTasks : public IGraphicsTasks
{
public:
	// Change palette to specific player.
	virtual TImagePtr recolorToPlayer(int player) const =0;

	// Sets/Unsets the yellow or blue glow animation effect.
	virtual TImagePtr setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha) const =0;

	virtual TImagePtr rotate(EImageRotation::EImageRotation rotation) const =0;
};

/*
 * Interface for images
 */
class IImage : public IImageTasks
{
protected:
	GraphicsLocator locator;

public:
	IImage(const GraphicsLocator & Locator = GraphicsLocator());
	virtual ~IImage() { };
	virtual IImage * clone() const =0;
	
	// Gets the locator object which stores the filesystem location, 
	// loaded groups and transformations of the animation.
	const GraphicsLocator & getLocator() const;

	// draws image on surface "where" at position
	virtual void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL, ui8 alpha = 255) const =0;

	virtual int width() const=0;
	virtual int height() const=0;
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

/*
 * Wrapper around SDL_Surface
 */
class CSDLImage : public IImage
{
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
	
	//Surface without empty borders
	SDL_Surface * surf;
	bool freeSurf;

	//size of left and top borders
	Point margins;
	
	//total size including borders
	Point fullSize;

public:
	// Loads an image from memory.
	// Params: imageType		E.g.: PCX, TGA, BMP, DEF
	CSDLImage(TMemoryStreamPtr data, const std::string & imageType, const GraphicsLocator & locator = GraphicsLocator());

	// Loads an sprite image from DEF file.
	CSDLImage(const CDefFile * defFile, size_t frame, size_t group, const GraphicsLocator & locator = GraphicsLocator());

	// Constructs an SDLImage object from a existing SDL_Surface
	CSDLImage(SDL_Surface * surf, bool freeSurf = true);

	CSDLImage(const CSDLImage & cpy);
	CSDLImage & operator=(const CSDLImage & cpy);
	~CSDLImage();

	IImage * clone() const;
	void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL,  ui8 alpha = 255) const;
	int width() const;
	int height() const;
	
	// Get raw pointer to SDL surface. It's only needed for SDL related tasks.
	SDL_Surface * getSDL_Surface() const;

	TImagePtr recolorToPlayer(int player) const;
	void recolorToPlayer(int player);
	void recolorToPlayerViaSelector(const GraphicsSelector & selector);

	TImagePtr setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha) const;
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);

	TImagePtr rotate(EImageRotation::EImageRotation rotation) const;
	void rotate(EImageRotation::EImageRotation rotation);
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

	//x,y - margins, w,h - sprite size
	Rect sprite;

	//total size including borders
	Point fullSize;

	//RLE-d data
	ui8 * surf;

	// Length of surf, needed for cpy-ctor
	size_t length;

	//array of offsets for each line
	ui32 * line;

	//palette
	SDL_Color * palette;

	//Used internally to blit one block of data
	template<int bpp, int dir>
	void blitBlock(ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha) const;
	void blitBlockWithBpp(ui8 bpp, ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha, bool rotated) const;

public:
	// Loads an sprite image from DEF file.
	CCompImage(const CDefFile * defFile, size_t frame, size_t group, const GraphicsLocator & locator = GraphicsLocator());

	CCompImage(const CCompImage & cpy);
	CCompImage & operator=(const CCompImage & cpy);
	~CCompImage();

	IImage * clone() const;

	void draw(TImagePtr where, int posX = 0, int posY = 0, Rect * src = NULL, ui8 alpha = 255) const;
	int width() const;
	int height() const;

	TImagePtr recolorToPlayer(int player) const;
	void recolorToPlayer(int player);
	void recolorToPlayerViaSelector(const GraphicsSelector & selector);

	TImagePtr setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha) const;
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha);

	TImagePtr rotate(EImageRotation::EImageRotation rotation) const;
	void rotate(EImageRotation::EImageRotation rotation);
};