#ifndef __UE4COMMON_H__
#define __UE4COMMON_H__

#include "Stream.h"
#include "Utils.h"

namespace UE4Common
{
    struct FString
    {
        bool is_ucs2;

        std::string str;
        std::u16string ucs2_str;

        FString()
        {
            is_ucs2 = false;
        }

        FString(const std::string &str)
        {
            this->str = str;
            is_ucs2 = false;
        }

        FString(const std::u16string &str)
        {
            this->ucs2_str = str;
            is_ucs2 = true;
        }

        uint32_t Length() const
        {
            return (is_ucs2) ? (uint32_t)ucs2_str.length() : (uint32_t)str.length();
        }

        uint32_t Size() const
        {
            uint32_t len = Length();

            if (len == 0)
                return len;

            if (is_ucs2)
                return (len*2)+2;

            return len+1;
        }

        uint32_t TotalSize() const
        {
            return Size()+sizeof(int32_t);
        }

        void Reset()
        {
            str.clear();
            ucs2_str.clear();
            is_ucs2 = false;
        }

        bool Read(FILE *file);
        bool Read(Stream *stream);

        bool Write(FILE *file) const;
        bool Write(Stream *stream) const;

        bool operator == (const FString &rhs) const
        {
            if (is_ucs2 != rhs.is_ucs2)
                return false;

            if (is_ucs2)
                return (ucs2_str == rhs.ucs2_str);

            return (str == rhs.str);
        }

        bool operator != (const FString &rhs) const
        {
            return !(*this == rhs);
        }
    };
}

#endif /* __UE4COMMON_H__ */
