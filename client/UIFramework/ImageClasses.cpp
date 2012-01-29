#include "StdInc.h"
#include "ImageClasses.h"

#include "SDL_Extensions.h"
#include "../../lib/CFileSystemHandler.h"
#include "../../lib/vcmi_endian.h"
#include "../../lib/GameConstants.h"
#include "../Graphics.h"

TImagePtr IImage::createImageFromFile(TMemoryStreamPtr data, const std::string & imageType)
{
	// always use SDL when loading image from file 
	TImagePtr img = shared_ptr<IImage>(new SDLImage());
	img->load(data, imageType);
	return img;
}

TImagePtr IImage::createSpriteFromDEF(const CDefFile * defFile, size_t frame, size_t group)
{
	// always use CompImage when loading from DEF file
	// we keep the possibility to load via SDL image
	static const bool useComp = true;

	TImagePtr img;
	if (useComp)
		img = shared_ptr<IImage>(new CompImage());

	else
		img = shared_ptr<IImage>(new SDLImage());

	img->load(defFile, frame, group);
	return img;
}

CDefFile::CDefFile(TMemoryStreamPtr Data): palette(NULL)
{
	//First 8 colors in def palette used for transparency
	static SDL_Color H3Palette[8] =
	{
		{   0,   0,   0,   0},// 100% - transparency
		{   0,   0,   0, 192},//  75% - shadow border,
		{   0,   0,   0, 128},// TODO: find exact value
		{   0,   0,   0, 128},// TODO: for transparency
		{   0,   0,   0, 128},//  50% - shadow body
		{   0,   0,   0,   0},// 100% - selection highlight
		{   0,   0,   0, 128},//  50% - shadow body   below selection
		{   0,   0,   0, 192} // 75% - shadow border below selection
	};

	this->data = Data;
	palette = new SDL_Color[256];

	ui32 type = data->readInt32();
	
	// skip width and height definitions
	data->incSeekPos(8);

	ui32 totalBlocks = data->readInt32();

	// read image palette
	for(size_t i = 0; i < 256; ++i)
	{
		palette[i].r = data->readInt8();
		palette[i].g = data->readInt8();
		palette[i].b = data->readInt8();
		palette[i].unused = 255;
	}

	if(type == 71)
	{
		// Buttons/buildings don't have shadows\semi-transparency
		memset(palette, 0, sizeof(SDL_Color) * 8);
	}
	else
	{
		//initialize shadow\selection colors
		memcpy(palette, H3Palette, sizeof(SDL_Color) * 8);
	}

	for (size_t i = 0; i < totalBlocks; ++i)
	{
		size_t blockID = data->readInt32();
		size_t totalEntries = data->readInt32();
		
		//8 unknown bytes - skipping
		data->incSeekPos(8);

		//13 bytes for name of every frame in this block - not used, skipping
		data->incSeekPos(13 * totalEntries);

		for (size_t j = 0; j < totalEntries; ++j)
		{
			size_t currOffset = data->readInt32();
			offset[blockID].push_back(currOffset);
		}
	}
}

template<class ImageLoader>
void CDefFile::loadFrame(size_t frame, size_t group, ImageLoader &loader) const
{
	std::map<size_t, std::vector <size_t> >::const_iterator it;
	it = offset.find(group);
	assert (it != offset.end());

	const ui8 * framePtr = data->getRawData(it->second[frame]);

	struct SSpriteDef
	{
		ui32 size;
		ui32 format;    // format in which pixel data is stored
		ui32 fullWidth; // full width and height of frame, including borders
		ui32 fullHeight;
		ui32 width;     // width and height of pixel data, borders excluded
		ui32 height;
		si32 leftMargin;
		si32 topMargin;
	};

	const SSpriteDef sd = * reinterpret_cast<const SSpriteDef *>(framePtr);
	SSpriteDef sprite;

	//sprite.size = SDL_SwapLE32(sd.size);//unused
	sprite.format = SDL_SwapLE32(sd.format);
	sprite.fullWidth = SDL_SwapLE32(sd.fullWidth);
	sprite.fullHeight = SDL_SwapLE32(sd.fullHeight);
	sprite.width = SDL_SwapLE32(sd.width);
	sprite.height = SDL_SwapLE32(sd.height);
	sprite.leftMargin = SDL_SwapLE32(sd.leftMargin);
	sprite.topMargin = SDL_SwapLE32(sd.topMargin);

	ui32 currentOffset = sizeof(SSpriteDef);
	ui32 BaseOffset = sizeof(SSpriteDef);

	loader.init(Point(sprite.width, sprite.height),
		Point(sprite.leftMargin, sprite.topMargin),
		Point(sprite.fullWidth, sprite.fullHeight), palette);

	switch(sprite.format)
	{
	case 0:
		{
			//pixel data is not compressed, copy data to surface
			for(size_t i = 0; i < sprite.height; ++i)
			{
				loader.load(sprite.width, framePtr[currentOffset]);
				currentOffset += sprite.width;
				loader.endLine();
			}
			break;
		}
	case 1:
		{
			//for each line we have offset of pixel data
			const ui32 * RWEntriesLoc = reinterpret_cast<const ui32 *>(framePtr + currentOffset);
			currentOffset += sizeof(ui32) * sprite.height;

			for(size_t i = 0; i < sprite.height; ++i)
			{
				//get position of the line
				currentOffset = BaseOffset + read_le_u32(RWEntriesLoc + i);
				ui32 TotalRowLength = 0;

				while(TotalRowLength < sprite.width)
				{
					ui8 type = framePtr[currentOffset++];
					ui32 length = framePtr[currentOffset++] + 1;

					//Raw data
					if (type == 0xFF)
					{
						loader.load(length, framePtr + currentOffset);
						currentOffset+=length;
					}
					// RLE
					else
					{
						loader.load(length, type);
					}
					TotalRowLength += length;
				}

				loader.endLine();
			}
			break;
		}
	case 2:
		{
			currentOffset = BaseOffset + read_le_u16(framePtr + BaseOffset);

			for(size_t i = 0; i < sprite.height; ++i)
			{
				ui32 TotalRowLength = 0;

				while(TotalRowLength < sprite.width)
				{
					ui8 SegmentType = framePtr[currentOffset++];
					ui8 code = SegmentType / 32;
					ui8 length = (SegmentType & 31) + 1;

					//Raw data
					if (code == 7)
					{
						loader.load(length, framePtr[currentOffset]);
						currentOffset += length;
					}
					//RLE
					else
					{
						loader.load(length, code);
					}
					TotalRowLength += length;
				}
				loader.endLine();
			}
			break;
		}
	case 3:
		{
			for(size_t i = 0; i < sprite.height; ++i)
			{
				currentOffset = BaseOffset + read_le_u16(framePtr + BaseOffset + i * 2 * (sprite.width / 32));
				ui32 TotalRowLength = 0;

				while(TotalRowLength < sprite.width)
				{
					ui8 segment = framePtr[currentOffset++];
					ui8 code = segment / 32;
					ui8 length = (segment & 31) + 1;

					//Raw data
					if (code == 7)
					{
						loader.load(length, framePtr + currentOffset);
						currentOffset += length;
					}
					//RLE
					else
					{
						loader.load(length, code);
					}
					TotalRowLength += length;
				}
				loader.endLine();
			}
			break;
		}
	default:
		tlog0 << "Error: unsupported format of def file:" << sprite.format << "\n";
		break;
	}
};

CDefFile::~CDefFile()
{
	delete[] palette;
}

std::map<size_t, size_t> CDefFile::getEntries() const
{
	std::map<size_t, size_t> ret;

	for (std::map<size_t, std::vector<size_t> >::const_iterator mapIt = offset.begin(); 
		mapIt != offset.end(); ++mapIt)
		ret[mapIt->first] =  mapIt->second.size();
	return ret;
}

SDLImageLoader::SDLImageLoader(SDLImage * Img) : image(Img), lineStart(NULL), position(NULL)
{
}

void SDLImageLoader::init(Point spriteSize, Point margins, Point fullSize, SDL_Color * pal)
{
	//Init image
	image->surf = SDL_CreateRGBSurface(SDL_SWSURFACE, spriteSize.x, spriteSize.y, 8, 0, 0, 0, 0);
	image->margins  = margins;
	image->fullSize = fullSize;

	//Prepare surface
	SDL_SetColors(image->surf, pal, 0, 256);
	SDL_LockSurface(image->surf);

	lineStart = position = reinterpret_cast<ui8 *>(image->surf->pixels);
}

void SDLImageLoader::load(size_t size, const ui8 * data)
{
	if (size)
	{
		memcpy(reinterpret_cast<void *>(position), data, size);
		position += size;
	}
}

void SDLImageLoader::load(size_t size, ui8 color)
{
	if (size)
	{
		memset(reinterpret_cast<void *>(position), color, size);
		position += size;
	}
}

void SDLImageLoader::endLine()
{
	lineStart += image->surf->pitch;
	position = lineStart;
}

SDLImageLoader::~SDLImageLoader()
{
	SDL_UnlockSurface(image->surf);
	SDL_SetColorKey(image->surf, SDL_SRCCOLORKEY, 0);
	//TODO: RLE if compressed and bpp>1
}

SDLImage::SDLImage() : surf(NULL), freeSurf(true)
{

}

SDLImage::~SDLImage()
{
	if (freeSurf)
		SDL_FreeSurface(surf);
}

void SDLImage::load(TMemoryStreamPtr data, const std::string & imageType)
{
	surf = CSDL_Ext::loadImage(data, imageType);
	freeSurf = true;
}

void SDLImage::load(const CDefFile * defFile, size_t frame, size_t group)
{
	SDLImageLoader loader(this);
	defFile->loadFrame(frame, group, loader);
	freeSurf = true;
}

void SDLImage::load(SDL_Surface * surf, bool freeSurf /*= true*/)
{
	this->surf = surf;
	this->freeSurf = freeSurf;
}

void SDLImage::draw(TImagePtr where, int posX, int posY, Rect * src,  ui8 alpha) const
{
	if(!surf)
		return;

	Rect sourceRect(margins.x, margins.y, surf->w, surf->h);
	
	//TODO: rotation and scaling
	if (src)
	{
		sourceRect = sourceRect & *src;
	}

	Rect destRect(posX, posY, surf->w, surf->h);
	destRect += sourceRect.topLeft();
	sourceRect -= margins;
	CSDL_Ext::blitSurface(surf, &sourceRect, 
		dynamic_cast<SDLImage *>(where.get())->getSDL_Surface(), &destRect);
}

int SDLImage::width() const
{
	return fullSize.x;
}

int SDLImage::height() const
{
	return fullSize.y;
}

SDL_Surface * SDLImage::getSDL_Surface() const
{
	return surf;
}

void SDLImage::recolorToPlayer(int player)
{
	assert(0);
	//graphics->blueToPlayersAdv(surf, player);
}

void SDLImage::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{
	assert(0);
}

void SDLImage::rotate(EImageRotation::EImageRotation rotation)
{
	assert(0);
}

CompImageLoader::CompImageLoader(CompImage * Img) : image(Img), position(NULL), entry(NULL),
	currentLine(0)
{

}

void CompImageLoader::init(Point spriteSize, Point margins, Point fullSize, SDL_Color * pal)
{
	image->sprite = Rect(margins, spriteSize);
	image->fullSize = fullSize;
	if(spriteSize.x && spriteSize.y)
	{
		image->palette = new SDL_Color[256];
		memcpy(reinterpret_cast<void *>(image->palette), 
			reinterpret_cast<void *>(pal), 256 * sizeof(SDL_Color));
		
		// Allocate enough space for worst possible case
		// c-style malloc used due to resizing after load
		image->surf = reinterpret_cast<ui8 *>(malloc(spriteSize.x * spriteSize.y * 3));
		image->line = new ui32[spriteSize.y + 1];
		image->line[0] = 0;
		position = image->surf;
	}
}

void CompImageLoader::newEntry(ui8 color, size_t size)
{
	assert(color != 0xff);
	assert(size && size < 256);
	entry = position;
	entry[0] = color;
	entry[1] = size;
	position += 2;
}

void CompImageLoader::newEntry(const ui8 * & data, size_t size)
{
	assert(size && size < 256);
	entry = position;
	entry[0] = 0xff;
	entry[1] = size;
	position += 2;
	memcpy(position, data, size);
	position += size;
	data += size;
}

ui8 CompImageLoader::typeOf(ui8 color)
{
	if(color == 0)
		return 0;
	if(image->palette[color].unused != 255)
		return 1;
	return 2;
}

void CompImageLoader::load(size_t size, const ui8 * data)
{
	while(size)
	{
		// Try to compress data
		while(true)
		{
			ui8 color = data[0];
			if(color != 0xff)
			{
				size_t runLength = 1;
				while(runLength < size && color == data[runLength])
					runLength++;

				// Row of one color found - use RLE
				if(runLength > 1)
				{
					load(runLength, color);
					data += runLength;
					size -= runLength;
					if (!size)
						return;
				}
				else
					break;
			}
			else
				break;
		}
		// Select length for new raw entry
		size_t runLength = 1;
		ui8 color = data[0];
		ui8 type = typeOf(color);
		ui8 color2;
		ui8 type2;

		if(size > 1)
		{
			do
			{
				color2 = data[runLength];
				type2 = typeOf(color2);
				++runLength;
			}
			// While we have data of this type and different colors
			while((runLength < size) && (type == type2) && ( (color2 != 0xff) || (color2 != color)));
		}
		size -= runLength;

		// add data to last entry
		if(entry && entry[0] == 0xff && type == typeOf(entry[2]))
		{
			size_t toCopy = std::min<size_t>(runLength, 255 - entry[1]);
			runLength -= toCopy;
			entry[1] += toCopy;
			memcpy(position, data, toCopy);
			data += toCopy;
			position += toCopy;
		}
		//Create new entries
		while(runLength > 255)
		{
			newEntry(data, 255);
			runLength -= 255;
		}
		if (runLength)
			newEntry(data, runLength);
	}
}

void CompImageLoader::load(size_t size, ui8 color)
{
	if(!size)
		return;

	if(color == 0xff)
	{
		ui8 * tmpbuf = new ui8[size];
		memset(reinterpret_cast<void *>(tmpbuf), color, size);
		load(size, tmpbuf);
		delete [] tmpbuf;
		return;
	}
	//Current entry is RLE with same color as new block
	if(entry && entry[0] == color)
	{
		size_t toCopy = std::min<size_t>(size, 255 - entry[1]);
		entry[1] = 255;
		size -= toCopy;
		entry[1] += toCopy;
	}
	//Create new entries
	while(size > 255)
	{
		newEntry(color, 255);
		size -= 255;
	}
	if(size)
		newEntry(color, size);
}

void CompImageLoader::endLine()
{
	++currentLine;
	image->line[currentLine] = position - image->surf;
	entry = NULL;

}

CompImageLoader::~CompImageLoader()
{
	if(!image->surf)
		return;

	ui8 * newPtr = reinterpret_cast<ui8 *>(realloc(reinterpret_cast<void *>(image->surf), 
		position - image->surf));

	if (newPtr)
		image->surf = newPtr;
}

CompImage::CompImage() : surf(NULL), line(NULL),
	palette(NULL)
{
}

CompImage::~CompImage()
{
	free(surf);
	delete [] line;
	delete [] palette;
}

void CompImage::load(const CDefFile * defFile, size_t frame, size_t group)
{
	CompImageLoader loader(this);
	defFile->loadFrame(frame, group, loader);
}

void CompImage::load(TMemoryStreamPtr data, const std::string & imageType)
{
	// TODO
	assert(0);
}

void CompImage::draw(TImagePtr where, int posX, int posY, Rect * src, ui8 alpha /* =255*/) const
{
	int rotation = 0; //TODO
	//rotation & 2 = horizontal rotation
	//rotation & 4 = vertical rotation
	
	if(!surf)
		return;
	Rect sourceRect(sprite);
	
	//TODO: rotation and scaling
	if(src)
		sourceRect = sourceRect & *src;
	
	//Limit source rect to sizes of surface
	SDL_Surface * screen = dynamic_cast<SDLImage *>(where.get())->getSDL_Surface();
	sourceRect = sourceRect & Rect(0, 0, screen->w, screen->h);

	//Starting point on SDL surface
	Point dest(posX + sourceRect.x, posY + sourceRect.y);
	if(rotation & 2)
		dest.y += sourceRect.h;
	if(rotation & 4)
		dest.x += sourceRect.w;

	sourceRect -= sprite.topLeft();

	for(int currY = 0; currY < sourceRect.h; ++currY)
	{
		ui8 * data = surf + line[currY + sourceRect.y];
		ui8 type = *(data++);
		ui8 size = *(data++);
		int currX = sourceRect.x;

		//Skip blocks until starting position reached
		while(currX > size)
		{
			currX -= size;
			if (type == 0xff)
				data += size;
			type = *(data++);
			size = *(data++);
		}

		//This block will be shown partially - calculate size\position
		size -= currX;
		if(type == 0xff)
			data += currX;

		currX = 0;
		ui8 bpp = screen->format->BytesPerPixel;

		//Calculate position for blitting: pixels + Y + X
		ui8 * blitPos = reinterpret_cast<ui8 *>(screen->pixels);
		if(rotation & 4)
			blitPos += (dest.y - currY) * screen->pitch;
		else
			blitPos += (dest.y + currY) * screen->pitch;
		blitPos += dest.x * bpp;

		//Blit blocks that must be fully visible
		while(currX + size < sourceRect.w)
		{
			//blit block, pointers will be modified if needed
			blitBlockWithBpp(bpp, type, size, data, blitPos, alpha, rotation & 2);

			currX += size;
			type = *(data++);
			size = *(data++);
		}
		//Blit last, semi-visible block
		size = sourceRect.w - currX;
		blitBlockWithBpp(bpp, type, size, data, blitPos, alpha, rotation & 2);
	}
}

#define CASEBPP(x,y) case x: blitBlock<x,y>(type, size, data, dest, alpha); break

//FIXME: better way to get blitter
void CompImage::blitBlockWithBpp(ui8 bpp, ui8 type, ui8 size, ui8 *&data, ui8 *&dest, ui8 alpha, bool rotated) const
{
	assert(bpp > 1 && bpp < 5);

	if(rotated)
		switch(bpp)
	{
		CASEBPP(2,1);
		CASEBPP(3,1);
		CASEBPP(4,1);
	}
	else
		switch(bpp)
	{
		CASEBPP(2,1);
		CASEBPP(3,1);
		CASEBPP(4,1);
	}
}
#undef CASEBPP

//Blit one block from RLE-d surface
template<int bpp, int dir>
void CompImage::blitBlock(ui8 type, ui8 size, ui8 *&data, ui8 * & dest, ui8 alpha) const
{
	//Raw data
	if (type == 0xff)
	{
		ui8 color = *data;
		if (alpha != 255)//Per-surface alpha is set
		{
			for(size_t i = 0; i < size; ++i)
			{
				SDL_Color col = palette[*(data++)];
				col.unused = (ui32)col.unused * (255 - alpha) / 255;
				ColorPutter<bpp, 1>::PutColorAlpha(dest, col);
			}
			return;
		}

		if (palette[color].unused == 255)
		{
			//Put row of RGB data
			for(size_t i = 0; i < size; ++i)
				ColorPutter<bpp, 1>::PutColor(dest, palette[*(data++)]);
		}
		else
		{
			//Put row of RGBA data
			for(size_t i = 0; i < size; ++i)
				ColorPutter<bpp, 1>::PutColorAlpha(dest, palette[*(data++)]);

		}
	}
	//RLE-d sequence
	else
	{
		//Per-surface alpha is set
		if(alpha != 255 && palette[type].unused !=0)
		{
			SDL_Color col = palette[type];
			col.unused = (int)col.unused * (255 - alpha) / 255;
			for(size_t i = 0; i < size; ++i)
				ColorPutter<bpp, 1>::PutColorAlpha(dest, col);
			return;
		}

		switch(palette[type].unused)
		{
		case 0:
			{
				//Skip row
				dest += size*bpp;
				break;
			}
		case 255:
			{
				//Put RGB row
				ColorPutter<bpp, 1>::PutColorRow(dest, palette[type], size);
				break;
			}
		default:
			{
				//Put RGBA row
				for (size_t i=0; i<size; i++)
					ColorPutter<bpp, 1>::PutColorAlpha(dest, palette[type]);
				break;
			}
		}
	}
}

int CompImage::width() const
{
	return fullSize.x;
}

int CompImage::height() const
{
	return fullSize.y;
}

void CompImage::recolorToPlayer(int player)
{
	SDL_Color * pal = NULL;
	if(player < GameConstants::PLAYER_LIMIT && player >= 0)
	{
		pal = graphics->playerColorPalette + 32 * player;
	}
	else if(player == 255 || player == -1)
	{
		pal = graphics->neutralColorPalette;
	}
	else
		assert(0);

	for(size_t i = 0; i < 32; ++i)
	{
		palette[224+i].r = pal[i].r;
		palette[224+i].g = pal[i].g;
		palette[224+i].b = pal[i].b;
		palette[224+i].unused = pal[i].unused;
	}
}

void CompImage::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{
	assert(0);
}

void CompImage::rotate(EImageRotation::EImageRotation rotation)
{
	assert(0);
}