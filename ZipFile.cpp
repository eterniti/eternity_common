#include <stdexcept>
#include "ZipFile.h"
#include "debug.h"


ZipFile::ZipFile()
{
    int error;
    archive = nullptr;    

    for (int retries = 0; retries < 5 && !archive; retries++)
    {
        temp_file = Utils::GetTempFileLocal(".tmp");
        archive = zip_open(temp_file.c_str(), ZIP_CREATE | ZIP_EXCL, &error);
    }

    if (!archive)
    {
        DPRINTF("Internal error in ZipFile ctor: %d %d %s  %s\n", error, errno, strerror(errno), temp_file.c_str());
        throw std::runtime_error("Internal error in ZipFile ctor");
    }
}

ZipFile::~ZipFile()
{
    if (archive)
        zip_discard(archive);

    Utils::RemoveFile(temp_file);

    //_rmdir("temp");
}

void ZipFile::Reload()
{
    if (!archive)
        return;

    zip_close(archive);
    int error;
    archive = zip_open(temp_file.c_str(), 0, &error);

    if (!archive)
    {
        DPRINTF("Internal error in ZipFile::Reload: %d %d %s  %s\n", error, errno, strerror(errno), temp_file.c_str());
        throw std::runtime_error("Internal error in ZipFile ctor");
    }
}

bool ZipFile::Load(const uint8_t *buf, size_t size)
{
    if (archive)
    {
        zip_discard(archive);
        archive = nullptr;
    }

    if (!Utils::WriteFileBool(temp_file, buf, size, false))
        return false;

    archive = zip_open(temp_file.c_str(), 0, nullptr);
    if (!archive)
        return false;

    return true;
}

uint8_t *ZipFile::Save(size_t *psize)
{
    if (IsEmpty())
        return nullptr;

    zip_close(archive);
    archive = nullptr;

    uint8_t *buf = Utils::ReadFile(temp_file, psize);

    {
        archive = zip_open(temp_file.c_str(), 0, nullptr);
        if (!archive)
            throw std::runtime_error("Internal error in ZipFile::Save");
    }

    return buf;
}

size_t ZipFile::GetNumEntries() const
{
    if (!archive)
        return 0;

    return zip_get_num_entries(archive, 0);
}

bool ZipFile::FileExists(const std::string &path) const
{
    return (archive && zip_name_locate(archive, path.c_str(), ZIP_FL_NOCASE) != -1);
}

bool ZipFile::DirExists(const std::string &path) const
{
    if (!archive)
        return false;

    size_t num_entries = GetNumEntries();

    std::string in_path = Utils::NormalizePath(path);
    if (!Utils::EndsWith(in_path, "/"))
        in_path += "/";

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (Utils::BeginsWith(entry_path, in_path, false))
            return true;
    }

    return false;
}

uint64_t ZipFile::GetFileSize(const std::string &path) const
{
    if (!archive)
        return (uint64_t)-1;

    zip_stat_t zstat;

    if (zip_stat(archive, path.c_str(), ZIP_FL_NOCASE, &zstat) == -1)
    {
        //DPRINTF("Fail here, %s\n", path.c_str());
        return (uint64_t)-1;
    }

    if (!(zstat.valid & ZIP_STAT_SIZE))
    {
        DPRINTF("Fail here, %s\n", path.c_str());
        return (uint64_t)-1;
    }

    return zstat.size;
}

uint8_t *ZipFile::ReadFile(const std::string &path, size_t *psize)
{
    uint64_t size = GetFileSize(path);
    if (size >= 0xFFFFFFFF)
        return nullptr;

    zip_file_t *fd = zip_fopen(archive, path.c_str(), ZIP_FL_NOCASE);
    if (!fd)
    {
        //DPRINTF("Fail here, %s  (%s)\n", path.c_str(), zip_strerror(archive));
        const char *err = zip_strerror(archive);
        if (err && strstr(err, "hanged"))
        {
            Reload();
            fd = zip_fopen(archive, path.c_str(), ZIP_FL_NOCASE);
            if (!fd)
            {
                //DPRINTF("Fail here, %s  (%s)\n", path.c_str(), zip_strerror(archive));
                return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }

    uint8_t *buf = new uint8_t[size];

    uint64_t rd = zip_fread(fd, buf, size);
    zip_fclose(fd);

    if (rd != size)
    {
        delete[] buf;
        return nullptr;
    }

    *psize = (size_t)rd;
    return buf;
}

bool ZipFile::WriteFile(const std::string &path, const void *buf, size_t size)
{
    if (!archive)
        return false;

    void *copy = malloc(size);
    memcpy(copy, buf, size);

    zip_source_t *source = zip_source_buffer(archive, copy, size, 1);
    if (!source)
    {
        DPRINTF("%s: fail in zip_source_buffer, libzip reports: %s\n", FUNCNAME, zip_strerror(archive));
        return false;
    }

    if (zip_file_add(archive, path.c_str(), source, ZIP_FL_OVERWRITE | ZIP_FL_NOCASE) == -1)
    {
        DPRINTF("%s: fail in zip_file_add (adding file %s), libzip reports: %s\n", FUNCNAME, path.c_str(), zip_strerror(archive));
        zip_source_free(source);
        return false;
    }

    return true;
}

char *ZipFile::ReadTextFile(const std::string &path)
{
    uint64_t size = GetFileSize(path);
    if (size > 0xFFFFFFFF)
        return nullptr;    

    zip_file_t *fd = zip_fopen(archive, path.c_str(), ZIP_FL_NOCASE);
    if (!fd)
        return nullptr;    

    char *buf = new char[size+1];
    buf[size] = 0;

    uint64_t rd = zip_fread(fd, buf, size);
    zip_fclose(fd);

    if (rd != size)
    {
        delete[] buf;
        return nullptr;
    }

    return buf;
}

bool ZipFile::ReadTextFile(const std::string &path, std::string &ret)
{
    char *cstr = ReadTextFile(path);
    if (!cstr)
        return false;

    ret = cstr;
    delete[] cstr;
    return true;
}

bool ZipFile::_DeleteFile(const std::string &path)
{
    if (!archive)
        return false;

    if (!FileExists(path))
        return true;

    zip_int64_t index = zip_name_locate(archive, path.c_str(), ZIP_FL_NOCASE);
    if (index < 0)
        return true;

    return (zip_delete(archive, index) != -1);
}

size_t ZipFile::DeleteDir(const std::string &path)
{
    size_t num_deleted = 0;
    size_t num_entries = GetNumEntries();

    std::string in_path = Utils::NormalizePath(path);
    if (!Utils::EndsWith(in_path, "/"))
        in_path += "/";

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (Utils::BeginsWith(entry_path, in_path, false))
        {
            if (zip_delete(archive, i) != -1)
                num_deleted++;
        }
    }

    return num_deleted;
}

size_t ZipFile::DeleteAll()
{
    size_t num_deleted = 0;
    size_t num_entries = GetNumEntries();

    for (size_t i = 0; i < num_entries; i++)
    {
        if (zip_delete(archive, i) != -1)
            num_deleted++;
    }

    return num_deleted;
}

bool ZipFile::VisitDirectory(const std::string &path, bool (*visitor)(const std::string &file, void *param), void *param)
{
    size_t num_entries = GetNumEntries();
    std::vector<std::string> found;

    std::string in_path = Utils::NormalizePath(path);
    if (!Utils::EndsWith(in_path, "/"))
        in_path += "/";

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (Utils::BeginsWith(entry_path, in_path, false))
        {
            found.push_back(entry_path);
        }
    }

    if (found.size() == 0)
        return false;

    for (const std::string &file : found)
    {
        if (!visitor(file, param))
            return false;
    }

    return true;
}

bool ZipFile::AddExternalFile(const std::string &external_path, const std::string &internal_path)
{
    size_t size;
    uint8_t *buf = Utils::ReadFile(external_path, &size);

    if (!buf)
        return false;

    bool ret = WriteFile(internal_path, buf, size);

    delete[] buf;
    return ret;
}

bool ZipFile::AddDirVisitor(const std::string &path, bool, void *param)
{
    const std::string path_norm = Utils::NormalizePath(path);
    ZipFile *pthis = (ZipFile *)param;

    const std::string &external_path = pthis->temp_param1;
    const std::string &internal_path = pthis->temp_param2;

    if (!Utils::BeginsWith(path_norm, external_path))
    {
        DPRINTF("%s: Bug in implementation.\n", FUNCNAME);
        return false;
    }

    std::string this_file_path = path_norm.substr(external_path.length());

    while (this_file_path.length() > 0 && this_file_path.front() == '/')
        this_file_path = this_file_path.substr(1);

    this_file_path = internal_path + this_file_path;

    return pthis->AddExternalFile(path_norm, this_file_path);
}

bool ZipFile::AddExternalDirectory(const std::string &external_path, const std::string &internal_path)
{
    temp_param1 = Utils::NormalizePath(external_path);
    temp_param2 = Utils::NormalizePath(internal_path);

    while (temp_param2.length() > 0 && temp_param2.front() == '/')
        temp_param2 = temp_param2.substr(1);

    if (temp_param2.length() > 0 && temp_param2.back() != '/')
        temp_param2 += '/';

    return Utils::VisitDirectory(temp_param1, true, false, true, AddDirVisitor, this, true);
}

bool ZipFile::RenameFile(const std::string &path, const std::string &new_path)
{
    if (!archive)
        return false;

    zip_stat_t zstat;    

    if (zip_stat(archive, path.c_str(), ZIP_FL_NOCASE, &zstat) == -1)
        return false;

    return (zip_rename(archive, zstat.index, new_path.c_str()) == 0);
}

bool ZipFile::RenamePath(const std::string &path, const std::string &new_path)
{
    size_t num_entries = GetNumEntries();

    std::string in_path = Utils::NormalizePath(path);
    if (in_path.back() != '/')
        in_path.push_back('/');

    for (size_t i = 0; i < num_entries; i++)
    {
        zip_stat_t zstat;

        if (zip_stat_index(archive, i, 0, &zstat) == -1)
            continue;

        if (!(zstat.valid & ZIP_STAT_NAME))
            continue;

        std::string entry_path = Utils::NormalizePath(zstat.name);

        if (Utils::BeginsWith(entry_path, in_path, false))
        {
            std::string new_fn = Utils::NormalizePath(new_path);

            if (new_fn.back() != '/')
                new_fn.push_back('/');

            new_fn += entry_path.substr(in_path.length());

            //DPRINTF("Rename %s -> %s\n", entry_path.c_str(), new_fn.c_str());

            if (zip_rename(archive, zstat.index, new_fn.c_str()) != 0)
                return false;
        }
    }

    std::string out_path = Utils::NormalizePath(new_path);
    if (out_path.back() != '/')
        out_path.push_back('/');

    RenameFile(in_path, out_path); // ignore any error, as the explicit entry may not exist
    return true;
}


