#ifndef __MEMORYSTREAM_H__
#define __MEMORYSTREAM_H__

#include "Stream.h"

#define MS_DEFAULT_GROW_SIZE	(4*1024*1024)

class MemoryStream : public Stream
{
protected:

	uint8_t *mem;

    uint64_t file_size;
    uint64_t file_pos;
    uint64_t capacity;

    uint32_t grow_size;
	
public:

    MemoryStream()
    {
        big_endian = false;
        mem = nullptr;
        file_size = file_pos = capacity = 0;
        grow_size = MS_DEFAULT_GROW_SIZE;
    }

    virtual ~MemoryStream() override;

    MemoryStream(const MemoryStream &other)
    {
        mem = nullptr;
        Copy(other);
    }

    // Assumes buf is new[] allocated. This object will take ownership of it
    MemoryStream(uint8_t *buf, size_t size, uint32_t grow_size=MS_DEFAULT_GROW_SIZE)
    {
        big_endian = false;
        mem = buf;
        file_pos = 0;
        file_size = capacity = size;
        this->grow_size = grow_size;
    }

    MemoryStream(uint32_t grow_size)
    {
        big_endian = false;
        mem = nullptr;
        file_size = file_pos = capacity = 0;
        this->grow_size = grow_size;
    }

    inline MemoryStream &operator=(const MemoryStream &other)
    {
        Copy(other);
        return *this;
    }

    using Stream::Copy;
	void Copy(const MemoryStream &other);

    uint8_t *GetMemory(bool unlink);
    void SetMemory(void *buf, size_t size);
    bool FastRead(uint8_t **pbuf, size_t size);
	
    virtual uint64_t GetSize() const override { return file_size; }
    virtual bool Resize(uint64_t size) override;
	
    virtual bool Read(void *buf, size_t size) override;
    virtual bool Write(const void *buf, size_t size) override;
	
    virtual bool Seek(off64_t offset, int whence) override;
    virtual uint64_t Tell() override { return file_pos; }
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
	virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
	virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;	

    virtual bool Align(unsigned int alignment) override;
};

#endif /* __MEMORYSTREAM_H__ */
