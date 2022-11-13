#include "MemoryStream.h"
#include "debug.h"


MemoryStream::~MemoryStream()
{
	if (mem)
        delete[] mem;
}

void MemoryStream::Copy(const MemoryStream &other)
{
    if (mem)
        delete[] mem;

    mem = new uint8_t[other.capacity];
    memcpy(mem, other.mem, other.capacity);

    file_size = other.file_size;
    file_pos = other.file_pos;
    capacity = other.capacity;
    grow_size = other.grow_size;
}

uint8_t *MemoryStream::GetMemory(bool unlink)
{
    uint8_t *ret = mem;

    if (mem && unlink)
    {
        mem = nullptr;
        Resize(0);
    }

    return ret;
}

void MemoryStream::SetMemory(void *buf, size_t size)
{
    if (mem)
        delete[] mem;

    mem = (uint8_t *)buf;
    file_size = size;
    capacity = size;

    if (file_pos > file_size)
        file_pos = file_size;
}

bool MemoryStream::FastRead(uint8_t **pbuf, size_t size)
{
    if (!mem)
        return false;

    if (file_pos+size > file_size)
        return false;

    *pbuf = mem+file_pos;
    file_pos += size;

    assert(file_pos <= file_size);
    return true;
}

bool MemoryStream::Resize(uint64_t size)
{
	assert(file_size <= capacity);
	
	if (size == 0)
	{
		if (mem)
		{
			delete[] mem;		
			mem = nullptr;
		}
		
		file_size = file_pos = capacity = 0;
		return true;
	}
	
	if (!mem || size > capacity)
	{
		if (!mem)
		{
			assert(file_size == 0 && file_pos == 0 && capacity == 0);
		}
		
        size_t alloc_size = (size >= (capacity+grow_size)) ? size : capacity+grow_size;
        uint8_t *new_mem;

        new_mem = new uint8_t[alloc_size];
		memset(new_mem+capacity, 0, alloc_size-capacity);	
		
		if (mem)
		{
			memcpy(new_mem, mem, file_size);
			delete[] mem;
		}
		
		capacity = alloc_size;
		file_size = size;
		mem = new_mem;		
		
		return true;
	}	
	
	// size <= capacity
	file_size = size;
		
	if (file_pos > file_size)
		file_pos = file_size;
		
	return true;		
}

bool MemoryStream::Read(void *buf, size_t size)
{
	if (!mem)
		return false;
	
	if (file_pos+size > file_size)
		return false;
	
	memcpy(buf, mem+file_pos, size);
	file_pos += size;
	
	assert(file_pos <= file_size);		
	return true;
}

bool MemoryStream::Write(const void *buf, size_t size)
{
	if (!mem)
	{
        if (!Grow((int64_t)size))
			return false;
	}
	
	else if (file_pos+size > file_size)
	{
        if (!Grow((int64_t)(file_pos+size-file_size)))
			return false;
	}
	
	memcpy(mem+file_pos, buf, size);
	file_pos += size;
	
	assert(file_pos <= file_size);	
	return true;
}

bool MemoryStream::Seek(int64_t offset, int whence)
{
	size_t new_pos;
	
	if (whence == SEEK_SET)
	{
        new_pos = (size_t)offset;
	}
	else if (whence == SEEK_CUR)
	{
        new_pos = (size_t)((int64_t)file_pos + offset);
	}
	else if (whence == SEEK_END)
	{
        new_pos = (size_t)((int64_t)file_size + offset);
	}
	else
	{
		return false;
	}
	
	if (new_pos > file_size)
		return false;
	
	file_pos = new_pos;
	
	return true;
}

bool MemoryStream::Load(const uint8_t *buf, size_t size)
{
	if (!Resize(size))
		return false;
	
	if (size != 0)
		memcpy(mem, buf, size);	
	
	return true;
}

uint8_t *MemoryStream::Save(size_t *psize)
{
	if (!mem || file_size == 0)
		return nullptr;
	
    uint8_t *buf = new uint8_t[file_size];
	memcpy(buf, mem, file_size);

    *psize = file_size;
	return buf;
}

bool MemoryStream::LoadFromFile(const std::string &path, bool show_error)
{
	size_t size = Utils::GetFileSize(path);	
	if (size == (size_t)-1)
	{
		if (show_error)
		{
			DPRINTF("%s: Cannot stat file \"%s\"\n", FUNCNAME, path.c_str());
		}
		
		return false;
	}
	
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
	{
		if (show_error)
		{
			DPRINTF("%s: Cannot open file \"%s\".\n", FUNCNAME, path.c_str());
		}
		
		return false;
	}
	
	if (!Resize(size))
	{
		fclose(f);
		return false;
	}
	
	if (fread(mem, 1, size, f) != size)
	{
		fclose(f);
		return false;
	}
	
	fclose(f);
	return true;
}

bool MemoryStream::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    return Utils::WriteFileBool(path, mem, file_size, show_error, build_path);
}

bool MemoryStream::Align(unsigned int alignment)
{
    unsigned int write_size = 0;

    if ((file_pos % alignment) != 0)
        write_size = (alignment - (file_pos % alignment));

    if (mem && write_size != 0 && (write_size + file_pos <= capacity))
    {
        memset(mem+file_pos, 0, write_size);
        file_pos += write_size;

        if (file_pos >= file_size)
            file_size = file_pos;

        return true;
    }

    for (unsigned int i = 0; i < write_size; i++)
    {
        if (!Write8(0))
            return false;
    }

    return true;
}

