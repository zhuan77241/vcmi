#pragma once

/*
 * CFileSystemHandlerFwd.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

class CMemoryStream;
struct ResourceIdentifier;
struct ResourceLocator;
class IResourceLoader;
class IArchiveLoader;
class CLodResourceLoader;
class CFileResourceLoader;
class CMediaResourceHandler;
class CSoundResourceHandler;
class CVideoResourceHandler;
class CFileSystemHandler;

typedef boost::unordered_map<ResourceIdentifier, std::list<ResourceLocator> > TResourcesMap;
typedef shared_ptr<CMemoryStream> TMemoryStreamPtr;

// Specifies the resource type
namespace EResType
{
	enum EResType
	{
		ANY,
		TEXT,
		ANIMATION,
		MASK,
		CAMPAIGN,
		MAP,
		FONT,
		GRAPHICS,
		VIDEO,
		SOUND,
		OTHER
	};
};
