#include "StdInc.h"
#include "ImageClasses.h"
#include "SDL_Extensions.h"

IImage * IImage::createInstance(TMemoryStreamPtr data, const std::string &imageType, bool useComp /*= false*/)
{
	IImage * rslt = NULL;
	// TODO: construction of HW-accelerated images(OpenGL)

	if (useComp)
	{
		// TODO: Comp image
	}
	else
	{
		rslt = new SDLImage();
	}

	rslt->load(data, imageType);
	return rslt;
}

SDLImage::SDLImage() : surf(NULL)
{

}

SDLImage::~SDLImage()
{
	SDL_FreeSurface(surf);
}

void SDLImage::load(TMemoryStreamPtr data, const std::string &imageType)
{
	surf = CSDL_Ext::loadImage(data, imageType);
}

void SDLImage::draw(IImage * where, int posX, int posY, Rect * src,  ui8 alpha) const
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
	CSDL_Ext::blitSurface(surf, &sourceRect, dynamic_cast<SDLImage *>(where)->surf, &destRect);
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

}

void SDLImage::setGlowAnimation(EGlowAnimationType::EGlowAnimationType glowType, ui8 alpha)
{

}

void SDLImage::rotate(EImageRotation::EImageRotation rotation)
{

}