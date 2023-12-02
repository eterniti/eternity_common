// TODO: file too bulky, split it into subfiles and include all parts here

#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>

#ifdef QT_VERSION
#include <QString>
#endif

#include <stdint.h>

#include <string>
#include <sstream>
#include <vector>
#include <math.h>

#include "tinyxml/tinyxml.h"

#ifndef NO_ZLIB
#include <zlib.h>
#else
#define Z_DEFAULT_COMPRESSION (-1)
#endif

#define UNUSED(x) (void)(x)

#ifdef _MSC_VER

#define FUNCNAME    __FUNCSIG__

// WARNING: not safe
#define snprintf    _snprintf
#include "vs/dirent.h"

#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#include <direct.h>

#else

#include <dirent.h>
#define FUNCNAME    __PRETTY_FUNCTION__

#define _rmdir rmdir

#endif

#ifdef  _MSC_VER
#define fseeko fseek
#define ftello ftell
#define fseeko64 _fseeki64
#define ftello64 _ftelli64
#define off64_t int64_t
#endif

#if defined(_M_X64) || defined(__x86_64__)
#define CPU_X86_64 1
#elif defined(_M_X86) || defined(__i386__)
#define CPU_X86 1
#endif

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
	
    size_t WriteFileST(const std::string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    size_t WriteFileST(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    bool WriteFileBool(const std::string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);
    bool WriteFileBool(const std::u16string &path, const uint8_t *buf, size_t size, bool show_error=true, bool build_path=false);

    bool Mkdir(const std::string &str);
    bool RemoveFile(const std::string &str);
    bool RemoveEmptyDir(const std::string &str);
    bool RemoveDirFull(const std::string &str, bool ui, bool recycle, bool confirmation);
    bool RenameFile(const std::string &oldp, const std::string &newp);
	
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
	
    inline uint32_t DifPointer(const void *ptr1, const void *ptr2) // ptr1-ptr2
	{
		return (uint32_t) ((uint64_t)ptr1 - (uint64_t)ptr2);
	}

    inline uint64_t DifPointer64(const void *ptr1, const void *ptr2) // ptr1-ptr2
    {
        return ((uint64_t)ptr1 - (uint64_t)ptr2);
    }

    // ////////////////// STRING SECTION // ///////////////////////

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

    // ////////////////////////// END OF STRING SECTION // ////////////////////////

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

    // ////////////////// XML SECTION // ///////////////////////
    TiXmlElement *FindRoot(TiXmlHandle *handle, const std::string &root_name);
	
    uint32_t GetUnsigned(const std::string &str, uint32_t default_value=0);
    uint64_t GetUnsigned64(const std::string &str, uint64_t default_value=0);
    int32_t GetSigned(const std::string &str, int32_t default_value=0);
    bool GetBoolean(const std::string &str, bool default_value=false);
    float GetFloat(const std::string &str, float default_value=0.0f);

    uint32_t GetShortVersion(uint32_t version);
    uint32_t GetLongVersion(uint32_t version);

    // XML section
	
    size_t GetElemCount(const TiXmlElement *root, const char *name, const TiXmlElement **first=nullptr);

    void WriteParamString(TiXmlElement *root, const char *name, const std::string &value);
	void WriteParamMultipleStrings(TiXmlElement *root, const char *name, const std::vector<std::string> &values);
	
    void WriteParamUnsigned(TiXmlElement *root, const char *name, uint64_t value, bool hexadecimal=false);
    void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint32_t> &values, bool hexadecimal=false);
	void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint16_t> &values, bool hexadecimal=false);
	void WriteParamMultipleUnsigned(TiXmlElement *root, const char *name, const std::vector<uint8_t> &values, bool hexadecimal=false);

    void WriteParamSigned(TiXmlElement *root, const char *name, int64_t value);

    void WriteParamFloat(TiXmlElement *root, const char *name, float value);
	void WriteParamMultipleFloats(TiXmlElement *root, const char *name, const std::vector<float> &values);
	
    void WriteParamGUID(TiXmlElement *root, const char *name, const uint8_t *value);
    void WriteParamBlob(TiXmlElement *root, const char *name, const uint8_t *value, size_t size);

    void WriteParamBoolean(TiXmlElement *root, const char *name, bool value);
	
    bool ReadAttrString(const TiXmlElement *root, const char *name, std::string &value);
    bool ReadAttrMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, char separator=',', bool omit_empty=true);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint32_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint16_t *value);
    bool ReadAttrUnsigned(const TiXmlElement *root,  const char *name, uint8_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int32_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int16_t *value);
    bool ReadAttrSigned(const TiXmlElement *root,  const char *name, int8_t *value);
    bool ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);
    bool ReadAttrMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);
    bool ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, std::vector<int32_t> &values);
    bool ReadAttrMultipleSigned(const TiXmlElement *root, const char *name, int32_t *values, size_t count);
    bool ReadAttrFloat(const TiXmlElement *root,  const char *name, float *value);
    bool ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool ReadAttrMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);
    bool ReadAttrBoolean(const TiXmlElement *root, const char *name, bool *value);
	
    bool ReadParamString(const TiXmlElement *root, const char *name, std::string & value, const TiXmlElement **ret=nullptr);
    bool ReadParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, const TiXmlElement **ret=nullptr);
	
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value);
    bool ReadParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value);

    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);
	
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count);
    bool ReadParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);

    bool ReadParamSigned(const TiXmlElement *root, const char *name, int32_t *value);

    bool ReadParamFloat(const TiXmlElement *root, const char *name, float *value);
    bool ReadParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool ReadParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);
	
    bool ReadParamGUID(const TiXmlElement *root, const char *name, uint8_t *value);
    uint8_t *ReadParamBlob(const TiXmlElement *root, const char *name, size_t *psize);
	
    bool ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool ReadParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool ReadParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);

    bool ReadParamBoolean(const TiXmlElement *root, const char *name, bool *value);

    bool GetParamString(const TiXmlElement *root, const char *name, std::string &value, const TiXmlElement **ret=nullptr);
    bool GetParamMultipleStrings(const TiXmlElement *root, const char *name, std::vector<std::string> &values, const TiXmlElement **ret=nullptr);
	
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint32_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint64_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint16_t *value);
    bool GetParamUnsigned(const TiXmlElement *root, const char *name, uint8_t *value);

    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint32_t> &values);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint16_t> &values);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, std::vector<uint8_t> &values);
	
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint32_t *values, size_t count);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint16_t *values, size_t count);
    bool GetParamMultipleUnsigned(const TiXmlElement *root, const char *name, uint8_t *values, size_t count);

    bool GetParamSigned(const TiXmlElement *root, const char *name, int32_t *value);

    bool GetParamFloat(const TiXmlElement *root, const char *name, float *value);
    bool GetParamMultipleFloats(const TiXmlElement *root, const char *name, std::vector<float> &values);
    bool GetParamMultipleFloats(const TiXmlElement *root, const char *name, float *values, size_t count);
	
    bool GetParamGUID(const TiXmlElement *root, const char *name, uint8_t *value);
    uint8_t *GetParamBlob(const TiXmlElement *root, const char *name, size_t *psize);

    bool GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint32_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool GetParamUnsignedWithMultipleNames(const TiXmlElement *root, uint16_t *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);
    bool GetParamFloatWithMultipleNames(const TiXmlElement *root, float *value, const char *name1, const char *name2, const char *name3=nullptr, const char *name4=nullptr, const char *name5=nullptr);

    void WriteComment(TiXmlElement *root, const std::string & comment);
// ////////////////////////// END OF XML SECTION // ////////////////////////


    std::string SSSSModelFileName(uint32_t cms_entry, uint32_t cms_model_spec_idx, const char *ext);

    uint64_t RandomInt(uint64_t min, uint64_t max);
    float RandomProbability();
    std::string GetRandomString(size_t len);
    void GetRandomData(void *buf, size_t len);

    std::u16string Utf8ToUcs2(const std::string &utf8);
    std::string Ucs2ToUtf8(const std::u16string &ucs2);
	
    std::string Base64Encode(const uint8_t *buf, size_t size, bool add_new_line);
    uint8_t *Base64Decode(const std::string &data, size_t *ret_size);
	
    void Sha1(const void *buf, uint32_t size, uint8_t *result);
    void Md5(const void *buf, uint32_t size, uint8_t *result);
	
	bool FileSha1(const std::string &path, uint8_t *result);
	bool FileMd5(const std::string &path, uint8_t *result);

    void AesEcbDecrypt(void *buf, size_t size, const uint8_t *key, int key_size);
    void AesEcbEncrypt(void *buf, size_t size, const uint8_t *key, int key_size);

    void AesCtrEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);
    static inline void AesCtrDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv)
    {
        return AesCtrEncrypt(buf, size, key, key_size, iv);
    }

    void AesCbcDecrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);
    void AesCbcEncrypt(void *buf, size_t size, const uint8_t *key, int key_size, const uint8_t *iv);

    inline void XorBuf(void *out, const void *in, size_t size)
    {
        uint8_t *out8 = (uint8_t *)out;
        const uint8_t *in8 = (const uint8_t *)in;

        for (size_t i = 0; i < size; i++)
        {
            out8[i] ^= in8[i];
        }
    }

    bool CompressZlib(void *comp_buf, long unsigned int *comp_size, const void *uncomp_buf, size_t uncomp_size, int level=Z_DEFAULT_COMPRESSION);
    uint8_t *CompressZlib(const void *uncomp_buf, size_t uncomp_size, size_t *ret_size, int level=Z_DEFAULT_COMPRESSION);
    bool UncompressZlib(void* uncomp_buf, uint32_t *uncomp_size, const void* comp_buf, uint32_t comp_size, int window=15);
	
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

#endif

    uint16_t FloatToHalf(float f);
    float HalfToFloat(uint16_t h);

    inline bool CompareFloat(float f1, float f2, float epsilon=0.0001f) { return fabs(f1 - f2) < epsilon; }

#ifdef QT_VERSION

#ifdef _WIN32

    static inline std::string QStringToStdString(const QString &str, bool locale=true)
    {
        if (locale)
        {
            QByteArray array = str.toLocal8Bit();
            return std::string(array.data());
        }

        return str.toStdString();
    }

    static inline QString StdStringToQString(const std::string &str, bool locale=true)
    {
        if (locale)
            return QString::fromLocal8Bit(str.c_str());

        return QString::fromStdString(str);
    }

#else

    static inline std::string QStringToStdString(const QString &str)
    {
        return str.toStdString();
    }

    static inline QString StdStringToQString(const std::string &str)
    {
        return QString::fromStdString(str));
    }

#endif

#endif // QT_VERSION

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



#endif
