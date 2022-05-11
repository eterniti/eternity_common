#include "IdxFile.h"

IdxFile::IdxFile()
{
    this->big_endian = false;
}

IdxFile::~IdxFile()
{

}

void IdxFile::Reset()
{
    packs.clear();
    names.clear();
    files.clear();
    fslss.clear();
}

bool IdxFile::ReadStrings(Stream *stream, uint64_t offset, std::vector<std::string> &strings)
{
    if (!stream->Seek((off64_t)offset, SEEK_SET))
    {
        DPRINTF("%s: Early end of file.\n", FUNCNAME);
        return false;
    }


    GENESTRTHeader gheader;
    if (!stream->Read(&gheader, sizeof(gheader)))
        return false;

    if (memcmp(gheader.signature, GENESTRT_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid GENESTRT signature.\n", FUNCNAME);
        return false;
    }

    std::vector<uint32_t> strings_table;
    strings_table.resize(gheader.num_strings);
    strings.resize(gheader.num_strings);


    if (!stream->Read(strings_table.data(), gheader.num_strings*sizeof(uint32_t)))
    {
        DPRINTF("%s: Early end of file (2).\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < strings_table.size(); i++)
    {
         std::string &str = strings[i];

         if (!stream->Seek((off64_t)offset + gheader.str_table_offset + 0x10 + strings_table[i], SEEK_SET))
         {
             DPRINTF("%s: Early end of file (3).\n", FUNCNAME);
             return false;
         }

         while (true)
         {
             int8_t ch;
             if (!stream->Read8(&ch))
             {
                 DPRINTF("%s: Early end of file (4).\n", FUNCNAME);
                 return false;
             }

             if (ch == 0)
                 break;

             str.push_back(ch);
         }
    }

    return true;
}

bool IdxFile::BuildPath(size_t idx, const std::string &path, std::vector<bool> &build)
{
    build[idx] = true;
    FileEntry &dir = files[idx];

    for (size_t i = dir.file_idx_start; i < dir.file_idx_start + dir.num_files; i++)
    {
        if (i > files.size())
        {
            DPRINTF("%s: Index out of bounds (0x%Ix/0x%Ix).\n", FUNCNAME, i, files.size());
            return false;
        }

        if (build[i])
            continue;

        FileEntry &file = files[i];
        file.path = Utils::MakePathString(path, file.name);

        if (file.attributes == ATTRIBUTE_DIRECTORY)
        {
            if (!BuildPath(i, file.path, build))
                return false;
        }
    }

    return true;
}

bool IdxFile::BuildPaths()
{
    if (files.size() == 0 || files[0].attributes != ATTRIBUTE_DIRECTORY)
        return false;

    std::vector<bool> build;
    build.resize(files.size(), false);

    return BuildPath(0, files[0].name, build);
}

bool IdxFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    ENDILTLEHeader header;

    if (!stream.Read(&header, sizeof(header)))
        return false;

    if (memcmp(header.signature, ENDILTLE_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid ENDILTLE signature.\n", FUNCNAME);
        return false;
    }

    PACKHEDRHeader pack;
    uint64_t rewind_offset = stream.Tell();

    while (stream.Read(&pack, sizeof(pack)))
    {
        if (memcmp(pack.signature, PACKHEDR_SIGNATURE, 8) == 0)
        {
            packs.push_back(pack);
            rewind_offset = stream.Tell();
        }
        else
        {
            if (!stream.Seek((off64_t)rewind_offset, SEEK_SET))
                return false;

            break;
        }
    }

    if (stream.Tell() >= stream.GetSize())
    {
        DPRINTF("%s: premature end of file.\n", FUNCNAME);
        return false;
    }

    if (packs.size() == 0)
    {
        DPRINTF("%s: this idx has no packs, cannot handle.\n", FUNCNAME);
        return false;
    }

    uint64_t toc_offset = stream.Tell();
    PACKTOCHeader toc;
    if (!stream.Read(&toc, sizeof(toc)))
        return false;

    if (memcmp(toc.signature, PACKTOC_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid PACKTOC signature.\n", FUNCNAME);
        return false;
    }

    uint64_t fsls_offset = toc_offset + toc.toc_size + 0x10;
    if (!stream.Seek((off64_t)fsls_offset, SEEK_SET))
    {
        DPRINTF("%s: Early end of file.\n", FUNCNAME);
        return false;
    }

    PACKFSLSHeader fsls;
    if (!stream.Read(&fsls, sizeof(fsls)))
        return false;

    if (memcmp(fsls.signature, PACKFSLS_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid PACKFSLS signature.\n", FUNCNAME);
        return false;
    }

    if (!ReadStrings(&stream, fsls_offset + fsls.fsls_size + 0x10, names))
        return false;

    if (!stream.Seek((off64_t)toc_offset + sizeof(PACKTOCHeader), SEEK_SET))
        return false;

    files.resize(toc.num_files);

    for (uint32_t i = 0; i < toc.num_files; i++)
    {
        TOCEntry entry;
        FileEntry &file = files[i];

        if (!stream.Read(&entry, sizeof(entry)))
        {
            DPRINTF("%s: failed to read toc entry.\n", FUNCNAME);
            return false;
        }

        if ((size_t)entry.name_idx >= names.size())
        {
            DPRINTF("%s: Name index out of bounds: 0x%x > 0x%Ix.\n", FUNCNAME, entry.name_idx, names.size());
            return false;
        }

        if (entry.attributes != ATTRIBUTE_UNCOMPRESSED && entry.attributes != ATTRIBUTE_COMPRESSED && entry.attributes != ATTRIBUTE_DIRECTORY)
        {
            DPRINTF("%s: warning, unrecognized attribute 0x%x.\n", FUNCNAME, entry.attributes);
        }

        file.attributes = entry.attributes;
        file.name = names[entry.name_idx];
        file.pack_idx = entry.pack_idx;

        if (entry.attributes == ATTRIBUTE_DIRECTORY)
        {
            //DPRINTF("DIR %s: %d %d\n", names[entry.name_idx].c_str(), entry.a.b.file_idx_start, entry.a.b.num_files);
            file.file_idx_start = entry.a.b.file_idx_start;
            file.num_files = entry.a.b.num_files;
        }
        else
        {
            //DPRINTF("%s: %I64d %I64d\n", names[entry.name_idx].c_str(), entry.size, entry.comp_size);
            file.file_offset = entry.a.file_offset;
            file.size = entry.size;
            file.comp_size = entry.comp_size;
        }
    }

    if (!BuildPaths())
    {
        DPRINTF("%s: Failed to create paths.\n", FUNCNAME);
        return false;
    }

    /*for (const FileEntry &file : files)
    {
        DPRINTF("%s%s\n", (file.attributes == ATTRIBUTE_DIRECTORY) ? "DIR " : "", file.path.c_str());
    }*/

    if (!stream.Seek((off64_t)fsls_offset + sizeof(PACKFSLSHeader), SEEK_SET))
        return false;

    fslss.resize(fsls.num_files);

    for (uint32_t i = 0; i < fsls.num_files; i++)
    {
        FSLSEntry entry;
        Fsls &fsls = fslss[i];

        if (!stream.Read(&entry, sizeof(entry)))
        {
            DPRINTF("%s: failed to read fsls entry.\n", FUNCNAME);
            return false;
        }

        /*uint64_t pos = apk->Tell();

        if (!ReadFshd(entry, fshd, show_error))
            return false;

        apk->Seek((off64_t)pos, SEEK_SET);*/
        if ((size_t)entry.name_idx >= names.size())
        {
            DPRINTF("%s: fsls name idx 0x%x out of bounds.\n", FUNCNAME, entry.name_idx);
            return false;
        }

        fsls.name = names[entry.name_idx];
        fsls.pack_idx = entry.pack_idx;
        fsls.offset = entry.offset;
        fsls.size = entry.size;
        memcpy(fsls.md5, entry.md5, sizeof(fsls.md5));
    }


    return true;
}

void IdxFile::BuildNamesTable(const std::vector<std::string> &names, std::unordered_map<std::string, uint64_t> &table)
{
    size_t idx = 0;
    uint64_t idx2 = 0;

    table.clear();
    //DPRINTF("---BUILD NAME TABLE.\n");

    for (const std::string &name : names)
    {
        table[name] = idx | (idx2 << 32ULL);
        idx += name.length() + 1;
        idx2++;
        //DPRINTF("%s\n", name.c_str());
    }
}

bool IdxFile::WriteNamesTable(Stream *out, const std::vector<std::string> &names, const std::unordered_map<std::string, uint64_t> &table)
{
    uint64_t pos = out->Tell();

    GENESTRTHeader header;
    header.num_strings = (uint32_t)names.size();
    if (!out->Write(&header, sizeof(header)))
        return false;

    for (const std::string &name : names)
    {
        if (!out->Write32(table.find(name)->second&0xFFFFFFFF))
            return false;
    }

    if (!out->Align(0x10))
        return false;

    header.str_table_offset = (uint32_t)(out->Tell() - pos - 0x10);
    for (const std::string &name : names)
    {
        if (!out->WriteString(name, true))
            return false;
    }

    if (!out->Align(0x10))
        return false;

    if (!out->WriteString("GENEEOF "))
        return false;

    if (!out->Align(0x10))
        return false;

    uint64_t end_pos = out->Tell();
    header.str_size = header.str_size2 = (uint32_t)(end_pos - pos - 0x20);

    // Go back and update
    if (!out->Seek((off64_t)pos, SEEK_SET))
        return false;

    if (!out->Write(&header, sizeof(header)))
        return false;

    return out->Seek((off64_t)end_pos, SEEK_SET);
}

uint8_t *IdxFile::Save(size_t *psize)
{
    MemoryStream out;

    ENDILTLEHeader header;
    if (!out.Write(&header, sizeof(header)))
        return nullptr;

    for (const PACKHEDRHeader &pack : packs)
    {
        if (!out.Write(&pack, sizeof(pack)))
            return nullptr;
    }

    uint64_t toc_offset = out.Tell(); // Will go back here for toc_size and write files offsets
    PACKTOCHeader toc;
    toc.num_files = (uint32_t)files.size();
    if (!out.Write(&toc, sizeof(toc)))
        return nullptr;

    std::unordered_map<std::string, uint64_t> names_table;
    BuildNamesTable(names, names_table);

    for (const FileEntry &file : files)
    {
        TOCEntry entry;

        entry.attributes = file.attributes;

        if (file.attributes == ATTRIBUTE_DIRECTORY)
        {
            entry.a.b.file_idx_start = file.file_idx_start;
            entry.a.b.num_files = file.num_files;
        }
        else
        {
            entry.size = file.size;
            entry.comp_size = file.comp_size;
            entry.a.file_offset = file.file_offset;
        }

        auto it = names_table.find(file.name);
        if (it == names_table.end())
        {
            DPRINTF("%s: Internal error, shouldn't have happened.\n", FUNCNAME);
            return nullptr;
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = file.pack_idx;
        if (!out.Write(&entry, sizeof(entry)))
            return nullptr;
    }

    if (!out.Align(0x10))
        return nullptr;

    uint64_t fsls_offset = out.Tell(); // Will go back here for fsls size
    toc.toc_size = (uint32_t)(fsls_offset - toc_offset - 0x10);
    PACKFSLSHeader fsls;
    fsls.num_files = (uint32_t)fslss.size();
    if (!out.Write(&fsls, sizeof(fsls)))
        return nullptr;

    for (const Fsls &fsls : fslss)
    {
        FSLSEntry entry;
        auto it = names_table.find(fsls.name);
        if (it == names_table.end())
        {
            DPRINTF("%s: Internal error 2, shouldn't have happened.\n", FUNCNAME);
            return nullptr;
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = fsls.pack_idx;
        entry.offset = fsls.offset;
        entry.size = fsls.size;
        memcpy(entry.md5, fsls.md5, sizeof(entry.md5));

        if (!out.Write(&entry, sizeof(entry)))
            return nullptr;
    }

    if (!out.Align(0x10))
        return nullptr;

    fsls.fsls_size = (uint32_t)(out.Tell() - fsls_offset - 0x10);
    if (!WriteNamesTable(&out, names, names_table))
        return nullptr;

    // Time to go back and update
    if (!out.Seek((off64_t)fsls_offset, SEEK_SET) || !out.Write(&fsls, sizeof(fsls)))
        return nullptr;

    if (!out.Seek((off64_t)toc_offset, SEEK_SET) || !out.Write(&toc, sizeof(toc)))
        return nullptr;

    *psize = (size_t)out.GetSize();
    return out.GetMemory(true);
}

bool IdxFile::UpdateFrom(ApkFile *apk, uint32_t pack_idx)
{
    if (pack_idx >= packs.size())
        return false;

    if (apk->apk == nullptr || apk->gfsls_offset == 0)
        return false;

    memcpy(packs[pack_idx].checksum, apk->checksum, sizeof(apk->checksum));
    uint32_t num_updated = 0;
    UNUSED(num_updated);

    for (size_t i = 0; i < files.size(); i++)
    {
        if (files[i].pack_idx != pack_idx)
            continue;

        FileEntry &out = files[i];

        for (size_t j = 0; j < apk->files.size(); j++)
        {
            const FileEntry &in = apk->files[j];

            if (in.path == out.path)
            {
                //DPRINTF("Updating \"%s\", %d->%d\n", in.path.c_str(), in.file_idx_start, out.file_idx_start);
                out.attributes = in.attributes;
                out.size = in.size;
                out.comp_size = in.comp_size;
                //out.file_idx_start = in.file_idx_start;
                out.file_offset = in.file_offset;
                //out.num_files = in.num_files;
                num_updated++;
                break;
            }
        }
    }

    //DPRINTF("Updated %d files.\n", num_updated);

    FileStream *file = apk->apk;
    num_updated = 0;
    if (!file->Seek((off64_t)apk->gfsls_offset, SEEK_SET))
    {
        DPRINTF("%s: Error seeking to fsls_offset.\n", FUNCNAME);
        return false;
    }

    PACKFSLSHeader fsls;
    if (!file->Read(&fsls, sizeof(fsls)))
    {
        DPRINTF("%s: failed to read fsls.\n", FUNCNAME);
        return false;
    }

    if (memcmp(fsls.signature, PACKFSLS_SIGNATURE, 8) != 0)
    {
        DPRINTF("%s: Invalid PACKFSLS signature.\n", FUNCNAME);
        return false;
    }

    std::vector<FSLSEntry> fsls_entries;
    fsls_entries.resize(fsls.num_files);

    if (!file->Read(fsls_entries.data(), fsls.num_files*sizeof(FSLSEntry)))
    {
        DPRINTF("%s: failed to read fsls entries.\n", FUNCNAME);
        return false;
    }

    for (size_t i = 0; i < fslss.size(); i++)
    {
        if (fslss[i].pack_idx != pack_idx)
            continue;

        Fsls &out = fslss[i];

        for (size_t j = 0; j < fsls_entries.size(); j++)
        {
            const FSLSEntry &in = fsls_entries[j];

            if (in.name_idx >= apk->names.size())
            {
                DPRINTF("%s: name_idx out of bounds.\n", FUNCNAME);
                return false;
            }

            if (out.name == apk->names[in.name_idx])
            {
                //DPRINTF("Updating \"%s\": %I64x->%I64x\n", out.name.c_str(), out.offset, in.offset);
                out.offset = in.offset;
                out.size = in.size;
                memcpy(out.md5, in.md5, sizeof(out.md5));
                num_updated++;
                break;
            }
        }
    }

    //DPRINTF("Updated %d fsls.\n", num_updated);
    return true;
}


