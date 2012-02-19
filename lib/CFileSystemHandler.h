#pragma once

#include "CFileSystemHandlerFwd.h"

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
class DLL_LINKAGE CMemoryStream
{
private:
	ui8 * data;
	mutable size_t seekPos;
	size_t length;

public:
	CMemoryStream(ui8 * Data, const size_t Length) : data(Data), seekPos(0), length(Length) { };
	explicit CMemoryStream(const std::string & filePath);
	CMemoryStream(const CMemoryStream & cpy);
	CMemoryStream & operator=(const CMemoryStream & cpy);
	~CMemoryStream();

	bool moreBytesToRead() const;
	size_t getLength() const;
	void reset() const;
	size_t getSeekPos() const;
	void setSeekPos(size_t pos) const;
	void incSeekPos(size_t add) const;

	ui8 readInt8() const;
	ui16 readInt16() const;
	ui32 readInt32() const;
	
	// Gets raw ui8 pointer of data. Do not delete that data. Ownership belongs to CMemoryStream.
	ui8 * getRawData() const;
	ui8 * getRawData(size_t seekPos) const;
	std::string getDataAsString() const;
};

class DLL_LINKAGE CFileInfo
{
	// Original URI(not modified) e.g. ./dir/foo.txt
	std::string name;

	// Timestamp of file
	std::time_t date;

	// Exact location of the file, can be used to load files with filesystemhandler directly
	ResourceLocator locator;

public:
	std::string getName() const;
	
	// Path to file e.g. ./dir/subdir/
	std::string getPath() const;

	// File extension with dot e.g. '.BMP'
	std::string getExtension() const;
	
	// File name e.g. foo.txt
	std::string getFilename() const;

	// File name without extension e.g. foo
	std::string getStem() const;

	// Extension type as enum
	EResType::EResType getType() const;
	
	// Locator object for loading resource directly
	ResourceLocator getLocator() const;
	
	// Get timestamp from file
	std::time_t getDate() const;

	explicit CFileInfo(const std::string & Name, std::time_t Date = 0, 
		ResourceLocator Locator = ResourceLocator()) : name(Name), date(Date), locator(Locator) { };
};

// Interface for general resource loaders
class DLL_LINKAGE IResourceLoader
{
protected:
	// Prefix would be: DATA/... or SPRITES/... e.g. 
	// The last character has always to be a slash
	std::string prefix;

	// Folder name of the resource relative from VCMI_DIR 
	// e.g. ./Data/H3bitmap.lod for lod, ./Maps for filesystem(no ending slash)
	const std::string folder;

	explicit IResourceLoader(const std::string & Folder, const std::string & Prefix);

	// Adds a detected resource to the global map
	void addResourceToMap(TResourcesMap map, const ResourceIdentifier & identifier, const ResourceLocator & locator);

	void addEntryToMap(TResourcesMap & map, const std::string & name);

public:
	// Full folder/filesystem/virtual folder scan of loaded resource name
	virtual void insertEntriesIntoResourcesMap(TResourcesMap & map) =0;
	
	// Loads a resource with the given resource name
	virtual CMemoryStream * loadResource(const std::string & resourceName) =0;

	// Get timestamp from file
	virtual std::time_t getTimestampFromFile(const std::string & resourceName) const { return 0; }

	virtual ~IResourceLoader() { }

	std::string getFolder() const;
};

// Interface for general archive loaders
class DLL_LINKAGE IArchiveLoader : public IResourceLoader
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

	boost::unordered_map<std::string, const ArchiveEntry> entries;

	IArchiveLoader(const std::string & ArchiveFile, const std::string & Prefix) 
		: IResourceLoader(ArchiveFile, Prefix) { };
};

// Responsible for loading resources from lod archives
class DLL_LINKAGE CLodResourceLoader : public IArchiveLoader
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
	CMemoryStream * loadResource(const std::string & resourceName);
};

// Responsible for loading files from filesystem
class DLL_LINKAGE CFileResourceLoader : public IResourceLoader
{
public:
	CFileResourceLoader(const std::string & PathToFolder, const std::string & Prefix) 
		: IResourceLoader(PathToFolder, Prefix) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
	CMemoryStream * loadResource(const std::string & resourceName);
	std::time_t getTimestampFromFile(const std::string & resourceName) const;
};

class DLL_LINKAGE CMediaResourceHandler : public IArchiveLoader
{
protected:
	CMediaResourceHandler(const std::string & file, const std::string & prefix): IArchiveLoader(file, prefix) { };

public:
	CMemoryStream * loadResource(const std::string & resourceName);
};

// Responsible for loading sounds from snd archives
class DLL_LINKAGE CSoundResourceHandler : public CMediaResourceHandler
{
public:
	CSoundResourceHandler(const std::string & file, const std::string & prefix) : CMediaResourceHandler(file, prefix) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
};

// Responsible for loading videos from vid archives
class DLL_LINKAGE CVideoResourceHandler : public CMediaResourceHandler
{
public:
	CVideoResourceHandler(const std::string & file, const std::string & prefix) : CMediaResourceHandler(file, prefix) { };

	void insertEntriesIntoResourcesMap(TResourcesMap & map);
};

// Responsible for loading and handling binary resources
class DLL_LINKAGE CFileSystemHandler
{
	TResourcesMap resources;
	std::vector<IResourceLoader *> loaders;
	boost::mutex * mutex;

	// Get unpacked file with the given filepath
	CMemoryStream * getUnpackedFile(const std::string & path) const;

	// Get unpacked data from memory
	CMemoryStream * getUnpackedData(const CMemoryStream * memStream) const;

public:
	CFileSystemHandler();
	~CFileSystemHandler();

	// Add a resource handler to the filesystem which can be used load data from LOD, filesystem, ZIP,...
	void addHandler(IResourceLoader * resHandler);

	// Get a resource as a flat, binary stream(shared) with the given identifier and a flag, whether to load
	// resource from LOD/first loaded or last inserted resource
	CMemoryStream * getResource(const ResourceIdentifier & identifier, bool fromBegin = false, bool unpackResource = false);
	
	CMemoryStream * getResource(const ResourceLocator & locator, bool unpackResource = false);

	ResourceLocator getResourceLocator(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Get a resource as a string(not shared, can be easily altered) with the given identifier and a flag, 
	// whether to load resource from LOD/first loaded or last inserted resource
	std::string getResourceAsString(const ResourceIdentifier & identifier, bool fromBegin = false);

	// Gets a resource unpacked with the given identifier and a flag,
	// whether to load resource from LOD/first loaded or last inserted resource
	CMemoryStream * getUnpackedResource(const ResourceIdentifier & identifier, bool fromBegin = false);

	void writeMemoryStreamToFile(CMemoryStream * memStream, const std::string & destFile) const;

	// Get all files with given prefix and resource type.
	void getFilesWithExt(std::vector<CFileInfo> & out, const std::string & prefix, const EResType::EResType & type);

	const TResourcesMap & getResourcesMap() const;

	// Helper method: Converts a filename ext to EResType enum
	static EResType::EResType convertFileExtToResType(const std::string & fileExt);
};