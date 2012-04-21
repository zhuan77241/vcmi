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
		SAVEGAME,
		OTHER
	};
};


// A simple struct which can identify several resources which belong together
// It has to reside here because key objects in a boost hashtable need to be defined first before used.
struct DLL_LINKAGE ResourceIdentifier
{
	// Resource name (for example "DATA/MAINMENU")
	// No extension so .pcx and .png can override each other, always in upper case
	std::string name;

	// resource type, (FILE_IMAGE), required to prevent conflicts if files with different type (text and image)
	// have same basename (we had this problem with garrison.txt and garrison.pcx in h3bitmap.lod)
	const EResType::EResType type;

	ResourceIdentifier(const std::string & Name, const EResType::EResType Type) : type(Type)
	{
		name.resize(Name.length());
		std::transform(Name.begin(), Name.end(), name.begin(), toupper);
	}

	inline bool operator==(ResourceIdentifier const & other) const
	{
		return name == other.name && type == other.type;
	}

	inline friend std::size_t hash_value(ResourceIdentifier const & p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.name);
		boost::hash_combine(seed, p.type);

		return seed;
	}
}; 

// A simple struct which represents the exact position of a resource
// Needed for loading resources
// It has to reside here because key objects in a boost hashtable need to be defined first before used.
struct DLL_LINKAGE ResourceLocator
{
	// interface that does actual resource loading
	IResourceLoader * loader;

	// name of resource (e.g. garrison.txt or /Sprites/Bla.png)
	std::string resourceName;

	ResourceLocator() : loader(NULL), resourceName("") { };
	ResourceLocator(IResourceLoader * Loader, const std::string & ResourceName) 
		: loader(Loader), resourceName(ResourceName) { };

	bool isEmpty() const { return loader == NULL || resourceName == ""; }
};
