#ifndef __ZIPFILE_H__
#define __ZIPFILE_H__

#include <zip.h>
#include "BaseFile.h"

class ZipFile : public BaseFile
{
private:

    std::string temp_file;

    std::string temp_param1, temp_param2;
    static bool AddDirVisitor(const std::string &path, bool, void *param);

protected:

    zip_t *archive;

    void Reload();

public:

    ZipFile();
    virtual ~ZipFile() override;

    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;

    size_t GetNumEntries() const;
    inline bool IsEmpty() const { return (GetNumEntries() == 0); }

    bool FileExists(const std::string &path) const;
    bool DirExists(const std::string &path) const;
    uint64_t GetFileSize(const std::string &path) const;

    uint8_t *ReadFile(const std::string &path, size_t *psize);
    bool WriteFile(const std::string &path, const void *buf, size_t size);

    char *ReadTextFile(const std::string &path);
    bool ReadTextFile(const std::string &path, std::string &ret);

    bool _DeleteFile(const std::string &path);
    size_t DeleteDir(const std::string &path);
    size_t DeleteAll();

    bool VisitDirectory(const std::string &path, bool (* visitor)(const std::string &file, void *param), void *param=nullptr);

    inline bool RemoveFile(const std::string &path) { return _DeleteFile(path); }

    bool AddExternalFile(const std::string &external_path, const std::string &internal_path);
    bool AddExternalDirectory(const std::string &external_path, const std::string &internal_path);

    bool RenameFile(const std::string &path, const std::string &new_path);
    bool RenamePath(const std::string &path, const std::string &new_path);
};

#endif // __ZIPFILE_H__
