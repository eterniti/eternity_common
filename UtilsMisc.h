#ifndef UTILSMISC_H
#define UTILSMISC_H

#include "Utils.h"

namespace Utils
{
    inline uint32_t DifPointer(const void *ptr1, const void *ptr2) // ptr1-ptr2
    {
        return (uint32_t) ((uint64_t)ptr1 - (uint64_t)ptr2);
    }

    inline uint64_t DifPointer64(const void *ptr1, const void *ptr2) // ptr1-ptr2
    {
        return ((uint64_t)ptr1 - (uint64_t)ptr2);
    }

    static inline size_t Align(size_t n, size_t alignment)
    {
        if ((n % alignment) != 0)
            n += (alignment - (n % alignment));

        return n;
    }

    // For power of 2 only
    static inline size_t Align2(size_t n, size_t alignment)
    {
        if (n & (alignment-1))
            n += (alignment - (n & (alignment-1)));

        return n;
    }

    uint32_t GetShortVersion(uint32_t version);
    uint32_t GetLongVersion(uint32_t version);

    std::string SSSSModelFileName(uint32_t cms_entry, uint32_t cms_model_spec_idx, const char *ext);

    uint64_t RandomInt(uint64_t min, uint64_t max);
    float RandomProbability();
    std::string GetRandomString(size_t len);
    void GetRandomData(void *buf, size_t len);

    std::u16string Utf8ToUcs2(const std::string &utf8);
    std::string Ucs2ToUtf8(const std::u16string &ucs2);

    std::string GetAppData();
    std::u16string GetAppData16();
    std::string GetAppDataPath(const std::string &rel_path);
    std::u16string GetAppDataPath(const std::u16string &rel_path);

    std::string GetTempFile(const std::string &prefix, const std::string &extension);
    std::string GetTempFileLocal(const std::string &extension, bool subdir=true);

    float GetExeVersion(const std::string &exe_path);

    int RunProgram(const std::string &program, const std::vector<std::string> &args, bool wait=true);
    int RunCmd(const std::string &program, const std::vector<std::string> &args);

#ifdef _WIN32

    bool GetRegistryValueBinary(HKEY key, const std::string &value_path, void *data, size_t data_size);
    bool SetRegistryValueBinary(HKEY key, const std::string &value_path, const void *data, size_t data_size);

    bool GetRegistryValueString(HKEY key, const std::string &value_path, std::string &string);

    bool IsWine();

    bool IsWindows10OrGreater();
    bool IsUtf8();

#endif

    uint16_t FloatToHalf(float f);
    float HalfToFloat(uint16_t h);

    inline bool CompareFloat(float f1, float f2, float epsilon=0.0001f) { return fabs(f1 - f2) < epsilon; }

    inline uint64_t Make64(uint32_t low, uint32_t high)
    {
        return ((uint64_t)high << 32ULL) | (uint64_t)low;
    }

    inline void Split64(uint64_t q, uint32_t &low, uint32_t &high)
    {
        low = (uint32_t)q&0xFFFFFFFF;
        high = (uint32_t)(q >> 32ULL);
    }
}

#endif // UTILSMISC_H
