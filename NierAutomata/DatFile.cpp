#include "DatFile.h"
#include "debug.h"

DatFile::DatFile()
{
    this->big_endian = false;
}

DatFile::~DatFile()
{

}

void DatFile::Reset()
{
    files.clear();
    unk_data.clear();
}

bool DatFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (!buf || size < sizeof(NADatHeader))
        return false;

    const NADatHeader *hdr = (const NADatHeader *)buf;
    if (hdr->signature != NA_DAT_SIGNATURE)
        return false;

    files.resize(hdr->num_files);

    const uint32_t *file_offsets = (const uint32_t *)(buf + hdr->file_offsets);
    //const char *file_extensions = (const char *)(buf + hdr->file_extensions);
    const char *file_names = (const char *)(buf + hdr->file_names);
    const uint32_t *file_sizes = (const uint32_t *)(buf + hdr->file_sizes);
    const uint8_t *file_unk_data = buf + hdr->unk_data_offset;

    uint32_t file_name_step = *(const uint32_t *)file_names;
    file_names += 4;

    for (size_t i = 0; i < files.size(); i++)
    {
        DatFileEntry &file = files[i];

        files[i].name = file_names + (file_name_step * i);
        files[i].content.resize(file_sizes[i]);
        memcpy(files[i].content.data(), buf + file_offsets[i], files[i].content.size());

        //DPRINTF("%s\n", files[i].name.c_str());
    }

    unk_data.resize(file_offsets[0]-hdr->unk_data_offset);
    memcpy(unk_data.data(), file_unk_data, unk_data.size());

    return true;
}

size_t DatFile::CalcFileStructure(size_t *file_offsets_offset, size_t *extensions_offset, uint32_t *names_step, size_t *names_offset, size_t *sizes_offset, size_t *unk_data_offset, size_t *content_offset) const
{
    size_t size = sizeof(NADatHeader);

    // Offsets
    *file_offsets_offset = size;
    size += sizeof(uint32_t) * files.size();

    // Extensions
    *extensions_offset = size;
    size += 4 * files.size();

    // Names
    *names_offset = size;
    *names_step = 0;

    for (const DatFileEntry &file : files)
    {
        if (file.name.length() > (size_t)*names_step)
        {
            *names_step = (uint32_t)file.name.length();
        }
    }

    *names_step = *names_step + 1;
    size += sizeof(uint32_t);
    size += *names_step * files.size();

    // Sizes
    *sizes_offset = size;
    size += sizeof(uint32_t) * files.size();

    // Unk data
    *unk_data_offset = size;
    size += unk_data.size();

    // Content
    size = Utils::Align2(size, 0x10);
    *content_offset = size;

    for (const DatFileEntry &file : files)
    {
        size += Utils::Align2(file.content.size(), 0x10);
    }

    return size;
}

uint8_t *DatFile::Save(size_t *psize)
{
    size_t files_offsets_offset, extensions_offset, names_offset, sizes_offset, unk_data_offset, content_offset;
    uint32_t names_step;

    *psize = CalcFileStructure(&files_offsets_offset, &extensions_offset, &names_step, &names_offset, &sizes_offset, &unk_data_offset, &content_offset);
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    NADatHeader *hdr = (NADatHeader *)buf;
    hdr->signature = NA_DAT_SIGNATURE;
    hdr->num_files = (uint32_t)files.size();
    hdr->file_offsets = (uint32_t)files_offsets_offset;
    hdr->file_extensions = (uint32_t)extensions_offset;
    hdr->file_names = (uint32_t)names_offset;
    hdr->file_sizes = (uint32_t)sizes_offset;
    hdr->unk_data_offset = (uint32_t)unk_data_offset;

    uint32_t *files_offset = (uint32_t *)(buf + files_offsets_offset);
    char *extensions = (char *)(buf + extensions_offset);
    char *names = (char *)(buf + names_offset);
    uint32_t *sizes = (uint32_t *)(buf + sizes_offset);
    uint8_t *file_unk_data = buf + unk_data_offset;
    uint8_t *data = buf + content_offset;

    *(uint32_t *)names = names_step;
    names += sizeof(uint32_t);

    for (size_t i = 0; i < files.size(); i++)
    {
        const DatFileEntry &file = files[i];

        if (file.name.length() < 3)
        {
            DPRINTF("%s: Name too short: %s\n", FUNCNAME, file.name.c_str());
            delete[] buf;
            return nullptr;
        }

        files_offset[i] = Utils::DifPointer(data, buf);
        sizes[i] = (uint32_t)file.content.size();

        std::string extension = file.name.substr(file.name.length()-3, 3);
        strcpy(extensions + 4*i, extension.c_str());
        strcpy(names + names_step*i, file.name.c_str());

        memcpy(data, file.content.data(), file.content.size());
        data += Utils::Align2(file.content.size(), 0x10);
    }

    memcpy(file_unk_data, unk_data.data(), unk_data.size());
    return buf;
}

DatFileEntry *DatFile::FindFile(const std::string &name)
{
    for (DatFileEntry &file : files)
    {
        if (file.name == name)
            return &file;
    }

    return nullptr;
}
