#include <map>
#include <algorithm>

#include "ApkFile.h"
#include "debug.h"

ApkFile::ApkFile()
{
    this->big_endian = false;
    apk = nullptr;
}

ApkFile::~ApkFile()
{
    Reset();
}

void ApkFile::Reset()
{
    if (apk)
    {
        delete apk;
        apk = nullptr;
    }

    names.clear();
    files.clear();
    fshds.clear();
    pack_idx = 0xFFFFFFFF;
    gfsls_offset = 0;
}

bool ApkFile::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    apk = new FileStream("rb");
    if (!apk->LoadFromFile(path, show_error))
        return false;


    ENDILTLEHeader header;
    if (!apk->Read(&header, sizeof(header)))
        return false;

    if (memcmp(header.signature, ENDILTLE_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid ENDILTLE signature.\n", FUNCNAME);
        }

        return false;
    }

    PACKHEDRHeader pheader;
    if (!apk->Read(&pheader, sizeof(pheader)))
        return false;

    if (memcmp(pheader.signature, PACKHEDR_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid PACKHEDR signature.\n", FUNCNAME);
        }

        return false;
    }

    pack_idx = pheader.pack_idx;
    memcpy(checksum, pheader.checksum, sizeof(checksum));

    uint64_t toc_offset = apk->Tell();
    PACKTOCHeader theader;
    if (!apk->Read(&theader, sizeof(theader)))
        return false;

    if (memcmp(theader.signature, PACKTOC_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid PACKTOC signature.\n", FUNCNAME);
        }

        return false;
    }

    uint64_t fsls_offset = toc_offset + theader.toc_size + 0x10;
    gfsls_offset = fsls_offset;
    if (!apk->Seek((off64_t)fsls_offset, SEEK_SET))
    {
        if (show_error)
        {
            DPRINTF("%s: Early end of file.\n", FUNCNAME);
        }

        return false;
    }

    PACKFSLSHeader fheader;
    if (!apk->Read(&fheader, sizeof(fheader)))
        return false;

    if (memcmp(fheader.signature, PACKFSLS_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid PACKFSLS signature.\n", FUNCNAME);
        }

        return false;
    }

    if (!ReadStrings(fsls_offset + fheader.fsls_size + 0x10, names, show_error))
        return false;

    if (!apk->Seek((off64_t)toc_offset + sizeof(PACKTOCHeader), SEEK_SET))
        return false;

    files.resize(theader.num_files);

    for (uint32_t i = 0; i < theader.num_files; i++)
    {
        TOCEntry entry;
        FileEntry &file = files[i];

        if (!apk->Read(&entry, sizeof(entry)))
        {
            if (show_error)
            {
                DPRINTF("%s: failed to read toc entry.\n", FUNCNAME);
            }

            return false;
        }

        if ((size_t)entry.name_idx >= names.size())
        {
            if (show_error)
            {
                DPRINTF("%s: Name index out of bounds: 0x%x > 0x%Ix.\n", FUNCNAME, entry.name_idx, names.size());
            }

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
        if (show_error)
        {
            DPRINTF("%s: Failed to create paths.\n", FUNCNAME);
        }

        return false;
    }

    /*for (const FileEntry &file : files)
    {
        DPRINTF("%s%s\n", (file.attributes == ATTRIBUTE_DIRECTORY) ? "DIR " : "", file.path.c_str());
    }*/

    if (!apk->Seek((off64_t)fsls_offset + sizeof(PACKFSLSHeader), SEEK_SET))
        return false;

    fshds.resize(fheader.num_files);

    for (uint32_t i = 0; i < fheader.num_files; i++)
    {
        FSLSEntry entry;
        Fshd &fshd = fshds[i];

        if (!apk->Read(&entry, sizeof(entry)))
        {
            if (show_error)
            {
                DPRINTF("%s: failed to read fsls entry.\n", FUNCNAME);
            }

            return false;
        }

        uint64_t pos = apk->Tell();

        if (!ReadFshd(entry, fshd, show_error))
            return false;

        apk->Seek((off64_t)pos, SEEK_SET);
    }

    /*DPRINTF("-----\n");
    for (const Fshd &f : fshds)
    {
        DPRINTF("%s\n", f.name.c_str());
    }
    DPRINTF("-----\n");*/

    return true;
}

bool ApkFile::SaveFailCleanup(FileStream *stream, const std::string &temp_file)
{
    delete stream;
    Utils::RemoveFile(temp_file);
    return false;
}

void ApkFile::BuildNamesTable(const std::vector<std::string> &names, std::unordered_map<std::string, uint64_t> &table)
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

bool ApkFile::WriteNamesTable(FileStream *out, const std::vector<std::string> &names, const std::unordered_map<std::string, uint64_t> &table)
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

bool ApkFile::WriteFshd(FileStream *out, const Fshd &fshd)
{
    uint64_t pos = out->Tell();

    ENDILTLEHeader header;
    if (!out->Write(&header, sizeof(header)))
        return false;

    PACKFSHDHeader fheader;
    fheader.num_files = (uint32_t)fshd.files.size();
    if (!out->Write(&fheader, sizeof(fheader)))
        return false;

    std::unordered_map<std::string, uint64_t> names_table;
    BuildNamesTable(fshd.names, names_table);

    for (const FileEntry &file : fshd.files)
    {
        FSHDEntry entry;

        entry.size = file.size;
        entry.comp_size = file.comp_size;

        auto it = names_table.find(file.path);
        if (it == names_table.end())
        {
            DPRINTF("%s: Internal error, couldn't find \"%s\"\n", FUNCNAME, file.path.c_str());
            return false;
        }

        entry.name_idx = (it->second >> 32);

        if (!out->Write(&entry, sizeof(entry)))
            return false;
    }

    if (!out->Align(0x10))
        return false;

    fheader.size = (uint32_t)(out->Tell() - pos - 0x20);
    if (!WriteNamesTable(out, fshd.names, names_table))
        return false;

    fheader.full_size = (uint32_t)(out->Tell() - pos - 0x10);
    std::unordered_map<std::string, uint64_t> files_pos;

    for (const FileEntry &file : fshd.files)
    {
        files_pos[file.path] = out->Tell() - pos;

        uint8_t *buf = file.buf;
        uint64_t size = (file.attributes == ATTRIBUTE_COMPRESSED) ? file.comp_size : file.size;
        if (!buf)
        {
            if (!apk->Seek((off64_t)file.file_offset, SEEK_SET))
                return false;

            buf = new uint8_t[size];
            if (!apk->Read(buf, (size_t)size))
            {
                delete[] buf;
                return false;
            }

        }

        bool ret = out->Write(buf, (size_t)size);
        if (!file.buf) delete[] buf;
        if (!ret)
            return false;

        if (!out->Align(0x10))
            return false;
    }

    uint64_t pos_end = out->Tell();

    // Go back and update

    if (!out->Seek((off64_t)pos + sizeof(header), SEEK_SET))
        return false;

    if (!out->Write(&fheader, sizeof(fheader)))
        return false;

    for (const FileEntry &file :fshd.files)
    {
        FSHDEntry entry;

        entry.size = file.size;
        entry.comp_size = file.comp_size;

        auto it = names_table.find(file.path);
        if (it == names_table.end())
        {
            DPRINTF("%s: Internal error 2.\n", FUNCNAME);
            return false;
        }

        entry.name_idx = (it->second>>32);
        entry.offset = files_pos[file.path];

        if (!out->Write(&entry, sizeof(entry)))
            return false;
    }

    return out->Seek((off64_t)pos_end, SEEK_SET);
}

bool ApkFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    if (!apk)
        return false;

    std::string temp_path = path + ".tmp" + Utils::RandomString(5);
    if (!Utils::WriteFileBool(temp_path, (const uint8_t *)"0", 1, show_error, build_path)) // Write dummy file
        return false;

    FileStream *out = new FileStream();

    if (!out->LoadFromFile(temp_path, show_error))
        return SaveFailCleanup(out, temp_path);

    ENDILTLEHeader header;
    if (!out->Write(&header, sizeof(header)))
        return SaveFailCleanup(out, temp_path);

    uint64_t phdr_offset = out->Tell();  // Will go back here for file_data_offset
    PACKHEDRHeader pheader;
    pheader.pack_idx = pack_idx;
    memcpy(&pheader.checksum, checksum, sizeof(pheader.checksum));
    if (!out->Write(&pheader, sizeof(pheader)))
        return SaveFailCleanup(out, temp_path);

    uint64_t toc_offset = out->Tell(); // Will go back here for toc_size and write files offsets
    PACKTOCHeader theader;
    theader.num_files = (uint32_t)files.size();
    if (!out->Write(&theader, sizeof(theader)))
        return SaveFailCleanup(out, temp_path);

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
        }

        auto it = names_table.find(file.name);
        if (it == names_table.end())
        {
            if (show_error)
            {
                DPRINTF("%s: Internal error, shouldn't have happened.\n", FUNCNAME);
            }

            return SaveFailCleanup(out, temp_path);
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = file.pack_idx;
        if (!out->Write(&entry, sizeof(entry)))
            return SaveFailCleanup(out, temp_path);
    }

    if (!out->Align(0x10))
        return SaveFailCleanup(out, temp_path);

    uint64_t fsls_offset = out->Tell(); // Will go back here for fsls whole size, size, offset and md5 of each fshd
    theader.toc_size = (uint32_t)(fsls_offset - toc_offset - 0x10);
    PACKFSLSHeader fheader;
    fheader.num_files = (uint32_t)fshds.size();
    if (!out->Write(&fheader, sizeof(fheader)))
        return SaveFailCleanup(out, temp_path);

    for (const Fshd &fshd : fshds)
    {
        FSLSEntry entry;
        auto it = names_table.find(fshd.name);
        if (it == names_table.end())
        {
            if (show_error)
            {
                DPRINTF("%s: Internal error 2, shouldn't have happened.\n", FUNCNAME);
            }

            return SaveFailCleanup(out, temp_path);
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = fshd.pack_idx;

        if (!out->Write(&entry, sizeof(entry)))
            return SaveFailCleanup(out, temp_path);
    }

    if (!out->Align(0x10))
        return SaveFailCleanup(out, temp_path);

    fheader.fsls_size = (uint32_t)(out->Tell() - fsls_offset - 0x10);
    if (!WriteNamesTable(out, names, names_table))
        return SaveFailCleanup(out, temp_path);

    std::unordered_map<std::string, uint64_t> files_pos;

    for (const FileEntry &file : files)
    {
        if (file.attributes != ATTRIBUTE_DIRECTORY)
        {
            if (!out->Align(0x200))
                return SaveFailCleanup(out, temp_path);

            files_pos[file.path] = out->Tell();
            if (pheader.file_data_offset == 0)
                pheader.file_data_offset = (uint32_t)out->Tell();

            uint8_t *buf = file.buf;
            uint64_t size = (file.attributes == ATTRIBUTE_COMPRESSED) ? file.comp_size : file.size;
            if (!buf)
            {
                if (!apk->Seek((off64_t)file.file_offset, SEEK_SET))
                    return SaveFailCleanup(out, temp_path);

                buf = new uint8_t[size];
                if (!apk->Read(buf, (size_t)size))
                {
                    delete[] buf;
                    return SaveFailCleanup(out, temp_path);
                }

            }

            bool ret = out->Write(buf, (size_t)size);
            if (!file.buf) delete[] buf;
            if (!ret)
                return SaveFailCleanup(out, temp_path);
        }
    }    

    std::unordered_map<std::string, uint64_t> fshds_pos, fshds_size;

    for (const Fshd &fshd : fshds)
    {
        if (!out->Align(0x800))
            return SaveFailCleanup(out, temp_path);

        fshds_pos[fshd.name] = out->Tell();

        if (!WriteFshd(out, fshd))
            return SaveFailCleanup(out, temp_path);

        fshds_size[fshd.name] = out->Tell() - fshds_pos[fshd.name];
    }

    // Time to go back and update

    if (!out->Seek((off64_t)fsls_offset, SEEK_SET))
        return SaveFailCleanup(out, temp_path);

    if (!out->Write(&fheader, sizeof(fheader)))
        return SaveFailCleanup(out, temp_path);

    for (const Fshd &fshd : fshds)
    {
        FSLSEntry entry;
        auto it = names_table.find(fshd.name);
        if (it == names_table.end())
        {
            if (show_error)
            {
                DPRINTF("%s: Internal error 3, shouldn't have happened.\n", FUNCNAME);
            }

            return SaveFailCleanup(out, temp_path);
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = fshd.pack_idx;
        entry.offset = fshds_pos[fshd.name];
        entry.size = fshds_size[fshd.name];

        uint64_t save_pos = out->Tell();

        if (!out->Seek((off64_t)entry.offset, SEEK_SET))
            return SaveFailCleanup(out, temp_path);

        uint8_t *buf = new uint8_t[entry.size];
        if (!out->Read(buf, (size_t)entry.size))
        {
            delete[] buf;
            return SaveFailCleanup(out, temp_path);
        }

        Utils::Md5(buf, (uint32_t)entry.size, entry.md5);
        delete[] buf;

        if (!out->Seek((off64_t)save_pos, SEEK_SET))
            return SaveFailCleanup(out, temp_path);

        if (!out->Write(&entry, sizeof(entry)))
            return SaveFailCleanup(out, temp_path);
    }

    if (!out->Seek((off64_t)toc_offset, SEEK_SET))
        return SaveFailCleanup(out, temp_path);

    if (!out->Write(&theader, sizeof(theader)))
        return SaveFailCleanup(out, temp_path);

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
            entry.a.file_offset = files_pos[file.path];
            entry.size = file.size;
            entry.comp_size = file.comp_size;
        }

        auto it = names_table.find(file.name);
        if (it == names_table.end())
        {
            if (show_error)
            {
                DPRINTF("%s: Internal error, shouldn't have happened.\n", FUNCNAME);
            }

            return SaveFailCleanup(out, temp_path);
        }

        entry.name_idx = (it->second>>32);
        entry.pack_idx = file.pack_idx;

        if (!out->Write(&entry, sizeof(entry)))
            return SaveFailCleanup(out, temp_path);
    }

    if (!out->Seek((off64_t)phdr_offset, SEEK_SET))
        return SaveFailCleanup(out, temp_path);

    if (!out->Write(&pheader, sizeof(pheader)))
        return SaveFailCleanup(out, temp_path);

    delete out;
    delete apk;
    apk = nullptr;

    Utils::RemoveFile(path);
    if (!Utils::RenameFile(temp_path, path))
    {
        DPRINTF("%s: failed to rename temporary file.\n", FUNCNAME);
        return false;
    }   

    return LoadFromFile(path, show_error); // Reload, as some offsets have changed
}

bool ApkFile::ReadStrings(uint64_t offset, std::vector<std::string> &strings, bool show_error)
{
    if (!apk->Seek((off64_t)offset, SEEK_SET))
    {
        if (show_error)
        {
            DPRINTF("%s: Early end of file (2).\n", FUNCNAME);
        }

        return false;
    }


    GENESTRTHeader gheader;
    if (!apk->Read(&gheader, sizeof(gheader)))
        return false;

    if (memcmp(gheader.signature, GENESTRT_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid GENESTRT signature.\n", FUNCNAME);
        }

        return false;
    }

    std::vector<uint32_t> strings_table;
    strings_table.resize(gheader.num_strings);
    strings.resize(gheader.num_strings);


    if (!apk->Read(strings_table.data(), gheader.num_strings*sizeof(uint32_t)))
    {
        if (show_error)
        {
            DPRINTF("%s: Early end of file (3).\n", FUNCNAME);
        }

        return false;
    }

    for (size_t i = 0; i < strings_table.size(); i++)
    {
         std::string &str = strings[i];

         if (!apk->Seek((off64_t)offset + gheader.str_table_offset + 0x10 + strings_table[i], SEEK_SET))
         {
             if (show_error)
             {
                 DPRINTF("%s: Early end of file (4).\n", FUNCNAME);
             }

             return false;
         }

         while (true)
         {
             int8_t ch;
             if (!apk->Read8(&ch))
             {
                 if (show_error)
                 {
                     DPRINTF("%s: Early end of file (5).\n", FUNCNAME);
                 }

                 return false;
             }

             if (ch == 0)
                 break;

             str.push_back(ch);
         }
    }

    return true;
}

bool ApkFile::BuildPath(size_t idx, const std::string &path, std::vector<bool> &build)
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

bool ApkFile::BuildPaths()
{
    if (files.size() == 0 || files[0].attributes != ATTRIBUTE_DIRECTORY)
        return false;

    std::vector<bool> build;
    build.resize(files.size(), false);

    return BuildPath(0, files[0].name, build);
}

bool ApkFile::ReadFshd(const FSLSEntry &entry, Fshd &fshd, bool show_error)
{
    if ((size_t)entry.name_idx >= names.size())
    {
        if (show_error)
        {
            DPRINTF("%s: fsls name idx 0x%x out of bounds.\n", FUNCNAME, entry.name_idx);
        }

        return false;
    }

    //DPRINTF("%s\n", names[entry.name_idx].c_str());
    fshd.name = names[entry.name_idx];
    fshd.pack_idx = entry.pack_idx;

    if (!apk->Seek((off64_t)entry.offset, SEEK_SET))
    {
        if (show_error)
        {
            DPRINTF("%s: premature end of file.\n", FUNCNAME);
        }

        return false;
    }

    ENDILTLEHeader header;
    if (!apk->Read(&header, sizeof(header)))
        return false;

    if (memcmp(header.signature, ENDILTLE_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid ENDILTLE signature.\n", FUNCNAME);
        }

        return false;
    }

    PACKFSHDHeader fheader;
    if (!apk->Read(&fheader, sizeof(fheader)))
        return false;

    if (memcmp(fheader.signature, PACKFSHD_SIGNATURE, 8) != 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Invalid PACKFSHD signature.\n", FUNCNAME);
        }

        return false;
    }

    if (!ReadStrings(entry.offset + fheader.size + 0x20, fshd.names, show_error))
        return false;

    if (!apk->Seek((off64_t)entry.offset + sizeof(ENDILTLEHeader) + sizeof(PACKFSHDHeader), SEEK_SET))
        return false;

    fshd.files.resize(fheader.num_files);

    for (uint32_t i = 0; i < fheader.num_files; i++)
    {
        FSHDEntry fentry;
        FileEntry &file = fshd.files[i];

        if (!apk->Read(&fentry, sizeof(fentry)))
        {
            if (show_error)
            {
                DPRINTF("%s: Premature end of file (2).\n", FUNCNAME);
            }

            return false;
        }

        if ((size_t)fentry.name_idx >= fshd.names.size())
        {
            if (show_error)
            {
                DPRINTF("%s: name idx %x out of bounds.\n", FUNCNAME, fentry.name_idx);
            }

            return false;
        }

        if (fentry.flags != 2)
        {
            DPRINTF("%s: warning, flags != 2.\n", FUNCNAME);
        }

        file.size = fentry.size;
        file.comp_size = fentry.comp_size;
        file.file_offset = fentry.offset + entry.offset;
        file.path = fshd.names[fentry.name_idx];
        file.name = Utils::GetFileNameString(file.path);
        file.attributes = (file.comp_size == 0) ? ATTRIBUTE_UNCOMPRESSED : ATTRIBUTE_COMPRESSED;
    }

    return true;
}

bool ApkFile::ExtractFile(const FileEntry &file, const std::string &dir_path, const std::string &fshd_name, const std::string &filter) const
{
    if (!apk)
        return false;

    std::string out_path;

    if (fshd_name.length() > 0)
    {
        out_path = Utils::MakePathString(dir_path, fshd_name);
        out_path = Utils::MakePathString(out_path, file.path);
    }
    else
    {
        out_path = Utils::MakePathString(dir_path, file.path);
    }

    UPRINTF("Extracting %s...\n", file.path.c_str());

    if (file.attributes == ATTRIBUTE_DIRECTORY)
    {
        if (filter.length() == 0)
            Utils::Mkdir(out_path);
    }
    else if (file.attributes == ATTRIBUTE_COMPRESSED)
    {
        if (filter.length() != 0 && !Utils::EndsWith(file.name, filter, false))
            return true;

        uint8_t *buf = file.buf;
        if (!buf)
        {
            if (!apk->Seek((off64_t)file.file_offset, SEEK_SET))
            {
                DPRINTF("%s: Failed to seek (file %s).\n", FUNCNAME, file.path.c_str());
                return false;
            }

            buf = new uint8_t[file.comp_size];
            if (!apk->Read(buf, (size_t)file.comp_size))
            {
                DPRINTF("%s: Failed to read file %s.\n", FUNCNAME, file.path.c_str());
                delete[] buf;
                return false;
            }
        }

        uint8_t *ubuf = new uint8_t[file.size];
        uint32_t ucomp_size = (uint32_t)file.size;

        bool ret = Utils::UncompressZlib(ubuf, &ucomp_size, buf, (uint32_t)file.comp_size);
        if (ret)
        {
            ret = Utils::WriteFileBool(out_path, ubuf, ucomp_size, true, true);
        }

        if (!file.buf) delete[] buf;
        delete[] ubuf;

        if (!ret)
        {
            DPRINTF("%s: Failed to write compressed file %s\n", FUNCNAME, file.path.c_str());
            return false;
        }
    }
    else
    {
        if (filter.length() != 0 && !Utils::EndsWith(file.name, filter, false))
            return true;

        uint8_t *buf = file.buf;
        if (!buf)
        {
            if (!apk->Seek((off64_t)file.file_offset, SEEK_SET))
            {
                DPRINTF("%s: Failed to seek (file %s).\n", FUNCNAME, file.path.c_str());
                return false;
            }

            buf = new uint8_t[file.size];
            if (!apk->Read(buf, (size_t)file.size))
            {
                DPRINTF("%s: Failed to read file %s.\n", FUNCNAME, file.path.c_str());
                delete[] buf;
                return false;
            }
        }

        if (!Utils::WriteFileBool(out_path, buf, (size_t)file.size, true, true))
            return false;

        if (!file.buf) delete[] buf;
    }

    return true;
}

bool ApkFile::DoReplaceFile(FileEntry &file, const uint8_t *buf, size_t size)
{
    if (file.buf)
    {
        delete[] file.buf;
        file.buf = nullptr;
    }

    if (file.attributes == ATTRIBUTE_UNCOMPRESSED)
    {
        file.buf = new uint8_t[size];
        file.size = size;
        file.comp_size = 0;
        memcpy(file.buf, buf, size);
    }
    else
    {
        size_t csize;

        file.buf = Utils::CompressZlib(buf, size, &csize, 9);
        if (!file.buf)
        {
            DPRINTF("%s: weird, zlib compress failed.\n", FUNCNAME);
            return false;
        }

        file.size = size;
        file.comp_size = csize;
    }

    return true;
}

bool ApkFile::Extract(const std::string &dir_path, const std::string &filter) const
{
    for (const FileEntry &file : files)
    {
        if (!ExtractFile(file, dir_path, "", filter))
            return false;
    }

    for (const Fshd &fshd : fshds)
    {
        for (const FileEntry &file : fshd.files)
        {
            if (!ExtractFile(file, dir_path, fshd.name, filter))
                return false;
        }
    }

    return true;
}

uint8_t *ApkFile::Extract(const std::string &file_path, size_t *psize) const
{
    const FileEntry *pfile = nullptr;
    size_t sc = file_path.find(':');

    if (sc == std::string::npos)
    {
        std::string wpath = file_path;
        if (wpath.length() > 0 && wpath.front() == '/')
            wpath = wpath.substr(1);

        for (const FileEntry &file : files)
        {
            if (file.attributes != ATTRIBUTE_DIRECTORY && file.path == wpath)
            {
                pfile = &file;
                break;
            }
        }
    }
    else
    {
        const Fshd *fshd = nullptr;
        std::string fshd_name = file_path.substr(0, sc);

        for (const Fshd &f : fshds)
        {
            if (f.name == fshd_name)
            {
                fshd = &f;
                break;
            }
        }

        if (fshd == nullptr)
            return nullptr;

        std::string wpath = file_path.substr(sc+1);
        if (wpath.length() > 0 && wpath.front() == '/')
            wpath = wpath.substr(1);

        for (const FileEntry &file : fshd->files)
        {
            //DPRINTF("%s\n", file.path.c_str());
            if (file.attributes != ATTRIBUTE_DIRECTORY && file.path == wpath)
            {
                pfile = &file;
                break;
            }
        }
    }

    if (pfile == nullptr)
    {
        //DPRINTF("Failed to find %s\n", file_path.c_str());
        return nullptr;
    }

    *psize = (size_t)pfile->size;
    uint8_t *ret = new uint8_t[pfile->size];

    if (pfile->attributes == ATTRIBUTE_UNCOMPRESSED)
    {
        if (pfile->buf)
        {
            memcpy(ret, pfile->buf, pfile->size);
        }
        else
        {
            if (!apk->Seek((off64_t)pfile->file_offset, SEEK_SET))
            {
                delete[] ret;
                return nullptr;
            }

            if (!apk->Read(ret, pfile->size))
            {
                delete[] ret;
                return nullptr;
            }
        }
    }
    else
    {
        uint8_t *comp_buf = pfile->buf;

        if (!comp_buf)
        {
            if (!apk->Seek((off64_t)pfile->file_offset, SEEK_SET))
            {
                delete[] ret;
                return nullptr;
            }

            comp_buf = new uint8_t[pfile->comp_size];
            if (!apk->Read(comp_buf, pfile->comp_size))
            {
                delete[] ret; delete[] comp_buf;
                return nullptr;
            }
        }

        uint32_t uncomp_size = (uint32_t)pfile->size;
        bool success = Utils::UncompressZlib(ret, &uncomp_size, comp_buf, (uint32_t)pfile->comp_size);

        if (!pfile->buf) delete[] comp_buf;
        if (!success)
        {
            delete[] ret;
            return nullptr;
        }
    }

    return ret;
}

bool ApkFile::Replace(const std::string &path, const uint8_t *buf, size_t size)
{
    size_t sc = path.find(':');

    if (sc == std::string::npos)
    {
        std::string wpath = path;
        if (wpath.length() > 0 && wpath.front() == '/')
            wpath = wpath.substr(1);

        for (FileEntry &file : files)
        {
            if (file.attributes != ATTRIBUTE_DIRECTORY && file.path == wpath)
                return DoReplaceFile(file, buf, size);
        }
    }
    else
    {
        Fshd *fshd = nullptr;
        std::string fshd_name = path.substr(0, sc);

        for (Fshd &f : fshds)
        {
            if (f.name == fshd_name)
            {
                fshd = &f;
                break;
            }
        }

        if (fshd == nullptr)
            return false;

        std::string wpath = path.substr(sc+1);
        if (wpath.length() > 0 && wpath.front() == '/')
            wpath = wpath.substr(1);

        for (FileEntry &file : fshd->files)
        {
            if (file.attributes != ATTRIBUTE_DIRECTORY && file.path == wpath)
                return DoReplaceFile(file, buf, size);
        }
    }

    return false;
}

bool ApkFile::Replace(const std::string &path, const std::string &src_path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(src_path, &size);
    if (!buf)
        return false;

    bool ret = Replace(path, buf, size);
    delete[] buf;
    return ret;
}

bool ApkFile::VisitFiles(bool (*visitor)(const std::string &, void *, void *, void *), const std::string &filter, void *custom_param1, void *custom_param2, void *custom_param3) const
{
    for (const FileEntry &file: files)
    {
        if (file.attributes == ATTRIBUTE_DIRECTORY)
            continue;

        if (filter.length() == 0 || Utils::EndsWith(file.name, filter, false))
        {
            if (!visitor(file.path, custom_param1, custom_param2, custom_param3))
                return false;
        }
    }

    for (const Fshd &fshd : fshds)
    {
        for (const FileEntry &file: fshd.files)
        {
            if (file.attributes == ATTRIBUTE_DIRECTORY)
                continue;

            if (filter.length() == 0 || Utils::EndsWith(file.name, filter, false))
            {
                std::string path = fshd.name + ":/" + file.path;
                if (!visitor(path, custom_param1, custom_param2, custom_param3))
                    return false;
            }
        }
    }

    return true;
}

void ApkFile::DebugDumpOffsets()
{
    std::vector<FileEntry> &cfiles = files;
    std::sort(cfiles.begin(), cfiles.end(), [](const FileEntry &f1, const FileEntry &f2) { return f1.file_offset < f2.file_offset; } );

    for (const FileEntry &file : cfiles)
    {
        if (file.attributes == ATTRIBUTE_DIRECTORY)
            continue;

        DPRINTF("%s: %I64x\n", file.path.c_str(), file.file_offset);
    }
}

void ApkFile::DebugDumpPaths(const std::string &filter) const
{
    for (const FileEntry &file : files)
    {
        if (filter.length() == 0 || Utils::EndsWith(file.name, filter, false))
            DPRINTF("%s\n", file.path.c_str());
    }

    for (const Fshd &fshd: fshds)
    {
        for (const FileEntry &file : fshd.files)
        {
            if (filter.length() == 0 || Utils::EndsWith(file.name, filter, false))
                DPRINTF("%s:/%s\n", fshd.name.c_str(), file.path.c_str());
        }
    }
}

bool ApkFile::CompareFile(const FileEntry &f1, const FileEntry &f2, FileStream *apk1, FileStream *apk2)
{
    if (f1.attributes != f2.attributes)
        return false;

    if (f1.name != f2.name)
        return false;

    if (f1.path != f2.path)
        return false;

    if (f1.pack_idx != f2.pack_idx)
        return false;

    if (f1.attributes == ATTRIBUTE_DIRECTORY)
    {
        if (f1.file_idx_start != f2.file_idx_start)
            return false;

        if (f1.num_files != f2.num_files)
            return false;
    }
    else
    {
        if (f1.size != f2.size || f1.comp_size != f2.comp_size)
            return false;

        uint8_t *buf1 = f1.buf;
        uint8_t *buf2 = f2.buf;
        uint64_t size = (f1.attributes == ATTRIBUTE_COMPRESSED) ? f1.comp_size : f1.size;

        if (!buf1)
        {
            if (!apk1->Seek((off64_t)f1.file_offset, SEEK_SET))
                return false;

            buf1 = new uint8_t[size];
            if (!apk1->Read(buf1, (size_t)size))
            {
                delete[] buf1;
                return false;
            }
        }

        if (!buf2)
        {
            if (!apk2->Seek((off64_t)f2.file_offset, SEEK_SET))
            {
                if (!f1.buf) delete[] buf1;
                return false;
            }

            buf2 = new uint8_t[size];
            if (!apk2->Read(buf2, (size_t)size))
            {
                if (!f1.buf) delete[] buf1;
                delete[] buf2;
                return false;
            }
        }

        bool ret = (memcmp(buf1, buf2, (size_t)size) == 0);
        if (!f1.buf) delete[] buf1;
        if (!f2.buf) delete[] buf2;
        //if (!ret) DPRINTF("Comparison failed at %s  (offset %I64x %I64x)\n", f1.path.c_str(), f1.file_offset, f2.file_offset);
        return ret;
    }

    return true;
}

bool ApkFile::operator==(const ApkFile &other)
{
    if (!apk || !other.apk)
        return false;

    if (memcmp(checksum, other.checksum, sizeof(checksum)) != 0)
        return false;

    if (pack_idx != other.pack_idx)
        return false;

    if (names.size() != other.names.size())
        return false;

    if (files.size() != other.files.size())
        return false;

    if (fshds.size() != other.fshds.size())
        return false;

    for (size_t i = 0; i < names.size(); i++)
        if (names[i] != other.names[i])
            return false;

    for (size_t i = 0; i< files.size(); i++)
    {
        if (!CompareFile(files[i], other.files[i], apk, other.apk))
            return false;
    }

    for (size_t i = 0; i < fshds.size(); i++)
    {
        if (fshds[i].name != other.fshds[i].name)
            return false;

        if (fshds[i].names.size() != other.fshds[i].names.size())
            return false;

        if (fshds[i].files.size() != other.fshds[i].files.size())
            return false;

        if (fshds[i].pack_idx != other.fshds[i].pack_idx)
            return false;

        for (size_t j = 0; j < fshds[i].names.size(); j++)
            if (fshds[i].names[j] != other.fshds[i].names[j])
                return false;

        for (size_t j = 0; j < fshds[i].files.size(); j++)
            if (!CompareFile(fshds[i].files[j], other.fshds[i].files[j], apk, other.apk))
                return false;
    }

    return true;
}
