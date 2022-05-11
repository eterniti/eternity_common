#include "HfsFile.h"
#include "HfsXorTable.h"
#include "FixedMemoryStream.h"
#include "debug.h"

#define COMP_SIGNATURE  0x706D6F63

std::string HfsEntry::GetName() const
{
    if (Utils::EndsWith(name, ".comp", false))
    {
        return name.substr(0, name.length()-5);
    }

    return name;
}

bool HfsEntry::Write(Stream *stream) const
{
    if (Utils::EndsWith(name, ".comp", false))
    {
        if (data.size() < 8)
            return false;

        uint8_t *comp_data = (uint8_t *)data.data();
        uint32_t signature = *(uint32_t *)comp_data;
        uint32_t uncomp_size = *(uint32_t *)&comp_data[4];

        if (signature != COMP_SIGNATURE)
            return false;

        uint8_t *uncomp_data = new uint8_t[uncomp_size];

        if (!Utils::UncompressZlib(uncomp_data, &uncomp_size, comp_data+8, (uint32_t)data.size()-8))
        {
            DPRINTF("%s: Uncompress failed.\n", FUNCNAME);
            return false;
        }

        bool ret = stream->Write(uncomp_data, uncomp_size);
        delete[] uncomp_data;
        return ret;
    }

    return stream->Write(data.data(), data.size());
}

HfsFile::HfsFile()
{
    this->big_endian = false;
}

HfsFile::~HfsFile()
{

}

void HfsFile::Reset()
{
    entries.clear();
}

void HfsFile::Decrypt(std::vector<uint8_t> &data, size_t start_pos)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        data[i] ^= hfs_xor_table[(start_pos+i)&0xFFF];
    }
}

void HfsFile::Decrypt(std::string &string, size_t start_pos)
{
    for (size_t i = 0; i < string.size(); i++)
    {
        string[i] ^= hfs_xor_table[(start_pos+i)&0xFFF];
    }
}

bool HfsFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream fs(const_cast<uint8_t *>(buf), size);

    if (!fs.Seek(-(int64_t)sizeof(HFSCentralDirHeader), SEEK_END))
        return false;

    HFSCentralDirHeader *hdr;

    if (!fs.FastRead((uint8_t **)&hdr, sizeof(HFSCentralDirHeader)))
        return false;

    if (hdr->signature != HFS_SIGNATUE_CD_HEADER)
    {
        //DPRINTF("%s: Invalid HFS central dir signature.\n", FUNCNAME);
        return false;
    }

    if (!fs.Seek(hdr->central_dir_offset, SEEK_SET))
        return false;

    entries.resize(hdr->num_entries);

    for (size_t i = 0; i < entries.size(); i++)
    {
        HfsEntry &entry = entries[i];
        HFSCentralDirEntry *cd_entry;
        HFSLocalHeader *lh;

        if (!fs.FastRead((uint8_t **)&cd_entry, sizeof(HFSCentralDirEntry)))
            return false;

        if (cd_entry->signature != HFS_SIGNATURE_ENTRY)
        {
            DPRINTF("%s: Invalid HFS signature in cd entry.\n", FUNCNAME);
            return false;
        }

        if (!fs.Seek(cd_entry->name_len+cd_entry->comment_len+cd_entry->extra_len, SEEK_CUR))
            return false;

        uint64_t ret_addr = fs.Tell();
        fs.Seek(cd_entry->offset, SEEK_SET);

        if (!fs.FastRead((uint8_t **)&lh, sizeof(HFSLocalHeader)))
            return false;

        if (lh->signature != HFS_SIGNATURE_ENTRY)
        {
            DPRINTF("%s: Invalid HFS signature in lh.\n", FUNCNAME);
            return false;
        }

        entry.name.reserve(lh->name_len);

        for (uint16_t i = 0; i < lh->name_len; i++)
        {
            char *c;

            if (!fs.FastRead((uint8_t **)&c, 1))
                return false;

            entry.name.push_back(*c);
        }

        Decrypt(entry.name, fs.Tell() - entry.name.length());

        if (!fs.Seek(lh->extra_len, SEEK_CUR))
            return false;

        entry.data.resize(lh->comp_size);

        if (!fs.Read(entry.data.data(), entry.data.size()))
            return false;

        Decrypt(entry.data, fs.Tell() - entry.data.size());

        //DPRINTF("%s  %Id\n", entry.name.c_str(), entry.GetSize());

        fs.Seek(ret_addr, SEEK_SET);
    }

    return true;
}


