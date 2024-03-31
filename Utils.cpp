#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <assert.h>

#include <sstream>
#include <ctime>
#include <cstdlib>
#include <stdexcept>
#include <climits>
#include <algorithm>

#include <codecvt>
#include <locale>

#ifdef _WIN32

#include <Windows.h>
#include <direct.h>
#include <shellapi.h>
#define do_mkdir(a, b) _mkdir(a)

#ifndef FOF_NO_UI
#define FOF_NO_UI (FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR)
#endif

#else
	
#define do_mkdir(a, b) mkdir(a, b)
	
#endif

#ifndef NO_ZLIB
#include <zlib.h>
#endif

#include "Utils.h"
#include "debug.h"

#ifndef NO_CRYPTO
#include "crypto/sha1.h"
#include "crypto/md5.h"
#include "crypto/rijndael.h"
#endif

#define FILE_BUFFER_SIZE	(16*1024*1024)

// If you get an error because of using a higher version of mingw that supports this, just delete me
#ifdef __MINGW32__
#define FindExInfoBasic	(FINDEX_INFO_LEVELS)1
#endif

bool Utils::FileExists(const std::string &path)
{
#ifdef UTILS_UTF8
    return FileExists(Utf8ToUcs2(path));
#else
    struct __stat64 info;
    return (_stat64(path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
#endif
}

bool Utils::FileExists(const std::u16string &path)
{
    struct __stat64 info;
    return (_wstat64((const wchar_t *)path.c_str(), &info) == 0 && S_ISREG(info.st_mode));
}

bool Utils::DirExists(const std::string &path)
{
#ifdef UTILS_UTF8
    return DirExists(Utf8ToUcs2(path));
#else
    struct stat info;
    return (stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
#endif
}

bool Utils::DirExists(const std::u16string &path)
{
    struct _stat info;
    return (_wstat((const wchar_t *)path.c_str(), &info) == 0 && S_ISDIR(info.st_mode));
}

bool Utils::AppDataFileExists(const std::string &rel_path)
{
#ifdef UTILS_UTF8
    return AppDataFileExists(Utf8ToUcs2(rel_path));
#else
    std::string full_path = Utils::WindowsPath(Utils::GetAppData() + "//" + rel_path);
    return FileExists(full_path);
#endif
}

bool Utils::AppDataFileExists(const std::u16string &rel_path)
{
    std::u16string full_path = Utils::WindowsPath(Utils::GetAppData16() + (const char16_t *)L"//" + rel_path);
    return FileExists(full_path);
}

size_t Utils::GetFileSize(const std::string &path)
{
#ifdef UTILS_UTF8
    return GetFileSize(Utf8ToUcs2(path));
#else
    struct stat info;

    if (stat(path.c_str(), &info) != 0)
        return (size_t)-1;

    return (size_t)info.st_size;
#endif
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
#ifdef UTILS_UTF8
        return GetFileDate(Utf8ToUcs2(path), mtime, ctime, atime);
#else
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
#endif
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

size_t Utils::WriteFileST(const std::string &path, const uint8_t *buf, size_t size, bool show_error, bool write_path)
{
#ifdef UTILS_UTF8
    return WriteFileST(Utf8ToUcs2(path), buf, size, show_error, write_path);
#else
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
#endif
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
#ifdef UTILS_UTF8
    return WriteFileBool(Utf8ToUcs2(path), buf, size, show_error, write_path);
#else
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
#endif
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

bool Utils::CreatePath(const std::string &path, bool last_is_directory)
{
#ifdef UTILS_UTF8
    return CreatePath(Utf8ToUcs2(path), last_is_directory);
#else
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
#endif
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
#ifdef UTILS_UTF8
    return fopen_create_path(Utf8ToUcs2(filename), mode);
#else
	if (!Utils::CreatePath(filename))
        return nullptr;
	
    return fopen(filename.c_str(), mode);
#endif
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

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

std::string Utils::UnsignedToString(uint64_t value, bool hexadecimal)
{
	char temp[16];
	std::string str;
	
	if (hexadecimal)
	{
        sprintf(temp, "0x%I64x", value);
	}
	else
	{
        sprintf(temp, "%I64u", value);
	}
	
	str = temp;
	return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

std::string Utils::UnsignedToHexString(uint8_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%02x", value);
        else
            sprintf(temp, "%02x", value);
    }

    str = temp;
    return str;
}

std::string Utils::UnsignedToHexString(uint16_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%04x", value);
        else
            sprintf(temp, "%04x", value);
    }

    str = temp;
    return str;
}

std::string Utils::UnsignedToHexString(uint32_t value, bool zeropad, bool prefix)
{
    char temp[16];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%x", value);
        else
            sprintf(temp, "%x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%08x", value);
        else
            sprintf(temp, "%08x", value);
    }

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

std::string Utils::UnsignedToHexString(uint64_t value, bool zeropad, bool prefix)
{
    char temp[20];
    std::string str;

    if (!zeropad)
    {
        if (prefix)
            sprintf(temp, "0x%I64x", value);
        else
            sprintf(temp, "%I64x", value);
    }
    else
    {
        if (prefix)
            sprintf(temp, "0x%16I64x", value);
        else
            sprintf(temp, "%16I64x", value);
    }

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

std::string Utils::SignedToString(int64_t value)
{
    char temp[16];
    std::string str;

    sprintf(temp, "%I64d", value);

    str = temp;
    return str;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

std::string Utils::FloatToString(float value)
{
	char temp[32];
	std::string str;
	
	sprintf(temp, "%.9g", value);
	str = temp;
	
	if (str.find('.') == std::string::npos && str.find('e') == std::string::npos)
	{
		str = str + ".0";
	}
	
	return str;
}

std::string Utils::Vectors2DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors2DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if (vectors.size() & 1)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if (!(vectors.size() & 1))
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if (vectors.size() & 1)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

std::string Utils::Vectors3DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors3DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if ((vectors.size() % 3) == 2)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if ((vectors.size() % 3) != 2)
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if ((vectors.size() % 3) != 0)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

std::string Utils::Vectors4DToString(const float *vectors, size_t count)
{
    std::string str;
    const float *ptr = vectors;

    for (size_t i = 0; i < count; i++)
    {
        str += "{ ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += ", ";
        str += FloatToString(*ptr++);
        str += " }";

        if (i != (count-1))
            str += ", ";
    }

    return str;
}

size_t Utils::GetVectors4DFromString(const std::string &str, std::vector<float> &vectors)
{
    size_t count = 0;
    int state = 0; // 0 -> waiting '{'; 1 -> waiting number; 2 -> in number; 3 -> waiting ',' (between '}' and '{')

    std::string number_str;
    vectors.clear();

    for (char ch : str)
    {
        if (ch <= ' ')
            continue;

        if (state == 0)
        {
            if (ch != '{')
                return (size_t)-1;

            state = 1;
        }
        else if (state == 1)
        {
            if (ch == '{' || ch == ',' || ch == '}')
                return (size_t)-1;

            number_str.clear();
            number_str.push_back(ch);
            state = 2;
        }
        else if (state == 2)
        {
            bool num_finished = false;

            if (ch == ',')
            {
                if ((vectors.size() % 4) == 3)
                    return (size_t)-1;

                num_finished = true;
            }
            else if (ch == '}')
            {
                if ((vectors.size() % 4) != 3)
                    return (size_t)-1;

                num_finished = true;
            }

            if (num_finished)
            {
                float value;

                if (sscanf(number_str.c_str(), "%f", &value) != 1)
                    return (size_t)-1;

                vectors.push_back(value);

                if ((vectors.size() % 4) != 0)
                {
                    state = 1;
                }
                else
                {
                    state = 3;
                    count++;
                }
            }
            else
            {
                number_str.push_back(ch);
            }
        }
        else if (state == 3)
        {
            if (ch != ',')
                return (size_t)-1;

            state = 0;
        }
    }

    if (state != 0 && state != 3)
        return (size_t)-1;

    return count;
}

void Utils::TrimString(std::string & str, bool trim_left, bool trim_right)
{
	size_t pos = 0;
	
	if (trim_left)
	{
		if (str.length() == 0)
			return;
		
		for (size_t i = 0; i < str.length(); i++)
		{
			if (str[i] > ' ' || str[i] < 0)
			{
				pos = i;
				break;
			}
		}
		
		if (pos != 0)
		{
			str = str.substr(pos);
		}
		else if (str.front() >= 0 && str.front() <= ' ')
		{
			// String only contained empty chars
			str.clear();
			return;
		}
	}
	
	if (trim_right)
	{
		if (str.length() == 0)
			return;
		
		pos = str.length()-1;
		
        for (size_t i = str.length()-1; i != std::string::npos; i--)
		{
			if (str[i] > ' ' || str[i] < 0)
			{
				pos = i;
				break;
			}
		}
		
		if (pos != (str.length()-1))
		{
			str = str.substr(0, pos + 1);
		}
		else if (str.back() >= 0 && str.back() <= ' ')
		{
			// String only contained empty chars
			str.clear();
			return;
		}
	}
}

bool Utils::IsEmptyString(const std::string & str)
{
	for (char c : str)
	{
		if (c > ' ' || c < 0)
			return false;
	}
	
	return true;
}

std::string Utils::GetFileNameString(const std::string &path)
{
    size_t s1 = path.rfind('/');
    size_t s2 = path.rfind('\\');
    size_t pos = std::string::npos;

    if (s1 == std::string::npos)
    {
        if (s2 != std::string::npos)
            pos = s2;
    }
    else
    {
        if (s2 != std::string::npos)
            pos = (s1 > s2) ? s1 : s2;
        else
            pos = s1;
    }

    if (pos == std::string::npos)
        return path;

    return path.substr(pos+1);
}

std::string Utils::GetDirNameString(const std::string &path)
{
    size_t s1 = path.rfind('/');
    size_t s2 = path.rfind('\\');
    size_t pos = std::string::npos;

    if (s1 == std::string::npos)
    {
        if (s2 != std::string::npos)
            pos = s2;
    }
    else
    {
        if (s2 != std::string::npos)
            pos = (s1 > s2) ? s1 : s2;
        else
            pos = s1;
    }

    if (pos == std::string::npos)
        return path;

    return path.substr(0, pos);
}

std::string Utils::MakePathString(const std::string &comp1, const std::string &comp2)
{
    size_t len = comp1.length();

    if (len == 0)
        return NormalizePath(comp2);

    if (len > 0 && (comp1[len-1] == '/' || comp1[len-1] == '\\'))
        return NormalizePath(comp1+comp2);

    return NormalizePath(comp1 + '/' + comp2);
}

size_t Utils::GetMultipleStrings(const std::string & str_param, std::vector<std::string> & list, char separator, bool omit_empty_strings, bool do_trimr)
{
	size_t pos;
	std::string str = str_param;
	
	list.clear();

    if (omit_empty_strings)
    {
        if (str.length() == 0 || str == "NULL")
        {
            return 0;
        }
    }
	
    while ((pos = str.find(separator)) != std::string::npos)
	{
		std::string this_string = str.substr(0, pos);
		
        if (!IsEmptyString(this_string) || !omit_empty_strings)
		{
            if (do_trimr)
                Utils::TrimString(this_string);
            else
                Utils::TrimString(this_string, true, false);

            if (this_string == "NULL")
                this_string.clear();

			list.push_back(this_string);
		}
		
		str = str.substr(pos+1);
	}
	
	// Add the last string, 
    if (!IsEmptyString(str) || !omit_empty_strings)
	{
		Utils::TrimString(str);

        if (str == "NULL")
            str.clear();

		list.push_back(str);	
    }

    return list.size();
}

size_t Utils::GetMultipleStrings(const std::string & str_param, std::vector<std::string> & list, const std::string &separator, bool omit_empty_strings)
{
    size_t pos;
    std::string str = str_param;

    list.clear();

    if (omit_empty_strings)
    {
        if (str.length() == 0 || str == "NULL")
        {
            return 0;
        }
    }

    while ((pos = str.find(separator)) != std::string::npos)
    {
        std::string this_string = str.substr(0, pos);

        if (!IsEmptyString(this_string) || !omit_empty_strings)
        {
            Utils::TrimString(this_string);

            if (this_string == "NULL")
                this_string.clear();

            list.push_back(this_string);
        }

        str = str.substr(pos+separator.length());
    }

    // Add the last string,
    if (!IsEmptyString(str) || !omit_empty_strings)
    {
        Utils::TrimString(str);

        if (str == "NULL" && omit_empty_strings)
            str.clear();

        list.push_back(str);
    }

    return list.size();
}

size_t Utils::GetMultipleStringsSpaces(const std::string &str_param, std::vector<std::string> &list)
{
    std::string str = str_param;
    list.clear();

    bool in_string = false;
    std::string cur;

    for (const char ch : str)
    {
        if (ch <= ' ')
        {
            if (in_string)
            {
                in_string = false;
                list.push_back(cur);
                cur.clear();
            }

        }
        else
        {
            in_string = true;
            cur.push_back(ch);
        }
    }

    list.push_back(cur);
    return list.size();
}

std::string Utils::ToSingleString(const std::vector<std::string> &list, const std::string &separator, bool omit_empty_strings)
{
	std::string ret;    
	
    for (size_t i = 0; i < list.size(); i++)
	{
        const std::string &s = list[i];

        if (i != 0)
            ret += separator;

        if (s.length() != 0 || !omit_empty_strings)
        {
            ret += s;
        }
        else
        {
            ret += "NULL";
        }
	}

	return ret;
}

std::string Utils::ToSingleString(const std::vector<uint32_t> &list, bool hexadecimal)
{
	std::string ret;
	
	for (uint32_t u : list)
	{
		if (ret.length() != 0)
			ret += ", ";
		
		ret += UnsignedToString(u, hexadecimal);
	}
	
	return ret;
}

std::string Utils::ToSingleString(const std::vector<uint16_t> &list, bool hexadecimal)
{
	std::string ret;
	
	for (uint16_t u : list)
	{
		if (ret.length() != 0)
			ret += ", ";
		
		ret += UnsignedToString(u, hexadecimal);
	}
	
	return ret;
}

std::string Utils::ToSingleString(const std::vector<uint8_t> &list, bool hexadecimal)
{
	std::string ret;
	
	for (uint8_t u : list)
	{
		if (ret.length() != 0)
			ret += ", ";
		
		ret += UnsignedToString(u, hexadecimal);
	}
	
	return ret;
}

std::string Utils::ToSingleString(const std::vector<float> &list)
{
	std::string ret;
	
	for (float f: list)
	{
		if (ret.length() != 0)
			ret += ", ";
		
		ret += FloatToString(f);
	}
	
	return ret;
}

std::string Utils::ToLowerCase(const std::string & str)
{
	std::string ret = str;
	
	for (char &c : ret)
	{
		if (c >= 'A' && c <= 'Z')
		{
			c = c + ('a' - 'A');
		}
	}
	
	return ret;
}

std::string Utils::ToUpperCase(const std::string & str)
{
    std::string ret = str;

    for (char &c : ret)
    {
        if (c >= 'a' && c <= 'z')
        {
            c = c - ('a' - 'A');
        }
    }

    return ret;
}

bool Utils::IsAlphaNumeric(const std::string &str)
{
    for (char c: str)
    {
        bool ok = false;

        if (c >= 'A' && c <= 'Z')
        {
            ok = true;
        }
        else if (c >= 'a' && c <= 'z')
        {
            ok = true;
        }
        else if (c >= '0' && c <= '9')
        {
            ok = true;
        }

        if (!ok)
            return false;
    }

    return true;
}

bool Utils::HasOnlyDigits(const std::string &str)
{
    for (char c: str)
    {
        bool ok = false;

        if (c >= '0' && c <= '9')
        {
            ok = true;
        }

        if (!ok)
            return false;
    }

    return true;
}

bool Utils::BeginsWith(const std::string &str, const std::string &substr, bool case_sensitive)
{
    size_t len1 = str.length();
    size_t len2 = substr.length();

    if (len2 > len1)
        return false;

    if (case_sensitive)
        return (str.substr(0, len2) == substr);

    std::string lstr = ToLowerCase(str);
    std::string lsubstr = ToLowerCase(substr);

    return (lstr.substr(0, len2) == lsubstr);
}

bool Utils::EndsWith(const std::string &str, const std::string &substr, bool case_sensitive)
{
	size_t len1 = str.length();
	size_t len2 = substr.length();
	size_t pos;
	
	if (len2 > len1)
		return false;
	
	if (case_sensitive)
	{
		pos = str.rfind(substr);
	}
	else
	{
		std::string lstr = ToLowerCase(str);
		std::string lsubstr = ToLowerCase(substr);
		
		pos = lstr.rfind(lsubstr);
	}
	
	return (pos == (len1-len2));
}

size_t Utils::CharCount(const std::string &str, char ch)
{
    size_t ret = 0;

    for (const char c : str)
    {
        if (c == ch)
            ret++;
    }

    return ret;
}

std::string Utils::GUID2String(const uint8_t *guid)
{
	std::string ret;
	
	for (int i = 0; i < 16; i++)
	{
		uint8_t h, l;
		char ch;
		
		h = guid[i] >> 4;
		l = guid[i] & 0xF;
		
        if (h <= 9)
			ch = h + '0';
		else
			ch = (h-10) + 'a';
		
		ret += ch;
		
        if (l <= 9)
			ch = l + '0';
		else
			ch = (l-10) + 'a';
		
		ret += ch;
		
		if (i == 3 || i == 5 || i == 7 || i == 9)
			ret += '-';
	}
	
	return ret;
}

bool Utils::String2GUID(uint8_t *guid, const std::string &str)
{
    if (str.length() != 36)
        return false;

    bool high_flag = true;
    uint8_t h = 0;

    for (size_t i = 0, j = 0; i < str.length(); i++)
    {
        if (i == 8 || i == 13 || i == 18 || i == 23)
        {
            if (str[i] != '-')
                return false;
        }
        else
        {
            uint8_t d;

            if (str[i] >= '0' && str[i] <= '9')
            {
                d = str[i] - '0';
            }
            else if (str[i] >= 'a' && str[i] <= 'f')
            {
                d = str[i] - 'a' + 10;
            }
            else if (str[i] >= 'A' && str[i] <= 'F')
            {
                d = str[i] - 'A' + 10;
            }
            else
            {
                return false;
            }

            if (high_flag)
            {
                h = d << 4;
            }
            else
            {
                guid[j++] = h | d;
            }

            high_flag = !high_flag;
        }
    }

    return true;
}

std::string Utils::ColorToString(uint32_t color, bool alpha)
{
    char str[9];

    if (alpha)
    {
        snprintf(str, sizeof(str), "#%08X", color);
    }
    else
    {
        snprintf(str, sizeof(str), "#%06X", color&0xFFFFFF);
    }

    return std::string(str);
}

std::string Utils::BinaryString(const uint8_t *buf, size_t size, bool separate, bool prefix)
{
    std::string ret;

    for (size_t i = 0; i < size; i++)
    {
        char tmp[4];

        snprintf(tmp, sizeof(tmp), "%02X", buf[i]);

        if (separate && i != 0)
            ret += " ";

        if (prefix)
            ret += "0x";

        ret += tmp;
    }

    return ret;
}

bool Utils::BinaryStringToBuf(const std::string &str, uint8_t *buf, size_t size)
{
    size_t pos = 0;
    bool high = true;

    uint8_t hnibble;

    for (size_t i = 0; i < str.length(); i++)
    {
        char ch = tolower(str[i]);
        uint8_t out;

        if (ch <= ' ')
            continue;

        if (ch == '0')
        {
            if (i != (str.length()-1) && str[i+1] == 'x')
            {
                i++;
                continue;
            }
        }

        if (ch >= '0' && ch <= '9')
        {
            out = ch - '0';
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            out = (ch - 'a') + 0xa;
        }
        else
        {
            return false;
        }

        if (high)
        {
            hnibble = out;
        }
        else
        {
            if (pos >= size)
                return false;

            buf[pos++] = (hnibble << 4) | out;
        }
    }

    return (pos == size);
}

std::string Utils::NormalizePath(const std::string &path)
{
#ifdef UTILS_UTF8
    return Ucs2ToUtf8(NormalizePath(Utf8ToUcs2(path)));
#else
	std::string new_path = path;
	
	for (char &c : new_path)
	{
		if (c == '\\')
			c = '/';
	}

    bool last_was_slash = false;

    for (size_t i = 0; i < new_path.length()-1; i++)
    {
        char ch = new_path[i];

        if (ch == '/')
        {
            if (last_was_slash)
            {
                new_path.erase(i, 1);
                i--;
            }
            else
            {
                last_was_slash = true;
            }
        }
        else
        {
            last_was_slash = false;
        }
    }
	
	return new_path;
#endif
}

std::u16string Utils::NormalizePath(const std::u16string &path)
{
    std::u16string new_path = path;

    for (char16_t &c : new_path)
    {
        if (c == '\\')
            c = '/';
    }

    bool last_was_slash = false;

    for (size_t i = 0; i < new_path.length()-1; i++)
    {
        char16_t ch = new_path[i];

        if (ch == '/')
        {
            if (last_was_slash)
            {
                new_path.erase(i, 1);
                i--;
            }
            else
            {
                last_was_slash = true;
            }
        }
        else
        {
            last_was_slash = false;
        }
    }

    return new_path;
}

std::string Utils::WindowsPath(const std::string &path)
{
#ifdef UTILS_UTF8
    return Ucs2ToUtf8(WindowsPath(Utf8ToUcs2(path)));
#else
    std::string new_path = path;

    for (char &c : new_path)
    {
        if (c == '/')
            c = '\\';
    }

    return new_path;
#endif
}

std::u16string Utils::WindowsPath(const std::u16string &path)
{
    std::u16string new_path = path;

    for (char16_t &c : new_path)
    {
        if (c == '/')
            c = '\\';
    }

    return new_path;
}

std::string Utils::SamePath(const std::string &file_path, const std::string &file_name)
{
#ifdef UTILS_UTF8
    return Ucs2ToUtf8(SamePath(Utf8ToUcs2(file_path), Utf8ToUcs2(file_name)));
#else
    std::string fp = NormalizePath(file_path);
    size_t rs = fp.rfind('/');

    if (rs == std::string::npos)
        return file_name;

    return fp.substr(0, rs+1) + file_name;
#endif
}

std::u16string Utils::SamePath(const std::u16string &file_path, const std::u16string &file_name)
{
    std::u16string fp = NormalizePath(file_path);
    size_t rs = fp.rfind('/');

    if (rs == std::string::npos)
        return file_name;

    return fp.substr(0, rs+1) + file_name;
}

std::string Utils::RandomString(size_t len)
{
    std::string str;
    str.resize(len);

    for (char &c : str)
    {
        if (RandomInt(0, 0x10000) & 1)
        {
            c = (char)Utils::RandomInt('A', 'Z');
        }
        else
        {
            c = (char)Utils::RandomInt('a', 'z');
        }
    }

    return str;
}

size_t Utils::Replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = 0;
    size_t num = 0;

    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        num++;
    }

    return num;
}

uint32_t Utils::GetUnsigned(const std::string &str, uint32_t default_value)
{
	uint32_t ret = 0;
    size_t len = str.length();
	
	if (len == 0)
	{
        //DPRINTF("WARNING: length of integer string = 0 (param \"%s\"), setting value to 0.\n", param_name.c_str());
        return default_value;
	}
	
	if (str[0] == '0')
	{
		if (len == 1)
			return 0;
		
		if (str[1] != 'x')
		{
            /*DPRINTF("WARNING: Integer format error on \"%s\". "
							"Value must be decimal values without any 0 on the left, or hexadecimal values with 0x prefix. "
							"Octal values not allowed (offending_string = %s). "
                            "Setting value to 0.\n", param_name.c_str(), str.c_str());*/
							
            for (size_t i = 1; i < str.length(); i++)
            {
                if (str[i] != '0')
                {
                    if (sscanf(str.c_str()+i, "%u", &ret) != 1)
                        return default_value;

                    return ret;
                }
            }

            return default_value;
		}
		
		if (len == 2)
		{
            //DPRINTF("WARNING: nothing on the right of hexadecimal prefix (on param \"%s\"). Setting value to 0.\n", param_name.c_str());
            return default_value;
		}
		
        if (sscanf(str.c_str()+2, "%x", &ret) != 1)
		{
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
		}
	}
	else
	{
        if (sscanf(str.c_str(), "%u", &ret) != 1)
		{
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
		}
	}
	
	return ret;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
#endif

uint64_t Utils::GetUnsigned64(const std::string &str, uint64_t default_value)
{
    uint64_t ret = 0;
    size_t len = str.length();

    if (len == 0)
    {
        //DPRINTF("WARNING: length of integer string = 0 (param \"%s\"), setting value to 0.\n", param_name.c_str());
        return default_value;
    }

    if (str[0] == '0')
    {
        if (len == 1)
            return 0;

        if (str[1] != 'x')
        {
            /*DPRINTF("WARNING: Integer format error on \"%s\". "
                            "Value must be decimal values without any 0 on the left, or hexadecimal values with 0x prefix. "
                            "Octal values not allowed (offending_string = %s). "
                            "Setting value to 0.\n", param_name.c_str(), str.c_str());*/


            for (size_t i = 1; i < str.length(); i++)
            {
                if (str[i] != '0')
                {
                    if (sscanf(str.c_str()+i, "%I64u", &ret) != 1)
                        return default_value;

                    return ret;
                }
            }

            return default_value;
        }

        if (len == 2)
        {
            //DPRINTF("WARNING: nothing on the right of hexadecimal prefix (on param \"%s\"). Setting value to 0.\n", param_name.c_str());
            return default_value;
        }

        if (sscanf(str.c_str()+2, "%I64x", &ret) != 1)
        {
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
        }
    }
    else
    {
        if (sscanf(str.c_str(), "%I64u", &ret) != 1)
        {
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
        }
    }

    return ret;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

int32_t Utils::GetSigned(const std::string &str, int32_t default_value)
{
    int32_t ret = 0;
    size_t len = str.length();

    if (len == 0)
    {
        //DPRINTF("WARNING: length of integer string = 0 (param \"%s\"), setting value to 0.\n", param_name.c_str());
        return default_value;
    }

    if (str[0] == '0')
    {
        if (len == 1)
            return 0;

        if (str[1] != 'x')
        {
            /*DPRINTF("WARNING: Integer format error on \"%s\". "
                            "Value must be decimal values without any 0 on the left, or hexadecimal values with 0x prefix. "
                            "Octal values not allowed (offending_string = %s). "
                            "Setting value to 0.\n", param_name.c_str(), str.c_str());*/

            return default_value;
        }

        if (len == 2)
        {
            //DPRINTF("WARNING: nothing on the right of hexadecimal prefix (on param \"%s\"). Setting value to 0.\n", param_name.c_str());
            return default_value;
        }

        if (sscanf(str.c_str()+2, "%x", &ret) != 1)
        {
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
        }
    }
    else
    {
        if (sscanf(str.c_str(), "%d", &ret) != 1)
        {
            //DPRINTF("sscanf failed on param \"%s\", offending string = \"%s\"\n. Setting value to 0.", param_name.c_str(), str.c_str());
            return default_value;
        }
    }

    return ret;
}

bool Utils::GetBoolean(const std::string &str, bool default_value)
{
    std::string boolean_str = Utils::ToLowerCase(str);

    if (boolean_str == "false" || boolean_str == "0")
        return false;

    if (boolean_str == "true" || GetUnsigned(str) > 0)
        return true;

    return default_value;
}

float Utils::GetFloat(const std::string &str, float default_value)
{
    float value;

    if (sscanf(str.c_str(), "%f", &value) != 1)
        return default_value;

    return value;
}

uint32_t Utils::GetShortVersion(uint32_t version)
{
    uint32_t result = 0;
    uint8_t *b = (uint8_t *)&version;

    uint32_t mult = 1;

    for (int i = 0; i < 4; i++)
    {
        if (b[i] >= '0' && b[i] <= '9')
        {
            result += (b[i] - '0') * mult;
            mult = mult*10;
        }
        else
            return 0xFFFFFFFF;
    }

    return result;
}

uint32_t Utils::GetLongVersion(uint32_t version)
{
    uint32_t result;

    if (version >= 10000)
        return 0xFFFFFFFF;

    uint32_t t = version / 1000;
    uint32_t h = (version / 100) % 10;
    uint32_t d = (version / 10) % 10;
    uint32_t u = version % 10;

    result = u + '0';
    result |= (d + '0') << 8;
    result |= (h + '0') << 16;
    result |= (t + '0') << 24;

    return result;
}

TiXmlElement *Utils::FindRoot(TiXmlHandle *handle, const std::string &root_name)
{
    for (TiXmlElement *elem = handle->FirstChildElement().Element(); elem != nullptr; elem = elem->NextSiblingElement())
    {
        std::string name = elem->ValueStr();

        if (name == root_name)
        {
            return elem;
        }
    }

    return nullptr;
}

size_t Utils::GetElemCount(const TiXmlElement *root, const char *name, const TiXmlElement **first)
{
    size_t count = 0;

    for (const TiXmlElement *elem = root->FirstChildElement(); elem; elem = elem->NextSiblingElement())
    {
        if (elem->ValueStr() == name)
        {
            if (first && count == 0)
            {
                *first = elem;
            }

            count++;
        }
    }

    return count;
}

void Utils::WriteParamString(TiXmlElement *root, const char *name, const std::string & value)
{
	TiXmlElement *param = new TiXmlElement(name);	
	param->SetAttribute("value", value);	
	root->LinkEndChild(param);
}

void Utils::WriteParamMultipleStrings(TiXmlElement *root, const char *name, const std::vector<std::string> & values)
{
	TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", ToSingleString(values));
	root->LinkEndChild(param);
}

void Utils::WriteParamUnsigned(TiXmlElement *root, const char *name, uint64_t value, bool hexadecimal)
{
	TiXmlElement *param = new TiXmlElement(name);	
	param->SetAttribute("value", UnsignedToString(value, hexadecimal));	
	root->LinkEndChild(param);
}

void Utils::WriteParamSigned(TiXmlElement *root, const char *name, int64_t value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", SignedToString(value));
    root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint32_t> &values, bool hexadecimal)
{
	TiXmlElement *param = new TiXmlElement(name);
	param->SetAttribute("value", ToSingleString(values, hexadecimal));
	root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint16_t> &values, bool hexadecimal)
{
	TiXmlElement *param = new TiXmlElement(name);
	param->SetAttribute("value", ToSingleString(values, hexadecimal));
	root->LinkEndChild(param);
}

void Utils::WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint8_t> &values, bool hexadecimal)
{
	TiXmlElement *param = new TiXmlElement(name);
	param->SetAttribute("value", ToSingleString(values, hexadecimal));
	root->LinkEndChild(param);
}

void Utils::WriteParamFloat(TiXmlElement *root, const char *name, float value)
{
	TiXmlElement *param = new TiXmlElement(name);	
	param->SetAttribute("value", FloatToString(value));	
	root->LinkEndChild(param);
}

void Utils::WriteParamMultipleFloats(TiXmlElement *root, const char *name, const std::vector<float> &values)
{
	TiXmlElement *param = new TiXmlElement(name);
	param->SetAttribute("value", ToSingleString(values));
	root->LinkEndChild(param);
}

void Utils::WriteParamGUID(TiXmlElement *root, const char *name, const uint8_t *value)
{
	std::string str = Utils::GUID2String(value);
	WriteParamString(root, name, str);
}

void Utils::WriteParamBlob(TiXmlElement *root, const char *name, const uint8_t *value, size_t size)
{
	TiXmlElement *param = new TiXmlElement(name);		
	TiXmlText *base64= new TiXmlText(Base64Encode(value, size, true));
	
	base64->SetCDATA(true);	
	param->LinkEndChild(base64);	
	root->LinkEndChild(param);
}

void Utils::WriteParamBoolean(TiXmlElement *root, const char *name, bool value)
{
    TiXmlElement *param = new TiXmlElement(name);
    param->SetAttribute("value", (value) ? "true" : "false");
    root->LinkEndChild(param);
}

bool Utils::ReadAttrString(const TiXmlElement *root, const char *name, std::string & value)
{
	if (root->QueryStringAttribute(name, &value) != TIXML_SUCCESS)
		return false;
	
	return true;
}

bool Utils::ReadAttrMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, char separator, bool omit_empty)
{
    std::string str;

    if (!ReadAttrString(root, name, str))
        return false;

    GetMultipleStrings(str, values, separator, omit_empty);
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint32_t *value)
{
	std::string str;
	
	if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
		return false;
	
    *value = GetUnsigned(str.c_str());
	return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint64_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    *value = GetUnsigned64(str.c_str());
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    uint32_t temp = GetUnsigned(str.c_str());
    if (temp > 0xFFFF)
        return false;

    *value = (uint16_t)temp;
    return true;
}

bool Utils::ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    std::string str;

    if (root->QueryStringAttribute(name, &str) != TIXML_SUCCESS)
        return false;

    uint32_t temp = GetUnsigned(str.c_str());
    if (temp > 0xFF)
        return false;

    *value = (uint8_t)temp;
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int32_t *value)
{
    std::string str;

    if (!ReadAttrString(root, name, str))
        return false;

    *value = GetSigned(str.c_str());
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int16_t *value)
{
    int32_t value32;

    if (!ReadAttrSigned(root, name, &value32))
        return false;

    if (value32 >= 32768 || value32 < -32768)
        return false;

    *value = (int16_t)value32;
    return true;
}

bool Utils::ReadAttrSigned(const TiXmlElement *root,  const char *name, int8_t *value)
{
    int32_t value32;

    if (!ReadAttrSigned(root, name, &value32))
        return false;

    if (value32 >= 128 || value32 < -128)
        return false;

    *value = (int8_t)value32;
    return true;
}

bool Utils::ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        uint32_t value = GetUnsigned(s.c_str());

        if (value > 0xFF)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    std::vector<uint8_t> vec;

    if (!ReadAttrMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint8_t));
    return true;
}

bool Utils::ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, std::vector<int32_t> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        values.push_back(GetSigned(s.c_str()));
    }

    return true;
}

bool Utils::ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, int32_t *values, size_t count)
{
    std::vector<int32_t> vec;

    if (!ReadAttrMultipleSigned(root, name, vec))
        return false;

    if (vec.size() != count)
         return false;

    memcpy(values, vec.data(), count*sizeof(int32_t));
    return true;
}

bool Utils::ReadAttrFloat(const TiXmlElement *root, const char *name, float *value)
{
    std::string str;

    if (root->QueryFloatAttribute(name, value) != TIXML_SUCCESS)
        return false;

    return true;
}

bool Utils::ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
    std::vector<std::string> values_str;

    values.clear();

    if (!ReadAttrMultipleStrings(root, name, values_str))
        return false;

    values.reserve(values_str.size());

    for (const std::string &s : values_str)
    {
        float value;

        if (sscanf(s.c_str(), "%f", &value) != 1)
            return false;

        values.push_back(value);
    }

    return true;
}

bool Utils::ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    std::vector<float> vec;

    if (!ReadAttrMultipleFloats(root, name, vec))
        return false;

    if (vec.size() != count)
         return false;

    memcpy(values, vec.data(), count*sizeof(float));
    return true;
}

bool Utils::ReadAttrBoolean(const TiXmlElement *root, const char *name, bool *value)
{
    std::string str;
    if (!ReadAttrString(root, name, str))
        return false;

    str = Utils::ToLowerCase(str);
    if (str == "true")
        *value = true;
    else if (str == "false")
        *value = false;
    else
    {
         *value = (GetUnsigned64(str) != 0);
    }

    return true;
}

bool Utils::ReadParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret)
{
	bool found = false;
	
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != nullptr; elem = elem->NextSiblingElement())
	{
		std::string str;
		
		str = elem->ValueStr();
		
		if (str == name)
		{
			if (elem->QueryStringAttribute("value", &value) != TIXML_SUCCESS)
				return false;
			
			found = true;
            if (ret)
                *ret = elem;

			break;
		}
	}
	
	return found;
}

bool Utils::ReadParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> & values, const TiXmlElement **ret)
{
	std::string str;
		
    if (!ReadParamString(root, name, str, ret))
		return false;
	
    GetMultipleStrings(str, values);
	return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value)
{
	std::string str;
	
	if (!ReadParamString(root, name, str))
		return false;
	
    *value = GetUnsigned(str.c_str());
	return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    *value = GetUnsigned64(str.c_str());
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    uint32_t temp;

    if (!ReadParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFFFF)
        return false;

    *value = (uint16_t)temp;
    return true;
}

bool Utils::ReadParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    uint32_t temp;

    if (!ReadParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFF)
        return false;

    *value = (uint8_t)temp;
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values)
{
	std::vector<std::string> values_str;
	
	values.clear();
	
	if (!ReadParamMultipleStrings(root, name, values_str))
		return false;
	
	values.reserve(values_str.size());
	
	for (const std::string &s : values_str)
	{
        values.push_back(GetUnsigned(s.c_str()));
	}
	
	return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values)
{
	std::vector<std::string> values_str;
	
	values.clear();
	
	if (!ReadParamMultipleStrings(root, name, values_str))
		return false;
	
	values.reserve(values_str.size());
	
	for (const std::string &s : values_str)
	{
        uint32_t value = GetUnsigned(s.c_str());
		
		if (value > 0xFFFF)
			return false;
		
		values.push_back(value);	
	}
	
	return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
	std::vector<std::string> values_str;
	
	values.clear();
	
	if (!ReadParamMultipleStrings(root, name, values_str))
		return false;
	
	values.reserve(values_str.size());
	
	for (const std::string &s : values_str)
	{
        uint32_t value = GetUnsigned(s.c_str());
		
		if (value > 0xFF)
			return false;
		
		values.push_back(value);	
	}
	
	return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count)
{
    std::vector<uint32_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint32_t));
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count)
{
    std::vector<uint16_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint16_t));
    return true;
}

bool Utils::ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    std::vector<uint8_t> vec;

    if (!ReadParamMultipleUnsigned(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(uint8_t));
    return true;
}

bool Utils::ReadParamSigned(const TiXmlElement *root, const char *name, int32_t *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    *value = GetSigned(str.c_str());
    return true;
}

bool Utils::ReadParamFloat(const TiXmlElement *root, const char *name, float *value)
{
	bool found = false;
	
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string str;
		
		str = elem->ValueStr();
		
		if (str == name)
		{
			if (elem->QueryFloatAttribute("value", value) != TIXML_SUCCESS)
				return false;
			
			found = true;
			break;
		}
	}
	
	return found;
}

bool Utils::ReadParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
	std::vector<std::string> values_str;
	
	values.clear();
	
	if (!ReadParamMultipleStrings(root, name, values_str))
		return false;
	
	values.reserve(values_str.size());
	
	for (const std::string &s : values_str)
	{
		float value;
		
		if (sscanf(s.c_str(), "%f", &value) != 1)
			return false;
		
		values.push_back(value);
	}
	
	return true;
}

bool Utils::ReadParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    std::vector<float> vec;

    if (!ReadParamMultipleFloats(root, name, vec))
        return false;

    if (vec.size() != count)
        return false;

    memcpy(values, vec.data(), count*sizeof(float));
    return true;
}

bool Utils::ReadParamGUID(const TiXmlElement *root, const char *name, uint8_t *value)
{
	std::string guid;
	
	if (!ReadParamString(root, name, guid))
		return false;
	
	TrimString(guid);
	
	if (guid.length() != 36)
		return false;
	
	bool high_flag = true;
	uint8_t h = 0;
	
	for (size_t i = 0, j = 0; i < guid.length(); i++)
	{
		if (i == 8 || i == 13 || i == 18 || i == 23)
		{
			if (guid[i] != '-')
				return false;
		}
		else
		{
			uint8_t d;
			
			if (guid[i] >= '0' && guid[i] <= '9')
			{
				d = guid[i] - '0';
			}
			else if (guid[i] >= 'a' && guid[i] <= 'f')
			{
				d = guid[i] - 'a' + 10;
			}
			else if (guid[i] >= 'A' && guid[i] <= 'F')
			{
				d = guid[i] - 'A' + 10;
			}
			else
			{
				return false;
			}
			
			if (high_flag)
			{
				h = d << 4;
			}
			else
			{
				value[j++] = h | d;				
			}
			
			high_flag = !high_flag;			
		}
	}	
	
	return true;
}

uint8_t *Utils::ReadParamBlob(const TiXmlElement *root, const char *name, size_t *psize)
{
    for (const TiXmlElement *elem = root->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
	{
		std::string str;
		
		str = elem->ValueStr();
		
		if (str == name)
		{
			std::string base64_data = elem->GetText();
			return Base64Decode(base64_data, psize);
		}
	}
	
	return nullptr;
}

bool Utils::ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
	if (ReadParamUnsigned(root, name1, value))
		return true;
	
	if (ReadParamUnsigned(root, name2, value))
		return true;
	
	if (name3 && ReadParamUnsigned(root, name3, value))
		return true;
	
	if (name4 && ReadParamUnsigned(root, name4, value))
		return true;
	
    return (name5 && ReadParamUnsigned(root, name5, value));
}

bool Utils::ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (ReadParamUnsigned(root, name1, value))
        return true;

    if (ReadParamUnsigned(root, name2, value))
        return true;

    if (name3 && ReadParamUnsigned(root, name3, value))
        return true;

    if (name4 && ReadParamUnsigned(root, name4, value))
        return true;

    return (name5 && ReadParamUnsigned(root, name5, value));
}

bool Utils::ReadParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (ReadParamFloat(root, name1, value))
        return true;

    if (ReadParamFloat(root, name2, value))
        return true;

    if (name3 && ReadParamFloat(root, name3, value))
        return true;

    if (name4 && ReadParamFloat(root, name4, value))
        return true;

    return (name5 && ReadParamFloat(root, name5, value));
}

bool Utils::ReadParamBoolean(const TiXmlElement *root, const char *name, bool *value)
{
    std::string str;

    if (!ReadParamString(root, name, str))
        return false;

    str = Utils::ToLowerCase(str);
    if (str == "true")
        *value = true;
    else if (str == "false")
        *value = false;
    else
    {
         *value = (GetUnsigned64(str.c_str()) != 0);
    }

    return true;
}

bool Utils::GetParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret)
{
    if (!ReadParamString(root, name, value, ret))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> & values, const TiXmlElement **ret)
{
    if (!ReadParamMultipleStrings(root, name, values, ret))
	{
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
		return false;
	}
	
	return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value)
{
    if (!ReadParamUnsigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value)
{
    if (!ReadParamUnsigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value)
{
    uint32_t temp;

    if (!GetParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFFFF)
    {
        DPRINTF("Parameter \"%s\" is out of 16 bits limit. (object at line %d)\n", name, root->Row());
        return false;
    }

    *value = (uint16_t)temp;
    return true;
}

bool Utils::GetParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value)
{
    uint32_t temp;

    if (!GetParamUnsigned(root, name, &temp))
        return false;

    if (temp > 0xFF)
    {
        DPRINTF("Parameter \"%s\" is out of 8 bits limit. (object at line %d)\n", name, root->Row());
        return false;
    }

    *value = (uint8_t)temp;
    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values)
{
	if (!ReadParamMultipleUnsigned(root, name, values))
	{
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
	}
	
	return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values)
{
	if (!ReadParamMultipleUnsigned(root, name, values))
	{
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
	}
	
	return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values)
{
	if (!ReadParamMultipleUnsigned(root, name, values))
	{
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
	}
	
	return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count)
{
    if (!ReadParamMultipleUnsigned(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamSigned(const TiXmlElement *root, const char *name, int32_t *value)
{
    if (!ReadParamSigned(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamFloat(const TiXmlElement *root, const char *name, float *value)
{
    if (!ReadParamFloat(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values)
{
	if (!ReadParamMultipleFloats(root, name, values))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;	
}

bool Utils::GetParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count)
{
    if (!ReadParamMultipleFloats(root, name, values, count))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamGUID(const TiXmlElement *root, const char *name, uint8_t *value)
{
    if (!ReadParamGUID(root, name, value))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
        return false;
    }

    return true;
}

uint8_t *Utils::GetParamBlob(const TiXmlElement *root, const char *name, size_t *psize)
{
	uint8_t *ret = ReadParamBlob(root, name, psize);
	if (!ret)
	{
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name, root->Row());
	}
	
	return ret;
}

bool Utils::GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamUnsignedWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamUnsignedWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

bool Utils::GetParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3, const char *name4, const char *name5)
{
    if (!ReadParamFloatWithMultipleNames(root, value, name1, name2, name3, name4, name5))
    {
        DPRINTF("Cannot read parameter \"%s\" (object at line %d)\n", name1, root->Row());
        return false;
    }

    return true;
}

void Utils::WriteComment(TiXmlElement *root, const std::string & comment)
{
	TiXmlComment *tx_comment = new TiXmlComment();
	
	tx_comment->SetValue(comment);
	root->LinkEndChild(tx_comment);
}

std::string Utils::SSSSModelFileName(uint32_t cms_entry, uint32_t cms_model_spec_idx, const char *ext)
{
    char filename[48];

    snprintf(filename, sizeof(filename), "0x%x_%d%s", cms_entry, cms_model_spec_idx, ext);
    return std::string(filename);
}

#ifdef _WIN32

#include <Wincrypt.h>

uint64_t Utils::RandomInt(uint64_t min, uint64_t max)
{
    HCRYPTPROV hProv;
    int64_t random;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        DPRINTF("%s: CryptAquireContext error\n", FUNCNAME);
        exit(-1);
    }

    if (!CryptGenRandom(hProv, sizeof(uint64_t), (BYTE *)&random))
    {
        DPRINTF("%s: CryptGenRandom error\n", FUNCNAME);
        exit(-1);
    }

    CryptReleaseContext(hProv, 0);

    uint64_t rnd =  random % (max - min);
    return rnd + min;
}

float Utils::RandomProbability()
{
    HCRYPTPROV hProv;
    uint16_t random;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        DPRINTF("%s: CryptAquireContext error\n", FUNCNAME);
        exit(-1);
    }

    if (!CryptGenRandom(hProv, sizeof(random), (BYTE *)&random))
    {
        DPRINTF("%s: CryptGenRandom error\n", FUNCNAME);
        exit(-1);
    }

    CryptReleaseContext(hProv, 0);

    return (float)random / 65535.0f;
}

void Utils::GetRandomData(void *buf, size_t len)
{
    HCRYPTPROV hProv;

    if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        DPRINTF("%s: CryptAquireContext error\n", FUNCNAME);
        exit(-1);
    }

    if (!CryptGenRandom(hProv, (DWORD)len, (BYTE *)buf))
    {
        DPRINTF("%s: CryptGenRandom error\n", FUNCNAME);
        exit(-1);
    }

    CryptReleaseContext(hProv, 0);
}

#endif

std::string Utils::GetRandomString(size_t len)
{
    static bool started = false;
    std::string s;

    if (!started)
    {
         std::srand((unsigned int)std::time(nullptr));
         started = true;
    }

    for (unsigned int i = 0; i < len; i++)
    {
        char ch;

        if (std::rand() & 1)
        {
            ch = 'A';
        }
        else
        {
            ch = 'a';
        }

        ch += (std::rand() % 26);
        s += ch;
    }

    return s;
}

// Don't need this anymore
/*static int utf8_to_ucs2(const unsigned char * input, const unsigned char ** end_ptr)
{
    *end_ptr = input;
    if (input[0] == 0) {
        return -1;
    }
    if (input[0] < 0x80) {
	// One byte (ASCII) case. 
        * end_ptr = input + 1;
        return input[0];
    }
    if ((input[0] & 0xE0) == 0xE0) {
	// Three byte case. 
        if (input[1] < 0x80 || input[1] > 0xBF ||
	    input[2] < 0x80 || input[2] > 0xBF) {
            return -1;
	}
        * end_ptr = input + 3;
        return
            (input[0] & 0x0F)<<12 |
            (input[1] & 0x3F)<<6  |
            (input[2] & 0x3F);
    }
    if ((input[0] & 0xC0) == 0xC0) {
	// Two byte case. 
        if (input[1] < 0x80 || input[1] > 0xBF) {
            return -1;
	}
        * end_ptr = input + 2;
        return
            (input[0] & 0x1F)<<6  |
            (input[1] & 0x3F);
    }
    return -1;
}*/

std::u16string Utils::Utf8ToUcs2(const std::string &utf8)
{
	// New GCC version don't like the dirty implementation 
	// It compiled, but caused freeze, there is probably a bug in the *c_utf8 != 0 check, but it doesn't worth to keep this implementation any more.
	
	/*if (utf8.length() == 0)
    {
        return std::u16string(); // Empty string
    }	
	
	const char *c_utf8 = utf8.c_str();
	std::u16string ucs2;
	int ret;
	
	while (*c_utf8 != 0 && (ret = utf8_to_ucs2((const unsigned char *)c_utf8, (const unsigned char **)&c_utf8)) != -1)
	{
		if (ret < 0x10000)
			ucs2.push_back((char16_t)ret);
	}
	
	return ucs2;*/
	
	// New implementation:
	std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.from_bytes(utf8);
}

static int ucs2_to_utf8 (int ucs2, unsigned char *utf8)
{
    if (ucs2 < 0x80) {
        utf8[0] = (unsigned char)ucs2;
        utf8[1] = '\0';
        return 1;
    }
    if (ucs2 >= 0x80  && ucs2 < 0x800) {
        utf8[0] = (ucs2 >> 6)   | 0xC0;
        utf8[1] = (ucs2 & 0x3F) | 0x80;
        utf8[2] = '\0';
        return 2;
    }
    if (ucs2 >= 0x800 && ucs2 < 0xFFFF) {
	if (ucs2 >= 0xD800 && ucs2 <= 0xDFFF) {
	    /* Ill-formed. */
	    return -1;
	}
        utf8[0] = ((ucs2 >> 12)       ) | 0xE0;
        utf8[1] = ((ucs2 >> 6 ) & 0x3F) | 0x80;
        utf8[2] = ((ucs2      ) & 0x3F) | 0x80;
        utf8[3] = '\0';
        return 3;
    }
    if (ucs2 >= 0x10000 && ucs2 < 0x10FFFF) {
	/* http://tidy.sourceforge.net/cgi-bin/lxr/source/src/utf8.c#L380 */
	utf8[0] = 0xF0 | (ucs2 >> 18);
	utf8[1] = 0x80 | ((ucs2 >> 12) & 0x3F);
	utf8[2] = 0x80 | ((ucs2 >> 6) & 0x3F);
	utf8[3] = 0x80 | ((ucs2 & 0x3F));
        utf8[4] = '\0';
        return 4;
    }
    return -1;;
}

std::string Utils::Ucs2ToUtf8(const std::u16string &ucs2)
{
	if (ucs2.length() == 0)
		return "";
	
	std::string utf8;
	
	for (char16_t c : ucs2)
	{
		char c_utf8[5];
		
		if (ucs2_to_utf8(c, (unsigned char *)c_utf8) < 0)
			break;
		
		utf8 += c_utf8;		
	}
	
	return utf8;
}

static const std::string base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string Utils::Base64Encode(const uint8_t *buf, size_t size, bool add_new_line)
{
    std::string ret;
    size_t written = 0;

    if (add_new_line)
        ret += '\n';

    for (size_t i = 0; i < size; i += 3)
    {
        unsigned int st;

        st = (buf[i] & 0xFC) >> 2;
        ret += base64_table[st];
        written++;

        st = (buf[i] & 3) << 4;

        if (i+1 < size)
        {
            st |= (buf[i+1]&0xF0) >> 4;
            ret += base64_table[st];
            written++;

            st = (buf[i+1]&0xF) << 2;

            if (i+2 < size)
            {
                st |= (buf[i+2]&0xC0) >> 6;
                ret += base64_table[st];

                st = buf[i+2]&0x3F;
                ret += base64_table[st];
                written += 2;
            }
            else
            {
                ret += base64_table[st];
                ret += '=';
                written += 2;
            }
        }
        else
        {
            ret += base64_table[st];
            ret += "==";
            written += 3;
        }

        if (((written % 76) == 0) && (i+3 < size))
            ret += '\n';
    }

    if (add_new_line && ret[ret.length()-1] != '\n')
        ret += '\n';

    return ret;
}

uint8_t *Utils::Base64Decode(const std::string &data, size_t *ret_size)
{
    char empty_chars[33];

    for (char c = 1; c < 33; c++)
       empty_chars[c-1] = c;

    empty_chars[32] = 0;

    size_t empty_count = 0;
    size_t pos = data.find_first_of(empty_chars);

    while (pos != std::string::npos)
    {
        empty_count++;
        pos = data.find_first_of(empty_chars, pos+1);
    }

    size_t data_len = data.length() - empty_count;

    if ((data_len % 4) != 0)
        return nullptr;

    *ret_size = (data_len*3) / 4;

    if (data.rfind('=') != std::string::npos)
    {
        size_t pos = data.find_last_not_of(empty_chars);

        if (pos == std::string::npos)
            *ret_size = *ret_size - data.length();
        else
            *ret_size = *ret_size - (pos+1 - data.find('='));
    }

    uint8_t *buf = new uint8_t[*ret_size];

    size_t st[4];
    size_t n = 0;
    size_t i = 0;

    while (i < data.length())
    {        
        if (data[i] <= ' ')
        {
            i++;
            continue;
        }

        for (size_t j = 0; j < 4; j++)
        {
            st[j] = base64_table.find(data[i+j]);
            if (st[j] == std::string::npos)
            {
                if (data[i+j] != '=' || i+j < (data_len - 2))
                {
                    //DPRINTF("Fail: %x\n", data[i+j]);
                    delete[] buf;
                    return nullptr;
                }
            }
        }        

        buf[n++] = (uint8_t) (st[0] << 2) | (uint8_t)(st[1] >> 4);
        if (st[2] != std::string::npos)
        {
            buf[n++] = (uint8_t)(st[1] << 4) | (uint8_t)(st[2] >> 2);
            if (st[3] != std::string::npos)
            {
                buf[n++] = (uint8_t)(st[2] << 6) | (uint8_t)st[3];
            }
        }

        i += 4;
    }

    assert(n == *ret_size);

    return buf;
}

#ifdef NO_CRYPTO

void Utils::Sha1(const void *, uint32_t, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::Md5(const void *, uint32_t, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

bool Utils::FileSha1(const std::string &, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
    return false;
}

bool Utils::FileMd5(const std::string &, uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
    return false;
}

void Utils::AesEcbDecrypt(void *, size_t, const uint8_t *, int)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::AesEcbEncrypt(void *, size_t, const uint8_t *, int)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

void Utils::AesCtrEncrypt(void *, size_t, const uint8_t *, int, const uint8_t *)
{
    DPRINTF("%s: Crypto is not enabled.\n", FUNCNAME);
}

#else

void Utils::Sha1(const void *buf, uint32_t size, uint8_t *result)
{
	SHA1_CTX ctx;
	
	__SHA1_Init(&ctx);
	__SHA1_Update(&ctx, (const uint8_t *)buf, size);
	__SHA1_Final(&ctx, result);
}

void Utils::Md5(const void *buf, uint32_t size, uint8_t *result)
{
	MD5_CTX ctx;
	
	__MD5_Init(&ctx);
	__MD5_Update(&ctx, buf, size);
	__MD5_Final(result, &ctx);
}

bool Utils::FileSha1(const std::string &path, uint8_t *result)
{
	size_t remaining;
	FILE *in;
	SHA1_CTX ctx;
	
	remaining = GetFileSize(path);
	if (remaining == (size_t)-1)
		return false;
	
	__SHA1_Init(&ctx);
	
	if (remaining == 0) // Special case, 0 bytes file
	{
		__SHA1_Final(&ctx, result);
		return true;
	}
	
	in = fopen(path.c_str(), "rb");
	if (!in)
		return false;	
	
	size_t buf_size = (remaining < FILE_BUFFER_SIZE) ? remaining : FILE_BUFFER_SIZE;
    uint8_t *buf;

    buf = new uint8_t[buf_size];

	while (remaining > 0)
	{
		size_t r = (remaining < buf_size) ? remaining : buf_size;
		
		if (fread(buf, 1, r, in) != r)
		{
			fclose(in);
			delete[] buf;
			return false;
		}
		
        __SHA1_Update(&ctx, buf, (uint32_t)r);
		remaining -= r;
	}
	
	fclose(in);
	__SHA1_Final(&ctx, result);
	
	return true;
}

bool Utils::FileMd5(const std::string &path, uint8_t *result)
{
	size_t remaining;
	FILE *in;
	MD5_CTX ctx;
	
	remaining = GetFileSize(path);
	if (remaining == (size_t)-1)
		return false;
	
	__MD5_Init(&ctx);
	
	if (remaining == 0) // Special case, 0 bytes file
	{
		__MD5_Final(result, &ctx);
		return true;
	}
	
	in = fopen(path.c_str(), "rb");
	if (!in)
		return false;	
	
	size_t buf_size = (remaining < FILE_BUFFER_SIZE) ? remaining : FILE_BUFFER_SIZE;
    uint8_t *buf;

    buf = new uint8_t[buf_size];

	while (remaining > 0)
	{
		size_t r = (remaining < buf_size) ? remaining : buf_size;
		
		if (fread(buf, 1, r, in) != r)
		{
			fclose(in);
			delete[] buf;
			return false;
		}
		
        __MD5_Update(&ctx, buf, (unsigned long)r);
		remaining -= r;
	}
	
	fclose(in);
	__MD5_Final(result, &ctx);
	
	return true;
}

void Utils::AesEcbDecrypt(void *buf, size_t size, const uint8_t *key, int key_size)
{
    if (key_size != 256 && key_size != 128 && key_size != 192)
        return;

    uint32_t rk[RKLENGTH(256)]; // Max size
    static const size_t block_size = 16;

    int nrounds = rijndaelSetupDecrypt(rk, key, key_size);
    size_t num_blocks = size / block_size;
    uint8_t *ptr = (uint8_t *)buf;
    bool last_pass = false;

    if ((size % block_size) != 0)
    {
        num_blocks++;
        last_pass = true;
    }

    for (size_t i = 0; i < num_blocks; i++)
    {
        if ( (i == (num_blocks-1)) && last_pass )
        {
            uint8_t temp[block_size];
            rijndaelDecrypt(rk, nrounds, ptr, temp);

            memcpy(ptr, temp, size % block_size);
        }
        else
        {
            rijndaelDecrypt(rk, nrounds, ptr, ptr);
        }

        ptr += block_size;
    }
}

void Utils::AesEcbEncrypt(void *buf, size_t size, const uint8_t *key, int key_size)
{
    if (key_size != 256 && key_size != 128 && key_size != 192)
        return;

    uint32_t rk[RKLENGTH(256)]; // Max size
    static const size_t block_size = 16;

    int nrounds = rijndaelSetupEncrypt(rk, key, key_size);
    size_t num_blocks = size / block_size;
    uint8_t *ptr = (uint8_t *)buf;

    if ((size % block_size) != 0)
    {
        DPRINTF("%s: Bad function usage. Encryption function requires a size multiple of block_size", FUNCNAME);
        return;
    }

    for (size_t i = 0; i < num_blocks; i++)
    {
        rijndaelEncrypt(rk, nrounds, ptr, ptr);
        ptr += block_size;
    }
}

void Utils::AesCtrEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t ctr[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(ctr, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t temp[block_size];
        size_t xor_size;

        memcpy(temp, ctr, block_size);
        Utils::AesEcbEncrypt(temp, block_size, key, key_size);

        if (i == (nblocks-1) && (size&(block_size-1)))
        {
            xor_size = size&(block_size-1);
        }
        else
        {
            xor_size = block_size;
        }

        for (size_t j = 0; j < xor_size; j++)
        {
            inout[i*block_size+j] ^= temp[j];
        }

        if (i != (nblocks-1))
        {
            for (int k = block_size-1, carry = 1; k >= 0 && carry; k--)
                carry = !++ctr[k];
        }
    }
}

void Utils::AesCbcDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t xblock[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(xblock, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t temp[block_size];
        uint8_t *cb = inout + i*block_size;

        memcpy(temp, cb, block_size);
        Utils::AesEcbDecrypt(cb, block_size, key, key_size);

        for (size_t j = 0; j < block_size; j++)
        {
            cb[j] ^= xblock[j];
        }

        memcpy(xblock, temp, block_size);
    }
}

void Utils::AesCbcEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
{
    static const size_t block_size = 16;

    uint8_t xblock[block_size];
    uint8_t *inout = (uint8_t *)buf;
    size_t nblocks = size / block_size;

    memcpy(xblock, iv, block_size);

    for (size_t i = 0; i < nblocks; i++)
    {
        uint8_t *cb = inout + i*block_size;

        for (size_t j = 0; j < block_size; j++)
        {
            cb[j] ^= xblock[j];
        }

        Utils::AesEcbEncrypt(cb, block_size, key, key_size);
        memcpy(xblock, cb, block_size);
    }
}

#endif // NO_CRYPTO

std::string Utils::GetAppData()
{
#ifdef UTILS_UTF8
    return Ucs2ToUtf8(GetAppData16());
#else
    char *appdata = getenv("APPDATA");

    if (!appdata)
    {
        DPRINTF("APPDATA doesn't exist in this system.\n");
        throw std::runtime_error("APPDATA doesn't exist in this system.\n");
    }

    return std::string(appdata);
#endif
}

std::u16string Utils::GetAppData16()
{
    wchar_t *appdata = _wgetenv(L"APPDATA");

    if (!appdata)
    {
        DPRINTF("APPDATA doesn't exist in this system.\n");
        throw std::runtime_error("APPDATA doesn't exist in this system.\n");
    }

    return std::u16string((char16_t *)appdata);
}

std::string Utils::GetAppDataPath(const std::string &rel_path)
{
#ifdef UTILS_UTF8
    return Utils::Ucs2ToUtf8(GetAppDataPath(Utf8ToUcs2(rel_path)));
#else
    return NormalizePath(GetAppData() + "/" + rel_path);
#endif
}

std::u16string Utils::GetAppDataPath(const std::u16string &rel_path)
{
    return NormalizePath(GetAppData16() + (const char16_t *)(L"/") + rel_path);
}

#ifdef _WIN32

#include <windows.h>

std::string Utils::GetTempFile(const std::string &prefix, const std::string &extension)
{
    char path[MAX_PATH-14];
    char file[MAX_PATH];

    if (GetTempPathA(sizeof(path), path) == 0)
        return "<NULL>";

    if (GetTempFileNameA(path, prefix.c_str(), (int)RandomInt(0, UINT64_MAX), file) == 0)
        return "<NULL>";

    return std::string(file) + extension;
}

#endif

std::string Utils::GetTempFileLocal(const std::string &extension, bool subdir)
{
    if (subdir)
    {
        Utils::Mkdir("temp");
    }

    std::string file, ext;

    ext = (Utils::BeginsWith(extension, ".")) ? extension : "." + extension;

    do
    {
        file = ToString(RandomInt(0, UINT64_MAX)) + ToString(RandomInt(0, UINT64_MAX)) + ext;
        if (subdir)
            file = "temp/" + file;

    } while (Utils::FileExists(file));

    return file;
}

#ifdef _WIN32

float Utils::GetExeVersion(const std::string &exe_path)
{
    DWORD info_size = GetFileVersionInfoSizeA(exe_path.c_str(), nullptr);
    if (info_size == 0)
        return 0.0f;

    char *info = new char[info_size];
    uint8_t *buf;
    UINT buf_size;

    if (GetFileVersionInfoA(exe_path.c_str(), 0, info_size, info))
    {
        if (VerQueryValueA(info, "\\", (void **)&buf, &buf_size) && buf_size != 0)
        {
            VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)buf;
            if (verInfo->dwSignature == 0xfeef04bd)
            {
                float ret = ((float)(verInfo->dwFileVersionMS >> 16))  + ((float)(verInfo->dwFileVersionMS&0xFFFF) / 100.0f);
                float m = ((float)(verInfo->dwFileVersionLS>>16)) / 1000.0f;

                ret += m;
                return ret;
            }
        }
    }

    return 0.0f;
}

#endif

#ifdef _WIN32

int Utils::RunProgram(const std::string &program, const std::vector<std::string> &args, bool wait)
{
    std::string cmd;

    cmd = program;

    for (const std::string &s : args)
    {
        cmd += " \"" + s + '"';
        //cmd += " " + s;
    }

    //DPRINTF("%s\n", cmd.c_str());

    PROCESS_INFORMATION process_info;
    STARTUPINFOA startup_info;

    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    BOOL result = CreateProcessA(nullptr,
                                 (LPSTR)cmd.c_str(),
                                 nullptr,
                                 nullptr,
                                 FALSE,
                                 NORMAL_PRIORITY_CLASS,
                                 nullptr,
                                 nullptr,
                                 &startup_info,
                                 &process_info);

    if (!result)
        return -1;

    if (!wait)
        return 0;

    DWORD ret;

    WaitForSingleObject(process_info.hProcess, INFINITE);
    if (!GetExitCodeProcess(process_info.hProcess, &ret))
        ret = -1;

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    return ret;
}

#else

int Utils::RunProgram(const std::string &program, const std::vector<std::string> &args)
{
    return RunCmd(program, args);
}

#endif


int Utils::RunCmd(const std::string &program, const std::vector<std::string> &args)
{
    std::string cmd;

    cmd = program;

    for (const std::string &s : args)
    {
        cmd += " \"" + s + '"';
    }

    return system(cmd.c_str());
}

#ifdef NO_ZLIB

bool Utils::UncompressZlib(void *, uint32_t *, const void *, uint32_t, int)
{
    DPRINTF("%s: zlib is not enabled.\n", FUNCNAME);
    return false;
}

#else

static void *zalloc(void *opaque, unsigned int size, unsigned int num)
{
    UNUSED(opaque);
    return malloc(size * num);
}

static void zfree(void *opaque, void *p)
{
    UNUSED(opaque);
    free(p);
}

bool Utils::CompressZlib(void *comp_buf, long unsigned int *comp_size, const void *uncomp_buf, size_t uncomp_size, int level)
{
    return (compress2((Bytef *)comp_buf, comp_size, (const Bytef *)uncomp_buf, (uLong)uncomp_size, level) == Z_OK);
}

uint8_t *Utils::CompressZlib(const void *uncomp_buf, size_t uncomp_size, size_t *ret_size, int level)
{
    long unsigned int comp_size = (long unsigned int)(uncomp_size + (uncomp_size / 10) + 12);
    uint8_t *buf = new uint8_t[comp_size];

    if (!CompressZlib(buf, &comp_size, uncomp_buf, uncomp_size, level))
    {
        delete[] buf;
        return nullptr;
    }

    *ret_size = (size_t)comp_size;
    return buf;
}

bool Utils::UncompressZlib(void *uncomp_buf, uint32_t *uncomp_size, const void *comp_buf, uint32_t comp_size, int window)
{
    z_stream stream;

    stream.zalloc = &zalloc;
    stream.zfree = &zfree;
    stream.opaque = Z_NULL;
    stream.next_in = (uint8_t *)comp_buf;
    stream.avail_in = comp_size;
    stream.next_out = (uint8_t *)uncomp_buf;
    stream.avail_out = *uncomp_size;

    if (inflateInit2(&stream, window) != Z_OK)
        return false;

    int ret = inflate(&stream, Z_FINISH);
    if (true/*ret == Z_STREAM_END*/)
    {
        *uncomp_size = stream.total_out;
    }

    ret = inflateEnd(&stream);
    return (ret == Z_OK);
}

#endif

#ifdef _WIN32

bool Utils::GetRegistryValueBinary(HKEY key, const std::string &value_path, void *data, size_t data_size)
{
    std::string value, path;
    HKEY subkey;

    path = Utils::WindowsPath(value_path);

    size_t pos = path.rfind('\\');
    if (pos == std::string::npos)
        return false;

    if (pos != (path.length()-1))
        value = path.substr(pos+1);

    path = path.substr(0, pos);

    LONG ret = RegOpenKeyExA(key, path.c_str(), 0, KEY_QUERY_VALUE, &subkey);
    if (ret != ERROR_SUCCESS)
    {
        //DPRINTF("Failed open %s with error 0x%x\n", path.c_str(), ret);
        return false;
    }

    DWORD ret_size = (DWORD) data_size;
    ret = RegQueryValueExA(subkey, (value.length() > 0) ? value.c_str() : nullptr, nullptr, nullptr, (LPBYTE)data, &ret_size);
    if (ret != ERROR_SUCCESS)
    {
        //DPRINTF("Failed query %s with error 0x%x. ret_size=%d\n", value.c_str(), ret, ret_size);
        return false;
    }

    if (ret_size != data_size)
    {
        //DPRINTF("Data size mismatch (%d %Id)\n", ret_size, data_size);
        return false;
    }

    return true;
}

bool Utils::SetRegistryValueBinary(HKEY key, const std::string &value_path, const void *data, size_t data_size)
{
    std::string value, path;
    HKEY subkey;

    path = Utils::WindowsPath(value_path);

    size_t pos = path.rfind('\\');
    if (pos == std::string::npos)
        return false;

    if (pos != (path.length()-1))
        value = path.substr(pos+1);

    path = path.substr(0, pos);

    LONG ret = RegCreateKeyExA(key, path.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &subkey, nullptr);
    if (ret != ERROR_SUCCESS)
        return false;

    ret = RegSetValueExA(subkey, (value.length() > 0) ? value.c_str() : nullptr, 0, REG_BINARY, (const BYTE *)data, (DWORD)data_size);
    if (ret != ERROR_SUCCESS)
        return false;

    return true;
}

bool Utils::GetRegistryValueString(HKEY key, const std::string &value_path, std::string &string)
{
    std::string value, path;
    HKEY subkey;

    path = Utils::WindowsPath(value_path);

    size_t pos = path.rfind('\\');
    if (pos == std::string::npos)
        return false;

    if (pos != (path.length()-1))
        value = path.substr(pos+1);

    path = path.substr(0, pos);

    LONG ret = RegOpenKeyExA(key, path.c_str(), 0, KEY_QUERY_VALUE, &subkey);
    if (ret != ERROR_SUCCESS)
    {
        //DPRINTF("Failed open %s with error 0x%x\n", path.c_str(), ret);
        return false;
    }

    DWORD type;
    char data[256]; // FIXME: allow any size
    DWORD ret_size = sizeof(data);

    ret = RegQueryValueExA(subkey, (value.length() > 0) ? value.c_str() : nullptr, nullptr, &type, (LPBYTE)data, &ret_size);
    if (ret != ERROR_SUCCESS)
    {
        //DPRINTF("Failed query \"%s\" with error 0x%x. ret_size=%d\n", value.c_str(), ret, ret_size);
        return false;
    }

    if (type != REG_SZ)
        return false;

    data[255] = 0; // Just in case
    string = data;
    return true;
}

bool Utils::IsWine()
{
    // Static variables to avoid recomputing
    static HMODULE mod = GetModuleHandleA("ntdll.dll");
    if (!mod)
        return false; // Not wine and not windows either :) Should not happen

    static bool is_wine = (GetProcAddress(mod, "wine_get_version") != nullptr);
    return is_wine;
}

#endif

// Implementation taken from DirectXMath
uint16_t Utils::FloatToHalf(float f)
{
    uint32_t Result;

    auto IValue = reinterpret_cast<uint32_t*>(&f)[0];
    uint32_t Sign = (IValue & 0x80000000U) >> 16U;
    IValue = IValue & 0x7FFFFFFFU;      // Hack off the sign

    if (IValue > 0x477FE000U)
    {
        // The number is too large to be represented as a half.  Saturate to infinity.
        if (((IValue & 0x7F800000) == 0x7F800000) && ((IValue & 0x7FFFFF) != 0))
        {
            Result = 0x7FFF; // NAN
        }
        else
        {
            Result = 0x7C00U; // INF
        }
    }
    else if (!IValue)
    {
        Result = 0;
    }
    else
    {
        if (IValue < 0x38800000U)
        {
            // The number is too small to be represented as a normalized half.
            // Convert it to a denormalized value.
            uint32_t Shift = 113U - (IValue >> 23U);
            IValue = (0x800000U | (IValue & 0x7FFFFFU)) >> Shift;
        }
        else
        {
            // Rebias the exponent to represent the value as a normalized half.
            IValue += 0xC8000000U;
        }

        Result = ((IValue + 0x0FFFU + ((IValue >> 13U) & 1U)) >> 13U) & 0x7FFFU;
    }
    return static_cast<uint16_t>(Result | Sign);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#endif

// Implementation taken from DirectXMath
float Utils::HalfToFloat(uint16_t h)
{
    auto Mantissa = static_cast<uint32_t>(h & 0x03FF);

    uint32_t Exponent = (h & 0x7C00);
    if (Exponent == 0x7C00) // INF/NAN
    {
        Exponent = 0x8f;
    }
    else if (Exponent != 0)  // The value is normalized
    {
        Exponent = static_cast<uint32_t>((static_cast<int>(h) >> 10) & 0x1F);
    }
    else if (Mantissa != 0)     // The value is denormalized
    {
        // Normalize the value in the resulting float
        Exponent = 1;

        do
        {
            Exponent--;
            Mantissa <<= 1;
        } while ((Mantissa & 0x0400) == 0);

        Mantissa &= 0x03FF;
    }
    else                        // The value is zero
    {
        Exponent = static_cast<uint32_t>(-112);
    }

    uint32_t Result =
        ((static_cast<uint32_t>(h) & 0x8000) << 16) // Sign
        | ((Exponent + 112) << 23)                      // Exponent
        | (Mantissa << 13);                             // Mantissa

    return reinterpret_cast<float*>(&Result)[0];
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
