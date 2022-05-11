#include <stdint.h>

#include "EmbFile.h"
#include "debug.h"

struct FileType
{
    std::string extension;
    std::string signature;
};

static const FileType file_types[] =
{
    { ".dds", "DDS "  },
    { ".emb", "#EMB" },
    { ".emo", "#EMO" },
    { ".emm", "#EMM" },
    { ".2ry", "#2RY" },
    { ".ema", "#EMA" },
    { ".emg", "#EMG" }, // Just a guess, never seen one of these
    { ".eme", "#EME" }, // Just a guess, never seen one of these
};

EmbContainedFile::EmbContainedFile()
{
    buf = nullptr;
    Reset();
}

EmbContainedFile::~EmbContainedFile()
{
    Reset();
}

void EmbContainedFile::Copy(const EmbContainedFile &other)
{
    Reset();

    if (other.buf)
    {
        this->buf = new uint8_t[other.size];
        memcpy(this->buf, other.buf, other.size);

        this->size = other.size;
        this->allocated = true;
    }

    this->name = other.name;

    this->is_emb = other.is_emb;
    this->emb = other.emb;
}

void EmbContainedFile::Reset()
{
    const static EmbFile empty_emb;

    if (buf && allocated)
    {
        delete[] buf;
    }

    buf = nullptr;
    size = 0;
    allocated = false;
    name.clear();
    is_emb = false;
    emb = empty_emb;
}

void EmbContainedFile::Decompile(TiXmlNode *root) const
{
    TiXmlElement *entry_root = new TiXmlElement("File");

    if (name != "")
    {
        entry_root->SetAttribute("name", name);
    }

    TiXmlText *blob = new TiXmlText(Utils::Base64Encode(buf, size, true));
    blob->SetCDATA(true);

    entry_root->LinkEndChild(blob);
    root->LinkEndChild(entry_root);
}

bool EmbContainedFile::Compile(const TiXmlElement *root)
{
    if (!Utils::ReadAttrString(root, "name", name))
        name = "";

    const char *base64 = root->GetText();
    if (!base64)
        return false;

    buf = Utils::Base64Decode(base64, &size);
    if (!buf)
        return false;

    allocated = true;
    return true;
}

EmbFile::EmbFile(bool recursive)
{
    this->recursive = recursive;
    buf = nullptr;
    size = 0;
    big_endian = false;
}

EmbFile::EmbFile(const uint8_t *buf, size_t size, bool recursive)
{
    this->recursive = recursive;
    Load(buf, size);
}

EmbFile::~EmbFile()
{
    Reset();
}

void EmbFile::Copy(const EmbFile &other)
{
    this->buf = nullptr;
    this->size = 0;
    this->big_endian = other.big_endian;

    this->recursive = other.recursive;
    this->files = other.files;
}

void EmbFile::Reset()
{
    files.clear();

    if (buf)
    {
        delete[] buf;
        buf = nullptr;
    }

    size = 0;    
}

std::string EmbFile::CreateFileName(uint16_t idx, const uint8_t *buf, size_t size)
{
    std::string ret = Utils::ToString(idx);

    if (size >= 4)
    {
        for (const FileType &ft : file_types)
        {
            if (memcmp(buf, ft.signature.c_str(), 4) == 0)
            {
                ret += ft.extension;
                return ret;
            }
        }
    }

    ret += ".bin";
    return ret;
}

bool EmbFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    EMBHeader *hdr = (EMBHeader *)buf;

    if (hdr->signature != EMB_SIGNATURE)
        return false;

    this->big_endian = (buf[4] != 0xFE);

    this->buf = new uint8_t[size];
    memcpy(this->buf, buf, size);
    this->size = size;

    hdr = (EMBHeader *)this->buf;
    EMBTable *table = (EMBTable *)GetOffsetPtr(hdr, hdr->table_start);
    uint32_t *fn_table = nullptr;

    if (hdr->filenames_table)
    {
        fn_table = (uint32_t *)GetOffsetPtr(hdr, hdr->filenames_table);
    }

    files.resize(val32(hdr->files_count));

    for (uint32_t i = 0; i < val32(hdr->files_count); i++)
    {
        files[i].buf = GetOffsetPtr(table, table[i].offset) + Utils::DifPointer(table + i, table);
        files[i].size = val32(table[i].file_size);
        files[i].allocated = false;

        if (fn_table)
        {
            const char *str = (const char *)GetOffsetPtr(hdr, fn_table, i);
            files[i].name = str;
        }

        files[i].is_emb = false;

        if (recursive)
        {
            files[i].emb.recursive = true;

            if (files[i].emb.Load(files[i].buf, files[i].size))
            {
                files[i].is_emb = true;
            }
        }
    }

    return true;
}

bool EmbFile::HasFileNames() const
{
    if (files.size() == 0)
        return false;

    return (files[0].name != "");
}

uint16_t EmbFile::FindIndexByName(const std::string &name)
{
    if (!HasFileNames())
        return (uint16_t)-1;

    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].name == name)
            return (uint16_t)i;
    }

    return (uint16_t)-1;
}

bool EmbFile::ReplaceFile(uint16_t idx, const uint8_t *buf, size_t size)
{
    if (idx >= files.size())
        return false;

    uint8_t *copy = new uint8_t[size];

    if (files[idx].allocated)
    {
        delete[] files[idx].buf;
    }

    memcpy(copy, buf, size);
    files[idx].buf = copy;
    files[idx].size = size;
    files[idx].allocated = true;

    return true;
}

bool EmbFile::ReplaceFile(const std::string &name, const uint8_t *buf, size_t size)
{
    uint16_t idx = FindIndexByName(name);
    if (idx == (uint16_t)-1)
        return false;

    return ReplaceFile(idx, buf, size);
}

uint16_t EmbFile::AppendFile(const uint8_t *buf, size_t size, const std::string &name, bool take_ownership)
{
    EmbContainedFile file;
    uint16_t idx = (uint16_t)files.size();    

    if (take_ownership)
    {
        file.buf = const_cast<uint8_t *>(buf);
    }
    else
    {

        uint8_t *copy = new uint8_t[size];
        memcpy(copy, buf, size);
        file.buf = copy;
    }

    bool force_name = false;

    if (files.size() == 0 && name.length() > 0)
    {
        force_name = true;
    }

    file.size = size;
    file.allocated = true;
    file.is_emb = false;

    files.push_back(file);

    if (recursive)
    {
        files[idx].emb.recursive = true;

        if (files[idx].emb.Load(files[idx].buf, files[idx].size))
        {
            files[idx].is_emb = true;
        }
    }

    if (HasFileNames() || force_name)
    {
        if (name.length() > 0)
        {
            files[idx].name = name;
        }
        else
        {
            files[idx].name = CreateFileName(idx, files[idx].buf, files[idx].size);
        }
    }

    return idx;
}

uint16_t EmbFile::AppendFile(const EmbFile &other, uint16_t idx_other)
{
    if (idx_other >= other.files.size())
        return (uint16_t)-1;

    return AppendFile(other.files[idx_other]);
}

uint16_t EmbFile::AppendFile(const EmbContainedFile &file)
{
    return AppendFile(file.buf, file.size, file.name);
}

uint16_t EmbFile::AppendFile(const std::string &file_path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(file_path, &size);
    if (!buf)
        return (uint16_t)-1;

    std::string name = Utils::NormalizePath(file_path);
    size_t slash_pos = name.rfind('/');

    if (slash_pos != std::string::npos)
        name = name.substr(slash_pos+1);

    return AppendFile(buf, size, name, true);
}

void EmbFile::RemoveFile(uint16_t idx)
{
    if (idx >= files.size())
        return;

    files.erase(files.begin()+idx);
}

unsigned int EmbFile::CalculateFileSize()
{
    unsigned int file_size = sizeof(EMBHeader);

    file_size += (unsigned int)files.size() * sizeof(EMBTable);
    if (HasFileNames())
    {
        file_size += (unsigned int)files.size() * sizeof(uint32_t);
    }

    for (const EmbContainedFile &f : files)
    {
        if (file_size & 0x3F)
        {
            file_size += (0x40 - (file_size & 0x3F));
        }

        file_size += (unsigned int)f.size;
    }

    if (HasFileNames())
    {
        if (file_size & 0x3F)
        {
            file_size += (0x40 - (file_size & 0x3F));
        }

        for (const EmbContainedFile &f : files)
        {
            file_size += (unsigned int)f.name.length() + 1;
        }
    }

    return file_size;
}

uint8_t *EmbFile::Save(size_t *psize)
{
    uint8_t *ret;
    unsigned int file_size;
    uint32_t offset;
    EMBHeader *hdr;
    EMBTable *table;
    uint32_t *fn_table;
	
	file_size = CalculateFileSize();
    ret = new uint8_t[file_size];
    memset(ret, 0, file_size);

    hdr = (EMBHeader *)ret;
    hdr->signature = EMB_SIGNATURE;
    hdr->endianess_check = val16(0xFFFE);
    hdr->unk_08 = val16(1);
    hdr->unk_0A = val16(1);

    offset = sizeof(EMBHeader);
    hdr->header_size = val16(sizeof(EMBHeader));
    hdr->table_start = val32(sizeof(EMBHeader));
    hdr->files_count = val32((uint32_t)files.size());

    table = (EMBTable *)GetOffsetPtr(ret, offset, true);
    offset += (uint32_t)files.size() * sizeof(EMBTable);

    if (HasFileNames())
    {
        hdr->filenames_table = val32(offset);
        fn_table = (uint32_t *)GetOffsetPtr(ret, offset, true);
        offset += (uint32_t) files.size() * sizeof(uint32_t);
    }
    else
    {
        hdr->filenames_table = 0;
        fn_table = nullptr;
    }

    for (size_t i = 0; i < files.size(); i++)
    {
        if (offset & 0x3F)
        {
            offset += (0x40 - (offset & 0x3F));
        }

        table[i].offset = val32(Utils::DifPointer(ret + offset, table + i));
        table[i].file_size = val32((uint32_t)files[i].size);

        memcpy(ret+offset, files[i].buf, files[i].size);
        offset += (uint32_t)files[i].size;
    }

    if (fn_table)
    {
        if (offset & 0x3F)
        {
            offset += (0x40 - (offset & 0x3F));
        }

        for (size_t i = 0; i < files.size(); i++)
        {
            fn_table[i] = val32(offset);

            strcpy((char *)ret+offset, files[i].name.c_str());
            offset += (uint32_t)files[i].name.length() + 1;
        }
    }

    assert(offset == file_size);

    *psize = file_size;
    return ret;
}

TiXmlDocument *EmbFile::Decompile() const
{
    TiXmlDocument *doc = new TiXmlDocument();

    TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", "" );
    doc->LinkEndChild(decl);

    TiXmlElement *root = new TiXmlElement("EMB");

    for (const EmbContainedFile &f : files)
    {
        f.Decompile(root);
    }

    doc->LinkEndChild(root);
    return doc;
}

bool EmbFile::Compile(TiXmlDocument *doc, bool big_endian)
{
    int has_file_names = -1;

    Reset();
    this->big_endian = big_endian;

    TiXmlHandle handle(doc);
    const TiXmlElement *root = Utils::FindRoot(&handle, "EMB");

    if (!root)
    {
        DPRINTF("Cannot find\"EMB\" in xml.\n");
        return false;
    }

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == "File")
        {
            EmbContainedFile file;

            if (!file.Compile(elem))
                return false;

            if (has_file_names == -1)
            {
                has_file_names = (file.name != "");
            }
            else
            {
                if ((has_file_names && file.name == "") || (!has_file_names && file.name != ""))
                {
                    DPRINTF("%s: parse error, either all files have names or none of them!\n", FUNCNAME);
                    return false;
                }
            }

            file.is_emb = false;

            size_t idx = files.size();
            files.push_back(file);

            if (recursive)
            {
                files[idx].emb.recursive = true;

                if (files[idx].emb.Load(files[idx].buf, files[idx].size))
                {
                    files[idx].is_emb = true;
                }
            }
        }
    }

    return true;
}
