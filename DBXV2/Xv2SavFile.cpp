#include "Xv2SavFile.h"
#include "debug.h"

#define ENCRYPTED_SIGNATURE 0x4C018948
#define ENCRYPTED_SIZE  	0x7B440
#define ENCRYPTED_SIZE_V8	0xB08C0
#define ENCRYPTED_SIZE_V17  0xDF2A0
#define DECRYPTED_SIZE  	0x7B390
#define DECRYPTED_SIZE_V8	0xB0818
#define DECRYPTED_SIZE_V17  0xDF1F8
#define STEAM_ID_KEY    	0x7468656265636F6E

#define FIRST_CAC_OFFSET  	0x1E830
#define CACS_DISTANCE 		0xC6C8

static const uint8_t *key1 = (const uint8_t *) "PR]-<Q9*WxHsV8rcW!JuH7k_ug:T5ApX";
static const uint8_t *iv1 = (const uint8_t *) "_Y7]mD1ziyH#Ar=0";

Xv2SavFile::Xv2SavFile()
{
    this->big_endian = false;
    this->version = 1;
}

Xv2SavFile::~Xv2SavFile()
{

}

uint32_t Xv2SavFile::GetEncryptedSize() const
{
    switch (version)
    {
        case 8:
            return ENCRYPTED_SIZE_V8;

        case 17:
            return ENCRYPTED_SIZE_V17;
    }

    return ENCRYPTED_SIZE;
}

uint32_t Xv2SavFile::GetDecryptedSize() const
{
    switch (version)
    {
        case 8:
            return DECRYPTED_SIZE_V8;

        case 17:
            return DECRYPTED_SIZE_V17;
    }

    return DECRYPTED_SIZE;
}

int Xv2SavFile::EncryptedSizeToVersion(size_t size)
{
    if (size == ENCRYPTED_SIZE)
        return 1;

    if (size == ENCRYPTED_SIZE_V8)
        return 8;

    if (size == ENCRYPTED_SIZE_V17)
        return 17;

    return -1;
}

int Xv2SavFile::DecryptedSizeToVersion(size_t size)
{
    if (size == DECRYPTED_SIZE)
        return 1;

    if (size == DECRYPTED_SIZE_V8)
        return 8;

    if (size == DECRYPTED_SIZE_V17)
        return 17;

    return -1;
}

bool Xv2SavFile::Load(const uint8_t *buf, size_t size)
{
    data.clear();

    if (!buf || size < 0x200)
        return false;

    if (memcmp(buf, "#SAV", 4) != 0)
    {
        is_encrypted = true;
        version = EncryptedSizeToVersion(size);

        if (version < 0)
        {
            DPRINTF("%s Encrypted size not supported (got 0x%Ix)\n", FUNCNAME, size);
            return false;
        }

        bool check = true;
        uint8_t *copy = new uint8_t[size];

        memcpy(copy, buf, size);
        uint32_t data_size = *(uint32_t *)&copy[8];

        if (check)
        {
            uint8_t md5[16];

            Utils::Md5(copy+0x20, data_size, md5);

            if (memcmp(md5, copy+0x10, 0x10) != 0)
            {
                DPRINTF("%s: Md5 mismatch.\n", FUNCNAME);
                return false;
            }

        }

        Utils::AesCtrDecrypt(copy+0x20, 0x80, key1, 256, iv1);

        if (memcmp(copy+0x20, "#SAV", 4) != 0)
        {
            DPRINTF("Failed at signature of first section.\n");
            delete[] copy;
            return false;
        }
		
        uint32_t decrypted_size = GetDecryptedSize();

        if (check)
        { 
			if (*(uint32_t *)&copy[0x9C] != decrypted_size)
            {
                DPRINTF("%s: Decrypted size mismatch (got %x).\n", FUNCNAME, *(uint32_t *)&copy[0x9C]);
                return false;
            }

            if (copy[0x24] != 0)
            {
                DPRINTF("%s: Value at 4 not zero.\n", FUNCNAME);
                return false;
            }

            uint8_t check = copy[0x25];
            for (int i = 0; i < 7; i++)
            {
                check += copy[0x35+i];
            }

            if (check != copy[0x34])
            {
                DPRINTF("%s Checksum1 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (uint32_t i = 0; i < decrypted_size; i+=0x20)
            {
                check += copy[0xA0+i];
            }

            if (check != copy[0x3B])
            {
                DPRINTF("%s: Checksum2 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (int i = 0; i < 4; i++)
            {
                check += copy[0x8C+i*4];
            }

            if (check != copy[0x39])
            {
                DPRINTF("%s: Checksum3 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (int i = 0; i < 4; i++)
            {
                check += copy[0x5C+i*4];
            }

            if (check != copy[0x38])
            {
                DPRINTF("%s: Checksum4 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (int i = 0; i < 8; i++)
            {
                check += copy[0x6C+i*4];
            }

            if (check != copy[0x37])
            {
                DPRINTF("%s: Checksum5 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (int i = 0; i < 8; i++)
            {
                check += copy[0x3C+i*4];
            }

            if (check != copy[0x36])
            {
                DPRINTF("%s: Checksum6 failed.\n", FUNCNAME);
                return false;
            }

            check = 0;
            for (int i = 0; i < 14; i++)
            {
                check += copy[0x26+i];
            }

            if (check != copy[0x35])
            {
                DPRINTF("%s: Checksum7 failed.\n", FUNCNAME);
                return false;
            }
        }

        if (copy[0x25] & 4)
        {
            Utils::AesCtrDecrypt(copy+0xA0, data_size-0x80, copy+0x6C, 256, copy+0x8C);
        }
        else
        {
            Utils::AesCtrDecrypt(copy+0xA0, data_size-0x80, copy+0x3C, 256, copy+0x5C);
        }

        if (memcmp(copy+0xA0, "#SAV", 4) != 0)
        {
            DPRINTF("Failed at signature of second section.\n");
            //Utils::WriteFile("temp.tmp", copy+0x20, data_size);
            delete[] copy;
            return false;
        }

        if (check)
        {
            uint8_t check = 0;
            for (uint32_t i = 0; i < decrypted_size; i+=0x20)
            {
                check += copy[0xA0+i];
            }

            if (check != copy[0x3A])
            {
                DPRINTF("%s: Checksum 8 failed.\n", FUNCNAME);
                return false;
            }
        }

        data.resize(data_size-0x80);
        memcpy(data.data(), copy+0xA0, data_size-0x80);

        delete[] copy;
    }
    else
    {
        version = DecryptedSizeToVersion(size);
        if (version < 0)
        {
            DPRINTF("%s Decrypted size not supported (got 0x%Ix)\n", FUNCNAME, size);
            return false;
        }

		is_encrypted = false;
        data.resize(size);
        memcpy(data.data(), buf, size);
    }

    if (data.size() != DECRYPTED_SIZE && data.size() != DECRYPTED_SIZE_V8 && data.size() != DECRYPTED_SIZE_V17)
    {
        DPRINTF("%s: Size of save is not expected one.\n", FUNCNAME);
        return false;
    }

    return true;
}

uint8_t *Xv2SavFile::Save(size_t *psize)
{
    uint8_t *buf;
    size_t size = data.size();

    if (is_encrypted)
    {
        uint32_t decrypted_size = GetDecryptedSize();
        uint32_t encrypted_size = GetEncryptedSize();
		
        size += 0x80 + ((version != 1) ? 0x28 : 0x30);
        buf = new uint8_t[size];
        memset(buf, 0, size);

        memcpy(buf+0xA0, data.data(), data.size());
        Utils::GetRandomData(buf+0x20, 0x80);
        buf[0x25] = 0x34;

        buf[0x3A] = 0;
        for (uint32_t i = 0; i < decrypted_size; i+=0x20)
        {
            buf[0x3A] += buf[0xA0+i];
        }

        Utils::AesCtrDecrypt(buf+0xA0, decrypted_size, buf+0x6C, 256, buf+0x8C);

        buf[0x35] = 0;

        for (int i = 0; i < 14; i++)
        {
            buf[0x35] += buf[0x26+i];
        }

        buf[0x36] = 0;
        for (int i = 0; i < 8; i++)
        {
            buf[0x36] += buf[0x3C+i*4];
        }

        buf[0x37] = 0;
        for (int i = 0; i < 8; i++)
        {
            buf[0x37] += buf[0x6C+i*4];
        }

        buf[0x38] = 0;
        for (int i = 0; i < 4; i++)
        {
            buf[0x38] += buf[0x5C+i*4];
        }

        buf[0x39] = 0;
        for (int i = 0; i < 4; i++)
        {
            buf[0x39] += buf[0x8C+i*4];
        }

        buf[0x3B] = 0;
        for (uint32_t i = 0; i < decrypted_size; i+=0x20)
        {
           buf[0x3B] += buf[0xA0+i];
        }

        buf[0x34] = buf[0x25];
        for (int i = 0; i < 7; i++)
        {
            buf[0x34] += buf[0x35+i];
        }

        memcpy(buf+0x20, "#SAV", 4);
        buf[0x24] = 0;
        *(uint32_t *)&buf[0x9C] = decrypted_size;

        Utils::AesCtrDecrypt(buf+0x20, 0x80, key1, 256, iv1);

        *(uint32_t *)&buf[0] = ENCRYPTED_SIGNATURE;
        *(uint32_t *)&buf[4] = encrypted_size;
        *(uint32_t *)&buf[8] = decrypted_size+0x80;

        Utils::Md5(buf+0x20, decrypted_size+0x80, buf+0x10);
    }
    else
    {
        buf = new uint8_t[size];
        memcpy(buf, data.data(), size);
    }

    *psize = size;
    return buf;
}

uint64_t Xv2SavFile::GetSteamID64() const
{
    const uint64_t *buf = (const uint64_t *)data.data();
    return (buf[1] ^ STEAM_ID_KEY);
}

void Xv2SavFile::SetSteamID64(uint64_t id)
{
    uint64_t *buf = (uint64_t *)data.data();
    buf[1] = id ^ STEAM_ID_KEY;
}

XV2Cac *Xv2SavFile::GetCac(uint32_t idx)
{
    if (idx >= XV2_NUM_CAC)
        return nullptr;

    return (XV2Cac *)(data.data() + FIRST_CAC_OFFSET + (idx*CACS_DISTANCE));
}

bool Xv2SavFile::IsValidCac(uint32_t idx) const
{
    if (idx >= XV2_NUM_CAC)
        return false;

    const XV2Cac *cac = (const XV2Cac *)(data.data() + FIRST_CAC_OFFSET + (idx*CACS_DISTANCE));

    if (cac->name[0] == 0)
        return false;

    if (cac->race >= CAC_NUM_RACES)
        return false;

    return true;
}
