#ifndef __IGGYFILE_H__
#define __IGGYFILE_H__

// This file is WIP. Anything is subject to change.
// In its current state, format reverse may be bogus and partially or totally erroneus
// (this include comments)

#include "BaseFile.h"
#include "FixedMemoryStream.h"

#define IGGY_SIGNATURE  0xED0A6749

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
    uint32_t signature; // 0
    uint32_t version; // 4
    uint8_t plattform[4]; // 8
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

// ****All relative offsets are relative from that specific field position****
// ****All relative offsets can get value "1" to indicate "nothing"****
typedef struct
{
   uint32_t main_offset; // 0 Relative offset to first section (matches sizeof header)
   uint32_t as3_section_offset; // 4  Relative offset to as3 file names table...
   uint32_t unk_offset; // 8   relative offset to something
   uint32_t unk_offset2; // 0xC  relative offset to something
   uint32_t unk_offset3; //  0x10 relative offset to something
   uint32_t unk_offset4; // 0x14 relative offset to something
   uint64_t unk_18; // maybe same as swf framesize->xmin and framesize->ymin but in pixels
   uint32_t width; // 0x20 maybe same as swf framesize->xmax but in pixels
   uint32_t height; // 0x24 maybe same as swf framesize->ymax but in pixels
   uint32_t unk_28; // probably numer of blocks/objects after header
   uint32_t unk_2C;
   uint32_t unk_30;
   uint32_t unk_34;
   uint32_t unk_38;
   uint32_t unk_3C;
   float unk_40;
   uint32_t unk_44;
   uint32_t unk_48;
   uint32_t unk_4C;
   uint32_t names_offset; // 0x50 relative offset to the names/import section of the file
   uint32_t unk_offset5; // 0x54 relative offset to something
   uint64_t unk_58; // Maybe number of imports/names pointed by names_offset
   uint32_t last_section_offset; // 0x60 relative offset, points to the small last section of the file
   uint32_t unk_offset6; // 0x64 relative offset to something
   uint32_t as3_code_offset; // 0x68 relative offset to as3 code (8 bytes header + abc blob)
   uint32_t as3_names_offset; // 0x6C relative offset to as3 file names table (or classes names or whatever)
   uint32_t unk_70;
   uint32_t unk_74;
   uint32_t unk_78; // Maybe number of classes / as3 names
   uint32_t unk_7C;

   // Offset 0x80 (outside header): there are *unk_28* relative offsets that point to flash objects.
   // The flash objects are in a format different to swf but there is probably a way to convert between them.
   // After the offsets, the bodies of objects pointed above, which apparently have a code like 0xFFXX to identify the type of object, followed by a (unique?) identifier
   // for the object.
   // A DefineEditText-like object can be easily spotted and apparently uses type code 0x06 (or 0xFF06) but as stated above,
   // it is written in a different way.

} PACKED IGGYFlashHeader32;

STATIC_ASSERT_STRUCT(IGGYFlashHeader32, 0x80);

typedef struct
{
    uint64_t main_offset; // 0 Relative offset to first section (matches sizeof header);
    uint64_t as3_section_offset; // 8  Relative offset to as3 file names table...
    uint64_t unk_offset; // 0x10   relative offset to something
    uint64_t unk_offset2; // 0x18  relative offset to something
    uint64_t unk_offset3; //  0x20 relative offset to something
    uint64_t unk_offset4; // 0x28 uint32_t names_offset; // 0x50 relative pointer to the names/import section of the file
    uint64_t unk_30; // maybe same as swf framesize->xmin and framesize->ymin but in pixels
    uint32_t width; // 0x38 maybe same as swf framesize->xmax but in pixels
    uint32_t height; // 0x3C maybe same as swf framesize->ymax but in pixels
    uint32_t unk_40; // probably numer of blocks/objects after header
    uint32_t unk_44;
    uint32_t unk_48;
    uint32_t unk_4C;
    uint32_t unk_50;
    uint32_t unk_54;
    float unk_58;
    uint32_t unk_5C;
    uint64_t unk_60;
    uint64_t unk_68;
    uint64_t names_offset; // 0x70 relative offset to the names/import section of the file
    uint64_t unk_offset5; // 0x78 relative offset to something
    uint64_t unk_80; // Maybe number of imports/names pointed by names_offset
    uint64_t last_section_offset; // 0x88 relative offset, points to the small last section of the file
    uint64_t unk_offset6; // 0x90 relative offset to something
    uint64_t as3_code_offset; // 0x98 relative offset to as3 code (16 bytes header + abc blob)
    uint64_t as3_names_offset; // 0xA0 relative offset to as3 file names table (or classes names or whatever)
    uint32_t unk_A8;
    uint32_t unk_AC;
    uint32_t unk_B0; // Maybe number of classes / as3 names
    uint32_t unk_B4;

    // Offset 0xB8 (outside header): there are *unk_40* relative offsets that point to flash objects.
    // The flash objects are in a format different to swf but there is probably a way to convert between them.
    // After the offsets, the bodies of objects pointed above, which apparently have a code like 0xFFXX to identify the type of object, followed by a (unique?) identifier
    // for the object.
    // A DefineEditText-like object can be easily spotted and apparently uses type code 0x06 (or 0xFF06) but as stated above,
    // it is written in a different way.
} PACKED IGGYFlashHeader64;

STATIC_ASSERT_STRUCT(IGGYFlashHeader64, 0xB8);

#ifdef _MSC_VER
#pragma pack(pop)
#endif

class IggyFile : public BaseFile
{
private:

    bool is_64;
    uint32_t unk_0C;
    std::vector<uint8_t> index_data;
    uint32_t as3_offset;

    // Until the format is better understood, let's divide the second section/subfile into the following sections
    // Main section with all the gui stuff and also including main header
    // AS3 names section (include header of AS3 section here)
    // AS3 code section (8 or 12 bytes header (depending on 32/64bits) + ABC blob)
    // Names section (or include or whatever this is)
    // Last section. A small section, apparently always 8/16 bytes (depending on 32/64bits) (seems like it could be a header specifying a 0 size)

    std::vector<uint8_t> main_section;
    std::vector<uint8_t> as3_names_section;
    std::vector<uint8_t> as3_code_section;
    std::vector<uint8_t> names_section;
    std::vector<uint8_t> last_section;

    bool LoadFlashData32(FixedMemoryStream &stream);
    bool LoadFlashData64(FixedMemoryStream &stream);

    void SaveFlashData32(uint8_t *buf, size_t size);
    void SaveFlashData64(uint8_t *buf, size_t size);

    size_t GetFlashDataSize();
    bool RebuildIndexAfterAbc(int32_t diff);

public:

    IggyFile();
    virtual ~IggyFile();

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    uint8_t *GetAbcBlob(uint32_t *psize) const;
    bool SetAbcBlob(void *buf, uint32_t size);

    // Debug function
    void PrintIndex() const;
};

#endif // __IGGYFILE_H__
