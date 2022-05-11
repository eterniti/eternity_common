#ifndef __MEMORYSTREAM_H__
#define __MEMORYSTREAM_H__

#include "Stream.h"

class MemoryStream : public Stream
{
protected:

	uint8_t *mem;

    uint64_t file_size;
    uint64_t file_pos;
    uint64_t capacity;
	
public:

	MemoryStream();
    virtual ~MemoryStream() override;

    MemoryStream(const MemoryStream &other)
    {
        Copy(other);
    }

    inline MemoryStream &operator=(const MemoryStream &other)
    {
        if (this == &other)
            return *this;

        Copy(other);
        return *this;
    }

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
