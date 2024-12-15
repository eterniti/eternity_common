#include <ctime>
#include <codecvt>
#include <locale>

#include "UtilsMisc.h"

#include "debug.h"

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

#ifdef _MSC_VER

// Don't need this anymore
static int utf8_to_ucs2(const unsigned char * input, const unsigned char ** end_ptr)
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
}

#endif

std::u16string Utils::Utf8ToUcs2(const std::string &utf8)
{
    // New GCC version don't like the dirty implementation
    // It compiled, but caused freeze, there is probably a bug in the *c_utf8 != 0 check
    // I'll keep old implementation for VS because it's faster

#ifdef _MSC_VER

    if (utf8.length() == 0)
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

    return ucs2;
#else

    // New implementation:
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.from_bytes(utf8);
#endif
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

typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

static bool GetRealWindowsVersion(DWORD &major, DWORD &minor, DWORD &buildNumber)
{
    HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
    if (hMod) {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW rovi = { sizeof(rovi) };
            if (RtlGetVersion(&rovi) == 0) { // STATUS_SUCCESS is 0
                major = rovi.dwMajorVersion;
                minor = rovi.dwMinorVersion;
                buildNumber = rovi.dwBuildNumber;
                return true;
            }
        }
    }
    return false;
}


static bool IsWindowsVersionOrGreater(DWORD majorVersion, DWORD minorVersion, DWORD servicePackMajor) {
    DWORD wMajor, wMinor, wService;

    if (!GetRealWindowsVersion(wMajor, wMinor, wService))
        return false;

    if (wMajor > majorVersion) return true;
    if (wMajor < majorVersion) return false;

    if (wMinor > minorVersion) return true;
    if (wMinor < minorVersion) return false;

    return (wService >= servicePackMajor);
}

bool Utils::IsWindows10OrGreater()
{
    return IsWindowsVersionOrGreater(10, 0, 0);
}

bool Utils::IsUtf8()
{
    static bool cached_result = false;
    static bool is_cached_result = false;

    if (is_cached_result)
        return cached_result;

    if (IsWindows10OrGreater())
    {
        HRSRC hRes = FindResource(NULL, MAKEINTRESOURCE(1), RT_MANIFEST);
        if (hRes)
        {
            HGLOBAL hResLoad = LoadResource(NULL, hRes);
            if (hResLoad)
            {
                DWORD dwSize = SizeofResource(NULL, hRes);
                LPVOID pResData = LockResource(hResLoad);

                if (pResData)
                {
                     std::string manifest(static_cast<char*>(pResData), dwSize);
                     // Maybe replace this with an actual parser...
                     if (manifest.find("UTF-8</activeCodePage>") != std::string::npos)
                         cached_result = true;
                }
            }
        }
    }

    is_cached_result = true;
    return cached_result;
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
