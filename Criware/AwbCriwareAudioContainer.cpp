#include "AwbCriwareAudioContainer.h"
#include "CpkFile.h"
#include "Afs2File.h"

AwbCriwareAudioContainer::AwbCriwareAudioContainer()
{
    awb = nullptr;
    temp_mem = nullptr;
}

AwbCriwareAudioContainer::~AwbCriwareAudioContainer()
{

}

bool AwbCriwareAudioContainer::Load(const uint8_t *buf, size_t size)
{
    if (awb)
    {
        delete awb;
        awb = nullptr;
    }

    if (size < 4)
        return false;

    if (*(uint32_t *)buf == CPK_SIGNATURE)
    {
        awb = new CpkFile();
    }
    else
    {
        awb = new Afs2File();
    }

    return awb->Load(buf, size);
}

uint8_t *AwbCriwareAudioContainer::Save(size_t *psize)
{
    return awb->Save(psize);
}

uint8_t *AwbCriwareAudioContainer::GetAwb(uint32_t *awb_size) const
{
    if (temp_mem)
    {
        delete[] temp_mem;
        temp_mem = nullptr;
    }

    size_t size;
    temp_mem = awb->Save(&size);

    *awb_size = (uint32_t)size;
    return temp_mem;
}

bool AwbCriwareAudioContainer::SetAwb(void *awb_buf, uint32_t awb_size, bool take_ownership)
{
    if (awb)
    {
        delete awb;
        awb = nullptr;
    }

    if (*(uint32_t *)awb_buf == CPK_SIGNATURE)
    {
        awb = new CpkFile();
    }
    else
    {
        awb = new Afs2File();
    }

    if (!awb->Load((const uint8_t *)awb_buf, awb_size))
        return false;

    if (take_ownership)
    {
        uint8_t *awb_buf8 = (uint8_t *)awb_buf;
        delete[] awb_buf8;
    }

    return true;
}
