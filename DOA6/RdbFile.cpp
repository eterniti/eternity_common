#include "RdbFile.h"
#include "DOA6/RnkFile.h"

#include "MemoryStream.h"
#include "debug.h"

//#define OPPW4

#ifndef OPPW4
#include "hashes/additional_fn_ce.h"
#include "hashes/additional_fn_me.h"
#include "hashes/additional_fn_rp.h"
#include "hashes/additional_fn_kd.h"
#include "hashes/additional_fn_fe4.h"
#include "hashes/additional_fn_sys.h"
#include "hashes/additional_fn_screen.h"
#else
#include "../OPPW4/hashes/ce.h"
#include "../OPPW4/hashes/me.h"
#include "../OPPW4/hashes/fe4.h"
#include "../OPPW4/hashes/rp.h"
#include "../OPPW4/hashes/kids.h"
#include "../OPPW4/hashes/sys.h"
#include "../OPPW4/hashes/screen.h"
#include "../OPPW4/hashes/seq.h"
#endif

#define RDB_COMP_CHUNK_SIZE 16384

#define BUFFER_SIZE_UNCOMP  (16384*1024)

static const std::unordered_map<int, std::string> files_extensions
{
// The real string is something like "TypeInfo::Resource::KTGL::ModelData"  -> .g1m
    { 0x0bd05b27, ".mit"  },
    { 0x0d34474d, ".srst" },
    { 0x17614af5, ".g1mx" },
    { 0x1fdcaa40, ".kidstask" },
    { 0x20a6a0bb, ".kidsobjdb" },
    { 0x27bc54b7, ".rigbin"},
    { 0x4d0102ac, ".g1em" },
    { 0x4f16d0ef, ".kts" },
    { 0x5153729b, ".mtl" },
    { 0x54738c76, ".g1co" },
    { 0x5599aa51, ".kscl" },
    { 0x563bdef1, ".g1m" },
    { 0x56d8deda, ".sid" },
    { 0x5c3e543c, ".swg" },
    { 0x6fa91671, ".g1a" },
    { 0x757347e0, ".bpo" },
    { 0x786dcd84, ".g1n" },
    { 0x79c724c2, ".g1p" },
    { 0x7bcd279f, ".g1s" },
    { 0x9cb3a4b6, ".oidex" },
    { 0xa027e46b, ".mov" },
    { 0xa8d88566, ".g1cox" },
    { 0xafbec60c, ".g1t" },
    { 0xb097d41f, ".g1e" },
    { 0xb0a14534, ".sgcbin" },
    { 0xb1630f51, ".kidsrender" },
    { 0xbbd39f2d, ".srsa" },
    { 0xbbf9b49d, ".grp" },
    { 0xbe144b78, ".ktid" },
    { 0xbf6b52c7, ".name" },
    { 0xd7f47fb1, ".efpl" }, // This is actually a generic type for binary, not only efpl, watch that. Other extensions seen for this: table, ktf2, 2dmap
    { 0xdbcb74a9, ".oid" },
    { 0xf20de437, ".texinfo" },

// New binary (former text) in Mai/Kula update...
// For this, the type info, replaces CSV by word Binary.
//E.g, .grp changed from TypeInfo::Resource::KTGL::PartsModelGroupBindTableCSVFile to TypeInfo::Resource::KTGL::PartsModelGroupBindTableBinaryFile

    { 0x1ab40ae8, ".oid" },
    { 0x56efe45c, ".grp" },
    { 0xe6a3c3bb, ".oidex" },
    { 0x8e39aa37, ".ktid" },
    { 0xb340861a, ".mtl" },

    // OPPW4
    { 0xed410290, ".kts" },
    { 0x82945a44, ".lsqtree" },
    { 0xf13845ef, ".sclshape" },
};

RdbFile::RdbFile()
{
    // Shouldn't be called
}

RdbFile::RdbFile(const std::string &rdb_path) : rdb_path(rdb_path)
{
    this->big_endian = false;
    Reset();
}

void RdbFile::Reset()
{
    entries.clear();

    for (auto &item : bin_files)
        delete item.second;

    wb = nullptr;

    bin_files.clear();
    names_map.clear();
    names_map_rev.clear();
    hash_to_idx.clear();
    name_db_file = 0;

    fe4_check = false;
    fe4_ret = false;
}

RdbFile::~RdbFile()
{
    for (auto &item : bin_files)
        delete item.second;
}

static bool decode_file_address(const std::string &addr, uint64_t *poffset, uint64_t *psize, int *pindex1, int *pindex2)
{
    std::string offset, size, index1, index2;
    size_t pos_a, pos_h, pos_amp;

    if (addr.length() == 0)
        return false;

    pos_a = addr.find('@');

    if (pos_a == std::string::npos)
        return false;

    offset = "0x" + addr.substr(0, pos_a);

    pos_h = addr.find('#', pos_a+1);

    if (pos_h == std::string::npos)
    {
        pos_amp = addr.find('&', pos_a+1);
        if (pos_amp == std::string::npos)
        {
            size = "0x" + addr.substr(pos_a+1);
        }
        else
        {
            size = "0x" + addr.substr(pos_a+1, pos_amp-pos_a-1);
            index2 = "0x" + addr.substr(pos_amp+1);
        }
    }
    else
    {
        size = "0x" + addr.substr(pos_a+1, pos_h-pos_a-1);

        pos_amp = addr.find('&', pos_h+1);
        if (pos_amp == std::string::npos)
        {
            index1 = "0x" + addr.substr(pos_h+1);
        }
        else
        {
            index1 = "0x" + addr.substr(pos_h+1, pos_amp-pos_h-1);
            index2 = "0x" + addr.substr(pos_amp+1);
        }

    }

    *poffset = Utils::GetUnsigned64(offset);
    *psize = Utils::GetUnsigned64(size);
    *pindex1 = (index1.length() == 0) ? -1 : (int)Utils::GetUnsigned(index1);
    *pindex2 = (index2.length() == 0) ? -1 : (int)Utils::GetUnsigned(index2);

    return true;
}

static std::string get_file_name(const std::string &name)
{
    static const char sep[4] = { (char)0xEF, (char)0xBC, (char)0xBB, 0x00 };
    static const char sep2[4] = { (char)0xEF, (char)0xBC, (char)0xBD, 0x00 };

    size_t pos = name.find(sep);
    std::string ret;
    bool is_r = (name.length() > 2 && name[0] == 'R' && name[1] == '_');

    if (pos == std::string::npos)
        ret = name;
    else
    {
        size_t pos2 = name.rfind(sep2);
        if (pos2 == std::string::npos)
        {
            ret = name.substr(pos+3);
        }
        else
        {
            ret = name.substr(pos+3, pos2-pos-3);
            if (is_r)
            {
                // Append lower case extension
                ret.push_back('.');
                for (size_t i = 2; i < pos; i++)
                {
                    char ch = name[i];

                    if (ch >= 'A' && ch <= 'Z')
                        ch += 'a' - 'A';

                    ret.push_back(ch);
                }
            }
        }
    }

    return ret;
}

bool RdbFile::LoadAndBuildMap(const uint8_t *buf, size_t size, std::vector<RDBEntry *> *ret_entries)
{
    Reset();

    const uint8_t *p = buf;
    const uint8_t *end = p + size;

    const RDBHeader *hdr = (const RDBHeader *)p;
    if (size < sizeof(RDBHeader) || memcmp(hdr->signature, RDB_SIGNATURE, sizeof(hdr->signature)) != 0)
    {
        DPRINTF("Invalid RDB file.\n");
        return false;
    }

    name_db_file = hdr->name_db_file;

    p += hdr->header_size;
    entries.reserve(hdr->num_files);

    std::unordered_set<std::string> bin_files_list;

    if (ret_entries)
    {
        ret_entries->clear();
        ret_entries->reserve(hdr->num_files);
    }

    while (p < end)
    {
        while ((uintptr_t)(p - buf) & 3)
            p++;

        if (p >= end)
            break;

        const RDBEntry *entry = (const RDBEntry *)p;

        if (memcmp(entry->signature, RDB_ENTRY_SIGNATURE, sizeof(entry->signature)))
        {
            DPRINTF("Unknown signature at offset %Ix\n", (size_t)Utils::DifPointer64(entry, buf));
            return false;
        }

        int address_offset = (int)entry->entry_size - (int)entry->c_size;
        if (address_offset < 0)
        {
            DPRINTF("Something wrong at offset %Ix\n", (size_t)Utils::DifPointer64(entry, buf));
            return false;
        }

        if (ret_entries)
        {
            ret_entries->push_back(const_cast<RDBEntry *>(entry));
        }

        RdbEntry new_entry;

        new_entry.unk_data.resize(address_offset - sizeof(RDBEntry));
        if (new_entry.unk_data.size() > 0)
            memcpy(new_entry.unk_data.data(), p + sizeof(RDBEntry), new_entry.unk_data.size());

        for (int i = 0; i < 4; i++)
            new_entry.version.push_back(entry->version[i]);

        new_entry.file_size = entry->file_size;
        new_entry.type = entry->type;
        new_entry.file_id = entry->file_id;
        new_entry.type_id = entry->type_id;
        new_entry.flags = entry->flags;

        /*auto test = files_extensions.find(new_entry.type_id);
        if (test == files_extensions.end())
        {
            DPRINTF("Unrecognized type 0x%08x\n", new_entry.type_id);
            exit(-1);
        }*/

        /*if (entry->flags != 0x120000 && entry->flags != 0x110000 && entry->flags != 0x10000)
        {
            DPRINTF("Flags %08X at %Ix", entry->flags, (size_t)Utils::DifPointer64(entry, buf));
        }*/

        // Actually can check flags & RDB_FILE_EXTERNAL too
        if (entry->c_size == 0)
        {
            new_entry.external = true;
        }
        else
        {
            const char *address = (const char *)(p + address_offset);
            //DPRINTF("%s\n", address);

            if ((const uint8_t *)address >= end)
                break;


            new_entry.address = address;

            if (!decode_file_address(new_entry.address,  &new_entry.offset, &new_entry.size, &new_entry.index1, &new_entry.index2))
            {
                DPRINTF("Cannot decode address %s at %Ix\n", address, (size_t)Utils::DifPointer64(entry, buf));
                return false;
            }

            new_entry.bin_file = ".bin";

            if (new_entry.index1 != -1)
                new_entry.bin_file += Utils::UnsignedToString(new_entry.index1, false);

            if (new_entry.index2 != -1)
            {
                new_entry.bin_file.push_back('_');
                new_entry.bin_file += Utils::UnsignedToString(new_entry.index2, false);
            }

            bin_files_list.insert(new_entry.bin_file);
        }

        entries.push_back(new_entry);
        p += entry->entry_size;
    }

    for (const std::string &bin : bin_files_list)
    {
        //DPRINTF("Bin file: %s\n", bin.c_str());

        FileStream *stream = new FileStream("rb");
        if (!stream->LoadFromFile(rdb_path + bin))
        {
            delete stream;
            return false;
        }

        bin_files[bin] = stream;
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        const RdbEntry &entry = entries[i];
        MemoryStream mem;

        if (entry.file_id == hdr->name_db_file && entry.type_id == 0xbf6b52c7 && ExtractFile(i, &mem, true))
        {
            RnkFile rnk;

            if (rnk.Load(mem.GetMemory(false), (size_t)mem.GetSize()))
            {
                const std::map<uint32_t, std::vector<std::string>> &map = rnk.GetMap();

                for (auto &it : map)
                {
                    if (it.second.size() > 0)
                    {
                        names_map[it.first] = get_file_name(it.second[0]);
                    }
                }
            }
        }
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        const RdbEntry &entry = entries[i];

        auto it = names_map.find(entry.file_id);
        if (it != names_map.end())
        {
            auto it2 = files_extensions.find(entry.type_id);
            if (it2 != files_extensions.end())
            {
                if (!Utils::EndsWith(it->second, it2->second, false) && entry.type_id != 0xd7f47fb1) // 0xd7f47fb1 -> generic binary type
                {
                    if (IsFieldEditor4() && it2->second == ".efpl")
                    {
                        it->second += ".ktf2";
                    }
                    else
                    {
                        it->second += it2->second;
                    }
                }
            }
            else
            {
                static bool message_shown = false;

                if (!message_shown)
                {
                    DPRINTF("Unknown type 0x%08x for file %s (0x%08x)\n", entry.type_id, it->second.c_str(), entry.file_id);
                    message_shown = true;
                }
            }
        }
    }

    if (IsCharacterEditor())
    {
        LoadAdditionalNames((const char *)additional_fn_ce);
    }
    else if (IsMaterialEditor())
    {
        //int tick = GetTickCount();
        LoadAdditionalNames((const char *)additional_fn_me);
        //UPRINTF("Elapsed: %d\n", GetTickCount() - tick);
    }
    else if (IsRRPreview())
    {
        LoadAdditionalNames((const char *)additional_fn_rp);
    }
    else if (IsKIDSSystemResource())
    {
        LoadAdditionalNames((const char *)additional_fn_kd);
    }
    else if (IsFieldEditor4())
    {
        LoadAdditionalNames((const char *)additional_fn_fe4);
    }
    else if (IsSystem())
    {
        LoadAdditionalNames((const char *)additional_fn_sys);
    }
    else if (IsScreenLayout())
    {
        LoadAdditionalNames((const char *)additional_fn_screen);
    }
#ifdef OPPW4
    else if (IsSequenceEditor())
    {
        LoadAdditionalNames((const char *)additional_fn_seq);
    }
#endif

    //DPRINTF("Num files = %Id\n", entries.size());
    return true;
}


bool RdbFile::Load(const uint8_t *buf, size_t size)
{
    return LoadAndBuildMap(buf, size, nullptr);
}

uint8_t *RdbFile::Save(size_t *psize)
{
    MemoryStream out;
    RDBHeader hdr;

    memset(&hdr, 0, sizeof(RDBHeader));
    memcpy(hdr.signature, RDB_SIGNATURE, sizeof(hdr.signature));
    memcpy(hdr.version, "0000", 4);
    hdr.header_size = 0x20;
    hdr.plattform = 0xA;
    hdr.num_files = (uint32_t)entries.size();
    hdr.name_db_file = name_db_file;
    strcpy(hdr.path, "data/");

    if (!out.Write(&hdr, sizeof(hdr)))
        return nullptr;

    for (const RdbEntry &entry : entries)
    {
        RDBEntry file_entry;

        memset(&file_entry, 0, sizeof(file_entry));
        memcpy(file_entry.signature , RDB_ENTRY_SIGNATURE, sizeof(file_entry.signature));
        memcpy(file_entry.version, entry.version.c_str(), 4);

        if (!entry.external)
            file_entry.c_size = (uint32_t)entry.address.length()+1;
        else
            file_entry.c_size = 0;

        file_entry.entry_size = sizeof(RDBEntry) + (uint32_t)entry.unk_data.size() + (uint32_t)file_entry.c_size;

        file_entry.file_size = entry.file_size;
        file_entry.type = entry.type;
        file_entry.file_id = entry.file_id;
        file_entry.type_id = entry.type_id;
        file_entry.flags = entry.flags;

        if (!out.Write(&file_entry, sizeof(file_entry)))
            return nullptr;

        if (entry.unk_data.size() > 0)
        {
            if (!out.Write(entry.unk_data.data(), entry.unk_data.size()))
                return nullptr;
        }

        if (!entry.external)
        {
            if (!out.Write(entry.address.c_str(), entry.address.length()+1))
                return nullptr;
        }

        while (out.GetSize() & 3)
        {
            if (!out.Write8(0))
                return nullptr;
        }
    }

    *psize = (size_t)out.GetSize();
    return out.GetMemory(true);
}

bool RdbFile::ReloadAsDeadFiles(const std::string &bin1, const std::string &bin2, const std::string &bin3, const std::string &bin4)
{
    std::vector<RdbEntry> new_entries;

    for (auto &it : bin_files)
    {
        if (it.first != bin1 && it.first != bin2 && it.first != bin3 && it.first != bin4)
            continue;

        FileStream *bin = it.second;
        bin->Seek(0, SEEK_SET);

        while (true)
        {
            RDBEntry nentry;
            RdbEntry entry;
            uint64_t pos = Utils::Align2((size_t)bin->Tell(), 4); // Mmm, this won't work on 32 bits...

            bool eof = false;

            while (true)
            {
                bin->Seek(pos, SEEK_SET);

                if (!bin->Read(&nentry, sizeof(RDBEntry)))
                {
                    eof = true;
                    break;
                }

                if (memcmp(nentry.signature, RDB_ENTRY_SIGNATURE, 4) == 0)
                    break;

                pos += 4;
                bin->Seek(pos, SEEK_SET);
            }

            if (eof)
                break;

            bool add = true;

             size_t idx = FindFileByID(nentry.file_id);
             if (idx != (size_t)-1 && entries[idx].offset == pos)
                add = false;

             for (int i = 0; i < 4; i++)
                 entry.version.push_back(nentry.version[i]);

             entry.address = "DEAD";
             entry.bin_file = it.first;
             entry.offset = pos;
             entry.size = nentry.entry_size;
             entry.index1 = entry.index2 = -1;
             entry.external = false;
             entry.file_size = nentry.file_size;
             entry.type = nentry.type;
             entry.file_id = nentry.file_id;
             entry.type_id = nentry.type_id;
             entry.flags = nentry.flags;

             if (add)
             {
                 new_entries.push_back(entry);
                 //printf("Adding %08x\n", entry.file_id);
             }

             bin->Seek(pos + nentry.entry_size, SEEK_SET);
        }
    }

    //DPRINTF("Found %Id dead files\n", new_entries.size());
    entries = new_entries;
    return true;
}

void RdbFile::BuildAdditionalLookup(bool build_name_rev, bool build_hash_to_idx)
{
    if (build_name_rev)
    {
        names_map_rev.clear();

        for (auto &it : names_map)
        {
            names_map_rev[Utils::ToLowerCase(it.second)] = it.first;
        }
    }

    if (build_hash_to_idx)
    {
        hash_to_idx.clear();

        for (size_t i = 0; i < entries.size(); i++)
        {
            hash_to_idx[entries[i].file_id] = i;
        }
    }
}

std::string RdbFile::GetExternalPath(size_t idx) const
{
    if (idx >= entries.size())
        return "";

    const RdbEntry &entry = entries[idx];

    if (!entry.external)
        return "";

    std::string dir;

    if (rdb_path.find('/') != std::string::npos || rdb_path.find('\\') != std::string::npos)
        dir = Utils::GetDirNameString(rdb_path);
    else
        dir = ".";

    return Utils::MakePathString(dir, "data/" + Utils::UnsignedToHexString(entry.file_id, true) + ".file");
}

bool RdbFile::ExtractFile(size_t idx, Stream *out, bool omit_external_error, bool external_error_is_success)
{
    FileStream *stream;

    if (idx >= entries.size())
        return false;

    const RdbEntry &entry = entries[idx];

    if (entry.external)
    {
        std::string dir;

        if (rdb_path.find('/') != std::string::npos || rdb_path.find('\\') != std::string::npos)
            dir = Utils::GetDirNameString(rdb_path);
        else
            dir = ".";

        std::string external_path = Utils::MakePathString(dir, "data/" + Utils::UnsignedToHexString(entry.file_id, true) + ".file");

        stream = new FileStream("rb");
        if (!stream->LoadFromFile(external_path, !omit_external_error))
        {
            delete stream;
            return external_error_is_success;
        }
    }
    else
    {
        auto it = bin_files.find(entry.bin_file);

        if (it == bin_files.end())
        {
            DPRINTF("Implementation error, binary file %s was not loaded.\n", entry.bin_file.c_str());
            return false;
        }

        stream = it->second;
    }

    if (!stream->Seek(entry.offset, SEEK_SET))
    {
        if (entry.external)
            delete stream;

        return false;
    }

    RDBEntry file_entry;

    if (!stream->Read(&file_entry, sizeof(file_entry)))
    {
        if (entry.external)
            delete stream;

        return false;
    }

    off64_t offset = file_entry.entry_size - file_entry.c_size - sizeof(RDBEntry);

    if (!stream->Seek(offset, SEEK_CUR))
    {
        if (entry.external)
            delete stream;

        return false;
    }

    if (file_entry.file_size == 0)
        return true;

    size_t out_size = file_entry.file_size;
    uint8_t *out_buf = new uint8_t[out_size];
    size_t extracted_size = 0;
    bool ret = false;

    if (!(entry.flags & RDB_FLAG_COMPRESSED))
    {
        // Not compressed
        // Fixme: read in chunks
        if (out_size > 0 && !stream->Read(out_buf, out_size))
        {
            delete[] out_buf;

            if (entry.external)
                delete stream;

            return false;
        }

        ret = out->Write(out_buf, out_size);
    }
    else
    {
        // Compressed
        while (extracted_size < out_size)
        {
            uint32_t in_size;
            uint8_t *in_buf;

            if (!stream->Read32(&in_size))
            {
                delete[] out_buf;

                if (entry.external)
                    delete stream;

                return false;
            }

            //DPRINTF("in_size = %Id out_size = %Id  offset = %Ix\n", in_size, out_size, stream->Tell());

            in_buf = new uint8_t[in_size];
            if (!stream->Read(in_buf, in_size))
            {
                delete[] out_buf;
                delete[] in_buf;

                if (entry.external)
                    delete stream;

                return false;
            }

            uint32_t this_size = (uint32_t)out_size;
            ret = Utils::UncompressZlib(out_buf, &this_size, in_buf, in_size);
            delete[] in_buf;

            //UPRINTF("%d uncompressed to %d\n", in_size, this_size);

            if (ret)
            {
                if (!out->Write(out_buf, this_size))
                {
                    ret = false;
                    break;
                }

                extracted_size += this_size;
            }
            else
            {
                break;
            }
        }
    }

    if (entry.external)
        delete stream;

    delete[] out_buf;
    return ret;
}

bool RdbFile::ExtractFile(size_t idx, const std::string &path, bool omit_external_error, bool external_error_is_success)
{
    std::string out_path = path;

    if (idx >= entries.size())
        return false;

    const RdbEntry &entry = entries[idx];

    if (path.length() == 0 || path.back() == '/' || path.back() == '\\')
    {
        auto it = names_map.find(entry.file_id);
        if (it != names_map.end())
        {
            out_path += it->second;
        }
        else
        {
            //out_path += entry.address;
            out_path += Utils::UnsignedToHexString(entry.file_id, true);
        }

        auto it2 = files_extensions.find(entry.type_id);
        if (it2 != files_extensions.end())
        {
            if (!Utils::EndsWith(out_path, it2->second, false))
            {
                if (IsFieldEditor4() && it2->second == ".efpl")
                {
                    out_path += ".ktf2";
                }
                else
                {
                    out_path += it2->second;
                }
            }
        }
    }

    FileStream out("wb");

    if (!out.LoadFromFile(out_path))
        return false;

    //return true;

    return ExtractFile(idx, &out, omit_external_error, external_error_is_success);
}

bool RdbFile::SetWriteBin(const std::string &file, int index1, int index2)
{
    std::string bin = ".bin";

    if (index1 != -1)
         bin += Utils::UnsignedToString(index1, false);

    if (index2 != -1)
    {
        bin.push_back('_');
        bin += Utils::UnsignedToString(index2, false);
    }

    auto it = bin_files.find(bin);
    if (it != bin_files.end())
    {
        wb = it->second;
        if (!wb->Reopen("r+b"))
        {
            //DPRINTF("Reopen failed.\n");
            return false;
        }
    }
    else
    {
        wb = new FileStream();
        if (!wb->LoadFromFile(file))
            return false;

        bin_files[bin] = wb;
    }

    w_index1 = index1;
    w_index2 = index2;
    return true;
}

bool RdbFile::ReimportFile(size_t idx, Stream *in)
{
    if (!wb)
        return false;

    if (idx >= entries.size())
        return false;

    RdbEntry &entry = entries[idx];

    if (entry.external)
    {
        DPRINTF("Fixme: currently cannot process external files for reimport.\n");
        return false;
    }

    auto it = bin_files.find(entry.bin_file);
    if (it == bin_files.end())
        return false;

    FileStream *stream = it->second;

    RDBEntry file_entry;
    std::vector<uint8_t> unk_data;

    if (!stream->Seek(entry.offset, SEEK_SET))
        return false;

    if (!stream->Read(&file_entry, sizeof(file_entry)))
        return false;

    off64_t unk_size = file_entry.entry_size - file_entry.c_size - sizeof(RDBEntry);
    unk_data.resize(unk_size);
    if (unk_data.size() > 0 && !stream->Read(unk_data.data(), unk_data.size()))
        return false;

    wb->Seek(0, SEEK_END);

    while (wb->GetSize() & 0xF)
        wb->Write8(0);

    uint64_t offset = wb->GetSize();
    uint64_t file_size = in->GetSize();
    uint64_t comp_size = 0;

    if (!wb->Write(&file_entry, sizeof(RDBEntry)))
        return false;

    if (unk_data.size() > 0 && !wb->Write(unk_data.data(), unk_data.size()))
        return false;

    uint64_t remaining_size = file_size;
    size_t read_size = (entry.flags & RDB_FLAG_COMPRESSED) ? RDB_COMP_CHUNK_SIZE : BUFFER_SIZE_UNCOMP;
    uint8_t *uncomp_buf = new uint8_t[read_size];
    size_t comp_buf_size = RDB_COMP_CHUNK_SIZE*2;
    uint8_t *comp_buf = ((entry.flags & RDB_FLAG_COMPRESSED) ? new uint8_t[comp_buf_size] : nullptr);

    while (remaining_size != 0)
    {
        size_t this_read_size = read_size;

        if (remaining_size < this_read_size)
            this_read_size = remaining_size;

        if (!in->Read(uncomp_buf, this_read_size))
        {
            delete[] uncomp_buf;

            if (comp_buf)
                delete[] comp_buf;

            return false;
        }

        remaining_size -= this_read_size;

        if (entry.flags & RDB_FLAG_COMPRESSED)
        {
            unsigned long int this_comp_size = (unsigned long int)comp_buf_size;

            if (!Utils::CompressZlib(comp_buf, &this_comp_size, uncomp_buf, this_read_size))
            {
                delete[] uncomp_buf;
                delete[] comp_buf;

                return false;
            }

            if (!wb->Write32(this_comp_size))
            {
                delete[] uncomp_buf;
                delete[] comp_buf;

                return false;
            }

            comp_size += 4;

            if (!wb->Write(comp_buf, this_comp_size))
            {
                delete[] uncomp_buf;
                delete[] comp_buf;

                return false;
            }

            comp_size += this_comp_size;
        }
        else
        {
            if (!wb->Write(uncomp_buf, this_read_size))
            {
                delete[] uncomp_buf;
                return false;
            }

            comp_size += this_read_size;
        }
    }

    delete[] uncomp_buf;
    if (comp_buf)
        delete[] comp_buf;

    if (entry.flags & RDB_FLAG_COMPRESSED)
    {
        if (!wb->Write32(0))
            return false;

        comp_size += 4;
    }

    file_entry.entry_size = sizeof(RDBEntry) + (uint32_t)unk_data.size() + (uint32_t)comp_size;
    file_entry.c_size = (uint32_t)comp_size;
    file_entry.file_size = file_size;

    if (!wb->Seek(offset, SEEK_SET))
        return false;

    if (!wb->Write(&file_entry, sizeof(RDBEntry)))
        return false;

    wb->Seek(0, SEEK_END);

    entry.address = Utils::UnsignedToHexString(offset, false, false);
    entry.address.push_back('@');
    entry.address += Utils::UnsignedToHexString(file_entry.entry_size, false, false);

    entry.bin_file = ".bin";
    if (w_index1 != -1)
    {
         entry.bin_file += Utils::UnsignedToString(w_index1, false);

         entry.address.push_back('#');
         entry.address += Utils::UnsignedToString(w_index1, false);
    }

    if (w_index2 != -1)
    {
        entry.bin_file.push_back('_');
        entry.bin_file += Utils::UnsignedToString(w_index2, false);

        entry.address.push_back('&');
        entry.address += Utils::UnsignedToString(w_index2, false);
    }

    entry.offset = offset;
    entry.size = file_entry.entry_size;
    entry.index1 = w_index1;
    entry.index2 = w_index2;
    entry.external = false;
    entry.file_size = file_entry.file_size;

    return true;
}

size_t RdbFile::FindFileByID(uint32_t id) const
{
    if (hash_to_idx.size() != 0)
    {
        auto it = hash_to_idx.find(id);
        if (it == hash_to_idx.end())
            return (size_t)-1;

        return it->second;
    }

    for (size_t i = 0; i < entries.size(); i++)
    {
        if (entries[i].file_id == id)
            return i;
    }

    return (size_t)-1;
}

size_t RdbFile::FindFileByName(const std::string &name) const
{    
    if (names_map_rev.size() > 0)
    {
        auto it = names_map_rev.find(Utils::ToLowerCase(name));
        if (it == names_map_rev.end())
            return (size_t)-1;

        return FindFileByID(it->second);
    }

    /*std::string ext;
    std::string base_name;
    std::string lc_name = Utils::ToLowerCase(name);
    uint32_t filter_type = 0;

    size_t dot = lc_name.rfind('.');
    if (dot != std::string::npos)
    {
        base_name = lc_name.substr(0, dot);
        ext = lc_name.substr(dot);
        filter_type = GetTypeByExtension(ext);

        if (filter_type == 0)
            base_name = lc_name;
    }
    else
    {
        base_name = lc_name;
    }

    for (auto it : names_map)
    {
        if (Utils::ToLowerCase(it.second) == base_name)
        {
            size_t idx = FindFileByID(it.first);
            if (idx != (size_t)-1)
            {
                if (filter_type == 0 || MatchesType(idx, filter_type))
                    return idx;
            }
        }
    }*/
    std::string lc_name = Utils::ToLowerCase(name);

    for (auto &it : names_map)
    {
        if (Utils::ToLowerCase(it.second) == lc_name)
        {
            size_t idx = FindFileByID(it.first);
            if (idx != (size_t)-1)
                return idx;
        }
    }

    return (size_t)-1;
}

uint32_t RdbFile::GetTypeByExtension(const std::string &ext) const
{
    std::string lc_ext = Utils::ToLowerCase(ext);

    if (lc_ext[0] != '.')
        lc_ext.insert(0, ".");

    if (lc_ext == ".ktf2")
        return 0xd7f47fb1;

    for (auto &it: files_extensions)
    {
        if (it.second == lc_ext)
            return it.first;
    }

    return 0;
}

bool RdbFile::MatchesType(size_t idx, uint32_t type) const
{
    if (idx >= entries.size())
        return false;

    return (entries[idx].type_id == type);
}

bool RdbFile::GetFileName(size_t idx, std::string &name) const
{
    if (idx >= entries.size())
        return false;

    const RdbEntry &entry = entries[idx];

    auto it = names_map.find(entry.file_id);
    if (it != names_map.end())
    {
        name = it->second;
    }
    else
    {
        name = Utils::UnsignedToHexString(entry.file_id, true);
        auto it2 = files_extensions.find(entry.type_id);
        if (it2 != files_extensions.end())
        {
            if (IsFieldEditor4() && it2->second == ".efpl")
            {
                name += ".ktf2";
            }
            else
            {
                name += it2->second;
            }
        }
    }

    return true;
}

bool RdbFile::GetFileNameByID(uint32_t id, std::string &name) const
{
    size_t idx = FindFileByID(id);
    if (idx == (size_t)-1)
        return false;

    return GetFileName(idx, name);
}

static bool get_hex_name(const std::string &fn, uint32_t *ret)
{
    if (!Utils::BeginsWith(fn, "0x", false))
        return false;

    if (fn.length() == 2)
        return false;

    for (size_t i = 2; i < fn.length(); i++)
    {
        char ch = tolower(fn[i]);

        if (ch == '.')
            break;

        bool ok = false;

        if (ch >= '0' && ch <= '9')
            ok = true;
        else if (ch >= 'a' && ch <= 'f')
            ok = true;

        if (!ok)
            return false;
    }

    *ret = Utils::GetUnsigned(fn);
    return true;
}

bool RdbFile::LoadAdditionalNames(const char *content)
{
    // New implementaton asumes there are no inner spaces in filenames
    char comp1[32], comp2[256];
    bool in_comment = false;
    bool in_c2 = false;

    size_t len = strlen(content);
    size_t c1 = 0, c2 = 0;
    int line_num = 1;


    for (size_t i = 0; i < len; i++)
    {
        char ch = content[i];

        if (ch == '\n')
        {
            if (in_comment)
            {
                in_comment = false;
            }
            else if (c1 != 0)
            {
                if (!in_c2)
                    DPRINTF("Faulty line %d.\n", line_num);

                comp1[c1] = 0;
                comp2[c2] = 0;

                uint32_t id;

                if (!get_hex_name(comp1, &id))
                {
                    DPRINTF("%s: parse error: \"%s\" is not a valid hash. (line %d)\n", FUNCNAME, comp1, line_num);
                    return false;
                }

                names_map[id] = comp2;

                if (names_map_rev.size() > 0)
                {
                    names_map_rev[comp2] = id;
                }
            }

            c1 = c2 = 0;
            in_c2 = false;
            line_num++;
            continue;
        }

        if (in_comment)
            continue;

        if (ch >= 0 && ch <= ' ')
            continue;

        if (c1 == 0 && (ch == ';' || ch == '#'))
        {
            in_comment = true;
            continue;
        }

        if (!in_c2 && ch == ',')
        {
            in_c2 = true;
            continue;
        }

        if (in_c2)
            comp2[c2++] = ch;
        else
            comp1[c1++] = ch;
    }

    // Old implementation, too slow!
    /*std::vector<std::string> lines;
    Utils::GetMultipleStrings(content, lines, '\n');

    for (std::string &line : lines)
    {
        Utils::TrimString(line);

        if (line.length() > 0 && (line[0] == ';' || line[0] == '#'))
            continue;

        std::vector<std::string> comps;

        if (Utils::GetMultipleStrings(line, comps) != 2)
        {
            DPRINTF("%s: parse error. Line doesn't have two components (line content = %s).\n", FUNCNAME, line.c_str());
            return false;
        }

        Utils::TrimString(comps[0]);
        Utils::TrimString(comps[1]);

        uint32_t id;

        if (!get_hex_name(comps[0], &id))
        {
            DPRINTF("%s: parse error: %s is not a valid hash.\n", FUNCNAME, comps[0].c_str());
            return false;
        }

        names_map[id] = comps[1];

        if (names_map_rev.size() > 0)
        {
            names_map_rev[comps[1]] = id;
        }
    }*/

    return true;
}

void RdbFile::LoadAdditionalNames(const std::unordered_map<uint32_t, const char *> &map)
{
    for (auto &it : map)
    {
        names_map[it.first] = it.second;

        if (names_map_rev.size() > 0)
        {
            names_map_rev[it.second] = it.first;
        }
    }
}

bool RdbFile::SaveNamesToTxt(const std::string &path)
{
    FILE *f = fopen(path.c_str(), "w");
    if (!f)
    {
        DPRINTF("%s: Error opening file \"%s\"\n", FUNCNAME, path.c_str());
        return false;
    }

    for (auto &it : names_map)
    {
        bool has_dot = (it.second.rfind(".") != std::string::npos);
        std::string name = it.second;

        if (!has_dot)
        {
            //fprintf(f, "; Discarded file---");
            size_t idx = FindFileByName(name);
            if (idx ==  (size_t)-1)
            {
                //fprintf(f, "; Discarded file---");
                continue;
            }
            else
            {
                auto it2 = files_extensions.find(entries[idx].type_id);
                if (it2 != files_extensions.end())
                {
                    if (IsFieldEditor4() && it2->second == ".efpl")
                    {
                        name += ".ktf2";
                    }
                    else
                    {
                        name += it2->second;
                    }
                }
            }
        }

        fprintf(f, "0x%08x,%s\n", it.first, name.c_str());
    }

    fclose(f);
    return true;
}

bool RdbFile::IsCharacterEditor() const
{
#ifndef OPPW4
    return (FindFileByName("CharacterEditor.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0xde91b561) != (size_t)-1);
#endif
}

bool RdbFile::IsMaterialEditor() const
{
#ifndef OPPW4
    return (FindFileByName("MaterialEditor.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0x906fb5bd) != (size_t)-1);
#endif
}

bool RdbFile::IsRRPreview() const
{
#ifndef OPPW4
    return (FindFileByName("RRPreview.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0x067a824f) != (size_t)-1);
#endif
}

bool RdbFile::IsKIDSSystemResource() const
{
#ifndef OPPW4
    return (FindFileByName("KIDSSystemResource.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0xbcc13c53) != (size_t)-1);
#endif
}

bool RdbFile::IsFieldEditor4() const
{
    if (!fe4_check)
    {
#ifndef OPPW4
        fe4_ret = (FindFileByName("FieldEditor4.rdb.name") != (size_t)-1);
#else
        fe4_ret = (FindFileByID(0xf437fe16) != (size_t)-1);
#endif
        fe4_check = true;
    }

    return fe4_ret;
}

bool RdbFile::IsSystem() const
{
#ifndef OPPW4
    return (FindFileByName("system.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0x0a603d98) != (size_t)-1);
#endif
}

bool RdbFile::IsScreenLayout() const
{
#ifndef OPPW4
    return (FindFileByName("ScreenLayout.rdb.name") != (size_t)-1);
#else
    return (FindFileByID(0x8c759c7f) != (size_t)-1);
#endif
}

bool RdbFile::IsSequenceEditor() const
{
    return (FindFileByID(0x45835957) != (size_t)-1);
}
