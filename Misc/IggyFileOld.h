#ifndef __IGGYFILE_H__
#define __IGGYFILE_H__

// This file is WIP. Anything is subject to change.
// In its current state, format reverse may be bogus and partially or totally erroneus
// (this include comments)

#include "BaseFile.h"

#define IGGY_SIGNATURE  0xED0A6749

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint32_t plattform; // 8
    uint32_t unk_0C; // 0xC  this is ignored in 32 bits version, but used in the 64 bits. Value 9 has been spotted on 64 bits version. (flags? flash version?)
    uint8_t reserved[0xC]; // 0x10, unused by dll
    uint32_t num_subfiles; // 0x1C
} PACKED IGGYHeader;

STATIC_ASSERT_STRUCT(IGGYHeader, 0x20);

typedef struct
{
    uint32_t id; // 0 Maybe some kind of id or id|type or something alike
    uint32_t size; // 4
    uint32_t size2; // 8, apparently same as size
    uint32_t offset; // 0xC absolute offset
} PACKED IGGYSubFileEntry;

STATIC_ASSERT_STRUCT(IGGYSubFileEntry, 0x10);

// TODO: header size is really 0x80
// ****All relative offsets are relative from that specific field position****
// ****All relative offsets can get value "1" to indicate "nothing"****
typedef struct
{
   uint32_t header_size; // 0  Apparently, real size of header (0x80)
   uint32_t as3_section_offset; // 4  Relative offset to as3 file names table...
   uint32_t unk_offset; // 8   relative offset to something
   uint32_t unk_offset2; // 0xC  relative offset to something
   uint32_t unk_offset3; //  "1"  or some relative offset.
   uint32_t unk_offset4; // 0x14 another relative offset
   uint64_t unk_18; // 0x18  maybe same as swf framesize->xmin and framesize->ymin but in pixels
   uint32_t width; // 0x20 maybe same as swf framesize->xmax but in pixels
   uint32_t height; // 0x24 maybe same as swf framesize->ymax but in pixels
   uint32_t unk_28; // 0x28 probably numer of blocks/objects after offset 0x80 (after header).
   uint32_t unk_2C;
   uint32_t unk_30;
   uint32_t unk_34;
   uint32_t unk_38;
   float unk_3C[5]; //  probably floats. Not 100% sure in the last three ones. float at 0x40 could be framerate. Maybe some of them are 16.16 fixed and not floats?
   uint32_t names_offset; // 0x50 relative pointer to the names/import section of the file
   uint32_t unk_54;
   uint32_t unk_58; // Maybe number of imports/names pointed by names_offset
   uint32_t unk_5C;
   uint32_t last_section_offset; // 0x60 relative offset, points to the small last section of the file
   uint32_t unk_64;
   uint32_t as3_code_offset; // 0x68 relative pointer to as3 code (8 bytes header + abc blob)
   uint32_t as3_names_offset; // 0x6C points to as3 file names table (or classes names or whatever)

   // 0x78 = number of classes / as3 names

   // Offset 0x80 (outside header): there are *unk_28* relative offsets that point to flash objects.
   // The flash objects are in a format different to swf but there is probably a way to convert between them.
   // After the offsets, the bodies of objects pointed above, which apparently have a code like 0xFFXX to identify the type of object, followed by a (unique?) identifier
   // for the object.
   // A DefineEditText-like object can be easily spotted and apparently uses type code 0x06 (or 0xFF06) but as stated above,
   // it is written in a different way.

} PACKED IGGYFlashHeader;

STATIC_ASSERT_STRUCT(IGGYFlashHeader, 0x70);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class IggySubFile : public BaseFile
{
private:

    uint32_t id;

public:

    IggySubFile() {}
    virtual ~IggySubFile() { }

    virtual size_t GetSize() const = 0;

    uint32_t GetID() const { return id; }
    void SetId(uint32_t id) { this->id = id; }
};

class IggyGenericSubFile : public IggySubFile
{
private:

    std::vector<uint8_t> data;

public:

    IggyGenericSubFile() { }
    virtual ~IggyGenericSubFile() { }

    virtual size_t GetSize() const override
    {
        return data.size();
    }

    virtual bool Load(const uint8_t *buf, size_t size) override
    {
        if (!buf)
            return false;

        data.resize(size);
        memcpy(data.data(), buf, size);

        return true;
    }

    virtual uint8_t *Save(size_t *psize) override
    {
        uint8_t *buf = new uint8_t[data.size()];
        memcpy(buf, data.data(), data.size());
        *psize = data.size();

        return buf;
    }

};

class IggyFlashFile : public IggySubFile
{
private:

    // Until the format is better understood, let's divide the file into the following sections
    // Main section with all the gui stuff and also including main header
    // AS3 names section (include header of AS3 section here)
    // AS3 code section (8 bytes header + ABC blob)
    // Names section (or include or whatever this is)
    // Last section. A small section, apparently always 8 bytes (seems like it could be a header specifying a 0 size)

    std::vector<uint8_t> main_section;
    std::vector<uint8_t> as3_names_section;
    std::vector<uint8_t> as3_code_section;
    std::vector<uint8_t> names_section;
    std::vector<uint8_t> last_section;

public:

    IggyFlashFile();
    virtual ~IggyFlashFile();

    virtual size_t GetSize() const;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    uint8_t *GetAbcBlob(uint32_t *psize) const;
    bool SetAbcBlob(void *buf, uint32_t size);
};

class IggyFile : public BaseFile
{
private:

    std::vector<IggySubFile *> sub_files;
    uint32_t plattform;

protected:

    void Reset();

public:

    IggyFile();
    virtual ~IggyFile();

    inline uint32_t GetNumSubFiles() const { return (uint32_t)sub_files.size(); }
    inline IggySubFile *GetSubFile(uint32_t idx)
    {
        if (idx >= sub_files.size())
            return nullptr;

        return sub_files[idx];
    }

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
};

#endif // __IGGYFILE_H__
