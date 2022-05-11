#ifndef __FILESTREAM_H__
#define __FILESTREAM_H__

#include "Stream.h"

class FileStream : public Stream
{
    FILE *handle;
    std::string mode;

    uint64_t stream_size;
    uint64_t stream_pos;
    uint64_t stream_capacity;

    uint64_t file_start;
    uint64_t file_capacity;

    std::string saved_path;

protected:

    void Reset();

public:

    FileStream(const std::string &mode = "r+b");
    virtual ~FileStream() override;

    bool SetRegion(uint64_t start, uint64_t size);

    virtual uint64_t GetSize() const override { return stream_size; }
    virtual bool Resize(uint64_t size) override;

    virtual bool Read(void *buf, size_t size) override;
    virtual bool Write(const void *buf, size_t size) override;

    virtual bool Seek(off64_t offset, int whence) override;
    virtual uint64_t Tell() override { return stream_pos; }

    virtual bool Reopen(const std::string &mode);

    virtual uint8_t *Save(size_t *psize) override;

    virtual bool LoadFromFile(const std::string &path, bool show_error=true) override;
    virtual bool SaveToFile(const std::string &path, bool show_error=true, bool build_path=false) override;
};


#endif /* __FILESTREAM_H__ */
