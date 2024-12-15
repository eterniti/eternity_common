#ifndef UTILSSTR_H
#define UTILSSTR_H

#include "Utils.h"

namespace Utils
{
    void TrimString(std::string &str, bool trim_left=true, bool trim_right=true);
    bool IsEmptyString(const std::string &str);
    std::string GetFileNameString(const std::string &path);
    std::string GetDirNameString(const std::string &path);
    std::string MakePathString(const std::string &comp1, const std::string &comp2);

    size_t GetMultipleStrings(const std::string &str, std::vector<std::string> &list, char separator = ',', bool omit_empty_strings=true, bool do_trimr=true);
    size_t GetMultipleStrings(const std::string &str, std::vector<std::string> &list, const std::string &separator, bool omit_empty_strings=true);
    size_t GetMultipleStringsSpaces(const std::string &str, std::vector<std::string> &list);

    std::string ToSingleString(const std::vector<std::string> &list, const std::string &separator=", ", bool omit_empty_strings=true);
    std::string ToSingleString(const std::vector<uint32_t> &list, bool hexadecimal=false);
    std::string ToSingleString(const std::vector<uint16_t> &list, bool hexadecimal=false);
    std::string ToSingleString(const std::vector<uint8_t> &list, bool hexadecimal=false);
    std::string ToSingleString(const std::vector<float> & list);

    std::string ToLowerCase(const std::string &str);
    std::string ToUpperCase(const std::string &str);

    bool IsAlphaNumeric(const std::string &str);
    bool HasOnlyDigits(const std::string &str);

    bool BeginsWith(const std::string &str, const std::string &substr, bool case_sensitive=true);
    bool EndsWith(const std::string &str, const std::string &substr, bool case_sensitive=true);
    size_t CharCount(const std::string &str, char ch);

    std::string UnsignedToString(uint64_t value, bool hexadecimal);
    std::string UnsignedToHexString(uint8_t value, bool zeropad, bool prefix=true);
    std::string UnsignedToHexString(uint16_t value, bool zeropad, bool prefix=true);
    std::string UnsignedToHexString(uint32_t value, bool zeropad, bool prefix=true);
    std::string UnsignedToHexString(uint64_t value, bool zeropad, bool prefix=true);

    std::string SignedToString(int64_t value);

    std::string FloatToString(float value);

    std::string Vectors2DToString(const float *vectors, size_t count);
    size_t GetVectors2DFromString(const std::string &str, std::vector<float> &vectors);

    std::string Vectors3DToString(const float *vectors, size_t count);
    size_t GetVectors3DFromString(const std::string &str, std::vector<float> &vectors);

    std::string Vectors4DToString(const float *vectors, size_t count);
    size_t GetVectors4DFromString(const std::string &str, std::vector<float> &vectors);

    std::string GUID2String(const uint8_t *guid);
    bool String2GUID(uint8_t *guid, const std::string &str);

    std::string ColorToString(uint32_t color, bool alpha=false);

    // std::to_string doesn't work in some versions of mingw... lets make a replacer.
    template <typename T>
    std::string ToString(T val)
    {
        std::stringstream stream;
        stream << val;
        return stream.str();
    }

    static inline std::string ToStringAndPad(int val, uint8_t pad)
    {
        std::string ret = ToString(val);

        if (ret.length() < pad)
        {
            size_t left_zeroes = (size_t)pad - ret.length();

            for (size_t i = 0; i < left_zeroes; i++)
                ret.insert(ret.begin(), '0');
        }

        return ret;
    }

    std::string BinaryString(const uint8_t *buf, size_t size, bool separate=true, bool prefix=false);
    bool BinaryStringToBuf(const std::string &str, uint8_t *buf, size_t size);

    std::string NormalizePath(const std::string &path);
    std::u16string NormalizePath(const std::u16string &path);
    std::string WindowsPath(const std::string &path);
    std::u16string WindowsPath(const std::u16string &path);
    std::string SamePath(const std::string &file_path, const std::string &file_name);
    std::u16string SamePath(const std::u16string &file_path, const std::u16string &file_name);

    std::string RandomString(size_t len);
    size_t Replace(std::string &str, const std::string &from, const std::string &to);

    size_t FORMAT_PRINTF3 Sprintf(std::string &buf, bool append, const char *fmt, ...);

    class StringFinder
    {
    private:

        std::string str1;

    public:

        StringFinder(const std::string &str) { str1 = str; }
        bool operator()(const std::string &str2) const
        {
            return (strcasecmp(str1.c_str(), str2.c_str()) == 0);
        }
    };
}

#endif // UTILSSTR_H
