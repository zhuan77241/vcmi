#pragma once

#include "../lib/CFileSystemHandlerFwd.h"
#include "UIFramework/ImageClassesFwd.h"

/*
 * CResourceHandler.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CResourceHandler
{
	boost::unordered_map<ResourceLocator, weak_ptr<IImage> > images;

	TImagePtr loadImage(ResourceIdentifier identifier, const std::string & fileExt, bool fromBegin = false);

public:

	TImagePtr getImage(ResourceIdentifier ident, bool fromBegin = false);
};