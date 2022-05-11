#include "Afs2File.h"
#include "CpkFile.h"
#include "HcaFile.h"
#include "debug.h"

#define COPY_BUF_SIZE	(16*1024*1024)

Afs2File::Afs2File()
{
	big_endian = false;
	r_handle = nullptr;
	
	version[0] = 1;
	version[1] = 4;
	version[2] = 2;
	version[3] = 0;
	
	alignment = 0x20;
}

Afs2File::~Afs2File()
{
	Reset();
}

void Afs2File::Reset()
{
	if (r_handle)
	{
		fclose(r_handle);
		r_handle = nullptr;
	}
	
	memset(version, 0, sizeof(version));
	alignment = 0;
	
	files.clear();
}

void Afs2File::GetOffsetsAndSizes(const void *buf, std::vector<uint32_t> &offsets, std::vector<uint32_t> &sizes) const
{
	uint32_t *ofs32 = (uint32_t *)buf;
	uint16_t *ofs16 = (uint16_t *)buf;
	
	assert(offsets.size() == sizes.size());
	
	for (size_t i = 0; i < offsets.size(); i++)
	{
		if (version[1] == 2)
		{
			offsets[i] = ofs16[i];
		}
		else
		{
			offsets[i] = ofs32[i];
		}
		
		if ((offsets[i] % alignment) != 0)
		{
			offsets[i] += (alignment - (offsets[i] % alignment));
		}
		
		if (version[1] == 2)
		{
			sizes[i] = ofs16[i+1] - offsets[i];
		}
		else
		{
			sizes[i] = ofs32[i+1] - offsets[i];
		}		
	}
}

uint32_t Afs2File::CalculateOffsetsSize()
{
	uint32_t offsets_size;
	
	if (version[1] != 2)
	{
        return ((uint32_t)files.size()+1) * sizeof(uint32_t);
	}
		
    offsets_size = ((uint32_t)files.size()+1) * sizeof(uint16_t);
	
	for (uint32_t i = 0, old_pos = 0; i <= files.size(); i++) // Yes <=, as n+1 entries are written
	{
		uint32_t pos;
		
		if (i == 0)
		{
            pos = sizeof(AFS2Header) + ((uint32_t)files.size()*2) + offsets_size;
		}
		else
		{
			uint32_t size;
			
			if ((old_pos % alignment) != 0)
			{
				old_pos += (alignment - (old_pos % alignment));
			}			
			
			if (!GetEntrySize(files[i-1], &size))
			{
				return (uint32_t)-1;
			}
			
			pos = old_pos + size;
		}		
		
		if (pos >= 0x10000)
		{
			// This format can't hold this file, let's switch to the other one
			version[0] = 1;
			version[1] = 4;
			version[2] = 2;
			version[3] = 0;
			return CalculateOffsetsSize();
		}

		old_pos = pos;		
	}
	
	return offsets_size;
}

uint32_t Afs2File::CalculateHeaderSize()
{
	uint32_t offsets_size = CalculateOffsetsSize();
	
	if (offsets_size == (uint32_t)-1)
		return offsets_size;
	
    return sizeof(AFS2Header) + ((uint32_t)files.size() * sizeof(uint16_t)) + offsets_size;
}

uint32_t Afs2File::CalculateFileSize()
{
	uint32_t file_size = CalculateHeaderSize();
	
	if (file_size == (uint32_t)-1)
		return file_size;
	
	for (Afs2Entry &entry : files)
	{
		if ((file_size % alignment) != 0)
		{
			file_size += alignment - (file_size % alignment);
		}
		
		if (!GetEntrySize(entry, &entry.size))
			return (uint32_t)-1;
		
		file_size += entry.size;
	}
	
	return file_size;
}

void *Afs2File::CreateOffsetsSection(uint32_t *offsets_size)
{
	void *buf;
	uint32_t *offsets32;
	uint16_t *offsets16;
	
	if (version[1] == 2)
	{
        *offsets_size = ((uint32_t)files.size()+1) * sizeof(uint16_t);
	}
	else
	{
        *offsets_size = ((uint32_t)files.size()+1) * sizeof(uint32_t);
	}
	
	buf = malloc(*offsets_size);
	if (!buf)
	{
		DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, *offsets_size);
		return nullptr;
	}
	
	offsets32 = (uint32_t *)buf;
	offsets16 = (uint16_t *)buf;
	
	for (uint32_t i = 0, old_pos = 0; i <= files.size(); i++) // Yes <=, as n+1 entries are written
	{
		uint32_t pos;
		
		if (i == 0)
		{
            pos = sizeof(AFS2Header) + ((uint32_t)files.size()*2) + *offsets_size;
		}
		else
		{
			uint32_t size;
			
			if ((old_pos % alignment) != 0)
			{
				old_pos += (alignment - (old_pos % alignment));
			}			
			
			if (!GetEntrySize(files[i-1], &size))
			{
				free(buf);
				return nullptr;
			}
			
			pos = old_pos + size;			
		}
						
		if (version[1] == 2)
		{
			if (pos >= 0x10000)
			{
				// This format can't hold this file, let's switch to the other one
				version[0] = 1;
				version[1] = 4;
				version[2] = 2;
				version[3] = 0;
				free(buf);
				return CreateOffsetsSection(offsets_size);
			}
			
			offsets16[i] = (uint16_t)pos;
		}		
		else
		{
			offsets32[i] = pos;
		}
		
		old_pos = pos;
	}
	
	return buf;
}

bool Afs2File::GetEntrySize(const Afs2Entry &entry, uint32_t *psize)
{
	if (entry.offset != (uint32_t)-1 || entry.buf)
	{
		*psize = entry.size;
		return true;
	}
	
	size_t size = Utils::GetFileSize(entry.path);
	if (size == (size_t)-1)
	{
		DPRINTF("%s: Cannot stat file \"%s\"\n", FUNCNAME, entry.path.c_str());
		return false;
	}
	
	*psize = (uint32_t)size;
	return true;
}

void Afs2File::PadFile(FILE *file)
{
	uint32_t pos = (uint32_t)ftell(file);
	
	if ((pos % alignment) != 0)
	{
		uint32_t num_pad = alignment - (pos % alignment);
		
		for (uint32_t i = 0; i < num_pad; i++)
		{
			fputc(0, file);
		}
    }
}

bool Afs2File::Load(const uint8_t *buf, size_t size)
{
	AFS2Header *header;
	unsigned int offsets_size, total_header_size;
	std::vector<uint32_t> offsets;
	std::vector<uint32_t> sizes;
	
	Reset();
	
	header = (AFS2Header *)buf;
	if (size < sizeof(AFS2Header) || header->signature != AFS2_SIGNATURE)
    {
        return false;
    }
	
	memcpy(version, header->version, sizeof(version));
	alignment = header->alignment;
	
	if (version[1] == 2)
	{
		offsets_size = (header->num_files+1) * sizeof(uint16_t);
	}
	else
	{
		offsets_size = (header->num_files+1) * sizeof(uint32_t);
	}
	
	total_header_size = sizeof(AFS2Header) + (header->num_files*2) + offsets_size;
	if (size < total_header_size)
	{
		Reset();
		return false;
	}
	
	offsets.resize(header->num_files);
	sizes.resize(header->num_files);
	
	GetOffsetsAndSizes(buf + total_header_size - offsets_size, offsets, sizes);	
	files.resize(header->num_files);
	
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i].offset = -1;
		files[i].size = sizes[i];		
        files[i].buf = new uint8_t[sizes[i]];
		
		if (offsets[i]+sizes[i] > size)
		{
			DPRINTF("%s: Memory out of bounds.\n", FUNCNAME);
			Reset();
			return false;
		}
		
		memcpy(files[i].buf, buf+offsets[i], sizes[i]);
	}
	
	return true;
}

bool Afs2File::LoadFromFile(const std::string &path, bool show_error)
{
	AFS2Header header;
	unsigned int offsets_size;
	void *buf;
	std::vector<uint32_t> offsets;
	std::vector<uint32_t> sizes;
	
	Reset();
	
	r_handle = fopen(path.c_str(), "rb");
	if (!r_handle)
	{
		if (show_error)
        {
            DPRINTF("%s: Cannot open file \"%s\"\n", FUNCNAME, path.c_str());
		}
		
		return false;
	}
	
	if (fread(&header, 1, sizeof(header), r_handle) != sizeof(header) || header.signature != AFS2_SIGNATURE)
	{
		if (show_error)
        {
            DPRINTF("%s: This file is not a valid afs2 file.\n", FUNCNAME);
		}
		
		Reset();
		return false;
	}
	
	memcpy(version, header.version, sizeof(version));
	alignment = header.alignment;
	
	if (version[1] == 2)
	{
		offsets_size = (header.num_files+1) * sizeof(uint16_t);
	}
	else
	{
		offsets_size = (header.num_files+1) * sizeof(uint32_t);
	}
	
	buf = malloc(offsets_size);
	if (!buf)
	{
		DPRINTF("%s: Memory allocation error (0x%x)\n", FUNCNAME, offsets_size);
		Reset();
		return false;
	}
	
	fseek(r_handle, header.num_files*2, SEEK_CUR);	
	
	if (fread(buf, 1, offsets_size, r_handle) != offsets_size)
	{
		if (show_error)
		{
			DPRINTF("%s: This afs2 file doesn't have enough data.\n", FUNCNAME);
		}
		
		free(buf);
		Reset();
		return false;
	}
	
	offsets.resize(header.num_files);
	sizes.resize(header.num_files);
	
	GetOffsetsAndSizes(buf, offsets, sizes);
	free(buf);
	
	files.resize(header.num_files);
	
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i].offset = offsets[i];
		files[i].size = sizes[i];
	}
	
	return true;
}

uint8_t *Afs2File::Save(size_t *psize)
{
	uint8_t *buf;
	uint32_t file_size, offset;
	void *p_offsets;
	unsigned int offsets_size;
	
	file_size = CalculateFileSize();
	if (file_size == (uint32_t)-1)
		return nullptr;
	
	p_offsets = CreateOffsetsSection(&offsets_size);
	if (!p_offsets)
	{
		return nullptr;
	}
	
    buf = new uint8_t[file_size];
    memset(buf, 0, file_size);
	
	AFS2Header *header = (AFS2Header *)buf;
	
	header->signature = AFS2_SIGNATURE;
	memcpy(header->version, version, sizeof(version));
    header->num_files = (uint32_t)files.size();
	header->alignment = alignment;	
	
	offset = sizeof(AFS2Header);
	uint16_t *idx = (uint16_t *)(buf + offset);

    assert(files.size() < 0x10000);
	
	for (size_t i = 0; i < files.size(); i++)
	{
        idx[i] = (uint16_t)i;
		offset += sizeof(uint16_t);
	}
	
	memcpy(buf+offset, p_offsets, offsets_size);
	free(p_offsets);
	p_offsets = nullptr;
	
	offset += offsets_size;
	
	for (Afs2Entry &entry : files)
	{
		if ((offset % alignment) != 0)
		{
			offset += alignment - (offset % alignment);
		}
		
		if (!GetEntrySize(entry, &entry.size))
		{
			delete[] buf;
			return nullptr;
		}
		
		if (entry.offset != (uint32_t)-1)
		{
			// Internal file in r_handle
			
			assert(r_handle != nullptr);			
			fseek(r_handle, entry.offset, SEEK_SET);	
			
            if (!DoCopyFile(r_handle, buf+offset, entry.size))
			{
                DPRINTF("%s: DoCopyFile file mem failed.\n", FUNCNAME);
				
				delete[] buf;
				return nullptr;
			}
		}
		else if (entry.buf)
		{
			// Memory file
            if (!DoCopyFile(entry.buf, buf+offset, entry.size))
			{
                DPRINTF("%s: DoCopyFile mem mem failed.\n", FUNCNAME);
				
				delete[] buf;
				return nullptr;
			}
		}
		else
		{
			// External file
			FILE *external = fopen(entry.path.c_str(), "rb");
			if (!external)
			{
				DPRINTF("%s: Failed to load external file \"%s\"\n", FUNCNAME, entry.path.c_str());								
				delete[] buf;
				return nullptr;
			}
			
            if (!DoCopyFile(external, buf+offset, entry.size))
			{
                DPRINTF("%s: DoCopyFile file mem failed on external file \"%s\"\n", FUNCNAME, entry.path.c_str());
				fclose(external);
				delete[] buf;
				return nullptr;
			}
			
			fclose(external);
		}
		
		offset += entry.size;
	}
	
	assert(offset == file_size);
	
	*psize = file_size;
	return buf;
}

bool Afs2File::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
	FILE *w_handle;
	AFS2Header header;	
	void *offsets;
	unsigned int offsets_size;
	
	w_handle = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
	if (!w_handle)
	{
		if (show_error)
		{
			DPRINTF("%s Cannot open/create file \"%s\"\n", FUNCNAME, path.c_str());			
		}
		
		return false;
	}
	
	offsets = CreateOffsetsSection(&offsets_size);
	if (!offsets)
	{
		fclose(w_handle);
		return false;
	}
	
	header.signature = AFS2_SIGNATURE;
	memcpy(header.version, version, sizeof(version));
    header.num_files = (uint32_t)files.size();
	header.alignment = alignment;
	
	if (fwrite(&header, 1, sizeof(header), w_handle) != sizeof(header))
	{
		if (show_error)
		{
			DPRINTF("%s: write error, to file \"%s\"\n", FUNCNAME, path.c_str());
		}
		
		fclose(w_handle);
		free(offsets);
		return false;
	}
	
	assert(files.size() < 65536);
	
	for (size_t i = 0; i < files.size(); i++)
	{
        uint16_t idx = (uint16_t)i;
		
		if (fwrite(&idx, 1, sizeof(idx), w_handle) != sizeof(idx))
		{
			if (show_error)
			{
				DPRINTF("%s: write error, to file \"%s\"\n", FUNCNAME, path.c_str());
			}
			
			fclose(w_handle);
			free(offsets);
			return false;
		}	
	}
	
	if (fwrite(offsets, 1, offsets_size, w_handle) != offsets_size)
	{
		if (show_error)
		{
			DPRINTF("%s: write error, to file \"%s\"\n", FUNCNAME, path.c_str());
		}
		
		fclose(w_handle);
		free(offsets);
		return false;
	}
	
	free(offsets);
	offsets = nullptr;
	
	for (Afs2Entry &entry : files)
	{
		PadFile(w_handle);
		
		if (!GetEntrySize(entry, &entry.size))
		{
			fclose(w_handle);
			return false;
		}
		
		if (entry.offset != (uint32_t)-1)
		{
			// Internal file in r_handle
			
			assert(r_handle != nullptr);			
			fseek(r_handle, entry.offset, SEEK_SET);	
			
            if (!DoCopyFile(r_handle, w_handle, entry.size))
			{
				if (show_error)
				{
                    DPRINTF("%s: DoCopyFile file file failed.\n", FUNCNAME);
				}
				
				fclose(w_handle);
				return false;
			}
		}
		else if (entry.buf)
		{
			// Memory file
            if (!DoCopyFile(entry.buf, w_handle, entry.size))
			{
				if (show_error)
				{
                    DPRINTF("%s: DoCopyFile mem file failed.\n", FUNCNAME);
				}
				
				fclose(w_handle);
				return false;
			}
		}
		else
		{
			// External file
			FILE *external = fopen(entry.path.c_str(), "rb");
			if (!external)
			{
				if (show_error)
				{
					DPRINTF("%s: Failed to load external file \"%s\"\n", FUNCNAME, entry.path.c_str());
				}
				
				fclose(w_handle);
				return false;
			}
			
            if (!DoCopyFile(external, w_handle, entry.size))
			{
				if (show_error)
				{
                    DPRINTF("%s: DoCopyFile file file failed on external file \"%s\"\n", FUNCNAME, entry.path.c_str());
				}
				
				fclose(w_handle);
				fclose(external);
				return false;
			}
			
			fclose(external);
		}
	}
	
	fclose(w_handle);	
	return true;
}

uint8_t *Afs2File::CreateHeader(unsigned int *psize, bool extra_word)
{
	uint32_t offsets_size;
	
	*psize = CalculateHeaderSize();
	
	if (*psize == (uint32_t)-1)
		return nullptr;
	
	if (extra_word)	
		*psize += 2;
	
    uint8_t *header = new uint8_t[*psize];
    memset(header, 0, *psize);
	
	void *offsets = CreateOffsetsSection(&offsets_size);
	if (!offsets)
	{
		delete[] header;
		return nullptr;
	}
	
	AFS2Header *afs2_header = (AFS2Header *)header;
	
	afs2_header->signature = AFS2_SIGNATURE;
	memcpy(afs2_header->version, version, sizeof(version));
    afs2_header->num_files = (uint32_t)files.size();
	afs2_header->alignment = alignment;
	
	uint16_t *idx = (uint16_t *)(header + sizeof(AFS2Header));
	
	for (size_t i = 0; i < files.size(); i++)
        idx[i] = (uint16_t)i;
	
	memcpy(header+sizeof(AFS2Header)+(files.size()*sizeof(uint16_t)), offsets, offsets_size);
	free(offsets);
	
	return header;
}

std::string Afs2File::ChooseFileName(uint32_t idx, uint32_t file_size) const
{
    char temp[32];
    std::string default_return;
    uint32_t signature = 0;

    assert(idx < files.size());
    snprintf(temp, sizeof(temp), "%04d", idx+1);
    default_return = std::string(temp) + ".bin";

    const Afs2Entry &entry = files[idx];

    if (file_size < 4)
        return default_return;

    if (entry.offset != (uint32_t)-1)
    {
        assert(r_handle);
        fseek(r_handle, entry.offset, SEEK_SET);

        fread(&signature, 1, sizeof(uint32_t), r_handle);
    }
    else if (entry.buf)
    {
        signature = *(uint32_t *)entry.buf;
    }
    else
    {
        FILE *f = fopen(entry.path.c_str(), "rb");
        if (!f)
            return default_return;

        fread(&signature, 1, sizeof(uint32_t), f);
        fclose(f);
    }

    if ((signature & 0x7F7F7F7F) == HCA_SIGNATURE)
    {
        return std::string(temp)+".hca";
    }
    else if ((signature&0xFFFF) == 0x80)
    {
        return std::string(temp)+".adx";
    }

    return default_return;
}

bool Afs2File::ExtractFile(uint32_t idx, const std::string &path, bool auto_path) const
{
	uint32_t size;
    std::string file_path = path;
	
	if (idx >= files.size())
		return false;
	
	const Afs2Entry &entry = files[idx];
	
	if (!GetEntrySize(entry, &size))
		return false;

    if (auto_path)
    {
        if (file_path.length() != 0 && !Utils::EndsWith(file_path, "/") && !Utils::EndsWith(file_path, "\\"))
            file_path += '/';

        file_path += ChooseFileName(idx, size);
    }
	
    FILE *out = Utils::fopen_create_path(file_path.c_str(), "wb");
	if (!out)
		return false;	
	
	if (entry.offset != (uint32_t)-1)
	{
		// Internal file in r_handle
		
		assert(r_handle != nullptr);			
		fseek(r_handle, entry.offset, SEEK_SET);	
		
        if (!DoCopyFile(r_handle, out, size))
		{			
			fclose(out);
			return false;
		}
	}
	else if (entry.buf)
	{
		// Memory file
        if (!DoCopyFile(entry.buf, out, size))
		{			
			fclose(out);
			return false;
		}
	}
	else
	{
		// External file
		FILE *external = fopen(entry.path.c_str(), "rb");
		if (!external)
		{			
			fclose(out);
			return false;
		}
		
        if (!DoCopyFile(external, out, size))
		{			
			fclose(out);
			fclose(external);
			return false;
		}
		
		fclose(external);
	}
	
	fclose(out);	
	return true;
}

uint8_t *Afs2File::ExtractFile(uint32_t idx, uint64_t *psize) const
{
	if (idx >= files.size())
		return nullptr;
	
	const Afs2Entry &entry = files[idx];
    uint32_t entry_size;
	
    if (!GetEntrySize(entry, &entry_size))
		return nullptr;

    *psize = entry_size;
	
    uint8_t *buf = new uint8_t[*psize];
	
	if (entry.offset != (uint32_t)-1)
	{
		// Internal file in r_handle
		
		assert(r_handle != nullptr);			
		fseek(r_handle, entry.offset, SEEK_SET);	
		
        if (!DoCopyFile(r_handle, buf, *psize))
		{			
			delete[] buf;
			return nullptr;
		}
	}
	else if (entry.buf)
	{
		// Memory file
        if (!DoCopyFile(entry.buf, buf, *psize))
		{			
			delete[] buf;
			return nullptr;
		}
	}
	else
	{
		// External file
		FILE *external = fopen(entry.path.c_str(), "rb");
		if (!external)
		{			
			delete[] buf;
			return nullptr;
		}
		
        if (!DoCopyFile(external, buf, *psize))
		{			
			delete[] buf;
			fclose(external);
			return nullptr;
		}
		
		fclose(external);
	}
	
    return buf;
}

bool Afs2File::SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership)
{
    if (idx >= files.size())
        return false;

    if (size > 0xFFFFFFFF)
        return false;

    uint8_t *new_buf;

    if (take_ownership)
    {
        new_buf = (uint8_t *)buf;
    }
    else
    {
        new_buf = new uint8_t[size];
        memcpy(new_buf, buf, size);
    }

    Afs2Entry &entry = files[idx];

    if (entry.buf)
        delete[] entry.buf;

    entry.offset = -1;
    entry.size = (uint32_t)size;
    entry.buf = new_buf;

    return true;
}

bool Afs2File::AddFile(const std::string &path)
{
	Afs2Entry entry;
	
	entry.offset = (uint32_t)-1;
	entry.buf = nullptr;
	entry.size = 0;
	entry.path = path;
	
	files.push_back(entry);	
	return true;
}

bool Afs2File::AddFile(void *buf, uint64_t size, bool take_ownership)
{
    uint8_t *new_buf;

    if (size > 0xFFFFFFFF)
        return false;

    if (take_ownership)
    {
        new_buf = (uint8_t *)buf;
    }
    else
    {
        new_buf = new uint8_t[size];
        memcpy(new_buf, buf, size);
    }
	
	Afs2Entry dummy_entry;	
	files.push_back(dummy_entry);
	
	Afs2Entry &entry = files.back();
	entry.offset = -1;
    entry.size = (uint32_t)size;
    entry.buf = new_buf;
		
	return true;
}
