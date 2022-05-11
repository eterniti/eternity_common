#ifndef __CRIFS_H__
#define __CRIFS_H__

#include <unordered_set>
#include "CpkFile.h"

class CriFs
{
private:

    std::vector<CpkFile *> cpks;
    std::unordered_set<std::string> directories;

    static bool BuildVisitor(const std::string &path, bool, void *param);
    static bool VisitVisitor(const std::string &path, bool, void *param);

    // Temp, for visitor
    std::unordered_set<std::string> visitor_files_list;

protected:

    std::string loose_files_root;
    bool read_only;

    std::string ConvertPath(const std::string &path);

public:
    CriFs(const std::string &loose_files_root, bool read_only=false);
    ~CriFs();

    const std::string &GetLooseRoot() const { return loose_files_root; }

    bool AddCpk(const std::string &cpk_path);

    void BuildDirList(); // Needed to be called at least once for VisitDirectory to work!

    size_t GetFileSize(const std::string &path);

    uint8_t *ReadFile(const std::string &path, size_t *psize, bool only_cpk=false);
    char *ReadTextFile(const std::string &path);
    bool WriteFile(const std::string &path, const void *buf, size_t size);

    bool RemoveFile(const std::string &path); // Only operates on the loose files
    void RemoveEmptyDir(const std::string &path); // Only operates on the loose files
    void RemoveDir(const std::string &path, bool remove_empty); // only operates on the loose files

    bool LoadFile(BaseFile *file, const std::string &path, bool only_cpk=false);
    bool SaveFile(BaseFile *file, const std::string &path);

    bool CompileFile(BaseFile *file, const std::string &path);
    bool DecompileFile(BaseFile *file, const std::string &path);

    bool FileExists(const std::string &file, bool check_cpk = true, bool check_loose = true);

    bool VisitDirectory(const std::string &path, bool files, bool directories, bool recursive, bool (* visitor)(const std::string &path, bool is_directory, void *custom_param), void *custom_param=nullptr);
};

#endif // __CRIFS_H__
