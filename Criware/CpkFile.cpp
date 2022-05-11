#include <algorithm>
#include "CpkFile.h"
#include "FixedMemoryStream.h"
#include "CrilaylaFixedBitStream.h"
#include "HcaFile.h"
#include "debug.h"

CpkFile::CpkFile()
{
    big_endian = false;
    fstream = nullptr;
    Reset();
}

CpkFile::~CpkFile()
{
    Reset();
}

void CpkFile::Reset()
{
    if (fstream)
    {
        delete fstream;
        fstream = nullptr;
    }

    cpk_header.Load(nullptr, 0);
    toc.Load(nullptr, 0);
    itoc.Load(nullptr, 0);
    etoc.Load(nullptr, 0);
    datah.Load(nullptr, 0);
    datal.Load(nullptr, 0);

    use_encryption = false;
    has_toc = false;
    has_itoc = false;
    has_etoc = false;
    is_itoc_extend = false;
    files_l = files_h = 0;

    content_offset = 0;
    align = 0x20;

    unk_04 = toc_unk_04 = itoc_unk_04 = etoc_unk_04 = 0;

    entries.clear();
}

void CpkFile::ToggleEncryption(uint8_t *buf, uint64_t size)
{
    int32_t m, t;
    uint8_t d;

    m = 0x0000655f;
    t = 0x00004115;

    for (uint64_t i = 0; i < size; i++)
    {
        d = buf[i];
        d = (uint8_t)(d ^ (uint8_t)(m & 0xff));
        buf[i] = d;
        m *= t;
    }
}

bool CpkFile::LoadTable(Stream *stream, UtfFile *table, const std::string &name, const std::string &alt_name)
{
    uint64_t table_size;
    uint32_t signature;
    uint8_t *utf;

    bool delete_utf = false;

    if (!stream->Read64(&table_size))
        return false;

    if (!stream->Read32(&signature))
        return false;

    stream->Seek(-4, SEEK_CUR);

    if (signature != UTF_SIGNATURE)
    {
        utf = new uint8_t[table_size];
        delete_utf = true;

        if (!stream->Read(utf, table_size))
        {
            delete[] utf;
            return false;
        }

        ToggleEncryption(utf, table_size);

        if (*(uint32_t *)utf != UTF_SIGNATURE)
        {
            delete[] utf;
            DPRINTF("%s: Failed to load table \"%s\". UTF signature not found.\n", FUNCNAME, name.c_str());
            return false;
        }

        use_encryption = true;
    }
    else
    {
        MemoryStream *memory = dynamic_cast<MemoryStream *>(stream);

        if (memory)
        {
            if (!memory->FastRead(&utf, table_size))
                return false;
        }
        else
        {
            utf = new uint8_t[table_size];
            delete_utf = true;

            if (!stream->Read(utf, table_size))
            {
                delete[] utf;
                return false;
            }
        }
    }

    bool ret = table->Load(utf, table_size);

    if (delete_utf)
        delete[] utf;

    if (!ret)
    {
        DPRINTF("%s: Failed to load table \"%s\".\n", FUNCNAME, name.c_str());
    }
    else
    {
        std::string table_name = table->GetTableName();

        if (Utils::ToLowerCase(name) != Utils::ToLowerCase(table_name))
        {
            if (alt_name.length() == 0)
            {
                DPRINTF("%s: Warning, table name doesn't match expected. Expected: \"%s\" found: \"%s\".\n", FUNCNAME, name.c_str(), table_name.c_str());
            }
            else
            {
                if (Utils::ToLowerCase(alt_name) != Utils::ToLowerCase(table_name))
                {
                    DPRINTF("%s: Warning, table_name doesn't match any of the expected. Expected \"%s\" or \"%s\" found: \"%s\".\n", FUNCNAME, name.c_str(), alt_name.c_str(), table_name.c_str());
                }
            }
        }
    }

    return ret;
}

bool CpkFile::ReadItocEntry(uint32_t id, CpkEntry &entry)
{
    uint32_t extract_size;
    uint16_t extract_size16;
    uint16_t size16, id16;

    if (files_l == 0)
    {
        if (!datah.GetDword("FileSize", &entry.size, id))
        {
            DPRINTF("%s: FileSize is mandatory (datah). Failed at row 0x%x\n", FUNCNAME, id);
            return false;
        }

        if (datah.GetDword("ExtractSize", &extract_size, id) && extract_size != entry.size)
        {
            DPRINTF("%s: Compressed CPK not supported yet.\n", FUNCNAME);
            return false;
        }

        if (datah.GetWord("ID", &id16, id))
        {
            entry.id = id16;
            entry.has_id = true;
        }

        if (id16 != id)
        {
            DPRINTF("%s: This ITOC is unordered, not supported.\n", FUNCNAME);
            return false;
        }

        return true;
    }

    if (files_h == 0)
    {
        if (!datal.GetWord("FileSize", &size16, id))
        {
            DPRINTF("%s: FileSize is mandatory (datal). Failed at row 0x%x\n", FUNCNAME, id);
            return false;
        }

        entry.size = size16;

        if (datal.GetWord("ExtractSize", &extract_size16, id) && extract_size16 != entry.size)
        {
            DPRINTF("%s: Compressed CPK not supported yet.\n", FUNCNAME);
            return false;
        }

        if (datal.GetWord("ID", &id16, id))
        {
            entry.id = id16;
            entry.has_id = true;
        }

        if (id16 != id)
        {
            DPRINTF("%s: This ITOC is unordered, not supported.\n", FUNCNAME);
            return false;
        }

        return true;
    }

    for (uint32_t i = 0; i < files_l; i++)
    {
        if (!datal.GetWord("ID", &id16, i) || id16 != id)
            continue;

        entry.id = id16;
        entry.has_id = true;

        if (!datal.GetWord("FileSize", &size16, i))
        {
            DPRINTF("%s: FileSize is mandatory (datal). Failed at row 0x%x\n", FUNCNAME, i);
            return false;
        }

        entry.size = size16;

        if (datal.GetWord("ExtractSize", &extract_size16, i) && extract_size16 != entry.size)
        {
            DPRINTF("%s: Compressed CPK not supported yet.\n", FUNCNAME);
            return false;
        }

        return true;
    }

    for (uint32_t i = 0; i < files_h; i++)
    {
        if (!datah.GetWord("ID", &id16, i) || id16 != id)
            continue;

        entry.id = id16;
        entry.has_id = true;

        if (!datah.GetDword("FileSize", &entry.size, i))
        {
            DPRINTF("%s: FileSize is mandatory (datal). Failed at row 0x%x\n", FUNCNAME, i);
            return false;
        }

        if (datah.GetDword("ExtractSize", &extract_size, i) && extract_size != entry.size)
        {
            DPRINTF("%s: Compressed CPK not supported yet.\n", FUNCNAME);
            return false;
        }

        return true;
    }

    return false;
}

bool CpkFile::WriteItocEntry(uint32_t id, const CpkEntry &entry)
{
    uint16_t id16;
    uint32_t size;

    if (!entry.GetSize(&size))
        return false;

    for (uint32_t i = 0; i < files_l; i++)
    {
        if (!datal.GetWord("ID", &id16, i) || id16 != id)
            continue;

        if (size >= 0x10000)
        {
            DPRINTF("%s: FIXME, move this file to DataH\n", FUNCNAME);
            abort();
            return false;
        }

        datal.SetWord("FileSize", (uint16_t)size, (unsigned int)i);
        datal.SetWord("ExtractSize", (uint16_t)size, (unsigned int)i);

        return true;
    }

    for (uint32_t i = 0; i < files_h; i++)
    {
        if (!datah.GetWord("ID", &id16, i) || id16 != id)
            continue;

        datah.SetDword("FileSize", size, (unsigned int)i);
        datah.SetDword("ExtractSize", size, (unsigned int)i);

        return true;
    }

    return false;
}

/*static bool CpkEntriesSorter(const CpkEntry &e1, const CpkEntry &e2)
{
    return (e1.toc_index < e2.toc_index);
}*/

bool CpkFile::LoadCommon(Stream *stream)
{
    CPKHeader hdr;
    uint64_t toc_offset;

    //DPRINTF("Loading cpk with size %I64d\n", stream->GetSize());

    if (!stream->Read(&hdr, sizeof(CPKHeader)))
        return false;

    if (hdr.signature != CPK_SIGNATURE)
        return false;

    unk_04 = hdr.unk_04;    

    if (!LoadTable(stream, &cpk_header, "CpkHeader"))
        return false;    

    if (cpk_header.GetQword("TocOffset", &toc_offset) && toc_offset != 0)
    {
        TOCHeader toc_hdr;

        if (!stream->Seek(toc_offset, SEEK_SET))
            return false;

        if (!stream->Read(&toc_hdr, sizeof(TOCHeader)) || toc_hdr.signature != TOC_SIGNATURE)
        {
            //DPRINTF("Failed toc hdr. %I64d %I64d\n", toc_offset, stream->Tell());
            return false;
        }

        if (!LoadTable(stream, &toc, "CpkTocInfo", "CpkTocInfoEx"))
            return false;

        has_toc = true;
        toc_unk_04 = toc_hdr.unk_04;
    }

    if (cpk_header.GetQword("ItocOffset", &toc_offset) && toc_offset != 0)
    {
        ITOCHeader itoc_hdr;

        if (!stream->Seek(toc_offset, SEEK_SET))
            return false;

        if (!stream->Read(&itoc_hdr, sizeof(ITOCHeader)) || itoc_hdr.signature != ITOC_SIGNATURE)
            return false;

        if (!LoadTable(stream, &itoc, "CpkItocInfo", "CpkExtendId"))
            return false;

        if (Utils::ToLowerCase(itoc.GetTableName()) == "cpkextendid")
        {
            is_itoc_extend = true;
        }

        has_itoc = true;
        itoc_unk_04 = itoc_hdr.unk_04;
    }

    if (cpk_header.GetQword("EtocOffset", &toc_offset) && toc_offset != 0)
    {
        ITOCHeader etoc_hdr;

        if (!stream->Seek(toc_offset, SEEK_SET))
            return false;

        if (!stream->Read(&etoc_hdr, sizeof(ITOCHeader)) || etoc_hdr.signature != ETOC_SIGNATURE)
            return false;

        if (!LoadTable(stream, &etoc, "CpkEtocInfo"))
            return false;

        has_etoc = true;
        etoc_unk_04 = etoc_hdr.unk_04;
    }

    if (cpk_header.GetQword("GtocOffset", &toc_offset) && toc_offset != 0)
    {
        DPRINTF("%s: CPK with Gtoc not supported.\n", FUNCNAME);
        return false;
    }

    if (!has_toc && !has_itoc)
    {
        DPRINTF("%s: cpk should have toc, itoc or both.\n", FUNCNAME);
        return false;
    }

    if (!cpk_header.GetQword("ContentOffset", &content_offset))
    {
        DPRINTF("%s: ContentOffset param is mandatory.\n", FUNCNAME);
        return false;
    }

    if (!cpk_header.GetWord("Align", &align))
    {
        DPRINTF("%s: Align param is mandatory.\n", FUNCNAME);
        return false;
    }

    uint32_t num_files;

    if (!cpk_header.GetDword("Files", &num_files))
    {
        DPRINTF("%s: parameter \"Files\" is mandatory.\n", FUNCNAME);
        return false;
    }

    if (has_itoc && !is_itoc_extend)
    {
        if (has_toc)
        {
            DPRINTF("%s: toc and itoc available, but itoc format is not CpkExtendId one. Cannot proceed.\n", FUNCNAME);
            return false;
        }

        if (!itoc.GetDword("FilesL", &files_l))
        {
            DPRINTF("%s: I was expecting FilesL to be 0.\n", FUNCNAME);
            return false;
        }

        if (!itoc.GetDword("FilesH", &files_h))
        {
            DPRINTF("%s: FilesH should be mandatory for this itoc format.\n", FUNCNAME);
            return false;
        }

        if (files_l+files_h != num_files)
        {
            DPRINTF("%s: FilesL+FilesH is not same than number of files. (0x%x != 0x%x)\n", FUNCNAME, files_l+files_h, num_files);
            return false;
        }

        if (files_h != 0)
        {
            unsigned int datah_size;
            uint8_t *datah_buf;

            datah_buf = itoc.GetBlob("DataH", &datah_size);
            if (!datah_buf)
            {
                DPRINTF("%s: DataH should be mandatory for this itoc format.\n", FUNCNAME);
                return false;
            }

            if (!datah.Load(datah_buf, datah_size))
            {
                DPRINTF("%s: Cannot load DataH table.\n", FUNCNAME);
                return false;
            }
        }

        if (files_l != 0)
        {
            unsigned int datal_size;
            uint8_t *datal_buf;

            datal_buf = itoc.GetBlob("DataL", &datal_size);
            if (!datal_buf)
            {
                DPRINTF("%s: DataL should be mandatory for this itoc format.\n", FUNCNAME);
                return false;
            }

            if (!datal.Load(datal_buf, datal_size))
            {
                DPRINTF("%s: Cannot load DataL table.\n", FUNCNAME);
                return false;
            }
        }
    }


    entries.resize(num_files);

    uint64_t current_offset = content_offset;

    //DPRINTF("Content offset = %I64x\n", content_offset);

    for (uint32_t i = 0; i < num_files; i++)
    {
        CpkEntry &entry = entries[i];
        uint32_t file_size;

        if (has_toc)
        {
            if (!toc.GetQword("FileOffset", &entry.offset, i))
            {
                DPRINTF("%s: FileOffset is mandatory. Failed at row 0x%x\n", FUNCNAME, i);
                return false;
            }

            if (!toc.GetDword("ExtractSize", &entry.size, i))
            {
                DPRINTF("%s: ExtractSize is mandatory. Failed at row 0x%x\n", FUNCNAME, i);
                return false;
            }

            if (!toc.GetDword("FileSize", &file_size, i))
            {
                DPRINTF("%s: FileSize is mandatory. Failed at row 0x%x\n", FUNCNAME, i);
                return false;
            }

            if (file_size != entry.size)
                entry.compressed_size = file_size;
            else
                entry.compressed_size = 0;

            if (toc.GetString("FileName", &entry.file_name, i))
            {
                entry.has_name = true;
            }

            if (toc.GetString("DirName", &entry.dir_name, i) && !entry.has_name)
            {
                DPRINTF("%s: DirectoryName available but FileName is not. Cannot compute.\n", FUNCNAME);
                return false;
            }

            //DPRINTF("Offset = %I64x name=%s\n", entry.offset, entry.file_name.c_str());

            if (toc.GetDword("ID", &entry.id, i))
            {
                entry.has_id = true;
            }

            entry.offset += 0x800;
        }

        if (has_itoc)
        {
            if (!is_itoc_extend)
            {
                entry.offset = current_offset;
                //DPRINTF("offset %I64x\n", entry.offset);

                if (!ReadItocEntry(i, entry))
                    return false;

                //DPRINTF("id = %x, size = %x\n", entry.id, entry.size);
                current_offset += entry.size;

                if ((current_offset % align) != 0)
                {
                    current_offset += (align - (current_offset % align));
                }
            }
        }

        if (has_etoc)
        {
            if (etoc.GetQword("UpdateDateTime", &entry.update_date_time, i))
            {
                //DPRINTF("%I64x\n", entry.update_date_time);
                entry.has_date = true;
            }
        }
    }

    MemoryStream *memory = dynamic_cast<MemoryStream *>(stream);
    if (memory)
    {
        for (CpkEntry &entry : entries)
        {
            if (!memory->Seek(entry.offset, SEEK_SET))
            {
                DPRINTF("%s: Seek failed.\n", FUNCNAME);
                return false;
            }

            entry.buf = new uint8_t[entry.size];
            if (!memory->Read(entry.buf, entry.size))
            {
                DPRINTF("%s: Read failed.\n", FUNCNAME);
                return false;
            }

            entry.offset = -1;
        }
    }

    return true;
}

bool CpkFile::Load(const uint8_t *buf, size_t size)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);

    Reset();
    return LoadCommon(&stream);
}

bool CpkFile::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    fstream = new FileStream("rb");
    if (!fstream->LoadFromFile(path, show_error))
        return false;

    return LoadCommon(fstream);
}

uint64_t CpkFile::GetContentSize(bool final_align) const
{
    uint64_t size = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        uint32_t file_size;
        const CpkEntry &entry = entries[i];

        entry.GetSize(&file_size);
        size += file_size;

        if (final_align || i != (entries.size()-1))
        {
            if ((size % align) != 0)
                size += (align - (size % align));
        }
    }

    return size;
}

uint64_t CpkFile::GetEnabledDatasize() const
{
    uint64_t size = 0;

    for (const CpkEntry &entry : entries)
    {
        uint32_t file_size;

        entry.GetSize(&file_size);
        size += file_size;
    }

    return size;
}

void CpkFile::UpdateToc()
{
    if (!has_toc)
        return;

    uint64_t offset = 0;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CpkEntry &entry = entries[i];
        uint32_t size;

        if (entry.has_name)
        {
            toc.SetString("DirName", entry.dir_name, (unsigned int)i);
            toc.SetString("FileName", entry.file_name, (unsigned int)i);
        }

        entry.GetSize(&size);

        toc.SetDword("FileSize", size, (unsigned int)i);
        toc.SetDword("ExtractSize", size, (unsigned int)i);

        toc.SetQword("FileOffset", offset, (unsigned int)i);

        offset += size;
        if ((offset % align) != 0)
            offset += (align - (offset % align));

        if (entry.has_id)
        {
            toc.SetDword("ID", entry.id, (unsigned int)i);
        }
    }
}

void CpkFile::UpdateItoc()
{
    if (!has_itoc || is_itoc_extend)
        return;

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CpkEntry &entry = entries[i];
        uint32_t size;

        entry.GetSize(&size);

        if (!WriteItocEntry((uint32_t)i, entry))
        {
            DPRINTF("%s: WriteItocEntry failed. Unexpected. Aborting.\n", FUNCNAME);
            abort();
            return;
        }
    }

    if (files_l > 0)
    {
        size_t size;
        uint8_t *buf = datal.Save(&size);

        itoc.SetBlob("DataL", buf, (unsigned int)size);
        delete[] buf;
    }

    if (files_h > 0)
    {
        size_t size;
        uint8_t *buf = datah.Save(&size);

        itoc.SetBlob("DataH", buf, (unsigned int)size);
        delete[] buf;
    }
}

void CpkFile::UpdateEtoc()
{
    if (!has_etoc)
        return;

    for (size_t i = 0; i < entries.size(); i++)
    {
         const CpkEntry &entry = entries[i];

         if (entry.has_date)
         {
             etoc.SetQword("UpdateDateTime", entry.update_date_time, (unsigned int)i);
         }
    }
}

void CpkFile::UpdateHeader()
{
    uint64_t content_size = GetContentSize(true);
    uint64_t enabled_data_size = GetEnabledDatasize();

    if (has_toc && has_itoc)
    {
        // For whatever reason, original files multiply value by 2 when both toc and itco are available
        // It is probably a bug of official tool, but I will mimic it anyway
        enabled_data_size *= 2;
    }

    cpk_header.SetQword("ContentOffset", 0x800);
    cpk_header.SetQword("ContentSize", content_size);
    cpk_header.SetQword("EnabledDataSize", enabled_data_size);
    cpk_header.SetQword("EnabledPackedSize", enabled_data_size);
    cpk_header.SetDword("Files", (uint32_t)entries.size());
    cpk_header.SetWord("Align", align);

    uint64_t current_offset = content_size+0x800;

    if (has_toc)
    {
        size_t toc_size = toc.CalculateFileSize();

        //DPRINTF("toc_offset %I64x\n", current_offset);
        cpk_header.SetQword("TocOffset", current_offset);
        cpk_header.SetQword("TocSize", toc_size+0x10);

        current_offset += toc_size+0x10;

        if ((current_offset % align) != 0)
            current_offset += (align - (current_offset % align));
    }

    if (has_itoc)
    {
        size_t itoc_size = itoc.CalculateFileSize();

        //DPRINTF("itoc_offset %I64x\n", current_offset);
        cpk_header.SetQword("ItocOffset", current_offset);
        cpk_header.SetQword("ItocSize", itoc_size+0x10);

        current_offset += itoc_size+0x10;

        if ((current_offset % align) != 0)
            current_offset += (align - (current_offset % align));
    }

    if (has_etoc)
    {
        size_t etoc_size = etoc.CalculateFileSize();

        //DPRINTF("etoc_offset %I64x\n", current_offset);
        cpk_header.SetQword("EtocOffset", current_offset);
        cpk_header.SetQword("EtocSize", etoc_size+0x10);

        current_offset += etoc_size+0x10;

        if ((current_offset % align) != 0)
            current_offset += (align - (current_offset % align));
    }
}

bool CpkFile::SaveTable(Stream *stream, UtfFile *table, const std::string &name)
{
    size_t size;
    uint8_t *buf = table->Save(&size);

    if (!buf)
    {
        DPRINTF("%s: Failed to save table \"%s\"\n", FUNCNAME, name.c_str());
        return false;
    }

    uint64_t size64 = size;
    if (!stream->Write64(size64))
    {
        delete[] buf;
        return false;
    }

    if (use_encryption)
    {
        ToggleEncryption(buf, size64);
    }

    bool ret = stream->Write(buf, size);

    delete[] buf;
    return ret;
}


bool CpkFile::SaveCommon(Stream *stream)
{
    CPKHeader hdr;

    hdr.signature = CPK_SIGNATURE;
    hdr.unk_04 = unk_04;

    if (!stream->Write(&hdr, sizeof(CPKHeader)))
        return false;

    UpdateToc();
    UpdateItoc();
    UpdateEtoc();
    UpdateHeader();

    if (!SaveTable(stream, &cpk_header, "CpkHeader"))
        return false;

    if (stream->Tell() > 0x7FA)
    {
        DPRINTF("%s: Header was too large.\n", FUNCNAME);
        return false;
    }

    if (!stream->Align(0x7FA))
        return false;

    if (!stream->Write("(c)CRI", 6))
        return false;

    // Offset 0x800, write content now
    for (uint32_t i = 0; i < (uint32_t)entries.size(); i++)
    {
        const CpkEntry &entry = entries[i];
        uint32_t size;

        if (!entry.GetSize(&size))
            return false;

        if (!ExtractCommon(entry, stream, size))
            return false;

        if (!stream->Align(align))
            return false;
    }

    if (has_toc)
    {
        TOCHeader toc_hdr;

        toc_hdr.signature = TOC_SIGNATURE;
        toc_hdr.unk_04 = toc_unk_04;

        if (!stream->Write(&toc_hdr, sizeof(TOCHeader)))
            return false;

        if (!SaveTable(stream, &toc, "CpkTocInfo"))
            return false;
    }

    if (has_itoc)
    {
        if (!stream->Align(align))
            return false;

        ITOCHeader itoc_hdr;

        itoc_hdr.signature = ITOC_SIGNATURE;
        itoc_hdr.unk_04 = itoc_unk_04;

        if (!stream->Write(&itoc_hdr, sizeof(ITOCHeader)))
            return false;

        if (!SaveTable(stream, &itoc, (!is_itoc_extend) ? "CpkItocInfo" : "CpkExtendId"))
            return false;
    }

    if (has_etoc)
    {
        if (!stream->Align(align))
            return false;

        ETOCHeader etoc_hdr;

        etoc_hdr.signature = ETOC_SIGNATURE;
        etoc_hdr.unk_04 = etoc_unk_04;

        if (!stream->Write(&etoc_hdr, sizeof(ETOCHeader)))
            return false;

        if (!SaveTable(stream, &etoc, "CpkEtocInfo"))
            return false;
    }

    return true;
}

bool CpkFile::ExtractCrylaila(Stream *input, Stream *output, uint32_t compressed_size, uint32_t uncompressed_size) const
{
    // Crilayla decompression is based on cripaktools code

    MemoryStream *memory = dynamic_cast<MemoryStream *>(input);
    uint8_t *input_buf, *output_buf = nullptr;

    if (compressed_size < 0x10)
        return false;

    if (memory)
    {
        if (!memory->FastRead(&input_buf, compressed_size))
            return false;
    }
    else
    {
        input_buf = new uint8_t[compressed_size];

        if (!input->Read(input_buf, compressed_size))
        {
            delete[] input_buf;
            return false;
        }
    }



    /*if (memcmp(input_buf, "CRILAYLA", 8) != 0)
    {
        DPRINTF("%s: Compression other than crilayla is not supported %x %x.\n", FUNCNAME, compressed_size, uncompressed_size);
        if (!memory)
            delete[] input_buf;

        return false;
    }*/

    bool ret = false;
    uint32_t file_size = val32(*(uint32_t *)&input_buf[8]);
    uint32_t header_offset = val32(*(uint32_t *)&input_buf[12]);

    CrilaylaFixedBitStream bits(input_buf, (header_offset+0x10)*8);
    static const std::vector<int> vle_lens = { 2, 3, 5, 8 };
    uint32_t output_end = uncompressed_size - 1;
    uint32_t written = 0;

    if (file_size+0x100 != uncompressed_size)
    {
        DPRINTF("%s: Size specified in crilayla doesn't match the one of toc. 0x%x != 0x%x\n", FUNCNAME, file_size+0x100, uncompressed_size);
        goto clean;
    }

    if (header_offset + 0x10 + 0x100 > compressed_size)
        goto clean;

    output_buf = new uint8_t[uncompressed_size];
    memcpy(output_buf, input_buf+header_offset+0x10, 0x100);

    while (written < (uncompressed_size-0x100))
    {
        uint8_t next_compressed;

        next_compressed = bits.FastReadU8(1);

        if (next_compressed)
        {
            uint16_t word = bits.FastReadU16(13);

            int backreference_offset = output_end - written + word + 3;
            int backreference_length = 3;
            int vle_level;

            for (vle_level = 0; vle_level < (int)vle_lens.size(); vle_level++)
            {
                uint8_t this_level = bits.FastReadU8(vle_lens[vle_level]);
                backreference_length += this_level;

                if (this_level != ((1 << vle_lens[vle_level]) - 1))
                    break;
            }

            if (vle_level == (int)vle_lens.size())
            {
                uint8_t this_level;

                do
                {
                    this_level = bits.FastReadU8(8);
                    backreference_length += this_level;
                } while (this_level == 255);
            }

            for (int i = 0; i < backreference_length; i++)
            {
                output_buf[output_end - written] = output_buf[backreference_offset--];
                written++;
            }
        }
        else
        {
            uint8_t byte = bits.FastReadU8(8);

            output_buf[output_end-written] = byte;
            written++;
        }
    }

    ret = output->Write(output_buf, uncompressed_size);

clean:

    if (!memory)
        delete[] input_buf;

    if (output_buf)
        delete[] output_buf;

    return  ret;
}

uint8_t *CpkFile::Save(size_t *psize)
{
    MemoryStream stream;

    if (!SaveCommon(&stream))
        return nullptr;

    *psize = stream.GetSize();
    return stream.GetMemory(true);
}

bool CpkFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    FILE *f = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
    if (!f)
    {
        if (show_error)
        {
            DPRINTF("%s: Cannot open/create file \"%s\"\n", FUNCNAME, path.c_str());
        }

        return false;
    }

    fclose(f);
    FileStream stream("w+b");

    if (!stream.LoadFromFile(path, show_error))
        return false;

    return SaveCommon(&stream);
}

bool CpkFile::GetFileSize(const std::string &path_in_cpk, uint64_t *psize) const
{
    uint32_t idx = FindEntryByPath(path_in_cpk);
    if (idx == (uint32_t)-1)
        return false;

    return GetFileSize(idx, psize);
}

bool CpkFile::ExtractCommon(const CpkEntry &entry, Stream *stream, uint32_t size) const
{    
    if (entry.offset != (uint64_t)-1)
    {
        // Internal file
        assert(fstream);

        if (!fstream->Seek(entry.offset, SEEK_SET))
            return false;

        if (entry.compressed_size != 0)
            return ExtractCrylaila(fstream, stream, entry.compressed_size, size);

        if (!stream->Copy(fstream, size))
        {
            //DPRINTF("Failed here. Offset: %I64x size: %x\n", entry.offset, size);
            return false;
        }        
    }
    else if (entry.buf)
    {
        if (!stream->Write(entry.buf, size))
            return false;
    }
    else
    {
        FileStream external("rb");

        if (!external.LoadFromFile(entry.external_path))
            return false;

        if (!stream->Copy(&external, size))
            return false;
    }

    return true;
}

std::string CpkFile::GetPath(uint32_t idx, uint32_t file_size) const
{
    assert(idx < entries.size());

    const CpkEntry &entry = entries[idx];

    if (entry.has_name)
    {
        std::string path;

        path += entry.dir_name;

        if (path.length() != 0 && !Utils::EndsWith(path, "/") && !Utils::EndsWith(path, "\\"))
            path += '/';

        path += entry.file_name;
        return path;
    }

    char temp[32];
    std::string default_return;
    uint32_t signature = 0;

    snprintf(temp, sizeof(temp), "%04d", idx+1);
    default_return = std::string(temp) + ".bin";

    if (file_size < 4)
        return default_return;

    if (entry.offset != (uint64_t)-1)
    {
        assert(fstream);
        fstream->Seek(entry.offset, SEEK_SET);

        fstream->Read32(&signature);
    }
    else if (entry.buf)
    {
        signature = *(uint32_t *)entry.buf;
    }
    else
    {
        FILE *f = fopen(entry.external_path.c_str(), "rb");
        if (!f)
            return default_return;

        fread(&signature, 1, sizeof(uint32_t), f);
        fclose(f);
    }

    if ((signature & 0x7F7F7F7F) == HCA_SIGNATURE)
    {
        return std::string(temp)+".hca";
    }
    else if ((signature&0xFFFF) == 0x80)
    {
        return std::string(temp)+".adx";
    }

    return default_return;
}

bool CpkFile::ExtractFile(uint32_t idx, const std::string &path, bool auto_path) const
{
    if (idx >= entries.size())
        return false;

    const CpkEntry &entry = entries[idx];
    uint32_t size;
    std::string file_path = path;    

    if (!entry.GetSize(&size))
        return false;

    if (auto_path)
    {
        if (file_path.length() != 0 && !Utils::EndsWith(file_path, "/") && !Utils::EndsWith(file_path, "\\"))
            file_path += '/';

        file_path += GetPath(idx, size);
    }

    FileStream output("w+b");

    if (!output.LoadFromFile(file_path))
        return false;   

    return ExtractCommon(entry, &output, size);
}

uint8_t *CpkFile::ExtractFile(uint32_t idx, uint64_t *psize) const
{
    if (idx >= entries.size())
        return nullptr;

    const CpkEntry &entry = entries[idx];
    uint32_t size;

    if (!entry.GetSize(&size))
        return nullptr;

    uint8_t *buf = new uint8_t[size];
    FixedMemoryStream output(buf, size);    

    if (!ExtractCommon(entry, &output, size))
    {
        delete[] buf;
        return nullptr;
    }    

    *psize = size;
    return buf;
}

uint32_t CpkFile::FindEntryByPath(const std::string &path) const
{
    std::string internal_path = Utils::ToLowerCase(Utils::NormalizePath(path));

    for (size_t i = 0; i < entries.size(); i++)
    {
        const CpkEntry &entry = entries[i];

        if (!entry.has_name)
            continue;

        std::string entry_path = Utils::NormalizePath(entry.dir_name);

        if (entry_path.length() != 0 && !Utils::EndsWith(entry_path, "/"))
        {
            entry_path += '/';
        }

        entry_path += entry.file_name;

        if (Utils::ToLowerCase(entry_path) == internal_path)
        {
            return (uint32_t)i;
        }
    }

    return (uint32_t)-1;
}

bool CpkFile::FileExists(const std::string &path) const
{
    if (!cache_built)
    {
        for (size_t i = 0; i < entries.size(); i++)
        {
            const CpkEntry &entry = entries[i];

            if (!entry.has_name)
                continue;

            std::string entry_path = Utils::NormalizePath(entry.dir_name);

            if (entry_path.length() != 0 && !Utils::EndsWith(entry_path, "/"))
            {
                entry_path += '/';
            }

            entry_path += entry.file_name;
            cache.insert(Utils::ToLowerCase(entry_path));
        }

        cache_built = true;
    }

    if (cache.find(Utils::ToLowerCase(path)) != cache.end())
        return true;

    return false;
}

uint8_t *CpkFile::ExtractFile(const std::string &path_in_cpk, uint64_t *psize) const
{
    uint32_t idx = FindEntryByPath(path_in_cpk);
    if (idx == (uint32_t)-1)
        return nullptr;

    return ExtractFile((uint32_t)idx, psize);
}

bool CpkFile::SetFile(uint32_t idx, void *buf, uint64_t size, bool take_ownership)
{
    if (idx >= entries.size())
        return false;

    uint8_t *new_buf;

    if (take_ownership)
    {
        new_buf = (uint8_t *)buf;
    }
    else
    {
        new_buf = new uint8_t[size];
        memcpy(new_buf, buf, size);
    }

    CpkEntry &entry = entries[idx];

    if (entry.buf)
        delete[] entry.buf;

    entry.offset = -1;
    entry.size = (uint32_t)size;
    entry.buf = new_buf;

    return true;
}

bool CpkFile::SetFile(uint32_t idx, const std::string &path)
{
    if (idx >= entries.size())
        return false;

    CpkEntry &entry = entries[idx];

    entry.offset = (uint64_t)-1;
    entry.buf = nullptr;
    entry.size = 0;
    entry.external_path = path;

    return true;
}

bool CpkFile::GetFilePath(uint32_t idx, std::string &path) const
{
    if (idx >= entries.size())
        return false;

    const CpkEntry &entry = entries[idx];

    if (!entry.has_name)
        return false;

    path = Utils::NormalizePath(entry.dir_name);

    if (path.length() != 0 && !Utils::EndsWith(path, "/"))
    {
        path += '/';
    }

    path += entry.file_name;
    return true;
}

bool CpkFile::GetParentDirectory(uint32_t idx, std::string &path) const
{
    if (idx >= entries.size())
        return false;

    const CpkEntry &entry = entries[idx];

    if (!entry.has_name)
        return false;

    path = Utils::NormalizePath(entry.dir_name);

    if (path.length() != 0 && !Utils::EndsWith(path, "/"))
    {
        path += '/';
    }

    if (entry.file_name.find('/') == std::string::npos)
        return true;

    path += entry.file_name;

    size_t pos = path.rfind('/');
    path = path.substr(0, pos);

    return true;
}

bool CpkEntry::GetSize(uint32_t *psize) const
{
    if (offset != (uint64_t)-1 || buf)
    {
        *psize = size;
        return true;
    }

    size_t size = Utils::GetFileSize(external_path);
    if (size == (size_t)-1)
    {
        DPRINTF("%s: Cannot stat file \"%s\"\n", FUNCNAME, external_path.c_str());
        return false;
    }

    *psize = (uint32_t)size;
    return true;
}
