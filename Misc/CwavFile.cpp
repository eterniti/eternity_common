/*
 * This file is based / uses modified portions of cwav.c/cwav.h from project ctr / ctrtool
 * Below is the copy of the ctrtool license.
 *
*/

/*
MIT License

Copyright (c) 2012 neimod
Copyright (c) 2014 3DSGuy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "CwavFile.h"
#include "WavFile.h"
#include "FixedMemoryStream.h"
#include "debug.h"

CwavFile::CwavFile()
{
    this->big_endian = false;
}

CwavFile::~CwavFile()
{

}

void CwavFile::Reset()
{
    channels.clear();
    raw_data.clear();
    num_samples = 0;
}

bool CwavFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < sizeof(CWAVHeader))
        return false;

    const CWAVHeader *hdr = (const CWAVHeader *)buf;

    if (hdr->signature != CWAV_SIGNATURE || hdr->header_size < sizeof(CWAVHeader) || size < hdr->file_size)
        return false;

    if ((hdr->info.offset + hdr->info.size) > size || hdr->info.size < sizeof(CWAVInfoHeader))
        return false;

    const CWAVInfoHeader *info = (const CWAVInfoHeader *)GetOffsetPtr(hdr, hdr->info.offset);

    if (info->signature != CWAV_INFO_SIGNATURE || info->size < sizeof(CWAVInfoHeader))
        return false;

    if (info->encoding != CWAV_ENCODING_DSPADPCM) // We only support this atm
    {
        DPRINTF("%s: Encoding other than DSPADPCM not supported yet.\n", FUNCNAME);
        return false;
    }

    has_loop = (info->loop != 0);
    sample_rate = info->sample_rate;
    loop_start = info->loop_start;
    loop_end = info->loop_end;
    channels.resize(info->num_channels);

    const uint8_t *channels_top = (const uint8_t *)(info+1);
    uint8_t *channels_current = (uint8_t *)channels_top;

    for (size_t i = 0; i < channels.size(); i++)
    {
        CwavChannel &channel = channels[i];
        const CWAVReference *ch_ref = (const CWAVReference *)channels_current;
        const CWAVChannelInfo *ch_info = (const CWAVChannelInfo *) (channels_top-4+ch_ref->offset);

        if (info->encoding == CWAV_ENCODING_DSPADPCM)
        {
            if (ch_info->codec.id_type != 0x300)
            {
                DPRINTF("%s: Bad id_type (0x%x) for this codec.\n", FUNCNAME, ch_info->codec.id_type);
                return false;
            }
            const CWAVDSPadPcmInfo *dspadpcm_info = (const CWAVDSPadPcmInfo *)GetOffsetPtr(ch_info, ch_info->codec.offset);
            channel.dspadpcm_info = *dspadpcm_info;
        }

        channel.start = ch_info->sample.offset;
        channels_current += sizeof(CWAVReference);
    }

    if ((hdr->data.offset + hdr->data.size) > size || hdr->data.size < sizeof(CWAVDataHeader))
        return false;

    const CWAVDataHeader *data = (const CWAVDataHeader *) GetOffsetPtr(hdr, hdr->data.offset);

    if (data->signature != CWAV_DATA_SIGNATURE || data->size < sizeof(CWAVDataHeader))
        return false;

    raw_data.resize(data->size-8);
    memcpy(raw_data.data(), data+1, raw_data.size());

    num_samples = (has_loop) ? (((uint32_t)raw_data.size() * 8) / 14) : loop_end;
    return true;
}

void CwavFile::DecodeBlock(CwavChannel &channel, int16_t *out, int max)
{
    uint8_t *in = raw_data.data();
    uint32_t b = channel.dsapdcm_state.sample_offset;

    int16_t yn1 = channel.dsapdcm_state.yn1;
    int16_t yn2 = channel.dsapdcm_state.yn2;

    uint8_t ln = in[b]&0xF;
    uint8_t hn = in[b]>>4;

    int16_t coef1 = channel.dspadpcm_info.coef[hn*2+0];
    int16_t coef2 = channel.dspadpcm_info.coef[hn*2+1];
    uint32_t shift = 17 - ln;
    int16_t table[14];

    b++;

    for (int i = 0 ; i < 7; i++)
    {
        table[i*2+0] = in[b]>>4;
        table[i*2+1] = in[b]&0xF;
        b++;
    }

    for(int i = 0; i < max; i++)
    {
        int32_t x = table[i] << 28;
        int32_t xshifted = x >> shift;
        int32_t prediction = (yn1 * coef1 + yn2 * coef2 + xshifted + 0x400)>>11;

        if (prediction < -0x8000)
            prediction = -0x8000;
        if (prediction > 0x7FFF)
            prediction = 0x7FFF;

        yn2 = yn1;
        yn1 = prediction;

        out[i] = prediction;
    }

    channel.dsapdcm_state.yn1 = yn1;
    channel.dsapdcm_state.yn2 = yn2;
    channel.dsapdcm_state.sample_offset += 8;
}

uint8_t *CwavFile::Decode(int *format, size_t *psize)
{
    for (size_t i = 0; i < channels.size(); i++)
    {
        CwavChannel &channel = channels[i];

        channel.dsapdcm_state.sample_offset = channel.start;
        channel.dsapdcm_state.yn1 = channel.dspadpcm_info.yn1;
        channel.dsapdcm_state.yn2 = channel.dspadpcm_info.yn2;
    }

    int16_t *buf = new int16_t[num_samples*channels.size()+0x200]; // allocate some extra for last block
    int16_t *ptr = buf;

    memset(buf, 0, num_samples*channels.size()+0x200);

    int remaining_samples = (int)num_samples;

    while (remaining_samples > 0)
    {
        int max = (remaining_samples >= 14) ? 14 : remaining_samples;
        int16_t block[14];

        for (size_t i = 0; i < channels.size(); i++)
        {
            DecodeBlock(channels[i], block, max);

            for (int j = 0; j < max; j++)
            {
                ptr[i*max+j] = block[j];
            }

            ptr += max*channels.size();
        }

        remaining_samples -= max;
    }

    *format = AUDIO_FORMAT_16BITS;
    *psize = num_samples*channels.size()*sizeof(int16_t);
    return (uint8_t *)buf;
}

bool CwavFile::DecodeToWav(const std::string &file, bool preserve_loop, int format)
{
    WavFile wav;
    FILE *w_handle;
    bool decode_as_float = (format == AUDIO_FORMAT_FLOAT);

    if (!decode_as_float && format != 8 && format != 16 && format != 24 && format != 32)
        return false;

    w_handle = fopen(file.c_str(), "wb");
    if (!w_handle)
        return false;

    wav.SetFormat((decode_as_float) ? 3 : 1);
    wav.SetNumChannels((uint16_t)channels.size());
    wav.SetSampleRate(sample_rate);
    wav.SetBitDepth((decode_as_float) ? 32 : format);

    if (has_loop && preserve_loop)
    {
       wav.SetLoopSample(loop_start, loop_end, 0);
    }

    uint32_t samples_size = num_samples*(uint16_t)channels.size();

    if (decode_as_float)
    {
        samples_size *= sizeof(float);
    }
    else if (format == 32)
    {
        samples_size *= sizeof(uint32_t);
    }
    else if (format == 24)
    {
        samples_size *= 3;
    }
    else if (format == 16)
    {
        samples_size *= sizeof(uint16_t);
    }

    size_t hdr_size;
    uint8_t *hdr = wav.CreateHeader(&hdr_size, samples_size);

    if (!hdr || fwrite(hdr, 1, hdr_size, w_handle) != hdr_size)
    {
        if (hdr)
            delete[] hdr;

        fclose(w_handle);
        return false;
    }

    delete[] hdr;

    // Decode
    size_t decoded_size;
    int decoded_format;
    uint8_t *decoded = Decode(&decoded_format, &decoded_size);

    if (!decoded)
     {
        fclose(w_handle);
        return false;
    }

    bool ret;

    if (format == 16)
    {
        // Direct write
        ret = (fwrite(decoded, 1, decoded_size, w_handle) == decoded_size);
    }
    else
    {
        uint8_t *mem = new uint8_t[samples_size];
        uint8_t *ptr_dec = mem;

        int16_t *decoded_buf = (int16_t *)decoded;

        for (size_t i = 0; i < num_samples*channels.size(); i++)
        {
            if (format == 8)
            {
               *(int8_t *)ptr_dec = decoded_buf[i]>>8;
               ptr_dec += sizeof(int8_t);
            }
            else if (format == 24)
            {
                int32_t sample = decoded_buf[i]<<8;
                memcpy(ptr_dec, &sample, 3);
                ptr_dec += 3;
            }
            else if (format == 32)
            {
                *(int32_t *)ptr_dec = decoded_buf[i]<<16;
                ptr_dec += sizeof(int32_t);
            }
            else // Float
            {
                *(float *)ptr_dec = ((float)decoded_buf[i]) / (float)32768;
                ptr_dec += sizeof(float);
            }
        }

        ret = (fwrite(mem, 1, samples_size, w_handle) == samples_size);
        delete[] mem;
    }

    delete[] decoded;
    fclose(w_handle);

    return ret;
}

bool CwavFile::CheckHeader(const uint8_t *buf, size_t size)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    CWAVHeader *hdr;
    CWAVInfoHeader *info;

    if (!stream.FastRead((uint8_t **)&hdr, sizeof(CWAVHeader)))
        return false;

    if (hdr->signature != CWAV_SIGNATURE)
        return false;

    if (!stream.Seek(hdr->info.offset, SEEK_SET))
        return false;

    if (!stream.FastRead((uint8_t **)&info, sizeof(CWAVInfoHeader)))
        return false;

    if (info->signature != CWAV_INFO_SIGNATURE)
        return false;

    if (info->encoding != CWAV_ENCODING_DSPADPCM) // We only support this atm
        return false;

    return true;
}
