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
class IImage : public ITransformational
{
protected:
	Point pos;

public:
	virtual ~IImage() { };
	virtual IImage * clone() const =0;

	virtual void draw() const =0;

	void setPosition(const Point & pos);
	Point getPosition() const;
};

/// Class for def loading, methods are based on CDefHandler
/// After loading will store general info (palette and frame offsets) and pointer to file itself
class CDefFile
{
	//offset[group][frame] - offset of frame data in file
	std::map<size_t, std::vector <size_t> > offset;

	const CMemoryStream * data;
	SDL_Color * palette;

public:
	struct SpriteDef
	{
		ui32 size;

		// format in which pixel data is stored
		ui32 format;    

		// full width and height of frame, including borders
		ui32 fullWidth; 
		ui32 fullHeight;

		// width and height of pixel data, borders excluded
		ui32 width;     
		ui32 height;

		si32 leftMargin;
		si32 topMargin;
	};
	
	CDefFile(const CMemoryStream * Data);
	~CDefFile();

	//load frame as SDL_Surface
	template<class ImageLoader>
	void loadFrame(size_t frame, size_t group, ImageLoader &loader) const;

	std::map<size_t, size_t> getEntries() const;
	size_t getOffset(size_t group, size_t frame) const;
	const CMemoryStream * getData() const;
	SpriteDef getSpriteDef(size_t group, size_t frame) const;
	SDL_Color getColorFromPalette(ui8 colorNr) const;
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
	CSDLImage(const CMemoryStream * data, const std::string & imageType);

	// Loads an sprite image from DEF file.
	CSDLImage(const CDefFile * defFile, size_t frame, size_t group);

	// Constructs an SDLImage object from a existing SDL_Surface
	CSDLImage(SDL_Surface * surf, bool freeSurf = true);

	CSDLImage(const CSDLImage & cpy);
	CSDLImage & operator=(const CSDLImage & cpy);
	~CSDLImage();

	IImage * clone() const;
	void draw() const;
	
	// Get raw pointer to SDL surface.
	SDL_Surface * getRawSurface() const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity);
	void setAlpha(ui8 alpha);
	void flipHorizontal(bool flipped);
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

	ui8 alpha;

	//Used internally to blit one block of data
	template<int bpp, int dir>
	void blitBlock(ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha) const;
	void blitBlockWithBpp(ui8 bpp, ui8 type, ui8 size, ui8 * & data, ui8 * & dest, ui8 alpha, bool rotated) const;

public:
	// Loads an sprite image from DEF file.
	CCompImage(const CDefFile * defFile, size_t frame, size_t group);

	CCompImage(const CCompImage & cpy);
	CCompImage & operator=(const CCompImage & cpy);
	~CCompImage();

	IImage * clone() const;

	void draw() const;

	void recolorToPlayer(int player);
	void setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 intensity);
	void setAlpha(ui8 alpha);
	void flipHorizontal(bool flipped);
};