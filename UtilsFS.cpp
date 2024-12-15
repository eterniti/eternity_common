#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include <sstream>
#include <ctime>
#include <cstdlib>
#include <stdexcept>
#include <climits>

#include "UtilsFS.h"
#include "debug.h"

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#define do_mkdir(a, b) _mkdir(a)

#ifndef FOF_NO_UI
#define FOF_NO_UI (FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR)
#endif

#else

#define do_mkdir(a, b) mkdir(a, b)

#endif

#define FILE_BUFFER_SIZE	(16*1024*1024)

// If you get an error because of using a higher version of mingw that supports this, just delete me
#ifdef __MINGW32__
#define FindExInfoBasic	(FINDEX_INFO_LEVELS)1
#endif

bool Utils::FileExists(const std::string &path)
{
    struct __stat64 info;
    return (_stat64(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool Utils::FileExists(const std::u16string &path)
{
    struct __stat64 info;
    return (_wstat64((const wchar_t *)path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool Utils::DirExists(const std::string &path)
{
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}

bool Utils::DirExists(const std::u16string &path)
{
    struct _stat info;
    return (_wstat((const wchar_t *)path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}

bool Utils::AppDataFileExists(const std::string &rel_path)
{
    std::string full_path = Utils::WindowsPath(Utils::GetAppData() + "//" + rel_path);
    return FileExists(full_path);
}

bool Utils::AppDataFileExists(const std::u16string &rel_path)
{
    std::u16string full_path = Utils::WindowsPath(Utils::GetAppData16() + (const char16_t *)L"//" + rel_path);
    return FileExists(full_path);
}

size_t Utils::GetFileSize(const std::string &path)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
        return (size_t)-1;

    return (size_t)info.st_size;
}

size_t Utils::GetFileSize(const std::u16string &path)
{
    struct _stat info;

    if (_wstat((const wchar_t *)path.c_str(), &info) != 0)
        return (size_t)-1;

    return (size_t)info.st_size;
}

bool Utils::GetFileDate(const std::string &path, time_t *mtime, time_t *ctime, time_t *atime)
{
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
        return false;

    if (mtime)
        *mtime = info.st_mtime;

    if (ctime)
        *ctime = info.st_ctime;

    if (atime)
        *atime = info.st_atime;

    return true;
}

bool Utils::GetFileDate(const std::u16string &path, time_t *mtime, time_t *ctime, time_t *atime)
{
    struct _stat info;

    if (_wstat((const wchar_t *)path.c_str(), &info) != 0)
        return false;

    if (mtime)
        *mtime = info.st_mtime;

    if (ctime)
        *ctime = info.st_ctime;

    if (atime)
        *atime = info.st_atime;

    return true;
}

uint8_t *Utils::ReadFile(const std::string &path, size_t *psize, bool show_error)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
    {
        if (show_error)
        {
            DPRINTF("Cannot open file \"%s\" for reading.\n"
                    "Error given by the system: %s\n", path.c_str(), strerror(errno));
        }

        return nullptr;
    }

    size_t size, rd;
    uint8_t *buf;

    fseeko64(f, 0, SEEK_END);
    size = (size_t)ftello64(f);
    fseeko64(f, 0, SEEK_SET);

    buf = new uint8_t[size];
    rd = fread(buf, 1, size, f);
    fclose(f);

    if (rd != size)
    {
        if (show_error)
            DPRINTF("Read failure on file \"%s\"\n", path.c_str());

        delete[] buf;
        return nullptr;
    }

    *psize = size;
    return buf;
}

uint8_t *Utils::ReadFileFrom(const std::string &path, size_t from, size_t size, bool show_error)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
    {
        if (show_error)
            DPRINTF("Cannot open file \"%s\" for reading.\n", path.c_str());

        return nullptr;
    }

    size_t rd;
    uint8_t *buf;

    fseeko64(f, from, SEEK_SET);

    buf = new uint8_t[size];
    rd = fread(buf, 1, size, f);
    fclose(f);

    if (rd != size)
    {
        if (show_error)
            DPRINTF("Read failure on file \"%s\"\n", path.c_str());

        delete[] buf;
        return nullptr;
    }

    return buf;
}

bool Utils::ReadTextFile(const std::string &path, std::string &text, bool show_error)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
    {
        if (show_error)
            DPRINTF("Cannot open file \"%s\" for reading.\n", path.c_str());

        return false;
    }

    size_t size, rd;
    char *buf;

    fseeko64(f, 0, SEEK_END);
    size = (size_t)ftello64(f);
    fseeko64(f, 0, SEEK_SET);

    buf = new char[size+1];
    buf[size] = 0;
    rd = fread(buf, 1, size, f);
    fclose(f);

    if (rd != size)
    {
        if (show_error)
            DPRINTF("Read failure on file \"%s\"\n", path.c_str());

        delete[] buf;
        return false;
    }

    text = buf;
    delete[] buf;

    return true;
}

bool Utils::WriteTextFile(const std::string &path, const std::string &text, bool show_error, bool build_path)
{
    FILE *f = (build_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");
    if (!f)
    {
        if (show_error)
            DPRINTF("Cannot open file \"%s\" for writing.\n", path.c_str());

        return false;
    }

    fwrite(text.c_str(), 1, text.length(), f);
    fclose(f);
    return true;
}

size_t Utils::WriteFileST(const std::string &path, const uint8_t *buf, size_t size, bool show_error, bool write_path)
{
    FILE *f = (write_path) ? Utils::fopen_create_path(path, "wb") : fopen(path.c_str(), "wb");

    if (!f)
    {
        if (show_error)
        {
            DPRINTF("Cannot open for overwrite/create, the file \"%s\"\n"
                    "Error given by the system: %s\n", path.c_str(), strerror(errno));
        }

        return -1;
    }

    if (size == 0)
    {
        fclose(f);
        return 0;
    }

    size_t wd = fwrite(buf, 1, size, f);
    fclose(f);

    return wd;
}

size_t Utils::WriteFileST(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error, bool write_path)
{
    FILE *f = (write_path) ? Utils::fopen_create_path(path, "wb") : _wfopen((const wchar_t *)path.c_str(), L"wb");

    if (!f)
    {
        if (show_error)
        {
            DPRINTF("Cannot open for overwrite/create, the file \"%S\"\n"
                    "Error given by the system: %s\n", (const wchar_t *)path.c_str(), strerror(errno));
        }

        return -1;
    }

    if (size == 0)
    {
        fclose(f);
        return 0;
    }

    size_t wd = fwrite(buf, 1, size, f);
    fclose(f);

    return wd;
}

bool Utils::WriteFileBool(const std::string &path, const uint8_t *buf, size_t size, bool show_error, bool write_path)
{
    size_t written = Utils::WriteFileST(path, buf, size, show_error, write_path);

    if ((int)written < 0)
        return false;

    if (written != size)
    {
        if (show_error)
        {
            DPRINTF("Cannot open for overwrite/create, the file \"%s\"\n"
                    "Error given by the system: %s\n", path.c_str(), strerror(errno));
        }

        return false;
    }

    return true;
}

bool Utils::WriteFileBool(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error, bool write_path)
{
    size_t written = Utils::WriteFileST(path, buf, size, show_error, write_path);

    if ((int)written < 0)
        return false;

    if (written != size)
    {
        if (show_error)
        {
            DPRINTF("Cannot open for overwrite/create, the file \"%S\"\n"
                    "Error given by the system: %s\n",(const wchar_t *) path.c_str(), strerror(errno));
        }

        return false;
    }

    return true;
}

bool Utils::Mkdir(const std::string &path)
{
    if (do_mkdir(path.c_str(), 0777) != 0)
    {
        if (errno != EEXIST)
            return false;
    }
    /*if (!CreateDirectoryA(path.c_str(), NULL))
    {
        if (GetLastError() != ERROR_ALREADY_EXISTS)
            return false;
    }*/

    return true;
}

bool Utils::RemoveFile(const std::string &path)
{
    if (remove(path.c_str()) == 0)
        return true;

    return (errno == ENOENT);
}

bool Utils::RemoveEmptyDir(const std::string &str)
{
    if (_rmdir(str.c_str()) == 0)
        return true;

    return (errno == ENOENT);
}

bool Utils::RemoveDirFull(const std::string &str, bool ui, bool recycle, bool confirmation)
{
    if (!Utils::DirExists(str))
        return true;

    SHFILEOPSTRUCTA op;
    memset(&op, 0, sizeof(op));

    char *temp = new char[str.length()+2];
    strcpy(temp, str.c_str());
    temp[str.length()+1] = 0;

    op.wFunc = FO_DELETE;
    op.pFrom = temp;

    if (!ui)
        op.fFlags = FOF_NO_UI;

    if (recycle)
        op.fFlags |= FOF_ALLOWUNDO;

    if (!confirmation)
        op.fFlags |= FOF_NOCONFIRMATION;

    bool ret = (SHFileOperationA(&op) == 0);
    delete[] temp;
    return ret;
}

bool Utils::RenameFile(const std::string &oldp, const std::string &newp)
{
    return (rename(oldp.c_str(), newp.c_str()) == 0);
}

bool Utils::MoveFileOrDir(const std::string &oldp, const std::string &newp)
{
    std::string oldp2 = Utils::WindowsPath(Utils::MakePathString(oldp, "*.*"));
    std::string newp2 = Utils::WindowsPath(newp);

    if (!Utils::DirExists(oldp))
        return false;

    SHFILEOPSTRUCTA op;
    memset(&op, 0, sizeof(op));
    op.wFunc = FO_MOVE;
    op.fFlags = FOF_NO_UI;

    char *src = new char[oldp2.length()+2];
    strcpy(src, oldp2.c_str());
    src[oldp2.length()+1] = 0;

    char *dst = new char[newp2.length()+2];
    strcpy(dst, newp2.c_str());
    dst[newp2.length()+1] = 0;

    op.pFrom = src;
    op.pTo = dst;

    int ret = SHFileOperationA(&op);

    delete[] src;
    delete[] dst;

    if (ret == 0)
        Utils::RemoveEmptyDir(oldp);

    return (ret == 0);
}

bool Utils::CreatePath(const std::string &path, bool last_is_directory)
{
    size_t pos = std::string::npos;
    size_t prev_pos = std::string::npos;

    while ((pos = path.find_first_of("/\\", pos+1)) != std::string::npos)
    {
        std::string current_dir;

        std::string dir = path.substr(0, pos);

        if (prev_pos != std::string::npos)
        {
            current_dir = dir.substr(prev_pos);
        }
        else
        {
            current_dir = dir;
        }

        if (current_dir.find_first_of("%:") == std::string::npos)
        {
            if (do_mkdir(dir.c_str(), 0777) != 0)
            {
                if (errno != EEXIST)
                {
                    //DPRINTF("returning false on %s\n", dir.c_str());
                    return false;
                }
            }
        }

        prev_pos = pos;
    }

    if (!last_is_directory)
        return true;

    if (do_mkdir(path.c_str(), 0777) != 0)
    {
        return (errno == EEXIST);
    }

    return true;
}

bool Utils::CreatePath(const std::u16string &path, bool last_is_directory)
{
    size_t pos = std::u16string::npos;
    size_t prev_pos = std::u16string::npos;

    while ((pos = path.find_first_of((const char16_t *)L"/\\", pos+1)) != std::u16string::npos)
    {
        std::u16string current_dir;

        std::u16string dir = path.substr(0, pos);

        if (prev_pos != std::string::npos)
        {
            current_dir = dir.substr(prev_pos);
        }
        else
        {
            current_dir = dir;
        }

        if (current_dir.find_first_of((const char16_t *)L"%:") == std::u16string::npos)
        {
            if (_wmkdir((const wchar_t *)dir.c_str()) != 0)
            {
                if (errno != EEXIST)
                {
                    //DPRINTF("returning false on %s\n", dir.c_str());
                    return false;
                }
            }
        }

        prev_pos = pos;
    }

    if (!last_is_directory)
        return true;

    if (_wmkdir((const wchar_t *)path.c_str()) != 0)
    {
        return (errno == EEXIST);
    }

    return true;
}

FILE *Utils::fopen_create_path(const std::string &filename, const char *mode)
{
    if (!Utils::CreatePath(filename))
        return nullptr;

    return fopen(filename.c_str(), mode);
}

FILE *Utils::fopen_create_path(const std::u16string &filename, const char *mode)
{
    if (!Utils::CreatePath(filename))
        return nullptr;

    std::u16string mode_str = Utf8ToUcs2(mode);
    return _wfopen((const wchar_t *)filename.c_str(), (const wchar_t *)mode_str.c_str());
}

#ifdef _WIN32

bool Utils::VisitDirectory(const std::string &path, bool files, bool directories, bool recursive, bool (* visitor)(const std::string &path, bool is_directory, void *custom_param), void *custom_param, bool show_error, bool ignore_error)
{
    std::string find_path = Utils::WindowsPath(path);

    if (find_path.length() > 0 && find_path.back() != '\\')
        find_path += '\\';

    find_path += '*';

    WIN32_FIND_DATAA fd;
    HANDLE hFile = FindFirstFileExA(find_path.c_str(), FindExInfoBasic, &fd, FindExSearchNameMatch, nullptr, 0);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        if (show_error)
        {
            DPRINTF("%s: FindFirstFileExA failed on \"%s\". Windows error=0x%x\n", FUNCNAME, find_path.c_str(), (unsigned int)GetLastError());
        }

        //DPRINTF("return %d\n", ignore_error);
        return ignore_error;
    }

    bool no_files = (directories && !files);

    do
    {
        if (fd.cFileName[0] == 0 || strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
            continue;

        if (no_files)
        {
            if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                continue;
        }

        std::string file_path = Utils::NormalizePath(path);

        if (file_path.length() > 0 && file_path.back() == '/')
        {
            file_path += fd.cFileName;
        }
        else
        {
            file_path.push_back('/');
            file_path += fd.cFileName;
        }

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (directories)
            {
                bool ret = visitor(file_path, true, custom_param);

                if (!ret)
                {
                    FindClose(hFile);
                    return false;
                }
            }

            if (recursive)
            {
                if (!VisitDirectory(file_path, files, directories, recursive, visitor, custom_param, show_error, ignore_error))
                {
                    FindClose(hFile);
                    return false;
                }
            }
        }
        else
        {
            if (files)
            {
                bool ret = visitor(file_path, false, custom_param);

                if (!ret)
                {
                    FindClose(hFile);
                    return false;
                }
            }
        }

    } while (FindNextFileA(hFile, &fd) != 0);

    FindClose(hFile);
    return true;
}

#else

bool Utils::VisitDirectory(const std::string &path, bool files, bool directories, bool recursive, bool (* visitor)(const std::string &path, bool is_directory, void *custom_param), void *custom_param)
{
    DIR *dir = opendir(path.c_str());
    struct dirent *entry;

    if (!dir)
        return false;

    while ((entry = readdir(dir)))
    {
        struct stat st;
        std::string file_path = Utils::NormalizePath(path);
        //std::string file_path = path + "/" + entry->d_name;

        if (file_path.length() > 0 && file_path.back() == '/')
        {
            file_path += entry->d_name;
        }
        else
        {
            file_path.push_back('/');
            file_path += entry->d_name;
        }

        //printf("%s\n", file_path.c_str());

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        if (stat(file_path.c_str(), &st) != 0)
            continue;

        if (S_ISREG(st.st_mode) && files)
        {
            bool ret = visitor(file_path, false, custom_param);

            if (!ret)
            {
                closedir(dir);
                return false;
            }
        }

        if (S_ISDIR(st.st_mode))
        {
            if (directories)
            {
                bool ret = visitor(file_path, true, custom_param);

                if (!ret)
                {
                    closedir(dir);
                    return false;
                }

            }

            if (recursive)
            {
                if (!VisitDirectory(file_path, files, directories, recursive, visitor, custom_param))
                {
                    closedir(dir);
                    return false;
                }
            }
        }
    }

    closedir(dir);
    return true;
}
#endif

static bool is_dir_empty_visitor(const std::string &path, bool is_directory, void *custom_param)
{
    UNUSED(path); UNUSED(is_directory); UNUSED(custom_param);
    return false;
}

bool Utils::IsDirectoryEmpty(const std::string &path, bool ignore_directories)
{
    return VisitDirectory(path, true, !ignore_directories, ignore_directories, is_dir_empty_visitor);
}

static bool count_files_visitor(const std::string &, bool, void *custom_param)
{
    size_t *count = (size_t *)custom_param;
    (*count)++;

    return true;
}

size_t Utils::CountFiles(const std::string &path, bool recursive)
{
    size_t ret = 0;

    Utils::VisitDirectory(path, true, false, recursive, count_files_visitor, &ret);
    return ret;
}

static bool list_files_visitor(const std::string &path, bool, void *custom_param)
{
    std::vector<std::string> *ppaths = (std::vector<std::string> *)custom_param;
    ppaths->push_back(path);
    return true;
}

bool Utils::ListFiles(const std::string &path, bool files, bool directories, bool recursive, std::vector<std::string> &paths)
{
    paths.clear();
    return Utils::VisitDirectory(path, files, directories, recursive, list_files_visitor, &paths);
}

bool Utils::CompareFiles(const std::string &file1, const std::string &file2)
{
    struct stat st1, st2;
    unsigned int buf_size;
    size_t remaining;

    if (stat(file1.c_str(), &st1) != 0 || stat(file2.c_str(), &st2) != 0)
        return false;

    if (st1.st_size != st2.st_size)
        return false;

    FILE *f1 = fopen(file1.c_str(), "rb");
    FILE *f2 = fopen(file2.c_str(), "rb");

    if (!f1 || !f2)
    {
        if (f1)
            fclose(f1);

        if (f2)
            fclose(f2);

        return false;
    }

    buf_size = (st1.st_size < FILE_BUFFER_SIZE) ? st1.st_size : FILE_BUFFER_SIZE;

    uint8_t *buf = new uint8_t[buf_size];

    remaining = st1.st_size;

    while (remaining > 0)
    {
        uint8_t sha1_1[20];
        uint8_t sha1_2[20];

        size_t r = (remaining < buf_size) ? remaining : buf_size;

        if (fread(buf, 1, r, f1) != r)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        Sha1(buf, (uint32_t)r, sha1_1);

        if (fread(buf, 1, r, f2) != r)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        Sha1(buf, (uint32_t)r, sha1_2);

        if (memcmp(sha1_1, sha1_2, sizeof(sha1_1)) != 0)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        remaining -= r;
    }

    fclose(f1);
    fclose(f2);
    delete[] buf;

    return true;
}

bool Utils::CompareFilesPartial(const std::string &file1, const std::string &file2, uint64_t compare_size)
{
    struct stat st1, st2;
    size_t buf_size;
    size_t remaining;

    if (stat(file1.c_str(), &st1) != 0 || stat(file2.c_str(), &st2) != 0)
        return false;

    if (st1.st_size < (off_t)compare_size || st2.st_size < (off_t)compare_size)
        return false;

    FILE *f1 = fopen(file1.c_str(), "rb");
    FILE *f2 = fopen(file2.c_str(), "rb");

    if (!f1 || !f2)
    {
        if (f1)
            fclose(f1);

        if (f2)
            fclose(f2);

        return false;
    }

    buf_size = (compare_size < FILE_BUFFER_SIZE) ? compare_size : FILE_BUFFER_SIZE;

    uint8_t *buf = new uint8_t[buf_size];

    remaining = compare_size;

    while (remaining > 0)
    {
        uint8_t sha1_1[20];
        uint8_t sha1_2[20];

        size_t r = (remaining < buf_size) ? remaining : buf_size;

        if (fread(buf, 1, r, f1) != r)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        Sha1(buf, (uint32_t)r, sha1_1);

        if (fread(buf, 1, r, f2) != r)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        Sha1(buf, (uint32_t)r, sha1_2);

        if (memcmp(sha1_1, sha1_2, sizeof(sha1_1)) != 0)
        {
            fclose(f1);
            fclose(f2);
            delete[] buf;
            return false;
        }

        remaining -= r;
    }

    fclose(f1);
    fclose(f2);
    delete[] buf;

    return true;
}

bool Utils::DoCopyFile(FILE *src, FILE *dst, uint64_t size)
{
    uint8_t *copy_buf;
    size_t remaining;

    if (size < FILE_BUFFER_SIZE)
    {
        copy_buf = new uint8_t[size];
    }
    else
    {
        copy_buf = new uint8_t[FILE_BUFFER_SIZE];
    }

    remaining = size;

    while (remaining > 0)
    {
        size_t r = (remaining > FILE_BUFFER_SIZE) ? FILE_BUFFER_SIZE: remaining;

        if (fread(copy_buf, 1, r, src) != r)
        {
            delete[] copy_buf;
            return false;
        }

        if (fwrite(copy_buf, 1, r, dst) != r)
        {
            delete[] copy_buf;
            return false;
        }

        remaining -= r;
    }

    delete[] copy_buf;
    return true;
}

bool Utils::DoCopyFile(const std::string &input, const std::string &output, bool build_path)
{
    FILE *r = fopen(input.c_str(), "rb");
    if (!r)
        return false;

    FILE *w= (build_path) ? fopen_create_path(output, "wb") : fopen(output.c_str(), "wb");
    if (!w)
    {
        fclose(r);
        return false;
    }

    fseeko64(r, 0, SEEK_END);
    off64_t size = ftello64(r);
    fseeko64(r, 0, SEEK_SET);

    bool ret = DoCopyFile(r, w, (size_t)size);

    fclose(r);
    fclose(w);
    return ret;
}

bool Utils::CopyDir(const std::string &input, const std::string &output, bool hard_link_if_possible)
{
    bool do_hardlink = false;

    if (hard_link_if_possible && input.length()>= 2 && output.length() >= 2 && input[1] == ':' && output[1] == ':' && input[0] == output[0])
    {
        //DPRINTF("Hardlink activated.\n");
        do_hardlink = true;
    }

    std::vector<std::string> files;

    std::string input_norm = Utils::NormalizePath(input);

    Utils::ListFiles(input_norm, true, false, true, files);

    for (const std::string &file : files)
    {
        std::string ofile = Utils::MakePathString(Utils::NormalizePath(output), file.substr(input_norm.length()));

        if (do_hardlink && !Utils::FileExists(ofile))
        {
            if (HardLink(file, ofile, true))
            {
                continue;
            }
            else
            {
                //DPRINTF("Hardlink failed, changing to copy.\n");
                do_hardlink = false;
            }
        }

        if (!Utils::DoCopyFile(file, ofile, true))
        {
            //DPRINTF("Copy error");
            return false;
        }
    }

    return true;
}

bool Utils::HardLink(const std::string &input, const std::string &output, bool build_path)
{
    if (build_path)
        CreatePath(output);

    if (!CreateHardLinkA(Utils::WindowsPath(output).c_str(), Utils::WindowsPath(input).c_str(), nullptr))
    {
        //DPRINTF("Last error: %x\n%s -> %s\n", GetLastError(), input.c_str(), output.c_str());
        return false;
    }

    return true;
}
