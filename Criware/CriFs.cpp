#include "CriFs.h"

CriFs::CriFs(const std::string &loose_files_root, bool read_only)
{
    this->loose_files_root = Utils::NormalizePath(loose_files_root);

    if (!Utils::EndsWith(this->loose_files_root, "/"))
    {
        this->loose_files_root += '/';
    }

    this->read_only = read_only;

}

CriFs::~CriFs()
{
    for (CpkFile *cpk : cpks)
        delete cpk;
}

std::string CriFs::ConvertPath(const std::string &path)
{
    std::string new_path = Utils::NormalizePath(path);

    if (Utils::BeginsWith(new_path, "/"))
        new_path = new_path.substr(1);

    std::vector<std::string> components;
    Utils::GetMultipleStrings(new_path, components, '/');

    for (size_t i = 0; i < components.size(); i++)
    {
        if (components[i] == ".")
        {
            components.erase(components.begin()+i);
            i--;
        }
        else if (components[i] == "..")
        {
            if (i == 0)
                return std::string();

            components.erase(components.begin()+i-1, components.begin()+i+1);
            i -= 2;
        }
    }

    new_path.clear();

    for (size_t i = 0; i < components.size(); i++)
    {
        new_path += components[i];

        if (i != (components.size()-1))
            new_path += '/';
    }

    return new_path;
}

bool CriFs::AddCpk(const std::string &cpk_path)
{
    CpkFile *cpk = new CpkFile();

    if (!cpk->LoadFromFile(cpk_path, false))
    {
        delete cpk;
        return false;
    }

    cpks.push_back(cpk);
    return true;
}

bool CriFs::BuildVisitor(const std::string &path, bool, void *param)
{
    CriFs *pthis = (CriFs *)param;
    std::string local_path = Utils::NormalizePath(path);

    if (!Utils::BeginsWith(local_path, pthis->loose_files_root, false))
        return true;

    local_path = local_path.substr(pthis->loose_files_root.length());

    while (Utils::BeginsWith(local_path, "/"))
        local_path = local_path.substr(1);

    if (local_path.length() != 0 && !Utils::EndsWith(local_path, "/"))
        local_path += '/';

    pthis->directories.insert(local_path);
    pthis->directories_loose.insert(local_path);
    return true;
}

void CriFs::BuildDirList()
{
    directories.clear();
    directories_cpk.clear();
    directories_loose.clear();

    Utils::VisitDirectory(loose_files_root, false, true, true, BuildVisitor, this);

    for (CpkFile *cpk : cpks)
    {
        uint32_t num = cpk->GetNumFiles();

        for (uint32_t i = 0; i < num; i++)
        {
            std::string path;

            if (!cpk->GetParentDirectory(i, path))
                continue;

            if (path.length() != 0 && !Utils::EndsWith(path, "/"))
                path += "/";

            if (directories.find(path) == directories.end())
            {
                directories.insert(path);
            }

            if (directories_cpk.find(path) == directories_cpk.end())
            {
                directories_cpk.insert(path);
            }
        }
    }
}

size_t CriFs::GetFileSize(const std::string &path)
{
    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return (size_t)-1;

    std::string loose_path = loose_files_root + conv_path;
    size_t ret = Utils::GetFileSize(loose_path);

    if (ret == (size_t)-1)
    {
        for (CpkFile *cpk : cpks)
        {
            uint64_t size;

            if (cpk->GetFileSize(conv_path, &size))
                return (size_t)size;
        }
    }

    return ret;
}

uint8_t *CriFs::ReadFile(const std::string &path, size_t *psize, bool only_cpk)
{
    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return nullptr;

    if (!only_cpk)
    {
        std::string loose_path = loose_files_root + conv_path;
        uint8_t *buf = Utils::ReadFile(loose_path, psize, false);

        if (buf)
            return buf;
    }

    for (CpkFile *cpk : cpks)
    {
        uint64_t size;
        uint8_t *buf = cpk->ExtractFile(conv_path, &size);

        if (buf)
        {
            *psize = (size_t)size;
            return buf;
        }
    }

    return nullptr;
}

char *CriFs::ReadTextFile(const std::string &path)
{
    size_t size;
    uint8_t *buf = ReadFile(path, &size);

    if (!buf)
        return nullptr;

    char *ret = new char[size+1];
    ret[size] = 0;

    memcpy(ret, buf, size);
    delete[] buf;
    return ret;
}

bool CriFs::WriteFile(const std::string &path, const void *buf, size_t size)
{
    if (read_only)
        return false;

    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return false;

    std::string loose_path = loose_files_root + conv_path;
    return Utils::WriteFileBool(loose_path, (const uint8_t *)buf, size, true, true);
}

bool CriFs::RemoveFile(const std::string &path)
{
    if (read_only)
        return false;

    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return false;

    std::string loose_path = loose_files_root + conv_path;
    return Utils::RemoveFile(loose_path);
}

void CriFs::RemoveEmptyDir(const std::string &path)
{
    if (read_only)
        return;

    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return;

    std::string loose_path = loose_files_root + conv_path;
    Utils::RemoveEmptyDir(loose_path);
}

bool remove_visitor(const std::string &path, bool, void *)
{
    Utils::RemoveFile(path);
    return true;
}

void CriFs::RemoveDir(const std::string &path, bool remove_empty)
{
    if (read_only)
        return;

    std::string conv_path = ConvertPath(path);

    if (conv_path.length() == 0)
        return;

    std::string loose_path = loose_files_root + conv_path;
    Utils::VisitDirectory(loose_path, true, false, true, remove_visitor);

    if (remove_empty)
        Utils::RemoveEmptyDir(loose_path);
}

bool CriFs::LoadFile(BaseFile *file, const std::string &path, bool only_cpk)
{
    size_t size;
    uint8_t *buf = ReadFile(path, &size, only_cpk);

    if (!buf)
        return false;

    bool ret = file->Load(buf, size);
    delete[] buf;

    return ret;
}

bool CriFs::SaveFile(BaseFile *file, const std::string &path)
{
    size_t size;
    uint8_t *buf = file->Save(&size);

    if (!buf)
        return false;

    bool ret = WriteFile(path, buf, size);
    delete[] buf;

    return ret;
}

bool CriFs::CompileFile(BaseFile *file, const std::string &path)
{
    char *xml_buf = ReadTextFile(path);

    if (!xml_buf)
        return false;

    TiXmlDocument doc;

    doc.Parse(xml_buf);
    delete[] xml_buf;

    if (doc.ErrorId() != 0)
    {
        return false;
    }

    return file->Compile(&doc);
}

bool CriFs::DecompileFile(BaseFile *file, const std::string &path)
{
    TiXmlDocument *doc = file->Decompile();

    if (!doc)
        return false;

    TiXmlPrinter printer;
    doc->Accept(&printer);

    const char *xml_buf = printer.CStr();
    bool ret = WriteFile(path, xml_buf, strlen(xml_buf));

    delete doc;
    return ret;
}

bool CriFs::FileExists(const std::string &file, bool check_cpk, bool check_loose)
{
    std::string conv_path = ConvertPath(file);

    if (conv_path.length() == 0)
        return false;

    if (check_loose)
    {
        std::string loose_path = loose_files_root + conv_path;
        if (Utils::FileExists(loose_path))
            return true;
    }

    if (!check_cpk)
        return false;

    for (CpkFile *cpk : cpks)
    {
        if (cpk->FileExists(conv_path))
            return true;
    }

    return false;
}

bool CriFs::DirExists(const std::string &dir, bool check_cpk, bool check_loose)
{
    std::string conv_path = ConvertPath(dir);

    if (conv_path.length() == 0)
        return false;

    if (check_loose)
    {
        std::string loose_path = loose_files_root + conv_path;
        if (Utils::DirExists(loose_path))
            return true;
    }

    if (!check_cpk)
        return false;

    if (directories_cpk.size() == 0)
        BuildDirList();

    if (!Utils::EndsWith(conv_path, "/"))
        conv_path.push_back('/');

    return (directories_cpk.find(conv_path) != directories_cpk.end());
}

bool CriFs::VisitVisitor(const std::string &path, bool, void *param)
{
    CriFs *pthis = (CriFs *)param;

    if (Utils::BeginsWith(path, pthis->loose_files_root, false))
    {
        const std::string add_path = path.substr(pthis->loose_files_root.length());
        pthis->visitor_files_list.insert(add_path);
    }
    else
    {
        pthis->visitor_files_list.insert(path);
    }

    return true;
}

bool CriFs::VisitDirectory(const std::string &path, bool files, bool directories, bool recursive, bool (*visitor)(const std::string &, bool, void *), void *custom_param)
{
    std::string conv_path;
    std::string conv_path_lower;

    if (path.length() != 0)
    {
        conv_path = ConvertPath(path);

        if (conv_path.length() == 0)
            return false;

        if (!Utils::EndsWith(conv_path, "/"))
            conv_path += '/';

        conv_path_lower = Utils::ToLowerCase(conv_path);
    }

    if (directories)
    {
        if (this->directories.size() == 0)
        {
            BuildDirList();
        }

        for (const std::string &dir : this->directories)
        {
            if (recursive)
            {
                if (Utils::BeginsWith(dir, conv_path, false))
                {
                    if (!visitor(dir, true, custom_param))
                        return false;
                }
            }
            else
            {
                size_t pos = dir.rfind('/');

                if (pos == std::string::npos) // Only case is directory ""
                {
                    if (dir == conv_path_lower)
                        if (!visitor(dir, true, custom_param))
                            return false;
                }
                else
                {
                    std::string subdir = dir.substr(0, pos);
                    pos = subdir.rfind('/');

                    if (pos == std::string::npos)
                    {
                        if (dir == conv_path_lower)
                            if (!visitor(dir, true, custom_param))
                                return false;
                    }
                    else
                    {
                        subdir = dir.substr(0, pos+1);

                        if (Utils::ToLowerCase(subdir) == conv_path_lower)
                        {
                            if (!visitor(dir, true, custom_param))
                                return false;
                        }
                    }
                }
            }
        }
    }

    if (files)
    {
        visitor_files_list.clear();
        std::string loose_path = loose_files_root + conv_path;

        Utils::VisitDirectory(loose_path, true, false, recursive, VisitVisitor, this);

        for (CpkFile *cpk : cpks)
        {
            uint32_t num = cpk->GetNumFiles();

            for (uint32_t i = 0; i < num; i++)
            {
                std::string file_path;

                if (!cpk->GetFilePath(i, file_path))
                    continue;

                if (recursive)
                {
                    if (Utils::BeginsWith(file_path, conv_path, false))
                    {
                        if (visitor_files_list.find(file_path) == visitor_files_list.end())
                        {
                            visitor_files_list.insert(file_path);
                        }
                    }
                }
                else
                {
                    size_t pos = file_path.rfind('/');
                    bool add_it = false;

                    if (pos != std::string::npos)
                    {
                        std::string subdir = file_path.substr(0, pos+1);

                        if (Utils::ToLowerCase(subdir) == conv_path_lower)
                        {
                            add_it = true;
                        }
                    }
                    else
                    {
                        if (conv_path.length() == 0)
                        {
                            add_it = true;
                        }
                    }

                    if (add_it && visitor_files_list.find(file_path) == visitor_files_list.end())
                    {
                        visitor_files_list.insert(file_path);
                    }
                }
            }
        }

        for (const std::string &file : visitor_files_list)
        {
            if (!visitor(file, false, custom_param))
                return false;
        }
    }

    return true;
}

static bool list_visitor(const std::string &path, bool, void *custom_param)
{
    std::vector<std::string> *plist = static_cast< std::vector<std::string> *>(custom_param);
    plist->push_back(path);
    return true;
}

bool CriFs::ListDirectory(const std::string &path, std::vector<std::string> &out_paths, bool files, bool directories, bool recursive)
{
    return VisitDirectory(path, files, directories, recursive, list_visitor, &out_paths);
}
