#include "StdInc.h"
#include "CFileSystemHandler.h"

#include <typeinfo>
#include "zlib.h"
#include "vcmi_endian.h"
#include "VCMIDirs.h"

ui8 CMemoryStream::readInt8() const
{
	assert(seekPos < length);
	
	ui8 rslt = *(data + seekPos);
	++seekPos;
	return rslt;
}

ui16 CMemoryStream::readInt16() const
{
	assert(seekPos < length - 1);
	
	ui16 rslt = read_le_u16(data + seekPos);
	seekPos += 2;
	return rslt;
}

ui32 CMemoryStream::readInt32() const
{
	assert(seekPos < length - 3);

	ui32 rslt = read_le_u32(data + seekPos);
	seekPos += 4;
	return rslt;
}

CMemoryStream::CMemoryStream(const std::string & filePath) : data(NULL), seekPos(0), length(0)
{
	std::ifstream fileInput(filePath.c_str(), std::ios::in | std::ios::binary);
	
	if (fileInput.is_open())
	{
		length = fileInput.tellg();
		data = new ui8[length];
		fileInput.read(reinterpret_cast<char *>(data), length);
		fileInput.close();
	}
	else
	{
		tlog1 << "File " << filePath << " doesn't exist." << std::endl;
	}
}

CMemoryStream::CMemoryStream(const CMemoryStream & cpy)
{
	*this = cpy;
}

CMemoryStream & CMemoryStream::operator=(const CMemoryStream & cpy)
{
	data = new ui8[cpy.length];
	memcpy(data, cpy.data, length);
	length = cpy.length;
	seekPos = cpy.seekPos;
	return *this;
}

CMemoryStream::~CMemoryStream()
{
	delete[] data;
}

ui8 * CMemoryStream::getRawData() const
{
	return data;
}

ui8 * CMemoryStream::getRawData(size_t seekPos) const
{
	return data + seekPos;
}

std::string CMemoryStream::getDataAsString() const
{
	std::string rslt(data, data + length); 
	return rslt;
}

void CMemoryStream::setSeekPos(size_t pos) const
{
	assert(pos < length);
	seekPos = pos;
}

void CMemoryStream::reset() const
{ 
	seekPos = 0; 
}

size_t CMemoryStream::getSeekPos() const 
{ 
	return seekPos; 
}

size_t CMemoryStream::getLength() const 
{ 
	return length; 
}

bool CMemoryStream::moreBytesToRead() const 
{ 
	return seekPos < length; 
}

void CMemoryStream::incSeekPos(size_t add) const
{
	seekPos += add;
}

std::string CFileInfo::getName() const
{
	return name;
}

std::string CFileInfo::getPath() const
{
	size_t found = name.find_last_of("/\\");
	return name.substr(0, found);
}

std::string CFileInfo::getExtension() const
{
	// Get position of file extension dot
	size_t dotPos = name.find_last_of("/.");

	if(dotPos != std::string::npos && name[dotPos] == '.')
		return name.substr(dotPos);
	else
		return "";
}

std::string CFileInfo::getFilename() const
{
	size_t found = name.find_last_of("/\\");
	return name.substr(found + 1);
}

std::string CFileInfo::getStem() const
{
	std::string rslt = name;

	// Remove file extension
	size_t dotPos = name.find_last_of("/.");

	if(dotPos != std::string::npos && name[dotPos] == '.')
		rslt.erase(dotPos);
	
	// Remove path
	size_t found = rslt.find_last_of("/\\");
	return rslt.substr(found + 1);
}

EResType::EResType CFileInfo::getType() const
{
	return CFileSystemHandler::convertFileExtToResType(getExtension());
}

std::time_t CFileInfo::getDate() const
{
	return date;
}

void IResourceLoader::addEntryToMap(TResourcesMap & map, const std::string & name)
{
	CFileInfo resData(name);
	ResourceIdentifier ident(prefix + resData.getStem(), resData.getType());
	ResourceLocator locator(this, name);
	map[ident].push_back(locator);
}

IResourceLoader::IResourceLoader(const std::string & Folder, const std::string & Prefix) : folder(Folder)
{
	assert(Prefix != "" && Folder != "");

	// Prefix should always end with a slash
	assert(Prefix[Prefix.length() - 1] == '/' && Folder[Folder.length() - 1] != GameConstants::PATH_SEPARATOR);

	prefix.resize(Prefix.length());
	std::transform(Prefix.begin(), Prefix.end(), prefix.begin(), toupper);
}

std::string IResourceLoader::getFolder() const
{
	return folder;
}

void CLodResourceLoader::insertEntriesIntoResourcesMap(TResourcesMap & map)
{
	// Open LOD file
	std::ifstream LOD;
	LOD.open(folder.c_str(), std::ios::in | std::ios::binary);

	if(!LOD.is_open()) 
	{
		tlog1 << "Cannot open " << folder << std::endl;
		return;
	}

	// Read count of total files
	ui32 temp;
	LOD.seekg(8);
	LOD.read((char *)&temp, 4);
	size_t totalFiles = SDL_SwapLE32(temp);

	// Check if LOD file is empty
	LOD.seekg(0x5c, std::ios::beg);
	if(!LOD)
	{
		tlog2 << folder << " doesn't store anything!\n";
		return;
	}

	// Define LodEntry struct
	struct LodEntryBlock {
		char filename[16];
		ui32 offset;				/* little endian */
		ui32 uncompressedSize;	/* little endian */
		ui32 unused;				/* little endian */
		ui32 size;				/* little endian */
	};

	// Allocate a LodEntry array and load data into it
	struct LodEntryBlock * lodEntries = new struct LodEntryBlock[totalFiles];
	LOD.read(reinterpret_cast<char *>(lodEntries), sizeof(struct LodEntryBlock) * totalFiles);

	// Insert all lod entries to a vector
	for(size_t i = 0; i < totalFiles; i++)
	{
		// Create lod entry with correct name, converted file ext, offset, size,...
		ArchiveEntry entry;
		CFileInfo lodName(lodEntries[i].filename);

		entry.name = lodName.getStem();
		entry.type = lodName.getType();
		entry.offset= SDL_SwapLE32(lodEntries[i].offset);
		entry.realSize = SDL_SwapLE32(lodEntries[i].uncompressedSize);
		entry.size = SDL_SwapLE32(lodEntries[i].size);
		
		// Add lod entry to local entries map
		entries.insert(std::make_pair(lodEntries[i].filename, entry));

		// Add resource locator to global map
		ResourceIdentifier mapIdent(prefix + entry.name, entry.type);
		ResourceLocator locator(this, lodEntries[i].filename);

		map[mapIdent].push_back(locator);
	}
	
	// Delete LodEntry array
	delete [] lodEntries;
}

CMemoryStream * CLodResourceLoader::loadResource(const std::string & resourceName)
{
	assert(entries.find(resourceName) != entries.end());

	const ArchiveEntry entry = entries[resourceName];

	ui8 * outp;
	CMemoryStream * rslt;

	std::ifstream LOD;
	LOD.open(folder.c_str(), std::ios::in | std::ios::binary);
	
	//file is not compressed
	if (entry.size == 0) 
	{
		outp = new ui8[entry.realSize];

		LOD.seekg(entry.offset, std::ios::beg);
		LOD.read(reinterpret_cast<char *>(outp), entry.realSize);
		rslt = new CMemoryStream(outp, entry.realSize);
		return rslt;
	}
	//we will decompress file
	else 
	{
		outp = new ui8[entry.size];

		LOD.seekg(entry.offset, std::ios::beg);
		LOD.read(reinterpret_cast<char *>(outp), entry.size);
		ui8 * decomp = NULL;

		if (decompressFile(outp, entry.size, entry.realSize, decomp))
		{
			tlog1 << "File decompression wasn't successful. Resource name: " << resourceName << std::endl;
		}
		delete[] outp;

		rslt = new CMemoryStream(decomp, entry.realSize);
		return rslt;
	}

	return rslt;
}

bool CLodResourceLoader::decompressFile(ui8 * in, int size, int realSize, ui8 *& out, int wBits)
{
	int ret;
	unsigned have;
	z_stream strm;
	out = new ui8 [realSize];
	int latPosOut = 0;

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit2(&strm, wBits);
	if (ret != Z_OK)
		return ret;
	int chunkNumber = 0;
	do
	{
		if(size < chunkNumber * LodDecompressHelper::FCHUNK)
			break;
		strm.avail_in = std::min(LodDecompressHelper::FCHUNK, size - chunkNumber * LodDecompressHelper::FCHUNK);
		if (strm.avail_in == 0)
			break;
		strm.next_in = in + chunkNumber * LodDecompressHelper::FCHUNK;

		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = realSize - latPosOut;
			strm.next_out = out + latPosOut;
			ret = inflate(&strm, Z_NO_FLUSH);
			//assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
			bool breakLoop = false;
			switch (ret)
			{
			case Z_STREAM_END:
				breakLoop = true;
				break;
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;	 /* and fall through */
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				(void)inflateEnd(&strm);
				return ret;
			}

			if(breakLoop)
				break;

			have = realSize - latPosOut - strm.avail_out;
			latPosOut += have;
		} while (strm.avail_out == 0);

		++chunkNumber;
		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

void CFileResourceLoader::insertEntriesIntoResourcesMap(TResourcesMap & map)
{
	boost::filesystem::recursive_directory_iterator enddir;
	if(boost::filesystem::exists(folder))
	{
		std::vector<std::string> path;
		for (boost::filesystem::recursive_directory_iterator dir(folder); dir!=enddir; dir++)
		{
			//If a directory was found - add name to vector to recreate full path later
			if (boost::filesystem::is_directory(dir->status()))
			{
				path.resize(dir.level() + 1);
				path.back() = dir->path().leaf();
			}
			if(boost::filesystem::is_regular(dir->status()))
			{
				//we can't get relative path with boost at the moment - need to create path to file manually
				std::string relativePath, name, ext;
				for (size_t i=0; i<dir.level() && i<path.size(); i++)
					relativePath += path[i] + '/';

				relativePath += dir->path().leaf();
				addEntryToMap(map, relativePath);
			}
		}
	}
	else
	{
		if(!folder.empty())
			tlog1 << "Warning: No " + folder + " folder!" << std::endl;
	}
}

CMemoryStream * CFileResourceLoader::loadResource(const std::string & resourceName)
{
	CMemoryStream * rslt = new CMemoryStream(resourceName);
	if(rslt->getLength() == 0)
		rslt = NULL;

	return rslt;
}

std::time_t CFileResourceLoader::getTimestampFromFile(const std::string & resourceName) const
{
	return boost::filesystem::last_write_time(folder + GameConstants::PATH_SEPARATOR + resourceName);
}

void CSoundResourceHandler::insertEntriesIntoResourcesMap(TResourcesMap & map)
{
	std::ifstream fileHandle(folder.c_str(), std::ios::in | std::ios::binary);
	if (!fileHandle.good())
	{
		tlog1 << "File " << folder << " couldn't be opened" << std::endl;
		return;
	}

	ui32 temp;
	fileHandle.read((char *)&temp, 4);
	size_t totalFiles = SDL_SwapLE32(temp);

	struct SoundEntryBlock
	{
		char filename[40];
		Uint32 offset;				/* little endian */
		Uint32 size;				/* little endian */
	};

	fileHandle.seekg(4);
	struct SoundEntryBlock * sndEntries = new struct SoundEntryBlock[totalFiles];
	fileHandle.read(reinterpret_cast<char *>(sndEntries), sizeof(struct SoundEntryBlock) * totalFiles);

	for (size_t i = 0; i < totalFiles; i++)
	{
		SoundEntryBlock sndEntry = sndEntries[i];
		ArchiveEntry entry;

		entry.name = sndEntry.filename;
		entry.offset = SDL_SwapLE32(sndEntry.offset);
		entry.size = SDL_SwapLE32(sndEntry.size);

		entries.insert(std::make_pair(entry.name, entry));

		addEntryToMap(map, entry.name);
	}

	delete[] sndEntries;
	fileHandle.close();
}

void CVideoResourceHandler::insertEntriesIntoResourcesMap(TResourcesMap & map)
{
	// Open archive file
	std::ifstream fileHandle(folder.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
	if (!fileHandle.good())
	{
		tlog1 << "File " << folder << " couldn't be opened" << std::endl;
		return;
	}
	
	// Check if file size is greater than 48 bytes
	size_t fileSize = fileHandle.tellg();
	if(fileSize < 48)
	{
		tlog1 << folder << " doesn't contain needed data!\n";
		return;
	}
	fileHandle.seekg(0);
	
	ui32 temp;
	fileHandle.read((char *)&temp, 4);
	size_t totalFiles = SDL_SwapLE32(temp);
	
	// video entry structure in catalog
	struct VideoEntryBlock
	{
		char filename[40];
		Uint32 offset;		/* little endian */
	};
	
	fileHandle.seekg(4);
	struct VideoEntryBlock * vidEntries = new struct VideoEntryBlock[totalFiles];
	fileHandle.read(reinterpret_cast<char *>(vidEntries), sizeof(struct VideoEntryBlock) * totalFiles);

	for(size_t i = 0; i < totalFiles; i++)
	{
		VideoEntryBlock vidEntry = vidEntries[i];
		ArchiveEntry entry;

		entry.name = vidEntry.filename;
		entry.offset = SDL_SwapLE32(vidEntry.offset);

		// There is no size, so check where the next file is
		if (i == totalFiles - 1) 
			entry.size = fileSize - entry.offset;
		else 
		{
			VideoEntryBlock ve_next = vidEntries[i + 1];
			entry.size = SDL_SwapLE32(ve_next.offset) - entry.offset;
		}

		entries.insert(std::make_pair(entry.name, entry));
		
		addEntryToMap(map, entry.name);
	}
}

CMemoryStream * CMediaResourceHandler::loadResource(const std::string & resourceName)
{
	assert(entries.find(resourceName) != entries.end());

	const ArchiveEntry entry = entries[resourceName];

	CMemoryStream * rslt = NULL;

	std::ifstream fileHandle;
	fileHandle.open(folder.c_str(), std::ios::in | std::ios::binary);
	if (!fileHandle.good())
	{
		tlog1 << "Archive file " << folder << " is corrupt." << std::endl;
		return rslt;
	}
	
	fileHandle.seekg(entry.offset);
	ui8 * outp = new ui8[entry.size];
	fileHandle.read(reinterpret_cast<char *>(outp), entry.size);
	fileHandle.close();

	rslt = new CMemoryStream(outp, entry.size);
	return rslt;
}

CFileSystemHandler::CFileSystemHandler()
{
	mutex = new boost::mutex;
}

CFileSystemHandler::~CFileSystemHandler()
{
	for (size_t i = 0; i < loaders.size(); ++i)
		delete loaders[i];

	delete mutex;
}

void CFileSystemHandler::addHandler(IResourceLoader * resHandler)
{
	loaders.push_back(resHandler);
	resHandler->insertEntriesIntoResourcesMap(resources);
}

EResType::EResType CFileSystemHandler::convertFileExtToResType(const std::string & fileExt)
{
	// Create convert map statically once
	using namespace EResType;
	using namespace boost::assign;

	static const std::map<std::string, ::EResType::EResType> extMap = map_list_of("TXT", TEXT)
		(".JSON", TEXT)(".DEF", ANIMATION)(".MSK", MASK)(".MSG", MASK)
		(".H3C", CAMPAIGN)(".H3M", MAP)(".FNT", FONT)(".BMP", GRAPHICS)
		(".JPG", GRAPHICS)(".PCX", GRAPHICS)(".PNG", GRAPHICS)(".TGA", GRAPHICS)
		(".WAV", SOUND)(".SMK", VIDEO)(".BIK", VIDEO)(".VLGM1", SAVEGAME);

	// Convert file ext(string) to resource type(enum)
	std::string fileExtUpper = fileExt;
	std::transform(fileExt.begin(), fileExt.end(), fileExtUpper.begin(), toupper);
	std::map<std::string, ::EResType::EResType>::const_iterator it = extMap.find(fileExtUpper);
	if(it == extMap.end())
		return OTHER;
	else
		return it->second;
}

CMemoryStream * CFileSystemHandler::getResource(const ResourceIdentifier & identifier, bool fromBegin /*=false */, bool unpackResource /*=false */)
{
	ResourceLocator loc = getResourceLocator(identifier, fromBegin);
	return getResource(loc, unpackResource);	
}

CMemoryStream * CFileSystemHandler::getResource(const ResourceLocator & locator, bool unpackResource /*= false*/)
{
	CMemoryStream * rslt = NULL;

	// load it
	mutex->lock();

	rslt = locator.loader->loadResource(locator.resourceName);
	if (unpackResource)
		rslt = getUnpackedData(rslt);

	mutex->unlock();
	return rslt;
}

ResourceLocator CFileSystemHandler::getResourceLocator(const ResourceIdentifier & identifier, bool fromBegin /*= false*/)
{
	// check if resource is registered
	if(resources.find(identifier) == resources.end())
	{
		tlog2 << "Resource with name " << identifier.name << " and type " 
			<< identifier.type << " wasn't found." << std::endl;
		return ResourceLocator();
	}

	// get former/origin resource e.g. from lod with fromBegin=true 
	// and get the latest inserted resource with fromBegin=false
	std::list<ResourceLocator> locators = resources.at(identifier);
	ResourceLocator loc;
	if (!fromBegin)
		return locators.back();
	else 
		return locators.front();
}

std::string CFileSystemHandler::getResourceAsString(const ResourceIdentifier & identifier, bool fromBegin /*=false */)
{
	CMemoryStream * memStream = getResource(identifier, fromBegin);
	return memStream->getDataAsString();
}

CMemoryStream * CFileSystemHandler::getUnpackedResource(const ResourceIdentifier & identifier, bool fromBegin /*=false */)
{
	return getResource(identifier, fromBegin, true);
}

//It is possible to use uncompress function from zlib but we  need to know decompressed size (not present in compressed data)
CMemoryStream * CFileSystemHandler::getUnpackedData(const CMemoryStream * memStream) const
{
	std::string filename = GVCMIDirs.UserPath + "/tmp_gzip";

	std::ofstream file(filename.c_str(), std::ios_base::binary);
	file.write(reinterpret_cast<char *>(memStream->getRawData()), memStream->getLength());
	file.close();

	CMemoryStream * ret = getUnpackedFile(filename);
	remove(filename.c_str());
	return ret;
}

CMemoryStream * CFileSystemHandler::getUnpackedFile(const std::string & path) const
{
	const int bufsize = 65536;
	int mapsize = 0;

	gzFile map = gzopen(path.c_str(), "rb");
	assert(map);
	std::vector<ui8 *> mapstr;

	// Read a map by chunks
	// We could try to read the map size directly (cf RFC 1952) and then read
	// directly the whole map, but that would create more problems.
	do 
	{
		ui8 * buf = new ui8[bufsize];

		int ret = gzread(map, buf, bufsize);
		if(ret == 0 || ret == -1) 
		{
			delete [] buf;
			break;
		}

		mapstr.push_back(buf);
		mapsize += ret;
	} while(1);

	gzclose(map);

	// Now that we know the uncompressed size, reassemble the chunks
	ui8 * initTable = new ui8[mapsize];

	std::vector<ui8 *>::iterator it;
	int offset;
	int tocopy = mapsize;
	for(it = mapstr.begin(), offset = 0; it != mapstr.end(); it++, offset += bufsize ) 
	{
		memcpy(&initTable[offset], *it, tocopy > bufsize ? bufsize : tocopy);
		tocopy -= bufsize;
		delete [] *it;
	}
	
	return new CMemoryStream(initTable, mapsize);
}

void CFileSystemHandler::writeMemoryStreamToFile(CMemoryStream * memStream, const std::string & destFile) const
{
	std::ofstream out(destFile.c_str(),std::ios_base::binary);
	out.write(reinterpret_cast<char *>(memStream->getRawData()), memStream->getLength());
	out.close();
}

const TResourcesMap & CFileSystemHandler::getResourcesMap() const
{
	return resources;
}

void CFileSystemHandler::getFilesWithExt(std::vector<CFileInfo> & out, const std::string & prefix, const EResType::EResType & type)
{
	namespace fs = boost::filesystem;

	for(TResourcesMap::iterator it = resources.begin(); it != resources.end(); ++it)
	{
		// Get all keys with the given prefix and resource type
		std::pair<ResourceIdentifier, std::list<ResourceLocator> > key = *it;
		ResourceIdentifier ident = key.first;
		size_t found = ident.name.find_last_of("/");
		std::string identPrefix = ident.name.substr(0, found + 1);
		std::string prefixUpper = prefix;
		std::transform(prefix.begin(), prefix.end(), prefixUpper.begin(), toupper);
		if(ident.type == type && identPrefix == prefixUpper)
		{
			std::list<ResourceLocator> locatorList = key.second;
			
			for(std::list<ResourceLocator>::iterator it2 = locatorList.begin(); it2 != locatorList.end(); ++it2)
			{
				ResourceLocator locator = *it2;
				
				time_t date = locator.loader->getTimestampFromFile(locator.resourceName);
				std::string file = locator.loader->getFolder() + GameConstants::PATH_SEPARATOR 
					+ locator.resourceName;
				
				CFileInfo fileInfo(file, date, locator);
				out.push_back(fileInfo);
			}
		}
	}
}