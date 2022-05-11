#include "AdxFile.h"
#include "WavFile.h"
#include "FixedMemoryStream.h"
#include "FileStream.h"
#include "debug.h"

#define ADX_BLOCK_SIZE  0x12

AdxFile::AdxFile()
{
    big_endian = true;
    raw_data = nullptr;
    Reset();
}

AdxFile::~AdxFile()
{
    Reset();
}

void AdxFile::Reset()
{
    num_channels = 0;
    sample_rate = 0;
    num_samples = 0;
    highpass_frequency = 500;

    unk1 = 0;
    v4_unk2 = 0;
    v4_unk3 = 0;

    has_extra_header = true;
    has_loop_data = false;

    loop = false;
    loop_start = 0;
    loop_end = 0;

    pad_start = 2;
    pad_final = 0xE;

    // For version 3
    /*version = 3;
    v4_unk4 = 0;
    has_extra_header_v4 = false;*/

    // For version 4
    version = 4;
    v4_unk4 = 1;
    has_extra_header_v4 = true;

    if (raw_data)
    {
        delete[] raw_data;
        raw_data = nullptr;
    }
	
	// TODO
	// version = 4;
	// has_extra_header_v4 = true;
	// v4_unk = val32(1);
}

uint32_t AdxFile::SamplesToBytes(uint32_t samples)
{
    uint32_t bytes = samples*num_channels;

    if (num_channels == 1)
    {
        if (bytes & 0x1F)
            bytes += 0x20 - (bytes & 0x1F);
    }
    else
    {
        if (bytes & 0x3F)
            bytes += 0x40 - (bytes & 0x3F);
    }

    bytes = (bytes/0x20) * ADX_BLOCK_SIZE;
    return bytes;
}

void AdxFile::CalculateCoeff()
{
    // This, taken from wikipedia ADX article
#define M_PI acos(-1.0)
    double a, b, c;
    a = sqrt(2.0) - cos(2.0 * M_PI * ((double)highpass_frequency / sample_rate));
    b = sqrt(2.0) - 1.0;
    c = (a - sqrt((a + b) * (a - b))) / b; //(a+b)*(a-b) = a*a-b*b, however the simpler formula loses accuracy in floating point

    c1 = c * 2.0;
    c2 = -(c * c);

    c1_16 = (int16_t)(c1 * 4096.0);
    c2_16 = (int16_t)(c2 * 4096.0);
}

bool AdxFile::Load(const uint8_t *buf, size_t size)
{
    Reset();

    if (size < 0x30)
        return false;

    const ADXHeader *hdr = (const ADXHeader *)buf;

    if (val16(hdr->signature) != 0x8000)
        return false;

    uint16_t copyright = val16(hdr->copyright_offset);
    if (copyright+4 >= size)
        return false;

    if (memcmp(buf+copyright-2, "(c)CRI", 6) != 0)
        return false;

    uint32_t max_header_size = copyright-2;
    if (max_header_size < sizeof(ADXHeader))
        return false;

    if (hdr->encoding_type != 3 || hdr->block_size != ADX_BLOCK_SIZE || hdr->bit_depth != 4)
        return false;

    if (hdr->num_channels == 0 || hdr->num_channels > 2)
        return false;

    if (hdr->flags != 0)
    {
        DPRINTF("%s: flags 0x%02X not supported.\n", FUNCNAME, hdr->flags);
        return false;
    }

    if (hdr->version != 4 && hdr->version != 3)
    {
        DPRINTF("%s: not supported version: 0x%02x\n", FUNCNAME, hdr->version);
        return false;
    }

    num_channels = hdr->num_channels;
    sample_rate = val32(hdr->sample_rate);
    num_samples = val32(hdr->num_samples);
    highpass_frequency = val16(hdr->highpass_frequency);
    version = hdr->version;

    if (version == 3)
    {
        if (sizeof(ADXHeader)+8 <= max_header_size)
        {
            const ADXLoop *loop_hdr = (const ADXLoop *)&buf[0x14];
            has_extra_header = true;

            unk1 = val32(loop_hdr->unk_00);

            if (sizeof(ADXHeader)+sizeof(ADXLoop) <= max_header_size)
            {
                has_loop_data = true;
                loop = (loop_hdr->has_loop != 0);

                if (loop)
                {
                    loop_start = val32(loop_hdr->loop_start);
                    loop_end = val32(loop_hdr->loop_end);

                    pad_start = max_header_size - sizeof(ADXHeader) - sizeof(ADXLoop);
                }
            }
            else
            {
                if (loop_hdr->has_loop)
                {
                    DPRINTF("%s: Header too short.\n", FUNCNAME);
                    return false;
                }

                pad_start = max_header_size - sizeof(ADXHeader) - 8;
            }
        }
        else
        {
            pad_start = 0;
        }
    }
    else // v4
    {
        if (sizeof(ADXHeader)+sizeof(ADXV4Unk) <= max_header_size)
        {
            const ADXV4Unk *unkv3 = (const ADXV4Unk *)&buf[0x14];
            has_extra_header = true;

            unk1 = val32(unkv3->unk_00);
            v4_unk2 = val32(unkv3->unk_04);
            v4_unk3 = val32(unkv3->unk_08);

            if (sizeof(ADXHeader)+sizeof(ADXV4Unk)+8 <= max_header_size)
            {
                const ADXLoop *loop_hdr = (const ADXLoop *)&buf[0x20];
                has_extra_header_v4 = true;

                v4_unk4 = val32(loop_hdr->unk_00);

                if (sizeof(ADXHeader)+sizeof(ADXLoop)+sizeof(ADXV4Unk) <= max_header_size)
                {
                    has_loop_data = true;
                    loop = (loop_hdr->has_loop != 0);

                    if (loop)
                    {
                        loop_start = val32(loop_hdr->loop_start);
                        loop_end = val32(loop_hdr->loop_end);

                        pad_start = max_header_size - sizeof(ADXHeader) - sizeof(ADXV4Unk) - sizeof(ADXLoop);
                    }
                }
                else
                {
                    if (loop_hdr->has_loop)
                    {
                        DPRINTF("%s: Header too short.\n", FUNCNAME);
                        return false;
                    }

                    pad_start = max_header_size - sizeof(ADXHeader) - sizeof(ADXV4Unk) - 8;
                }
            }
        }
        else
        {
            pad_start = 0;
        }
    }

    uint32_t data_start = copyright+4;
    uint32_t data_size = SamplesToBytes(num_samples);

    if (data_start+data_size+sizeof(ADXFooter) > size)
    {
        if (num_samples < 0x20)
        {
            const ADXFooter *footer = (ADXFooter *)(buf+data_start);

            if (val16(footer->signature) != 0x8001)
            {
                DPRINTF("%s: Footer signature error (2).\n", FUNCNAME);
                return false;
            }

            raw_data = new uint8_t[data_size];
            memset(raw_data, 0, data_size);
            pad_final = 0;
            return true;
        }

        return false;
    }

    raw_data = new uint8_t[data_size];
    memcpy(raw_data, buf+data_start, data_size);

    const ADXFooter *footer = (ADXFooter *)(buf+data_start+data_size);

    if (val16(footer->signature) != 0x8001)
    {
        DPRINTF("%s: Footer signature error (1).\n", FUNCNAME);
        return false;
    }

    pad_final = val16(footer->pad_count);
    return true;
}

uint32_t AdxFile::CalculateHeaderSize() const
{
    uint32_t size = sizeof(ADXHeader);

    if (version == 3)
    {
        if (has_extra_header)
        {
            if (has_loop_data)
            {
                size += sizeof(ADXLoop);
            }
            else
            {
                size += 8;
            }
        }
    }
    else
    {
        if (has_extra_header)
        {
            size += sizeof(ADXV4Unk);

            if (has_extra_header_v4)
            {
                if (has_loop_data)
                {
                    size += sizeof(ADXLoop);
                }
                else
                {
                    size += 8;
                }
            }
        }
    }

    size += pad_start + 6;
    return size;
}

uint8_t *AdxFile::Save(size_t *psize)
{
    uint32_t header_size = CalculateHeaderSize();
    uint32_t footer_size = sizeof(ADXFooter) + pad_final;
    uint32_t data_size = SamplesToBytes(num_samples);

    *psize = header_size+footer_size+data_size;
    uint8_t *buf = new uint8_t[*psize];
    memset(buf, 0, *psize);

    ADXHeader *hdr = (ADXHeader *)buf;

    hdr->signature = val16(0x8000);
    hdr->copyright_offset = val16(header_size-4);
    hdr->encoding_type = 3;
    hdr->block_size = ADX_BLOCK_SIZE;
    hdr->bit_depth = 4;
    hdr->num_channels = num_channels;
    hdr->sample_rate = val32(sample_rate);
    hdr->num_samples = val32(num_samples);
    hdr->highpass_frequency = val16(highpass_frequency);
    hdr->version = version;
    hdr->flags = 0;

    ADXLoop *loop_hdr = nullptr;
    if (version == 3)
    {
        if (has_extra_header)
        {
            ADXLoop *v3_loop = (ADXLoop *)(buf+sizeof(ADXHeader));
            v3_loop->unk_00 = val32(unk1);

            if (has_loop_data)
            {
                loop_hdr = v3_loop;
            }
        }
    }
    else
    {
        if (has_extra_header)
        {
            ADXV4Unk *v4_unk = (ADXV4Unk *)(buf+sizeof(ADXHeader));

            v4_unk->unk_00 = val32(unk1);
            v4_unk->unk_04 = val32(v4_unk2);
            v4_unk->unk_08 = val32(v4_unk3);

            if (has_extra_header_v4)
            {
                ADXLoop *v4_loop = (ADXLoop *)(buf+sizeof(ADXHeader)+sizeof(ADXV4Unk));
                v4_loop->unk_00 = val32(v4_unk4);

                if (has_loop_data)
                {
                    loop_hdr = v4_loop;
                }
            }
        }
    }

    if (loop_hdr)
    {
        loop_hdr->has_loop = val32(loop);

        if (loop)
        {
            loop_hdr->loop_start = val32(loop_start);
            loop_hdr->loop_start_offset = val32(SamplesToBytes(loop_start)+header_size);
            loop_hdr->loop_end = val32(loop_end);
            loop_hdr->loop_end_offset = val32(SamplesToBytes(loop_end)+header_size);

        }
    }

    memcpy(buf+header_size-6, "(c)CRI", 6);
    memcpy(buf+header_size, raw_data, data_size);

    ADXFooter *footer = (ADXFooter *)(buf+header_size+data_size);
    footer->signature = val16(0x8001);
    footer->pad_count = val16(pad_final);

    return buf;
}

void AdxFile::RemoveLoop()
{
    loop = false;
    has_loop_data = false;

    if (version == 4)
    {
        has_extra_header_v4 = false;
    }

    pad_start = 2;
    pad_final = 0xE;
}

bool AdxFile::SetLoopSample(uint32_t sample_start, uint32_t sample_end, int)
{
    if (sample_start >= sample_end)
        return false;

    loop_start = sample_start;
    loop_end = sample_end;    

    loop_start = (uint32_t)Utils::Align2(loop_start, 0x20);
    if (loop_end <  num_samples)
        loop_end = (uint32_t)Utils::Align2(loop_end, 0x20);

    if (loop_end > num_samples)
        loop_end = num_samples;


    if (!has_extra_header)
    {
        unk1 = 0;
        has_extra_header = true;
    }

    if (version == 4 && !has_extra_header_v4)
    {
        v4_unk2 = 0;
        v4_unk3 = 0;
        has_extra_header_v4 = true;
    }

    if (!has_loop_data)
    {
        if (version == 4)
        {
            v4_unk4 = 1;
        }

        has_loop_data = true;
    }

    if (version == 3)
    {
        pad_start = 0x6AE;
        pad_final = 0x35C;

    }
    else
    {
        pad_start = 0x7B6;
        pad_final = 0x32C;

        uint32_t header_size = CalculateHeaderSize();


        if (header_size & 0x3F)
        {
            pad_start += 0x40 - (header_size & 0x3F);
            header_size = CalculateHeaderSize();
        }

        uint32_t footer_size = sizeof(ADXFooter) + pad_final;
        uint32_t data_size = SamplesToBytes(num_samples);

        uint32_t size = header_size + footer_size + data_size;
        if (size & 0x3F)
            pad_final += 0x40 - (size & 0x3F);
    }

    loop = true;
    return true;
}

void AdxFile::DecodeBlock(const uint8_t *in, int16_t *out, int &s1, int &s2)
{
    // Based on adx2wav from BERO and wikipedia adx article
    int scale = (in[0] << 8) | in[1];
    in += 2;

    for (int i = 0; i < 16; i++)
    {
        int cs = (in[i]>>4);
        if (cs&8) cs -= 16;
        //int sample = (BASEVOL*cs*scale + COEFF1*s1 - COEFF2*s2)>>14;

        int sample_error = cs*scale;
        double sample_prediction = c1 * s1 + c2 * s2;
        int sample = sample_error + (int)sample_prediction;

        if (sample > 32767)
            sample = 32767;
        else if (sample < -32768)
            sample = -32768;

        *out++ = sample;
        s2 = s1;
        s1 = sample;

        cs = in[i]&0xF;
        if (cs&8) cs -= 16;
        //sample = (BASEVOL*cs*scale + COEFF1*s1 - COEFF2*s2)>>14;

        sample_error = cs*scale;
        sample_prediction = c1 * s1 + c2 * s2;
        sample = sample_error + (int)sample_prediction;

        if (sample > 32767)
            sample = 32767;
        else if (sample < -32768)
            sample = -32768;

        *out++ = sample;
        s2 = s1;
        s1 = sample;
    }
}

void AdxFile::EncodeBlock(const int16_t *in, uint8_t *out, int16_t &s1, int16_t &s2)
{
    int max = 0;
    double max_d;
    int16_t data[32];
    int16_t scale;

    int16_t prev_s1 = s1;
    int16_t prev_s2 = s2;    

    bool zero = true;

    for (int i = 0; i < 32; i++)
    {
        int16_t sample = in[i];
        int cs = sample - ((c1_16*s1) >> 12) - ((c2_16 *s2) >> 12);

        if (sample != 0)
            zero = false;

        if (cs > 32767)
            cs = 32767;
        else if (cs < -32768)
            cs = -32768;        

        //data[i] = cs;

        if (abs(cs) > max)
            max = abs(cs);

        s2 = s1;
        s1 = sample;
    }

    if (zero)
    {
        memset(out, 0, ADX_BLOCK_SIZE);
        return;
    }

    scale = (max - 1) / 7 + 1;
    if (scale > 4096)
       scale = 4096;
    else if (scale < 1)
        scale = 1;

    out[0] = (scale-1)>>8;
    out[1] = (scale-1)&0xFF;

    max_d = 32767.0 / (double)max;   

    s1 = prev_s1;
    s2 = prev_s2;

    for (int i = 0; i < 0x20; i++)
    {
        uint8_t nibble;
        int16_t sample = in[i];
        int cs = sample - ((c1_16*s1) >> 12) - ((c2_16 *s2) >> 12);

        if (cs > 32767)
            cs = 32767;
        else if (cs < -32768)
            cs = -32768;

        data[i] = cs;

        int val = (int) ((double)data[i] * max_d);
        if (val < -32768)
            val = -32768;
        else if (val > 32767)
            val = 32767;        

        if (val >= 0)
            val = (val + 2340) / 4681;
        else
            val = (val - 2340) / 4681;

        if (val > 7)
            val = 7;
        else if (val < -8)
            val = -8;        

        nibble = val&0xF;
        if (i&1)
        {
            out[2+(i/2)] |= nibble;
        }
        else
        {
            out[2+(i/2)] = nibble << 4;
        }

        val *= scale;
        if (val < -32768)
            val = -32768;
        else if (val > 32767)
            val = 32767;

        val = val + ((c1_16*s1) >> 12) + ((c2_16 *s2) >> 12);
        if (val < -32768)
            val = -32768;
        else if (val > 32767)
            val = 32767;

        s2 = s1;
        s1 = val;
    }    
}

uint8_t *AdxFile::Decode(int *format, size_t *psize)
{
    int16_t *buf, *ptr;
    uint32_t num_blocks = SamplesToBytes(num_samples) / (num_channels * ADX_BLOCK_SIZE);
    int s1 = 0, s2 = 0, s1_c2 = 0, s2_c2 = 0;

    buf = new int16_t[num_samples*num_channels+0x40]; // allocate some extra for last block
    ptr = buf;

    CalculateCoeff();    

    for (uint32_t n = 0; n < num_blocks; n++)
    {
        if (num_channels == 1)
        {
            DecodeBlock(raw_data+n*ADX_BLOCK_SIZE, ptr, s1, s2);
            ptr += 0x20;
        }
        else // 2
        {
            int16_t temp[0x20*2];

            DecodeBlock(raw_data+(n*2)*ADX_BLOCK_SIZE, temp, s1, s2);
            DecodeBlock(raw_data+((n*2)+1)*ADX_BLOCK_SIZE, temp+0x20, s1_c2, s2_c2);

            for(int i = 0; i < 0x20; i++)
            {
                ptr[i*2] = temp[i];
                ptr[i*2+1] = temp[i+0x20];
            }

            ptr += 0x20*2;
        }

    }

    *format = AUDIO_FORMAT_16BITS;
    *psize = num_samples*num_channels*sizeof(int16_t);
    return (uint8_t *)buf;
}

bool AdxFile::DecodeToWav(const std::string &file, bool preserve_loop, int format)
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
    wav.SetNumChannels(num_channels);
    wav.SetSampleRate(sample_rate);
    wav.SetBitDepth((decode_as_float) ? 32 : format);

    if (loop && preserve_loop)
    {
       wav.SetLoopSample(loop_start, loop_end, 0);
    }

    uint32_t samples_size = num_samples*num_channels;

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
    const uint8_t *ptr_enc = raw_data;
    const uint8_t *ptr_bottom = ptr_enc + SamplesToBytes(num_samples);
    int s1 = 0, s2 = 0, s1_c2 = 0, s2_c2 = 0;

    uint8_t *mem, *ptr_dec;
    mem = new uint8_t[samples_size];
    ptr_dec = mem;

    CalculateCoeff();

    for (uint32_t remaining_samples = num_samples; ptr_enc < ptr_bottom;)
    {
        if (num_channels == 1)
        {
            int16_t decoded_buf[0x20];

            if (remaining_samples >= 0x20 && format == 16) // Special case optimization
                DecodeBlock(ptr_enc, (int16_t *)ptr_dec, s1, s2);
            else
                DecodeBlock(ptr_enc, decoded_buf, s1, s2);

            ptr_enc += ADX_BLOCK_SIZE;

            uint32_t count = (remaining_samples > 0x20) ? 0x20 : remaining_samples;
            remaining_samples -= count;

            if (format == 16)
            {
                if (count != 0x20)
                    memcpy(ptr_dec, decoded_buf, count*2);

                ptr_dec += count*sizeof(int16_t);
            }
            else if (format == 8)
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    *(int8_t *)ptr_dec = decoded_buf[i]>>8;
                    ptr_dec += sizeof(int8_t);
                }
            }
            else if (format == 24)
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    int32_t sample = decoded_buf[i]<<8;
                    memcpy(ptr_dec, &sample, 3);
                    ptr_dec += 3;
                }
            }
            else if (format == 32)
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    *(int32_t *)ptr_dec = decoded_buf[i]<<16;
                    ptr_dec += sizeof(int32_t);
                }
            }
            else // float
            {
                for (uint32_t i = 0; i < count; i++)
                {
                    *(float *)ptr_dec = ((float)decoded_buf[i]) / (float)32768;
                    ptr_dec += sizeof(float);
                }
            }
        }
        else
        {
            int16_t decoded_buf[0x20*2];
            int16_t temp[0x20*2];

            DecodeBlock(ptr_enc, temp, s1, s2);
            ptr_enc += ADX_BLOCK_SIZE;

            DecodeBlock(ptr_enc, temp+0x20, s1_c2, s2_c2);
            ptr_enc += ADX_BLOCK_SIZE;

            int16_t *out = (remaining_samples >= 0x20 && format == 16) ? (int16_t *)ptr_dec : decoded_buf;

            for(int i = 0; i < 0x20; i++)
            {
                out[i*2] = temp[i];
                out[i*2+1] = temp[i+0x20];
            }

            uint32_t count = (remaining_samples > 0x20) ? 0x20 : remaining_samples;
            remaining_samples -= count;

            if (format == 16)
            {
                if (count != 0x20)
                    memcpy(ptr_dec, decoded_buf, count*2*2);

                ptr_dec += count*sizeof(int16_t)*2;
            }
            else if (format == 8)
            {
                for (uint32_t i = 0; i < count*2; i++)
                {
                    *(int8_t *)ptr_dec = decoded_buf[i]>>8;
                    ptr_dec += sizeof(int8_t);
                }
            }
            else if (format == 24)
            {
                for (uint32_t i = 0; i < count*2; i++)
                {
                    int32_t sample = decoded_buf[i]<<8;
                    memcpy(ptr_dec, &sample, 3);
                    ptr_dec += 3;
                }
            }
            else if (format == 32)
            {
                for (uint32_t i = 0; i < count*2; i++)
                {
                    *(int32_t *)ptr_dec = decoded_buf[i]<<16;
                    ptr_dec += sizeof(int32_t);
                }
            }
            else // float
            {
                for (uint32_t i = 0; i < count*2; i++)
                {
                    *(float *)ptr_dec = ((float)decoded_buf[i]) / (float)32768;
                    ptr_dec += sizeof(float);
                }
            }
        }
    }

    bool ret = (fwrite(mem, 1, samples_size, w_handle) == samples_size);

    delete[] mem;
    fclose(w_handle);

    return ret;    
}

bool AdxFile::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    Reset();

    if (num_channels > 2 || (format != 8 && format != 16 && format != 24 && format != 32))
    {
        if (take_ownership)
            delete[] buf;

        return false;
    }

    this->num_channels = (uint8_t)num_channels;
    this->sample_rate = sample_rate;
    num_samples = (uint32_t)size / num_channels;

    if (format == 0 || format == 32)
        num_samples /= 4;
    else if (format == 16)
        num_samples /= 2;
    else if (format == 24)
        num_samples /= 3;

    raw_data = new uint8_t[SamplesToBytes(num_samples)];

    uint32_t remaining_samples = num_samples;
    uint8_t *ptr_enc = raw_data;
    uint8_t *ptr_dec = buf;

    int16_t s1 = 0, s2 = 0, s1_c2 = 0, s2_c2 = 0;
    CalculateCoeff();

    while (remaining_samples != 0)
    {
        uint32_t count = (remaining_samples > 0x20) ? 0x20 : remaining_samples;
        size_t advance;

        if (num_channels == 1)
        {
            int16_t samples_buf[0x20];
            int16_t *dec_samples = (int16_t *)samples_buf;

            if (count < 0x20)
            {
                memset(samples_buf+count, 0, (0x20-count)*sizeof(int16_t));
            }

            if (format == 16)
            {
                if (count < 0x20)
                {
                    memcpy(dec_samples, ptr_dec, count*sizeof(int16_t));
                }
                else
                {
                    dec_samples = (int16_t *)ptr_dec;
                }

                advance = count*sizeof(int16_t);
            }
            else if (format == 8)
            {
                int8_t *ptr = (int8_t *)ptr_dec;

                for (uint32_t i = 0; i < count; i++)
                {
                    dec_samples[i] = ptr[i]<<8;
                }

                advance = count;
            }
            else if (format == 24)
            {
                uint8_t *ptr = (uint8_t *)ptr_dec;

                for (uint32_t i = 0; i < count; i++)
                {
                    int32_t sample = 0;

                    memcpy(&sample, ptr, 3);
                    ptr += 3;

                    if (sample & 0x00800000)
                        sample |= 0xFF000000;

                    dec_samples[i] = sample>>8;
                }

                advance = count*3;
            }
            else if (format == 32)
            {
                int32_t *ptr = (int32_t *)ptr_dec;

                for (uint32_t i = 0; i < count; i++)
                {
                    dec_samples[i] = ptr[i]>>16;
                }

                advance = count*sizeof(int32_t);
            }
            else // Float
            {
                float *ptr = (float *)ptr_dec;

                for (uint32_t i = 0; i < count; i++)
                {
                    float f = ptr[i] * 32768;

                    if (f > 32767)
                        f = 32767;
                    else if (f < -32768)
                        f = -32768;

                    dec_samples[i] = (int16_t)f;
                }

                advance = count*sizeof(float);
            }

            EncodeBlock(dec_samples, ptr_enc, s1, s2);
        }
        else // 2
        {
            int16_t samples_buf[0x20*2];
            int16_t temp[0x20*2];
            int16_t *dec_samples = (int16_t *)samples_buf;

            if (count < 0x20)
            {
                memset(temp+(count*2), 0, (0x20-count)*sizeof(int16_t)*2);
            }

            if (format == 16)
            {
                if (count < 0x20)
                {
                    memcpy(dec_samples, ptr_dec, count*sizeof(int16_t)*2);
                }
                else
                {
                    dec_samples = (int16_t *)ptr_dec;
                }

                advance = count*sizeof(int16_t)*2;
            }
            else if (format == 8)
            {
                int8_t *ptr = (int8_t *)ptr_dec;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    dec_samples[i] = ptr[i]<<8;
                }

                advance = count*2;
            }
            else if (format == 24)
            {
                uint8_t *ptr = (uint8_t *)ptr_dec;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    int32_t sample = 0;

                    memcpy(&sample, ptr, 3);
                    ptr += 3;

                    if (sample & 0x00800000)
                        sample |= 0xFF000000;

                    dec_samples[i] = sample>>8;
                }

                advance = count*3*2;
            }
            else if (format == 32)
            {
                int32_t *ptr = (int32_t *)ptr_dec;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    dec_samples[i] = ptr[i]>>16;
                }

                advance = count*sizeof(int32_t)*2;
            }
            else // float
            {
                float *ptr = (float *)ptr_dec;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    float f = ptr[i] * 32768;

                    if (f > 32767)
                        f = 32767;
                    else if (f < -32768)
                        f = -32768;

                    dec_samples[i] = (int16_t)f;
                }

                advance = count*sizeof(float)*2;
            }

            for (uint32_t i = 0; i < count; i++)
            {
                temp[i] = dec_samples[i*2];
                temp[i+0x20] = dec_samples[i*2+1];
            }

            EncodeBlock(temp, ptr_enc, s1, s2);
            EncodeBlock(temp+0x20, ptr_enc+ADX_BLOCK_SIZE, s1_c2, s2_c2);
        }

        remaining_samples -= count;
        ptr_enc += ADX_BLOCK_SIZE*num_channels;
        ptr_dec += advance;
    }

    if (take_ownership)
        delete[] buf;

    return true;
}

bool AdxFile::EncodeFromWav(const std::string &file, bool move_to_memory, bool preserve_loop)
{
    WavFile wav;
    int wav_format, format;

    if (!wav.LoadFromFile(file))
        return false;    

    wav_format = wav.GetFormat();
    if (wav_format == 1)
    {
        format = wav.GetBitDepth();

        if (format != 8 && format != 16 && format != 24 && format != 32)
            return false;
    }
    else if (wav_format == 3)
    {
        if (wav.GetBitDepth() != 32)
            return false;

        format = AUDIO_FORMAT_FLOAT;
    }
    else
    {
        return false;
    }

    Reset();

    uint16_t ch_count = wav.GetNumChannels();
    if (ch_count > 2)
        return false;

    num_channels = (uint8_t)ch_count;
    sample_rate = wav.GetSampleRate();
    num_samples = wav.GetNumSamples();

    if (preserve_loop && wav.HasLoop())
    {
        uint32_t loop_start, loop_end;
        int loop_count;

        wav.GetLoopSample(&loop_start, &loop_end, &loop_count);
        SetLoopSample(loop_start, loop_end, loop_count);
    }
    else
    {
        wav.RemoveLoop();
    }

    raw_data = new uint8_t[SamplesToBytes(num_samples)];

    uint32_t remaining_samples = num_samples;
    uint8_t *ptr_enc = raw_data;

    int16_t s1 = 0, s2 = 0, s1_c2 = 0, s2_c2 = 0;
    CalculateCoeff();

    if (move_to_memory)
        wav.MoveToMemory();

    Stream *samples = wav.GetSamplesStream();
    samples->Seek(0, SEEK_SET);
    MemoryStream *memory = dynamic_cast<MemoryStream *>(samples);

    while (remaining_samples != 0)
    {
        uint32_t count = (remaining_samples > 0x20) ? 0x20 : remaining_samples;

        if (num_channels == 1)
        {
            int16_t samples_buf[0x20];
            int16_t *dec_ptr = (int16_t *)samples_buf;

            if (count < 0x20)
            {
                memset(samples_buf+count, 0, (0x20-count)*sizeof(int16_t));
            }

            if (format == 16)
            {
                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&dec_ptr, count*sizeof(int16_t)))
                        return false;
                }

                else if (!samples->Read(samples_buf, count*sizeof(int16_t)))
                    return false;
            }
            else if (format == 8)
            {
                int8_t buf[0x20];
                int8_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count))
                        return false;
                }

                else if (!samples->Read(buf, count))
                    return false;

                for (uint32_t i = 0; i < count; i++)
                {
                    dec_ptr[i] = ptr[i]<<8;
                }
            }
            else if (format == 24)
            {
                uint8_t buf[0x20*3];
                uint8_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead(&ptr, count*3))
                        return false;
                }

                else if (!samples->Read(buf, count*3))
                    return false;

                for (uint32_t i = 0; i < count; i++)
                {
                    int32_t sample = 0;

                    memcpy(&sample, ptr, 3);
                    ptr += 3;

                    if (sample & 0x00800000)
                        sample |= 0xFF000000;

                    dec_ptr[i] = sample>>8;
                }
            }
            else if (format == 32)
            {
                int32_t buf[0x20];
                int32_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count*sizeof(int32_t)))
                        return false;
                }

                else if (!samples->Read(buf, count*sizeof(int32_t)))
                    return false;

                for (uint32_t i = 0; i < count; i++)
                {
                    dec_ptr[i] = ptr[i]>>16;
                }
            }
            else // Float
            {
                float buf[0x20];
                float *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count*sizeof(float)))
                        return false;
                }

                else if (!samples->Read(buf, count*sizeof(float)))
                    return false;

                for (uint32_t i = 0; i < count; i++)
                {
                    float f = ptr[i] * 32768;

                    if (f > 32767)
                        f = 32767;
                    else if (f < -32768)
                        f = -32768;

                    dec_ptr[i] = (int16_t)f;
                }
            }

            EncodeBlock(dec_ptr, ptr_enc, s1, s2);
        }
        else // 2
        {
            int16_t samples_buf[0x20*2];
            int16_t temp[0x20*2];
            int16_t *dec_ptr = (int16_t *)samples_buf;

            if (count < 0x20)
            {
                memset(temp+(count*2), 0, (0x20-count)*sizeof(int16_t)*2);
            }

            if (format == 16)
            {
                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&dec_ptr, count*sizeof(int16_t)*2))
                        return false;
                }

                else if (!samples->Read(samples_buf, count*sizeof(int16_t)*2))
                    return false;
            }
            else if (format == 8)
            {
                int8_t buf[0x20*2];
                int8_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count*2))
                        return false;
                }

                else if (!samples->Read(buf, count*2))
                    return false;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    dec_ptr[i] = ptr[i]<<8;
                }
            }
            else if (format == 24)
            {
                uint8_t buf[0x20*3*2];
                uint8_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead(&ptr, count*3*2))
                        return false;
                }

                else if (!samples->Read(buf, count*3*2))
                    return false;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    int32_t sample = 0;

                    memcpy(&sample, ptr, 3);
                    ptr += 3;

                    if (sample & 0x00800000)
                        sample |= 0xFF000000;

                    dec_ptr[i] = sample>>8;
                }
            }
            else if (format == 32)
            {
                int32_t buf[0x20*2];
                int32_t *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count*sizeof(int32_t)*2))
                        return false;
                }

                else if (!samples->Read(buf, count*sizeof(int32_t)*2))
                    return false;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    dec_ptr[i] = ptr[i]>>16;
                }
            }
            else // float
            {
                float buf[0x20*2];
                float *ptr = buf;

                if (memory && count == 0x20)
                {
                    if (!memory->FastRead((uint8_t **)&ptr, count*sizeof(float)*2))
                        return false;
                }

                else if (!samples->Read(buf, count*sizeof(float)*2))
                    return false;

                for (uint32_t i = 0; i < count*2; i++)
                {
                    float f = ptr[i] * 32768;

                    if (f > 32767)
                        f = 32767;
                    else if (f < -32768)
                        f = -32768;

                    dec_ptr[i] = (int16_t)f;
                }
            }

            for (uint32_t i = 0; i < count; i++)
            {
                temp[i] = dec_ptr[i*2];
                temp[i+0x20] = dec_ptr[i*2+1];
            }

            EncodeBlock(temp, ptr_enc, s1, s2);
            EncodeBlock(temp+0x20, ptr_enc+ADX_BLOCK_SIZE, s1_c2, s2_c2);
        }

        remaining_samples -= count;
        ptr_enc += ADX_BLOCK_SIZE*num_channels;
    }

    return true;
}

bool AdxFile::Concat(const AdxFile &other, bool keep_loop)
{
    if (other.num_channels != num_channels || other.sample_rate != sample_rate)
        return false;

    uint32_t size_bytes_this = SamplesToBytes(num_samples);
    uint32_t size_bytes_other = SamplesToBytes(other.num_samples);

    uint32_t new_num_samples = num_samples;
    if (new_num_samples & 0x1F)
        new_num_samples += 0x20 - (new_num_samples & 0x1F);

    new_num_samples += other.num_samples;
    uint8_t *new_raw_data = new uint8_t[SamplesToBytes(new_num_samples)];

    memcpy(new_raw_data, raw_data, size_bytes_this);
    memcpy(new_raw_data+size_bytes_this, other.raw_data, size_bytes_other);

    num_samples = new_num_samples;
    delete[] raw_data;
    raw_data = new_raw_data;

    if (!keep_loop)
        RemoveLoop();

    return true;
}

bool AdxFile::Split(AdxFile &other1, AdxFile &other2, uint32_t split_sample)
{
    uint32_t samples_first = split_sample;
    uint32_t samples_second;

    if (samples_first & 0x1F)
        samples_first += 0x20 - (samples_first & 0x1F);

    if (samples_first >= num_samples)
        return false;

    samples_second = num_samples-samples_first;

    if (samples_second == 0)
        return false;

    other1.Reset(); other2.Reset();

    other1.num_channels = other2.num_channels = num_channels;
    other1.sample_rate = other2.sample_rate = sample_rate;
    other1.highpass_frequency = other2.highpass_frequency = highpass_frequency;
    other1.version = other2.version = version;
    other1.unk1 = other2.unk1 = unk1;
    other1.v4_unk2 = other2.v4_unk2 = v4_unk2;
    other1.v4_unk3 = other2.v4_unk3 = v4_unk3;
    other1.v4_unk4 = other2.v4_unk4 = v4_unk4;
    other1.has_extra_header = other2.has_extra_header = has_extra_header;
    other1.has_extra_header_v4 = other2.has_extra_header_v4 = has_extra_header_v4;
    other1.has_loop_data = other2.has_loop_data = has_loop_data;

    other1.SetMaxLoop(); other2.SetMaxLoop();
    other1.RemoveLoop(); other2.RemoveLoop();
    other1.num_samples = samples_first; other2.num_samples = samples_second;

    uint32_t size_bytes1 = SamplesToBytes(samples_first);
    uint32_t size_bytes2 = SamplesToBytes(samples_second);

    other1.raw_data = new uint8_t[size_bytes1];
    other2.raw_data = new uint8_t[size_bytes2];

    memcpy(other1.raw_data, raw_data, size_bytes1);
    memcpy(other2.raw_data, raw_data+size_bytes1, size_bytes2);

    return true;
}

bool AdxFile::CheckHeader(const uint8_t *buf, size_t size, uint32_t *num_channels, uint32_t *sample_rate, uint32_t *num_samples)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    ADXHeader *hdr;
    uint8_t *copyright;

    if (!stream.FastRead((uint8_t **)&hdr, sizeof(ADXHeader)))
        return false;

    if (hdr->signature != 0x0080)
        return false;

    if (!stream.Seek(re16(hdr->copyright_offset)-2, SEEK_SET))
        return false;

    if (!stream.FastRead(&copyright, 6))
        return false;

    if (memcmp(copyright, "(c)CRI", 6) != 0)
        return false;

    if (num_channels)
        *num_channels = hdr->num_channels;

    if (sample_rate)
        *sample_rate = re32(hdr->sample_rate);

    if (num_samples)
        *num_samples = re32(hdr->num_samples);

    return true;
}

bool AdxFile::CheckHeader(const std::string &file, uint32_t *num_channels, uint32_t *sample_rate, uint32_t *num_samples)
{
    FileStream stream("rb");
    ADXHeader hdr;
    uint8_t copyright[6];

    if (!stream.LoadFromFile(file))
        return false;

    if (!stream.Read(&hdr, sizeof(ADXHeader)))
        return false;

    if (hdr.signature != 0x0080)
        return false;

    if (!stream.Seek(re16(hdr.copyright_offset)-2, SEEK_SET))
        return false;

    if (!stream.Read(copyright, 6))
        return false;

    if (memcmp(copyright, "(c)CRI", 6) != 0)
        return false;

    if (num_channels)
        *num_channels = hdr.num_channels;

    if (sample_rate)
        *sample_rate = be32(hdr.sample_rate);

    if (num_samples)
        *num_samples = be32(hdr.num_samples);

    return true;
}

