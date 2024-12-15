#ifndef UTILSFS_H
#define UTILSFS_H

#include "Utils.h"

namespace Utils
{
    bool FileExists(const std::string &path);
    bool FileExists(const std::u16string &path);
    bool DirExists(const std::string &path);
    bool DirExists(const std::u16string &path);

    bool AppDataFileExists(const std::string &rel_path);
    bool AppDataFileExists(const std::u16string &rel_path);

    size_t GetFileSize(const std::string &path);
    size_t GetFileSize(const std::u16string &path);
    bool GetFileDate(const std::string &path, time_t *mtime, time_t *ctime=nullptr, time_t *atime=nullptr);
    bool GetFileDate(const std::u16string &path, time_t *mtime, time_t *ctime=nullptr, time_t *atime=nullptr);

    uint8_t *ReadFile(const std::string &path, size_t *psize, bool show_error=true);
    uint8_t *ReadFileFrom(const std::string &path, size_t from, size_t size, bool show_error=true);

    bool ReadTextFile(const std::string &path, std::string &text, bool show_error=true);
    bool WriteTextFile(const std::string &path, const std::string &text, bool show_error=true, bool build_path=false);

    size_t WriteFileST(const std::string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    size_t WriteFileST(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    bool WriteFileBool(const std::string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    bool WriteFileBool(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);

    bool Mkdir(const std::string &str);
    bool RemoveFile(const std::string &str);
    bool RemoveEmptyDir(const std::string &str);
    bool RemoveDirFull(const std::string &str, bool ui, bool recycle, bool confirmation);
    bool RenameFile(const std::string &oldp, const std::string &newp);
    bool MoveFileOrDir(const std::string &oldp, const std::string &newp);

    bool CreatePath(const std::string &path, bool last_is_directory=false);
    bool CreatePath(const std::u16string &path, bool last_is_directory=false);
    FILE *fopen_create_path(const std::string &filename, const char *mode);
    FILE *fopen_create_path(const std::u16string &filename, const char *mode);

    bool VisitDirectory(const std::string &path, bool files, bool directories, bool recursive, bool (* visitor)(const std::string &path, bool is_directory, void *custom_param), void *custom_param=nullptr, bool show_error=false, bool ignore_error=false);
    bool IsDirectoryEmpty(const std::string &path, bool ignore_directories);
    size_t CountFiles(const std::string &path, bool recursive);
    bool ListFiles(const std::string &path, bool files, bool directories, bool recursive, std::vector<std::string> &paths);

    bool CompareFiles(const std::string &file1, const std::string &file2);
    bool CompareFilesPartial(const std::string &file1, const std::string &file2, uint64_t compare_size);

    bool DoCopyFile(FILE *src, FILE *dst, uint64_t size);
    bool DoCopyFile(const std::string &input, const std::string &output, bool build_path=false);
    bool CopyDir(const std::string &input, const std::string &output, bool hard_link_if_possible=false);

    bool HardLink(const std::string &input, const std::string &output, bool build_path=false);
}


#endif // UTILSFS_H
