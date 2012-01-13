#pragma once

#include "boost/unordered_map.hpp"

/*
 * CFileSystemHandler.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */

// This class wraps a raw char array. Can be used for reading from memory.
class CMemoryStream
{
private:
	ui8 * data;
	size_t seekPos;
	size_t length;

public:
	CMemoryStream(ui8 * Data, const size_t Length) : data(Data), seekPos(0), length(Length) { };
	CMemoryStream(const std::string & filePath);
	CMemoryStream(const CMemoryStream & cpy);
	CMemoryStream & operator=(const CMemoryStream & cpy);
	~CMemoryStream() { delete[] data; }

	bool moreBytesToRead() const { return seekPos < length; }
	void reset() { seekPos = 0; }
	ui8 readByte8();
	ui16 readByte16();
	ui32 readByte32();
	ui8 * getRawData();
	size_t getLength() const { return length; }
	std::string getDataAsString() const { std::string rslt(data, data + length); return rslt; }
	void writeToFile(const std::string & destFile) const;
};

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

// A simple struct which can identify several resources which belong together
struct ResourceIdentifier
{
	// Resource name (for example "DATA/MAINMENU")
	// No extension so .pcx and .png can override each other, always in upper case
	const std::string name;

	// resource type, (FILE_IMAGE), required to prevent conflicts if files with different type (text and image)
	// have same basename (we had this problem with garrison.txt and garrison.pcx in h3bitmap.lod)
	const EResType::EResType type;

	ResourceIdentifier(const std::string & Name, const EResType::EResType Type) 
		: name(Name), type(Type) { };

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

class IResourceLoader;

// A simple struct which represents the exact position of a resource
// Needed for loading resources
struct ResourceLocator
{
	// interface that does actual resource loading
	IResourceLoader * loader;

	// name of resource (e.g. garrison.txt or /Sprites/Bla.png)
	std::string resourceName;

	ResourceLocator() : loader(NULL), resourceName("") { };
	ResourceLocator(IResourceLoader * Loader, const std::string & ResourceName) 
		: loader(Loader), resourceName(ResourceName) { };

	inline bool operator==(ResourceLocator const & other) const
	{
		return loader == other.loader && resourceName == other.resourceName;
	}

	inline friend std::size_t hash_value(ResourceLocator const & p)
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, p.loader);
		boost::hash_combine(seed, p.resourceName);

		return seed;
	}
};

typedef boost::unordered_map<ResourceIdentifier, std::list<ResourceLocator> > TResourcesMap;
typedef shared_ptr<CMemoryStream> CMemoryStreamPtr;

// Interface for general resource loaders
class IResourceLoader
{
protected:
	// Prefix would be: DATA/... or SPRITES/... e.g.
	const std::string prefix;

	IResourceLoader(const std::string & Prefix) : prefix(Prefix) {  }

	// Adds a detected resource to the global map
	void addResourceToMap(TResourcesMap map, const ResourceIdentifier & identifier, const ResourceLocator & locator);

	void addEntryToMap(TResourcesMap & map, const std::string & name);

public:
	// Full folder/filesystem/virtual folder scan of loaded resource name
	// Should be overridden by sub-classes
	virtual void insertEntriesIntoResourcesMap(TResourcesMap & map);
	
	// Loads a resource with the given resource name
	// Should be overridden by sub-classes
	virtual CMemoryStreamPtr loadResource(const std::string & resourceName);

	virtual ~IResourceLoader() { }
};

// Interface for general archive loaders
class IArchiveLoader : public IResourceLoader
{
protected:
	struct ArchiveEntry
	{
		// Info extracted from archive(LOD,snd,..) file
		std::string name;

		// file type determined by extension
		EResType::EResType type;

		// Distance in bytes from beginning
		int offset;

		// Size without compression in bytes
		int realSize;

		// Size within compression in bytes
		int size;

		ArchiveEntry(const std::string & Name, EResType::EResType Type)
			: name(Name), type(Type), offset(0), realSize(0), size(0) {};
		ArchiveEntry(const std::string & Name): name(Name), type(EResType::OTHER), offset(0),
			realSize(0), size(0) {};
		ArchiveEntry() : name(""), type(EResType::OTHER), offset(0), realSize(0), size(0) {};
	};

	const std::string archiveFile;
	std::map<std::string, const ArchiveEntry> entries;

	IArchiveLoader(const std::string & ArchiveFile, const std::string & Prefix) 
		: IResourceLoader(Prefix), archiveFile(ArchiveFile) { };
};

// Responsible for loading resources from lod archives
class CLodResourceLoader : public IArchiveLoader
{	
	struct LodDecompressHelper
	{
		static const int DMHELP = 0;
		static const int DMNOEXTRACTINGMASK = 1;
		static const int FCHUNK = 50000;
	};
	
	bool decompressFile(ui8 * in, int size, int realSize, ui8 *& out, int wBits = 15);

public:
	CLodResourceLoader(const std::string & lodFile, const std::string & prefix) 
		: IArchiveLoader(lodFile, prefix) { }; 

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
	CMemoryStreamPtr loadResource(const std::string & resourceName);
};

// Responsible for loading files from filesystem
class CFileResourceLoader : public IResourceLoader
{
	const std::string pathToFolder;

public:
	CFileResourceLoader(const std::string & PathToFolder, const std::string & Prefix) 
		: IResourceLoader(Prefix), pathToFolder(PathToFolder) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
	CMemoryStreamPtr loadResource(const std::string & resourceName);
};

class CMediaResourceHandler : public IArchiveLoader
{
protected:
	CMediaResourceHandler(const std::string & file, const std::string & prefix): IArchiveLoader(file, prefix) { };

public:
	CMemoryStreamPtr loadResource(const std::string & resourceName);
};

// Responsible for loading sounds from snd archives
class CSoundResourceHandler : public CMediaResourceHandler
{
public:
	CSoundResourceHandler(const std::string & file, const std::string & prefix) : CMediaResourceHandler(file, prefix) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
};

// Responsible for loading videos from vid archives
class CVideoResourceHandler : public CMediaResourceHandler
{
public:
	CVideoResourceHandler(const std::string & file, const std::string & prefix) : CMediaResourceHandler(file, prefix) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
};

// Responsible for loading and handling binary resources
class CFileSystemHandler
{
	TResourcesMap resources;
	std::vector<IResourceLoader *> loaders;
	boost::mutex * mutex;

	boost::unordered_map<ResourceLocator, weak_ptr<CMemoryStream> > memoryStreams;

	CMemoryStreamPtr addResource(const ResourceLocator & locator, bool unpackResource = false);
	void addResourceToMap(const ResourceIdentifier & identifier, const ResourceLocator & locator);

	// Get unpacked file with the given filepath
	CMemoryStreamPtr getUnpackedFile(const std::string & path) const;

	// Get unpacked data from memory
	CMemoryStreamPtr getUnpackedData(CMemoryStreamPtr memStream) const;

public:
	~CFileSystemHandler();

	// Add a resource handler to the filesystem which can be used load data from LOD, filesystem, ZIP,...
	void addHandler(IResourceLoader * resHandler);

	// Get a resource as a flat, binary stream(shared) with the given identifier and a flag, whether to load
	// resource from LOD/first loaded or last inserted resource
	CMemoryStreamPtr getResource(const ResourceIdentifier & identifier, bool fromBegin = false, bool unpackResource = false);
	
	// Get a resource as a string(not shared, can be easily altered) with the given identifier and a flag, 
	// whether to load resource from LOD/first loaded or last inserted resource
	std::string getResourceAsString(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Gets a resource unpacked with the given identifier and a flag,
	// whether to load resource from LOD/first loaded or last inserted resource
	CMemoryStreamPtr getUnpackedResource(const ResourceIdentifier & identifier, bool fromBegin = false);

	void writeMemoryStreamToFile(CMemoryStreamPtr memStream, const std::string & destFile) const;

	// Helper method: Converts a filename ext to EResType enum
	static EResType::EResType convertFileExtToResType(const std::string & fileExt);
	
	// Helper method: Converts a resource name to uppercase and returns a pair of strings where the 1.value is
	// the raw resource name and the 2.value is the extension name (with preceding dot)
	static std::pair<std::string, std::string> adaptResourceName(const std::string & resName);
};