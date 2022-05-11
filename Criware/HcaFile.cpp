// Most of the code in this file was taken from HCA decoder, which was in public domain
// The code has been adapated to fit a new design, but the decoding is mostly the same code
//
// Because HCA decoder was likely compiled with 387 floating point, the output when compared to SSE didn't match.
// For that reason, some smalls changes where done to the decoder (all those casts to double) and now 387 and SSE produce
// same output, which is identical to original HCA decoder.

#include <math.h>
#include "HcaFile.h"
#include "FixedMemoryStream.h"
#include "FileStream.h"
#include "debug.h"

#if defined(__SSE__) || _M_IX86_FP!=0 || _M_X64
#define USING_SSE
#endif

#define ENCODER_PROGRAM "hcaenctool0.exe"
#define ENCODER_DLL "hcaenc_lite.dll"
#define ENCODER_ERROR "encoder_error.txt"

HcaFile::HcaFile()
{
	big_endian = true;
	raw_data = nullptr;
	Reset();
}

HcaFile::~HcaFile()
{
	Reset();
}

void HcaFile::Reset()
{
	has_comp = false;
	has_dec = false;
	has_vbr = false;
	has_ath = false;
	has_loop = false;
	has_ciph = false;
	has_rva = false;
	has_comm = false;
	has_pad = false;

    pad_size = 0;
	
	if (raw_data)
	{
		delete[] raw_data;
		raw_data = nullptr;
	}
	
    // Keys for some old games
    /*cipher56_key1 = 0x30DBE1AB;
    cipher56_key2 = 0xCC554639;*/

    // Keys for Magia Record
    cipher56_key1 = 0x1395C51;
    cipher56_key2 = 0;

    cipher56_inited = false;
}

uint16_t HcaFile::CheckSum(const void *data, unsigned int size, uint16_t sum)
{
	static const uint16_t v[] =
	{
		0x0000,0x8005,0x800F,0x000A,0x801B,0x001E,0x0014,0x8011,0x8033,0x0036,0x003C,0x8039,0x0028,0x802D,0x8027,0x0022,
		0x8063,0x0066,0x006C,0x8069,0x0078,0x807D,0x8077,0x0072,0x0050,0x8055,0x805F,0x005A,0x804B,0x004E,0x0044,0x8041,
		0x80C3,0x00C6,0x00CC,0x80C9,0x00D8,0x80DD,0x80D7,0x00D2,0x00F0,0x80F5,0x80FF,0x00FA,0x80EB,0x00EE,0x00E4,0x80E1,
		0x00A0,0x80A5,0x80AF,0x00AA,0x80BB,0x00BE,0x00B4,0x80B1,0x8093,0x0096,0x009C,0x8099,0x0088,0x808D,0x8087,0x0082,
		0x8183,0x0186,0x018C,0x8189,0x0198,0x819D,0x8197,0x0192,0x01B0,0x81B5,0x81BF,0x01BA,0x81AB,0x01AE,0x01A4,0x81A1,
		0x01E0,0x81E5,0x81EF,0x01EA,0x81FB,0x01FE,0x01F4,0x81F1,0x81D3,0x01D6,0x01DC,0x81D9,0x01C8,0x81CD,0x81C7,0x01C2,
		0x0140,0x8145,0x814F,0x014A,0x815B,0x015E,0x0154,0x8151,0x8173,0x0176,0x017C,0x8179,0x0168,0x816D,0x8167,0x0162,
		0x8123,0x0126,0x012C,0x8129,0x0138,0x813D,0x8137,0x0132,0x0110,0x8115,0x811F,0x011A,0x810B,0x010E,0x0104,0x8101,
		0x8303,0x0306,0x030C,0x8309,0x0318,0x831D,0x8317,0x0312,0x0330,0x8335,0x833F,0x033A,0x832B,0x032E,0x0324,0x8321,
		0x0360,0x8365,0x836F,0x036A,0x837B,0x037E,0x0374,0x8371,0x8353,0x0356,0x035C,0x8359,0x0348,0x834D,0x8347,0x0342,
		0x03C0,0x83C5,0x83CF,0x03CA,0x83DB,0x03DE,0x03D4,0x83D1,0x83F3,0x03F6,0x03FC,0x83F9,0x03E8,0x83ED,0x83E7,0x03E2,
		0x83A3,0x03A6,0x03AC,0x83A9,0x03B8,0x83BD,0x83B7,0x03B2,0x0390,0x8395,0x839F,0x039A,0x838B,0x038E,0x0384,0x8381,
		0x0280,0x8285,0x828F,0x028A,0x829B,0x029E,0x0294,0x8291,0x82B3,0x02B6,0x02BC,0x82B9,0x02A8,0x82AD,0x82A7,0x02A2,
		0x82E3,0x02E6,0x02EC,0x82E9,0x02F8,0x82FD,0x82F7,0x02F2,0x02D0,0x82D5,0x82DF,0x02DA,0x82CB,0x02CE,0x02C4,0x82C1,
		0x8243,0x0246,0x024C,0x8249,0x0258,0x825D,0x8257,0x0252,0x0270,0x8275,0x827F,0x027A,0x826B,0x026E,0x0264,0x8261,
		0x0220,0x8225,0x822F,0x022A,0x823B,0x023E,0x0234,0x8231,0x8213,0x0216,0x021C,0x8219,0x0208,0x820D,0x8207,0x0202,
	};
	
	for(uint8_t * s= (uint8_t *)data, *e = s+size; s < e; s++)
		sum = (sum<<8) ^ v[(sum>>8) ^ *s];
	
	return sum;
}

static inline unsigned int ceil2(unsigned int a,unsigned int b)
{
	return (b > 0) ? (a/b + ((a%b) ? 1 : 0)) : 0;
}

bool HcaFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	const uint8_t *ptr = buf;
	const uint8_t *bottom = buf + size;
	
	if (size < sizeof(HCAHeader))
		return false;
	
	const HCAHeader *hdr = (const HCAHeader *)ptr;
	
    if ((hdr->signature & 0x7F7F7F7F) != HCA_SIGNATURE || val16(hdr->data_offset) >= size)
		return false;
	
	if (CheckSum(buf, val16(hdr->data_offset)))
	{
		DPRINTF("%s: Warning, bad header checksum.\n", FUNCNAME);
	}
	
	const uint8_t *data = buf + val16(hdr->data_offset);
	uint32_t data_size;
	
	version = val16(hdr->version);
	ptr += sizeof(HCAHeader);
	
	if (ptr + sizeof(HCAFmt) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == FMT_SIGNATURE)
	{
		const HCAFmt *fmt = (const HCAFmt *)ptr;
		
		this->fmt.num_channels = fmt->num_channels;
		this->fmt.sample_rate = ((fmt->sample_rate&0xFF) << 16) | (fmt->sample_rate & 0x00FF00) | ((fmt->sample_rate & 0xFF0000) >> 16);
		this->fmt.block_count = val32(fmt->block_count);
		this->fmt.r01 = val16(fmt->r01);
		this->fmt.r02 = val16(fmt->r02);
		
		ptr += sizeof(HCAFmt);
	}
	else
	{
		// Fmt is mandatory        
		return false;
	}
	
	if (ptr + sizeof(HCAComp) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == COMP_SIGNATURE)
	{
		const HCAComp *comp = (const HCAComp *)ptr;
		
		block_size = this->comp.block_size = val16(comp->block_size);
		comp_r01 = this->comp.r01 = comp->r01;
		comp_r02 = this->comp.r02 = comp->r02;
		comp_r03 = this->comp.r03 = comp->r03;
		comp_r04 = this->comp.r04 = comp->r04;
		comp_r05 = this->comp.r05 = comp->r05;
		comp_r06 = this->comp.r06 = comp->r06;
		comp_r07 = this->comp.r07 = comp->r07;
		comp_r08 = this->comp.r08 = comp->r08;
		this->comp.reserved = val16(comp->reserved);
		
		if (this->comp.reserved != 0)
		{
			DPRINTF("%s: Warning: comp reserved not 0.\n", FUNCNAME);			
		}
		
		if (!((block_size >=8 && block_size <=0xFFFF) || (block_size == 0))) 
			return false;
		
        if (!(/*comp_r01 >=0 && */comp_r01 <= comp_r02 && comp_r02 <= 0x1F))
			return false;
		
		has_comp = true;
		ptr += sizeof(HCAComp);
	}
    else if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == DEC_SIGNATURE)
	{
		const HCADec *dec = (const HCADec *)ptr;
		
		block_size = this->dec.block_size = val16(dec->block_size);
		comp_r01 = this->dec.r01 = dec->r01;
		comp_r02 = this->dec.r02 = dec->r02;
		comp_r03= this->dec.r04 = dec->r04;
		comp_r04 = this->dec.r03 = dec->r03;
		
		this->dec.count1 = dec->count1;
		this->dec.count2 = dec->count2;
		this->dec.enable_count2 = dec->enable_count2;
		
		comp_r05 = dec->count1+1;
		comp_r06= ((dec->enable_count2) ? dec->count2 : dec->count1) + 1;
		comp_r07 = comp_r05 - comp_r06;
		comp_r08 = 0;
		
		if (!((block_size >= 8 && block_size <= 0xFFFF) || (block_size == 0)))
			return false;
		
        if (!(/*comp_r01 >=0 &&*/ comp_r01 <= comp_r02 && comp_r02 <= 0x1F))
			return false;
		
		if(comp_r03 == 0)
			comp_r03 = 1;
		
		has_dec = true;
		ptr += sizeof(HCADec);
	}
	else
	{
		// Comp or Dec mandatory
		return false;
	}	
	
	if (block_size == 0)
	{
		DPRINTF("%s: Block size 0 not supported.\n", FUNCNAME);
		return false;
	}
	
	if (ptr+sizeof(HCAVbr) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == VBR_SIGNATURE)
	{
		const HCAVbr *vbr = (const HCAVbr *)ptr;
		
		this->vbr.r01 = val16(vbr->r01);
		this->vbr.r02 = val16(vbr->r02);
		
        if (!(block_size == 0 /*&& this->vbr.r01 >= 0*/ && this->vbr.r01 <= 0x1FF))
			return false;
		
		has_vbr = true;
		ptr += sizeof(HCAVbr);
	}
	
	if (ptr+sizeof(HCAAth) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == ATH_SIGNATURE)
	{
		const HCAAth *ath = (const HCAAth *)ptr;
		
		ath_type = ath->type;
		
		has_ath = true;
		ptr += sizeof(HCAAth);
	}
	else
	{
		ath_type = (version < 0x200) ? 1 : 0;
	}
	
	if (ptr+sizeof(HCALoop) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == LOOP_SIGNATURE)
	{
		const HCALoop *loop = (const HCALoop *)ptr;
		
		this->loop.loop_start = val32(loop->loop_start);
		this->loop.loop_end = val32(loop->loop_end);
		this->loop.r01 = val16(loop->r01);
		this->loop.r02= val16(loop->r02);
		
        if (!(/*this->loop.loop_start >= 0 &&*/ this->loop.loop_start <= this->loop.loop_end && this->loop.loop_end < fmt.block_count))
			return false;
		
		has_loop = true;
		ptr += sizeof(HCALoop);
	}
	
	if (ptr+sizeof(HCACiph) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == CIPH_SIGNATURE)
	{
		const HCACiph *ciph = (const HCACiph *)ptr;
		
		ciph_type = val16(ciph->type);
		
		if (!(ciph_type == 0 || ciph_type ==1 || ciph_type == 0x38))
			return false;
		
		has_ciph = true;
		ptr += sizeof(HCACiph);
	}
	else
	{
		ciph_type = 0;
	}
	
	if (ptr+sizeof(HCARva) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == RVA_SIGNATURE)
	{
		const HCARva *rva = (const HCARva *)ptr;
		
		copy_float(&volume, rva->volume);
		
		has_rva = true;
		ptr += sizeof(HCARva);
	}
	else
	{
		volume = 1.0f;
	}
	
	if (ptr+sizeof(HCAComm) >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == COMM_SIGNATURE)
	{
		const HCAComm *comm = (const HCAComm *)ptr;		
		ptr += sizeof(HCAComm);
		
		if (ptr+256 >= bottom)
			return false;
		
        if (strnlen((const char *)ptr, 256)+1 != comm->len)
		{
			DPRINTF("%s: failure in comm.\n", FUNCNAME);
			return false;
		}
		
        this->comm = (const char *)ptr;
		has_comm = true;
		ptr += this->comm.length() + 1;
	}
	
	if (ptr+4 >= bottom)
		return false;
	
    if ((*(const uint32_t *)ptr & 0x7F7F7F7F) == PAD_SIGNATURE)
	{
		has_pad = true;
		ptr += sizeof(uint32_t);
	}
	
	if (ptr > data)
		return false;
	
    pad_size = (uint32_t)(data - ptr);
	data_size = fmt.block_count*block_size;
	
	if ((data + data_size) > bottom)
		return false;	

    raw_data = new uint8_t[data_size];
	memcpy(raw_data, data, data_size);	
	ptr = raw_data;	
	
	for (uint32_t i = 0; i < fmt.block_count; i++)
	{
		if (CheckSum(ptr, block_size))
		{
			DPRINTF("%s: Warning, bad checksum in block %d\n", FUNCNAME, i);
		}
		
		ptr += block_size;
	}	
	
	return true;
}

unsigned int HcaFile::CalculateHeaderSize() const
{
	unsigned int size = sizeof(HCAHeader) + sizeof(HCAFmt);
	
	if (has_comp)
		size += sizeof(HCAComp);
	else 
		size += sizeof(HCADec);
		
	if (has_vbr)
		size += sizeof(HCAVbr);
		
	if (has_ath)
		size += sizeof(HCAAth);
		
	if (has_loop)
		size += sizeof(HCALoop);
	
	if (has_ciph)
		size += sizeof(HCACiph);
	
	if (has_rva)
		size += sizeof(HCARva);
	
	if (has_comm)
        size += sizeof(HCAComm) + (uint32_t)comm.length() + 1;
	
	if (has_pad)
		size += sizeof(uint32_t);
	
	size += pad_size;
	return size;
}

uint8_t *HcaFile::Save(size_t *psize)
{
	unsigned int header_size = CalculateHeaderSize();
	unsigned int file_size = header_size + fmt.block_count*block_size;
	
	assert(header_size < 0x10000);
	
    uint8_t *buf = new uint8_t[file_size];
    memset(buf, 0, file_size);
	
	uint8_t *ptr = buf;
	HCAHeader *hdr = (HCAHeader *)ptr;
	
	hdr->signature = HCA_SIGNATURE;
	hdr->version = val16(version);
	hdr->data_offset = val16(header_size);
	ptr += sizeof(HCAHeader);
	
	HCAFmt *fmt = (HCAFmt *)ptr;
	
	fmt->signature = FMT_SIGNATURE;
	fmt->num_channels = this->fmt.num_channels;;
	fmt->sample_rate = ((this->fmt.sample_rate&0xFF) << 16) | (this->fmt.sample_rate & 0x00FF00) | ((this->fmt.sample_rate & 0xFF0000) >> 16);
	fmt->block_count = val32(this->fmt.block_count);
	fmt->r01 = val16(this->fmt.r01);
	fmt->r02 = val16(this->fmt.r02);
	ptr += sizeof(HCAFmt);
	
	if (has_comp)
	{
		HCAComp *comp = (HCAComp *)ptr;
		
		comp->signature = COMP_SIGNATURE;
		comp->block_size = val16(block_size);
		comp->r01 = this->comp.r01;
		comp->r02 = this->comp.r02;
		comp->r03 = this->comp.r03;
		comp->r04 = this->comp.r04;
		comp->r05 = this->comp.r05;
		comp->r06 = this->comp.r06;
		comp->r07 = this->comp.r07;
		comp->r08 = this->comp.r08;
		comp->reserved = val16(this->comp.reserved);
		
		ptr += sizeof(HCAComp);
	}
	else
	{
		HCADec *dec = (HCADec *)ptr;
		
		dec->signature = DEC_SIGNATURE;
		dec->block_size = val16(block_size);
		dec->r01 = this->dec.r01;
		dec->r02 = this->dec.r02;
		dec->count1 = this->dec.count1;
		dec->count2 = this->dec.count2;
		dec->r03 = this->dec.r03;
		dec->r04 = this->dec.r04;
		dec->enable_count2 = this->dec.enable_count2;
		
		ptr += sizeof(HCADec);
	}
	
	if (has_vbr)
	{
		HCAVbr *vbr = (HCAVbr *)ptr;
		
		vbr->signature = VBR_SIGNATURE;
		vbr->r01 = val16(this->vbr.r01);
		vbr->r02 = val16(this->vbr.r02);
		
		ptr += sizeof(HCAVbr);
	}
	
	if (has_ath)
	{
		HCAAth *ath = (HCAAth *)ptr;
		
		ath->signature = ATH_SIGNATURE;
		ath->type = ath_type;
		
		ptr += sizeof(HCAAth);
	}
	
	if (has_loop)
	{
		HCALoop *loop = (HCALoop *)ptr;
		
		loop->signature = LOOP_SIGNATURE;
		loop->loop_start = val32(this->loop.loop_start);
		loop->loop_end = val32(this->loop.loop_end);
		loop->r01 = val16(this->loop.r01);
		loop->r02 = val16(this->loop.r02);
		
		ptr += sizeof(HCALoop);
	}
	
	if (has_ciph)
	{
		HCACiph *ciph = (HCACiph *)ptr;
		
		ciph->signature = CIPH_SIGNATURE;
		ciph->type = val16(ciph_type);
		
		ptr += sizeof(HCACiph);
	}
	
	if (has_rva)
	{
		HCARva *rva = (HCARva *)ptr;
		
		rva->signature = RVA_SIGNATURE;
		copy_float(&rva->volume, rva->volume);
		
		ptr += sizeof(HCARva);
	}
	
	if (has_comm)
	{
		HCAComm *comm = (HCAComm *)ptr;
		
		comm->signature = COMM_SIGNATURE;
        comm->len = (uint32_t)this->comm.length()+1;
		ptr += sizeof(HCAComm);
		
		strcpy((char *)ptr, this->comm.c_str());
		ptr += this->comm.length()+1;
	}
	
	if (has_pad)
	{
		*(uint32_t *)ptr = PAD_SIGNATURE;
		ptr += sizeof(uint32_t);
	}
	
	*(uint16_t *)(buf+header_size-2) = val16(CheckSum(buf, header_size-2));	
	ptr = buf+header_size;
	
	for (uint32_t i = 0; i < this->fmt.block_count; i++)
	{
		memcpy(ptr, raw_data+(i*block_size), block_size);
		*(uint16_t *)(ptr+block_size-2) = val16(CheckSum(ptr, block_size-2));
		
		ptr += block_size;
	}
	
	*psize = file_size;
    return buf;
}

uint8_t *HcaFile::Decode(int *format, size_t *psize)
{
    float *buf, *bottom, *ptr;
    uint32_t num_samples = 0x80*8*fmt.block_count*fmt.num_channels;

    buf = new float[num_samples];
    ptr = buf;
    bottom = buf + num_samples;

    if (ciph_type == 1)
    {
        InitCipher1();
    }
    else if (ciph_type == 56)
    {
        InitCipher56();
    }

    if (!InitDecoder())
        return nullptr;

    for (uint32_t n = 0; n < fmt.block_count; n++)
    {
        if (!DecodeBlock(raw_data+block_size*n, block_size))
            return nullptr;

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 0x80; j++)
            {
                for (unsigned int k = 0; k < fmt.num_channels; k++)
                {
                    float f = channels[k].wave[i][j] * volume;

                    if(f > 1)
                    {
                        f = 1;
                    }
                    else if (f < -1)
                    {
                        f = -1;
                    }

                    assert(ptr < bottom);
                    *(ptr++) = f;
                }
            }
        }
    }

    *format = AUDIO_FORMAT_FLOAT;
    *psize = num_samples*sizeof(float);
    return (uint8_t *)buf;
}

int HcaFile::default_quality = 0;
int HcaFile::default_cutoff = 0;

#ifdef ENCODER_PROGRAM

bool HcaFile::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    WavFile wav;

    if (!wav.Encode(buf, size, format, num_channels, sample_rate, take_ownership))
        return false;

    std::string temp = Utils::GetTempFileLocal(".wav");
    if (!wav.SaveToFile(temp.c_str(), true, true))
        return false;

    bool ret = EncodeFromWav(temp, default_quality, default_cutoff);

    Utils::RemoveFile(temp);
    return ret;
}

#else

bool HcaFile::Encode(uint8_t *, size_t, int, uint16_t, uint32_t, bool)
{
    DPRINTF("%s: You are going to have to provide an implementation for this.", FUNCNAME);
    return false;
}


#endif

bool HcaFile::SetLoop(float start, float end, int count)
{
    if (start >= end || count >= 128)
        return false;

    uint32_t temp_start = lrint((start*(float)fmt.sample_rate) / 1024.0f);
    uint32_t temp_end = lrint((end*(float)fmt.sample_rate) / 1024.0f);

    if (temp_end >= fmt.block_count)
        temp_end = fmt.block_count-1;

    if (temp_start >= temp_end)
        return false;

    loop.loop_start = temp_start;
    loop.loop_end = temp_end;
    loop.r01 = (count == 0) ? 0x80 : count;
    loop.r02 = 0x400;
    has_loop = true;

    return true;
}

bool HcaFile::SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count)
{
    if (sample_start >= sample_end || count >= 128)
        return false;

    uint32_t temp_start = lrint((float)sample_start / 1024.0);
    uint32_t temp_end = lrint((float)sample_end / 1024.0);

    if (temp_end >= fmt.block_count)
        temp_end = fmt.block_count-1;

    if (temp_start >= temp_end)
        return false;

    loop.loop_start = temp_start;
    loop.loop_end = temp_end;
    loop.r01 = (count == 0) ? 0x80 : count;
    loop.r02 = 0x400;
    has_loop = true;

    return true;
}

void HcaFile::InitCipher1()
{
	for (int i = 1, v = 0; i < 0xFF; i++)
	{
		v = (v*13+11)&0xFF;
		if (v == 0 || v == 0xFF)
			v = (v*13+11)&0xFF;
		
		cipher_table[i]=v;
	}
	
	cipher_table[0] = 0;
	cipher_table[0xFF] = 0xFF;
}

static void Init56_CreateTable(unsigned char *r,unsigned char key)
{
	int mul = ((key&1)<<3) | 5;
	int add = (key&0xE) | 1;
	key >>= 4;
	
	for (int i = 0; i < 0x10; i++)
	{
		key = (key*mul+add)&0xF;
		*(r++) = key;
	}
}

void HcaFile::InitCipher56()
{
	uint8_t t1[8];

    if (cipher56_inited)
        return;

    cipher56_inited = true;
	
	if (!cipher56_key1)
		cipher56_key2--;
	
	cipher56_key1--;
	
	for (int i = 0; i < 7; i++)
	{
		t1[i] = cipher56_key1;
		cipher56_key1 = (cipher56_key1 >> 8) | (cipher56_key2 << 24);
		cipher56_key2 >>= 8;
	}

	uint8_t t2[0x10] =
	{
		t1[1], (uint8_t)(t1[1]^t1[6]),
		(uint8_t)(t1[2]^t1[3]), t1[2],
		(uint8_t)(t1[2]^t1[1]), (uint8_t)(t1[3]^t1[4]),
		t1[3], (uint8_t)(t1[3]^t1[2]),
		(uint8_t)(t1[4]^t1[5]), t1[4],
		(uint8_t)(t1[4]^t1[3]), (uint8_t)(t1[5]^t1[6]),
		t1[5], (uint8_t)(t1[5]^t1[4]),
		(uint8_t)(t1[6]^t1[1]), t1[6],
	};

	uint8_t t3[0x100], t31[0x10], t32[0x10], *t=t3;
	
	Init56_CreateTable(t31, t1[0]);	
	for (int i = 0; i < 0x10; i++)
	{
		Init56_CreateTable(t32, t2[i]);
		uint8_t v = t31[i]<<4;
		
		for(int j = 0; j < 0x10; j++)
		{
			*(t++) = v | t32[j];
		}
	}

	t = &cipher_table[1];
	for (int i = 0, v = 0; i < 0x100; i++)
	{
		v = (v+0x11)&0xFF;
		uint8_t a = t3[v];
		
		if (a!=0 && a!=0xFF)
			*(t++)=a;
	}
	
	cipher_table[0]=0;
	cipher_table[0xFF]=0xFF;
}

static void ReverseTable(uint8_t *table)
{
	uint8_t temp[0x100];
	
	memcpy(temp, table, 0x100);
	
	for (int i = 0; i < 0x100; i++)
	{
		for (int j = 0; j < 0x100; j++)
		{
			if (temp[j] == i)
			{
				table[i] = j;
				break;
			}
		}
	}
}

void HcaFile::InitCipher1Rev()
{
	InitCipher1();
	ReverseTable(cipher_table);
}

void HcaFile::InitCipher56Rev()
{
	InitCipher56();
	ReverseTable(cipher_table);
}

void HcaFile::Mask(uint8_t *buf, unsigned int size)
{
	for (unsigned int i = 0; i < size; i++)
	{
		buf[i] = cipher_table[buf[i]];
	}
}

void HcaFile::InitAth1(uint32_t key)
{
	static const uint8_t list[]=
	{
		0x78,0x5F,0x56,0x51,0x4E,0x4C,0x4B,0x49,0x48,0x48,0x47,0x46,0x46,0x45,0x45,0x45,
		0x44,0x44,0x44,0x44,0x43,0x43,0x43,0x43,0x43,0x43,0x42,0x42,0x42,0x42,0x42,0x42,
		0x42,0x42,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x40,0x40,0x40,0x40,
		0x40,0x40,0x40,0x40,0x40,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
		0x3F,0x3F,0x3F,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,
		0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,
		0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,
		0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3B,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,0x3C,
		0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3D,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3E,0x3F,
		0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,
		0x3F,0x3F,0x3F,0x3F,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
		0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
		0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x41,
		0x41,0x41,0x41,0x41,0x41,0x41,0x41,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,
		0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x42,0x43,0x43,0x43,
		0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x43,0x44,0x44,
		0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x45,0x45,0x45,0x45,
		0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x45,0x46,0x46,0x46,0x46,0x46,0x46,0x46,0x46,
		0x46,0x46,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x47,0x48,0x48,0x48,0x48,
		0x48,0x48,0x48,0x48,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x49,0x4A,0x4A,0x4A,0x4A,
		0x4A,0x4A,0x4A,0x4A,0x4B,0x4B,0x4B,0x4B,0x4B,0x4B,0x4B,0x4C,0x4C,0x4C,0x4C,0x4C,
		0x4C,0x4D,0x4D,0x4D,0x4D,0x4D,0x4D,0x4E,0x4E,0x4E,0x4E,0x4E,0x4E,0x4F,0x4F,0x4F,
		0x4F,0x4F,0x4F,0x50,0x50,0x50,0x50,0x50,0x51,0x51,0x51,0x51,0x51,0x52,0x52,0x52,
		0x52,0x52,0x53,0x53,0x53,0x53,0x54,0x54,0x54,0x54,0x54,0x55,0x55,0x55,0x55,0x56,
		0x56,0x56,0x56,0x57,0x57,0x57,0x57,0x57,0x58,0x58,0x58,0x59,0x59,0x59,0x59,0x5A,
		0x5A,0x5A,0x5A,0x5B,0x5B,0x5B,0x5B,0x5C,0x5C,0x5C,0x5D,0x5D,0x5D,0x5D,0x5E,0x5E,
		0x5E,0x5F,0x5F,0x5F,0x60,0x60,0x60,0x61,0x61,0x61,0x61,0x62,0x62,0x62,0x63,0x63,
		0x63,0x64,0x64,0x64,0x65,0x65,0x66,0x66,0x66,0x67,0x67,0x67,0x68,0x68,0x68,0x69,
		0x69,0x6A,0x6A,0x6A,0x6B,0x6B,0x6B,0x6C,0x6C,0x6D,0x6D,0x6D,0x6E,0x6E,0x6F,0x6F,
		0x70,0x70,0x70,0x71,0x71,0x72,0x72,0x73,0x73,0x73,0x74,0x74,0x75,0x75,0x76,0x76,
		0x77,0x77,0x78,0x78,0x78,0x79,0x79,0x7A,0x7A,0x7B,0x7B,0x7C,0x7C,0x7D,0x7D,0x7E,
		0x7E,0x7F,0x7F,0x80,0x80,0x81,0x81,0x82,0x83,0x83,0x84,0x84,0x85,0x85,0x86,0x86,
		0x87,0x88,0x88,0x89,0x89,0x8A,0x8A,0x8B,0x8C,0x8C,0x8D,0x8D,0x8E,0x8F,0x8F,0x90,
		0x90,0x91,0x92,0x92,0x93,0x94,0x94,0x95,0x95,0x96,0x97,0x97,0x98,0x99,0x99,0x9A,
		0x9B,0x9B,0x9C,0x9D,0x9D,0x9E,0x9F,0xA0,0xA0,0xA1,0xA2,0xA2,0xA3,0xA4,0xA5,0xA5,
		0xA6,0xA7,0xA7,0xA8,0xA9,0xAA,0xAA,0xAB,0xAC,0xAD,0xAE,0xAE,0xAF,0xB0,0xB1,0xB1,
		0xB2,0xB3,0xB4,0xB5,0xB6,0xB6,0xB7,0xB8,0xB9,0xBA,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
		0xC0,0xC1,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xC9,0xCA,0xCB,0xCC,0xCD,
		0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,
		0xDE,0xDF,0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xED,0xEE,
		0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFF,0xFF,
	};
	
	for (unsigned int i = 0,v = 0; i < 0x80; i++, v += key)
	{
		unsigned int index=v>>13;
		
		if(index >= 0x28E)
		{
			memset(&ath_table[i], 0xFF, 0x80-i);
			break;
		}
		
		ath_table[i]= list[index];
	}
}

bool HcaFile::SetCiphType(uint16_t new_ciph_type)
{
	if (new_ciph_type == ciph_type)
		return true;
	
	if (new_ciph_type == 0)
	{
		if (ciph_type == 1)
		{
			// Decrypt 1 to 0
			InitCipher1();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else if (ciph_type == 56)
		{
			// Decrypt 56 to 0
			InitCipher56();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else
		{
			assert(0);
		}
	}
	else if (new_ciph_type == 1)
	{
		if (ciph_type == 0)
		{
			// Encrypt 0 to 1
			InitCipher1Rev();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else if (ciph_type == 56)
		{
			// Decrypt 56 to 0 + Encrypt 0 to 1
			InitCipher56();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
			
			InitCipher1Rev();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else
		{
			assert(0);
		}
	}
	else if (new_ciph_type == 56)
	{
		if (ciph_type == 0)
		{
			// Encrypt 0 to 56
			InitCipher56Rev();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else if (ciph_type == 1)
		{
			// Decrypt 1 to 0 and Encrypt 0 to 56
			InitCipher1();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
			
			InitCipher56Rev();
			
			for (uint32_t i = 0; i < fmt.block_count; i++)
			{
				Mask(raw_data+i*block_size, block_size-2);
			}
		}
		else
		{
			assert(0);
		}
	}
	else
	{
		return false;
	}
	
	ciph_type = new_ciph_type;
	has_ciph = true;
	
	return true;
} 

bool HcaFile::InitDecoder() 
{	
	if (ath_type == 0)
	{
		memset(ath_table, 0, sizeof(ath_table));
	}
	else if (ath_type == 1)
	{
		InitAth1(fmt.sample_rate);
	}
	else
	{
		DPRINTF("%s: Unknown ath type, 0x%x aborting decoder.\n", FUNCNAME, ath_type);
		return false;
	}
	
	if (!comp_r03)
		comp_r03 = 1;

	memset(channels, 0, sizeof(channels));
	if(!(comp_r01 == 1 && comp_r02 == 15))
		return false;
	
	comp_r09 = ceil2(comp_r05 - (comp_r06+comp_r07), comp_r08);
	
	char r[0x10];
	memset(r, 0, sizeof(r));	
	unsigned int b = fmt.num_channels / comp_r03;
	
	if(comp_r07 && b > 1)
	{
		char *c=r;
		for(unsigned int i = 0; i < comp_r03; i++, c+=b)
		{
			switch(b)
			{
				case 2:
					c[0] = 1; c[1] = 2;
				break;
				
				case 3:
					c[0] = 1; c[1] = 2;
				break;
				
				case 4:				
					c[0] = 1; c[1] = 2;
					if (comp_r04 == 0)
					{
						c[2] = 1; c[3] = 2;
					}
				break;
				
				case 5:				
					c[0]=1;c[1]=2;
					if(comp_r04 <= 2)
					{
						c[3]=1;c[4]=2;
					}					
				break;
				
				case 6:
					c[0] =1 ; c[1] = 2; c[4] = 1; c[5] = 2;
				break;
				
				case 7:
					c[0] = 1; c[1] = 2; c[4] = 1; c[5] = 2;
				break;
				
				case 8:
					c[0] =1 ; c[1] = 2; c[4] = 1; c[5] = 2; c[6] = 1; c[7] = 2;
				break;
			}
		}
	}
	
	for (unsigned int i = 0; i < fmt.num_channels; i++)
	{
		channels[i].type = r[i];
		channels[i].value3 = &channels[i].value[comp_r06+comp_r07];
		channels[i].count = comp_r06 + ((r[i] != 2) ? comp_r07: 0);
	}
	
	return true;
}	

bool HcaFile::DecodeToWav(const std::string &file, bool preserve_loop, int format)
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
    wav.SetNumChannels(fmt.num_channels);
    wav.SetSampleRate(fmt.sample_rate);
    wav.SetBitDepth((decode_as_float) ? 32 : format);
	
	if (has_loop && preserve_loop)
    {
        // This is what HCA decoder did, but HCA decoder sets the smpl data wrongly!
        //wav.SetLoopRaw(loop.loop_start*0x80*8*wav.GetSampleSize(), loop.loop_end*0x80*8*wav.GetSampleSize(), (loop.r01 == 0x80) ? 0 : loop.r01);

        // Better like this, bytes offset is not bytes offset, but samples offset...
        wav.SetLoopSample(loop.loop_start*0x80*8, loop.loop_end*0x80*8, (loop.r01 == 0x80) ? 0 : loop.r01);
	}

    uint32_t samples_size = 0x80*8*fmt.block_count*fmt.num_channels;

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
	
	if (ciph_type == 1)
	{
		InitCipher1();
	}
	else if (ciph_type == 56)
	{
		InitCipher56();
	}

	if (!InitDecoder())
    {
        fclose(w_handle);
        return false;
    }

    uint8_t *mem, *ptr, *bottom;

    mem = new uint8_t[samples_size];
    ptr = mem;
    bottom = mem + samples_size;
	
	for (uint32_t n = 0; n < fmt.block_count; n++)
	{
		if (!DecodeBlock(raw_data+block_size*n, block_size))
        {
            delete[] mem;
            fclose(w_handle);
            return false;
        }
		
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 0x80; j++)
			{
				for (unsigned int k = 0; k < fmt.num_channels; k++)
				{
					float f = channels[k].wave[i][j] * volume;
					
					if(f > 1)
					{
						f = 1;
					}
					else if (f < -1)
					{
						f = -1;
					}

                    assert(ptr < bottom);
					
					if (decode_as_float)
					{
                        memcpy(ptr, &f, sizeof(float));
                        ptr += sizeof(float);
					}
                    else if (format == 32)
					{
						int v = (int)(f*0x7FFFFFFF);
						
                        memcpy(ptr, &v, sizeof(uint32_t));
                        ptr += sizeof(uint32_t);
					}
                    else if (format == 24)
					{

#ifdef USING_SSE
                        int v = (int)((double)f*0x7FFFFF);
#else
                        int v = (int)(f*0x7FFFFF);
#endif
						
                        memcpy(ptr, &v, 3);
                        ptr += 3;
					}
                    else if (format == 16)
					{
#ifdef USING_SSE
                        int v = (int)((double)f*0x7FFF);
#else
						int v = (int)(f*0x7FFF);
#endif
						
                        memcpy(ptr, &v, 2);
                        ptr += 2;
					}
					else 
					{
						// 8 bits
#ifdef USING_SSE
                        int v = (int)((double)f*0x7F)+0x80;
#else
						int v = (int)(f*0x7F)+0x80;
#endif
						
                        memcpy(ptr, &v, 1);
                        ptr++;
					}
				}
			}
		}
	}

    bool ret = (fwrite(mem, 1, samples_size, w_handle) == samples_size);
	
    delete[] mem;
    fclose(w_handle);

    return ret;
}

#ifdef ENCODER_PROGRAM

bool HcaFile::EncodeFromWav(const std::string &file, int quality, int cutoff_freq, bool preserve_loop)
{
    std::string program = ENCODER_PROGRAM;
    std::string temp;
    std::vector<std::string> args;

    if (quality < 0 || quality > 4)
    {
        DPRINTF("%s: Bad parameter, quality.\n", FUNCNAME);
        return false;
    }

    //DPRINTF("cutoff %d\n", cutoff_freq);

    temp = Utils::GetTempFileLocal(".hca");

    args.push_back(file);
    args.push_back(temp);
    args.push_back(Utils::ToString(quality));
    args.push_back(Utils::ToString(cutoff_freq));;

    int ret = Utils::RunProgram(program, args);

    switch (ret)
    {
        case 0:
            if (!LoadFromFile(temp))
            {
                ret = -1;
            }
            else
            {
                if (!preserve_loop)
                    RemoveLoop();
            }
        break;

        case -1:
            DPRINTF("%s: %s or %s missing.\n", FUNCNAME, ENCODER_PROGRAM, ENCODER_DLL);
        break;

        case -2:
            DPRINTF("%s: Weird error.\n", FUNCNAME);
        break;

        case -3:
            DPRINTF("%s: Interesting and weird error.\n", FUNCNAME);
        break;

        case -4:
        {
            std::string error;
            Utils::ReadTextFile(ENCODER_ERROR, error, false);

            DPRINTF("%s: Encoder failed on file \"%s\".\n%s", FUNCNAME, file.c_str(), error.c_str());
        }
        break;

        default:
            DPRINTF("%s: Unknown error %d (0x%x).\n", FUNCNAME, ret, ret);
            //DPRINTF("%s %s %d %d\n", file.c_str(), temp.c_str(), quality, cutoff_freq);
    }

    Utils::RemoveFile(temp);
    return (ret == 0);
}

#else

bool HcaFile::EncodeFromWav(const std::string &, int, int)
{
    DPRINTF("%s: You are going to have to provide an implementation for this.", FUNCNAME);
    return false;
}

#endif

bool HcaFile::CheckHeader(const uint8_t *buf, size_t size, uint32_t *num_channels, uint32_t *sample_rate, uint32_t *block_count)
{
    FixedMemoryStream stream(const_cast<uint8_t *>(buf), size);
    HCAHeader *hdr;
    HCAFmt *fmt;

    if (!stream.FastRead((uint8_t **)&hdr, sizeof(HCAHeader)) || (hdr->signature & 0x7F7F7F7F) != HCA_SIGNATURE)
        return false;

    if (!stream.FastRead((uint8_t **)&fmt, sizeof(HCAFmt)) || (fmt->signature & 0x7F7F7F7F) != FMT_SIGNATURE)
        return false;

    if (num_channels)
        *num_channels = fmt->num_channels;

    if (sample_rate)
        *sample_rate = ((fmt->sample_rate&0xFF) << 16) | (fmt->sample_rate & 0x00FF00) | ((fmt->sample_rate & 0xFF0000) >> 16);

    if (block_count)
        *block_count = be32(fmt->block_count);

    return true;
}

bool HcaFile::CheckHeader(const std::string &file, uint32_t *num_channels, uint32_t *sample_rate, uint32_t *block_count)
{
    FileStream stream("rb");
    HCAHeader hdr;
    HCAFmt fmt;

    if (!stream.LoadFromFile(file))
        return false;

    if (!stream.Read(&hdr, sizeof(HCAHeader)) || (hdr.signature & 0x7F7F7F7F) != HCA_SIGNATURE)
        return false;

    if (!stream.Read(&fmt, sizeof(HCAFmt)) || (fmt.signature & 0x7F7F7F7F) != FMT_SIGNATURE)
        return false;

    if (num_channels)
        *num_channels = fmt.num_channels;

    if (sample_rate)
        *sample_rate = ((fmt.sample_rate&0xFF) << 16) | (fmt.sample_rate & 0x00FF00) | ((fmt.sample_rate & 0xFF0000) >> 16);

    if (block_count)
        *block_count = re32(fmt.block_count);

    return true;
}

bool HcaFile::DecodeBlock(const uint8_t *buf, unsigned int size)
{
	if (size == 0)
		return false;
	
    uint8_t *data = new uint8_t[size];
	memcpy(data, buf, size);
	
	if (ciph_type != 0)
		Mask(data, size);
	
	clData d(data, size);
	int magic = d.GetBit(16);
	
	if (magic == 0xFFFF)
	{
		int a = (d.GetBit(9) << 8) - d.GetBit(7);
		
		for (unsigned int i = 0; i< fmt.num_channels; i++)
			channels[i].Decode1(&d, comp_r09, a, ath_table);
		
		for (int i = 0; i < 8; i++)
		{
			for (unsigned int j = 0; j < fmt.num_channels; j++)
				channels[j].Decode2(&d);
			
			for (unsigned int j = 0; j < fmt.num_channels; j++)
				channels[j].Decode3(comp_r09, comp_r08, comp_r07+comp_r06, comp_r05);
			
			for (unsigned int j = 0; j < (unsigned int)(fmt.num_channels-1); j++)
				channels[j].Decode4(i, comp_r05-comp_r06, comp_r06, comp_r07, channels[j+1]);
			
			for (unsigned int j = 0; j < fmt.num_channels; j++)
				channels[j].Decode5(i);
		}
	}
	
	delete[] data;
	return true;
}

int HcaFile::clData::CheckBit(int bitSize)
{
	int v=0;
	
	if (_bit+bitSize<=_size)
	{
		static int mask[] = {0xFFFFFF,0x7FFFFF,0x3FFFFF,0x1FFFFF,0x0FFFFF,0x07FFFF,0x03FFFF,0x01FFFF};
		uint8_t *data = &_data[_bit>>3];
		
		v = data[0]; v = (v<<8) | data[1]; v = (v<<8) | data[2];
		v &= mask[_bit&7];
		v >>= 24 - (_bit&7) - bitSize;
	}
	
	return v;
}
int HcaFile::clData::GetBit(int bitSize)
{
	int v = CheckBit(bitSize);
	
	_bit += bitSize;
	return v;
}
void HcaFile::clData::AddBit(int bitSize)
{
	_bit += bitSize;
}

#ifdef USING_SSE

void HcaFile::Channel::Decode1(clData *data, unsigned int a, int b, uint8_t *ath)
{
	static const uint8_t scalelist[] =
	{
		// v2.0
		0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
		0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
		0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
		0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
		0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
		0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
		0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
		0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		// v1.3
		//0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
		//0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
		//0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
		//0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
		//0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
		//0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
		//0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
		//0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
	};
	
	static const unsigned int valueInt[] =
	{
		0x342A8D26,0x34633F89,0x3497657D,0x34C9B9BE,0x35066491,0x353311C4,0x356E9910,0x359EF532,
		0x35D3CCF1,0x360D1ADF,0x363C034A,0x367A83B3,0x36A6E595,0x36DE60F5,0x371426FF,0x3745672A,
		0x37838359,0x37AF3B79,0x37E97C38,0x381B8D3A,0x384F4319,0x388A14D5,0x38B7FBF0,0x38F5257D,
		0x3923520F,0x39599D16,0x3990FA4D,0x39C12C4D,0x3A00B1ED,0x3A2B7A3A,0x3A647B6D,0x3A9837F0,
		0x3ACAD226,0x3B071F62,0x3B340AAF,0x3B6FE4BA,0x3B9FD228,0x3BD4F35B,0x3C0DDF04,0x3C3D08A4,
		0x3C7BDFED,0x3CA7CD94,0x3CDF9613,0x3D14F4F0,0x3D467991,0x3D843A29,0x3DB02F0E,0x3DEAC0C7,
		0x3E1C6573,0x3E506334,0x3E8AD4C6,0x3EB8FBAF,0x3EF67A41,0x3F243516,0x3F5ACB94,0x3F91C3D3,
		0x3FC238D2,0x400164D2,0x402C6897,0x4065B907,0x40990B88,0x40CBEC15,0x4107DB35,0x413504F3,
	};
	
	static const unsigned int scaleInt[] = 
	{
		0x00000000,0x3F2AAAAB,0x3ECCCCCD,0x3E924925,0x3E638E39,0x3E3A2E8C,0x3E1D89D9,0x3E088889,
		0x3D842108,0x3D020821,0x3C810204,0x3C008081,0x3B804020,0x3B002008,0x3A801002,0x3A000801,
	};
	
	static const float *valueFloat = (float *)valueInt;
	static const float *scaleFloat = (float *)scaleInt;
	int v = data->GetBit(3);
	
	if(v >= 6)
	{
		for (unsigned int i = 0; i < count; i++)
			value[i] = data->GetBit(6);
	}
	else if(v)
	{
		int v1 = data->GetBit(6), v2 = (1<<v)-1, v3 = v2>>1, v4;
		
		value[0] = v1;
		for(unsigned int i = 1; i < count; i++)
		{
			v4 = data->GetBit(v);
			
			if (v4 != v2)
			{
				v1 += v4-v3;
			}
			else
			{
				v1=data->GetBit(6);
			}
			
			value[i]=v1;
		}
	}
	else
	{
		memset(value, 0, 0x80);
	}
	
	if(type==2)
	{
		v = data->CheckBit(4); value2[0] = v;
		
		if(v<15)
			for (int i=0; i<8; i++)
				value2[i]=data->GetBit(4);
	}
	else
	{
		for (unsigned int i = 0; i<a; i++)
			value3[i] = data->GetBit(6);
	}
	
	for (unsigned int i = 0; i < count; i++)
	{
		v = value[i];
		
		if (v)
		{
			v = ath[i]+((b+i)>>8)-((v*5)>>1)+1;
			
			if (v < 0)
				v = 15;
			else if (v >= 0x39)
				v = 1;
			else
				v = scalelist[v];
		}
		
		scale[i]=v;
	}
	
	memset(&scale[count], 0, 0x80-count);
	for (unsigned int i = 0; i < count; i++)
        base[i] = (float)((double)valueFloat[(int)value[i]]*(double)scaleFloat[(int)scale[i]]);
}

void HcaFile::Channel::Decode2(clData *data)
{
	static const char list1[] =
	{
		0,2,3,3,4,4,4,4,5,6,7,8,9,10,11,12,
	};
	
	static const char list2[] = 
	{
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,
		2,2,2,2,2,2,3,3,0,0,0,0,0,0,0,0,
		2,2,3,3,3,3,3,3,0,0,0,0,0,0,0,0,
		3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,
		3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,
		3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,
		3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
	};
	
	static const float list3[] =
	{
		+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
		+0,+0,+1,-1,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
		+0,+0,+1,+1,-1,-1,+2,-2,+0,+0,+0,+0,+0,+0,+0,+0,
		+0,+0,+1,-1,+2,-2,+3,-3,+0,+0,+0,+0,+0,+0,+0,+0,
		+0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,+3,-3,-3,+4,-4,
		+0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,-3,+4,-4,+5,-5,
		+0,+0,+1,+1,-1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,
		+0,+0,+1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,+7,-7,
	};
	
	for (unsigned int i = 0; i < count; i++)
	{
		float f;
		int s = scale[i];
		int bitSize = list1[s];
		int v = data->GetBit(bitSize);
		
		if (s < 8)
		{
			v += s<<4;
			data->AddBit(list2[v]-bitSize);
            f = list3[v];
		}
		else
		{
			v = (1-((v&1)<<1))*(v>>1);
			
			if (!v)
				data->AddBit(-1);
			
            f=(float)v;
		}
		
        block[i] = (float)((double)base[i]*(double)f);
	}
	
	memset(&block[count], 0, sizeof(float)*(0x80-count));
}

void HcaFile::Channel::Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
	if(type!=2&&b)
	{
		static const unsigned int listInt[2][0x40]=
		{
			{
				0x00000000,0x00000000,0x32A0B051,0x32D61B5E,0x330EA43A,0x333E0F68,0x337D3E0C,0x33A8B6D5,
				0x33E0CCDF,0x3415C3FF,0x34478D75,0x3484F1F6,0x34B123F6,0x34EC0719,0x351D3EDA,0x355184DF,
				0x358B95C2,0x35B9FCD2,0x35F7D0DF,0x36251958,0x365BFBB8,0x36928E72,0x36C346CD,0x370218AF,
				0x372D583F,0x3766F85B,0x3799E046,0x37CD078C,0x3808980F,0x38360094,0x38728177,0x38A18FAF,
				0x38D744FD,0x390F6A81,0x393F179A,0x397E9E11,0x39A9A15B,0x39E2055B,0x3A16942D,0x3A48A2D8,
				0x3A85AAC3,0x3AB21A32,0x3AED4F30,0x3B1E196E,0x3B52A81E,0x3B8C57CA,0x3BBAFF5B,0x3BF9295A,
				0x3C25FED7,0x3C5D2D82,0x3C935A2B,0x3CC4563F,0x3D02CD87,0x3D2E4934,0x3D68396A,0x3D9AB62B,
				0x3DCE248C,0x3E0955EE,0x3E36FD92,0x3E73D290,0x3EA27043,0x3ED87039,0x3F1031DC,0x3F40213B,
			},
			{
				0x3F800000,0x3FAA8D26,0x3FE33F89,0x4017657D,0x4049B9BE,0x40866491,0x40B311C4,0x40EE9910,
				0x411EF532,0x4153CCF1,0x418D1ADF,0x41BC034A,0x41FA83B3,0x4226E595,0x425E60F5,0x429426FF,
				0x42C5672A,0x43038359,0x432F3B79,0x43697C38,0x439B8D3A,0x43CF4319,0x440A14D5,0x4437FBF0,
				0x4475257D,0x44A3520F,0x44D99D16,0x4510FA4D,0x45412C4D,0x4580B1ED,0x45AB7A3A,0x45E47B6D,
				0x461837F0,0x464AD226,0x46871F62,0x46B40AAF,0x46EFE4BA,0x471FD228,0x4754F35B,0x478DDF04,
				0x47BD08A4,0x47FBDFED,0x4827CD94,0x485F9613,0x4894F4F0,0x48C67991,0x49043A29,0x49302F0E,
				0x496AC0C7,0x499C6573,0x49D06334,0x4A0AD4C6,0x4A38FBAF,0x4A767A41,0x4AA43516,0x4ADACB94,
				0x4B11C3D3,0x4B4238D2,0x4B8164D2,0x4BAC6897,0x4BE5B907,0x4C190B88,0x4C4BEC15,0x00000000,
			}
		};
		
		static const float *listFloat=(float *)listInt[1];
		
		for (unsigned int i = 0, k = c, l = c-1; i < a; i++)
		{
			for (unsigned int j = 0; j < b && k < d; j++,l--)
			{
                block[k++] = (float)((double)listFloat[value3[i]-value[l]]*(double)block[l]);
			}
		}
		
		block[0x80-1] = 0;
	}
}

void HcaFile::Channel::Decode4(int index, unsigned int a, unsigned int b, unsigned int c, Channel &next)
{
	if(type == 1 && c)
	{
		static const unsigned int listInt[] =
		{
			// v2.0
			0x40000000,0x3FEDB6DB,0x3FDB6DB7,0x3FC92492,0x3FB6DB6E,0x3FA49249,0x3F924925,0x3F800000,
			0x3F5B6DB7,0x3F36DB6E,0x3F124925,0x3EDB6DB7,0x3E924925,0x3E124925,0x00000000,0x00000000,
			0x00000000,0x32A0B051,0x32D61B5E,0x330EA43A,0x333E0F68,0x337D3E0C,0x33A8B6D5,0x33E0CCDF,
			0x3415C3FF,0x34478D75,0x3484F1F6,0x34B123F6,0x34EC0719,0x351D3EDA,0x355184DF,0x358B95C2,
			0x35B9FCD2,0x35F7D0DF,0x36251958,0x365BFBB8,0x36928E72,0x36C346CD,0x370218AF,0x372D583F,
			0x3766F85B,0x3799E046,0x37CD078C,0x3808980F,0x38360094,0x38728177,0x38A18FAF,0x38D744FD,
			0x390F6A81,0x393F179A,0x397E9E11,0x39A9A15B,0x39E2055B,0x3A16942D,0x3A48A2D8,0x3A85AAC3,
			0x3AB21A32,0x3AED4F30,0x3B1E196E,0x3B52A81E,0x3B8C57CA,0x3BBAFF5B,0x3BF9295A,0x3C25FED7,
			//???2??????
			0x3C5D2D82,0x3C935A2B,0x3CC4563F,0x3D02CD87,0x3D2E4934,0x3D68396A,0x3D9AB62B,0x3DCE248C,
			0x3E0955EE,0x3E36FD92,0x3E73D290,0x3EA27043,0x3ED87039,0x3F1031DC,0x3F40213B,0x00000000,
			// v1.3
			//0x40000000,0x3FEDB6DB,0x3FDB6DB7,0x3FC92492,0x3FB6DB6E,0x3FA49249,0x3F924925,0x3F800000,
			//0x3F5B6DB7,0x3F36DB6E,0x3F124925,0x3EDB6DB7,0x3E924925,0x3E124925,0x00000000,0x00000000,
		};
		
		float f1 = ((float *)listInt)[(int)next.value2[index]];
        float f2 = (float)((double)f1-(double)2.0f);
		float *s = &block[b];
		float *d = &next.block[b];
		
        for (unsigned int i = 0; i < a; i++)
		{
            *(d++) = (float)((double)*s * (double)f2);
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
#endif
            *(s++) = (float)((double)*s * (double)f1);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
		}
	}
}

void HcaFile::Channel::Decode5(int index)
{
	static const unsigned int list1Int[7][0x40] =
	{
		{
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
			0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
		},
		{
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
			0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
		},
		{
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
			0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
		},
		{
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
			0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
		},
		{
			0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
			0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
			0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
			0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
			0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
			0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
			0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
			0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
		},
		{
			0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
			0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
			0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
			0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
			0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
			0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
			0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
			0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
		},
		{
			0x3F7FFEC4,0x3F7FF4E6,0x3F7FE129,0x3F7FC38F,0x3F7F9C18,0x3F7F6AC7,0x3F7F2F9D,0x3F7EEA9D,
			0x3F7E9BC9,0x3F7E4323,0x3F7DE0B1,0x3F7D7474,0x3F7CFE73,0x3F7C7EB0,0x3F7BF531,0x3F7B61FC,
			0x3F7AC516,0x3F7A1E84,0x3F796E4E,0x3F78B47B,0x3F77F110,0x3F772417,0x3F764D97,0x3F756D97,
			0x3F748422,0x3F73913F,0x3F7294F8,0x3F718F57,0x3F708066,0x3F6F6830,0x3F6E46BE,0x3F6D1C1D,
			0x3F6BE858,0x3F6AAB7B,0x3F696591,0x3F6816A8,0x3F66BECC,0x3F655E0B,0x3F63F473,0x3F628210,
			0x3F6106F2,0x3F5F8327,0x3F5DF6BE,0x3F5C61C7,0x3F5AC450,0x3F591E6A,0x3F577026,0x3F55B993,
			0x3F53FAC3,0x3F5233C6,0x3F5064AF,0x3F4E8D90,0x3F4CAE79,0x3F4AC77F,0x3F48D8B3,0x3F46E22A,
			0x3F44E3F5,0x3F42DE29,0x3F40D0DA,0x3F3EBC1B,0x3F3CA003,0x3F3A7CA4,0x3F385216,0x3F36206C,
		}
	};
	
	static const unsigned int list2Int[7][0x40] = 
	{
		{
			0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
			0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
			0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
			0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
			0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
			0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
			0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
			0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
		},
		{
			0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
			0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
			0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
			0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
			0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
			0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
			0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
			0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
		},
		{
			0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
			0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
			0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
			0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
			0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
			0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
			0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
			0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
		},
		{
			0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
			0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
			0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
			0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
			0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
			0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
			0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
			0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
		},
		{
			0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
			0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
			0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
			0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
			0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
			0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
			0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
			0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
		},
		{
			0xBC490E90,0xBD16C32C,0xBD7B2B74,0xBDAFB680,0xBDE1BC2E,0xBE09CF86,0xBE22ABB6,0xBE3B6ECF,
			0xBE541501,0xBE6C9A7F,0xBE827DC0,0xBE8E9A22,0xBE9AA086,0xBEA68F12,0xBEB263EF,0xBEBE1D4A,
			0xBEC9B953,0xBED53641,0xBEE0924F,0xBEEBCBBB,0xBEF6E0CB,0xBF00E7E4,0xBF064B82,0xBF0B9A6B,
			0xBF10D3CD,0xBF15F6D9,0xBF1B02C6,0xBF1FF6CB,0xBF24D225,0xBF299415,0xBF2E3BDE,0xBF32C8C9,
			0x3C490E90,0x3D16C32C,0x3D7B2B74,0x3DAFB680,0x3DE1BC2E,0x3E09CF86,0x3E22ABB6,0x3E3B6ECF,
			0x3E541501,0x3E6C9A7F,0x3E827DC0,0x3E8E9A22,0x3E9AA086,0x3EA68F12,0x3EB263EF,0x3EBE1D4A,
			0x3EC9B953,0x3ED53641,0x3EE0924F,0x3EEBCBBB,0x3EF6E0CB,0x3F00E7E4,0x3F064B82,0x3F0B9A6B,
			0x3F10D3CD,0x3F15F6D9,0x3F1B02C6,0x3F1FF6CB,0x3F24D225,0x3F299415,0x3F2E3BDE,0x3F32C8C9,
		},
		{
			0xBBC90F88,0xBC96C9B6,0xBCFB49BA,0xBD2FE007,0xBD621469,0xBD8A200A,0xBDA3308C,0xBDBC3AC3,
			0xBDD53DB9,0xBDEE3876,0xBE039502,0xBE1008B7,0xBE1C76DE,0xBE28DEFC,0xBE354098,0xBE419B37,
			0xBE4DEE60,0xBE5A3997,0xBE667C66,0xBE72B651,0xBE7EE6E1,0xBE8586CE,0xBE8B9507,0xBE919DDD,
			0xBE97A117,0xBE9D9E78,0xBEA395C5,0xBEA986C4,0xBEAF713A,0xBEB554EC,0xBEBB31A0,0xBEC1071E,
			0xBEC6D529,0xBECC9B8B,0xBED25A09,0xBED8106B,0xBEDDBE79,0xBEE363FA,0xBEE900B7,0xBEEE9479,
			0xBEF41F07,0xBEF9A02D,0xBEFF17B2,0xBF0242B1,0xBF04F484,0xBF07A136,0xBF0A48AD,0xBF0CEAD0,
			0xBF0F8784,0xBF121EB0,0xBF14B039,0xBF173C07,0xBF19C200,0xBF1C420C,0xBF1EBC12,0xBF212FF9,
			0xBF239DA9,0xBF26050A,0xBF286605,0xBF2AC082,0xBF2D1469,0xBF2F61A5,0xBF31A81D,0xBF33E7BC,
		}
	};
	
	static const unsigned int list3Int[2][0x40] = 
	{
		{
			0x3A3504F0,0x3B0183B8,0x3B70C538,0x3BBB9268,0x3C04A809,0x3C308200,0x3C61284C,0x3C8B3F17,
			0x3CA83992,0x3CC77FBD,0x3CE91110,0x3D0677CD,0x3D198FC4,0x3D2DD35C,0x3D434643,0x3D59ECC1,
			0x3D71CBA8,0x3D85741E,0x3D92A413,0x3DA078B4,0x3DAEF522,0x3DBE1C9E,0x3DCDF27B,0x3DDE7A1D,
			0x3DEFB6ED,0x3E00D62B,0x3E0A2EDA,0x3E13E72A,0x3E1E00B1,0x3E287CF2,0x3E335D55,0x3E3EA321,
			0x3E4A4F75,0x3E56633F,0x3E62DF37,0x3E6FC3D1,0x3E7D1138,0x3E8563A2,0x3E8C72B7,0x3E93B561,
			0x3E9B2AEF,0x3EA2D26F,0x3EAAAAAB,0x3EB2B222,0x3EBAE706,0x3EC34737,0x3ECBD03D,0x3ED47F46,
			0x3EDD5128,0x3EE6425C,0x3EEF4EFF,0x3EF872D7,0x3F00D4A9,0x3F0576CA,0x3F0A1D3B,0x3F0EC548,
			0x3F136C25,0x3F180EF2,0x3F1CAAC2,0x3F213CA2,0x3F25C1A5,0x3F2A36E7,0x3F2E9998,0x3F32E705,
		},
		{
			0xBF371C9E,0xBF3B37FE,0xBF3F36F2,0xBF431780,0xBF46D7E6,0xBF4A76A4,0xBF4DF27C,0xBF514A6F,
			0xBF547DC5,0xBF578C03,0xBF5A74EE,0xBF5D3887,0xBF5FD707,0xBF6250DA,0xBF64A699,0xBF66D908,
			0xBF68E90E,0xBF6AD7B1,0xBF6CA611,0xBF6E5562,0xBF6FE6E7,0xBF715BEF,0xBF72B5D1,0xBF73F5E6,
			0xBF751D89,0xBF762E13,0xBF7728D7,0xBF780F20,0xBF78E234,0xBF79A34C,0xBF7A5397,0xBF7AF439,
			0xBF7B8648,0xBF7C0ACE,0xBF7C82C8,0xBF7CEF26,0xBF7D50CB,0xBF7DA88E,0xBF7DF737,0xBF7E3D86,
			0xBF7E7C2A,0xBF7EB3CC,0xBF7EE507,0xBF7F106C,0xBF7F3683,0xBF7F57CA,0xBF7F74B6,0xBF7F8DB6,
			0xBF7FA32E,0xBF7FB57B,0xBF7FC4F6,0xBF7FD1ED,0xBF7FDCAD,0xBF7FE579,0xBF7FEC90,0xBF7FF22E,
			0xBF7FF688,0xBF7FF9D0,0xBF7FFC32,0xBF7FFDDA,0xBF7FFEED,0xBF7FFF8F,0xBF7FFFDF,0xBF7FFFFC,
		}
	};
	
	float *s,*d,*s1,*s2;
	s = block; d = wav1;
	
	for (int i = 0, count1 = 1, count2 = 0x40; i < 7; i++, count1 <<=1, count2 >>= 1)
	{
		float *d1=d;
		float *d2=&d[count2];
		
		for (int j = 0; j < count1; j++)
		{
			for (int k = 0; k < count2; k++)
			{
				float a = *(s++);
				float b = *(s++);
                *(d1++) = (float)((double)b+(double)a);
                *(d2++) = (float)((double)a-(double)b);
			}
			
            d1 += count2;
			d2 += count2;
		}
		
		float *w =&s [-0x80]; s = d; d = w;
	}
	
	s = wav1; d = block;
	
	for (int i = 0, count1 = 0x40, count2 = 1; i < 7; i++, count1 >>= 1, count2 <<= 1)
	{
		float *list1Float = (float *)list1Int[i];
		float *list2Float = (float *)list2Int[i];
		float *s1 = s;
		float *s2 = &s1[count2];
		float *d1 = d;
		float *d2 = &d1[count2*2-1];
		
		for(int j = 0; j < count1; j++)
		{
			for(int k = 0; k < count2; k++)
			{
                float a = (float)((double)*(s1++));
                float b = (float)((double)*(s2++));
                float c = (float)((double)*(list1Float++));
                float d = (float)((double)*(list2Float++));
                *(d1++) = (float)((double)a*(double)c-(double)b*(double)d);
                *(d2--) = (float)((double)a*(double)d+(double)b*(double)c);
			}
			
			s1 += count2;
			s2 += count2;
			d1 += count2;
			d2 += count2*3;
		}
		
		float *w=s; s=d; d=w;
	}
	
	d = wav2;
	for (int i = 0; i < 0x80;i++)
        *(d++) = *(s++);
	
	s = (float *)list3Int; d = wave[index];
	s1 = &wav2[0x40]; s2 = wav3;
	
	for (int i = 0; i < 0x40; i++)
        *(d++) = (float)((double)*(s1++)*(double)*(s++)+(double)*(s2++));
	
	for(int i=0;i<0x40;i++)
        *(d++) = (float)((double)*(s++)*(double)*(--s1)-(double)*(s2++));
	
	s1 = &wav2[0x40-1]; s2 = wav3;	
	for (int i = 0; i < 0x40; i++)
        *(s2++) = (float)((double)*(s1--)*(double)*(--s));
	
	for (int i = 0; i < 0x40; i++)
        *(s2++) = (float)((double)*(--s)*(double)*(++s1));
}

#else

void HcaFile::Channel::Decode1(clData *data, unsigned int a, int b, uint8_t *ath)
{
    static const uint8_t scalelist[] =
    {
        // v2.0
        0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
        0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
        0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
        0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
        0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
        0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
        0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
        0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        // v1.3
        //0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0D,0x0D,
        //0x0D,0x0D,0x0D,0x0D,0x0C,0x0C,0x0C,0x0C,
        //0x0C,0x0C,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,
        //0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x09,
        //0x09,0x09,0x09,0x09,0x09,0x08,0x08,0x08,
        //0x08,0x08,0x08,0x07,0x06,0x06,0x05,0x04,
        //0x04,0x04,0x03,0x03,0x03,0x02,0x02,0x02,
        //0x02,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
    };

    static const unsigned int valueInt[] =
    {
        0x342A8D26,0x34633F89,0x3497657D,0x34C9B9BE,0x35066491,0x353311C4,0x356E9910,0x359EF532,
        0x35D3CCF1,0x360D1ADF,0x363C034A,0x367A83B3,0x36A6E595,0x36DE60F5,0x371426FF,0x3745672A,
        0x37838359,0x37AF3B79,0x37E97C38,0x381B8D3A,0x384F4319,0x388A14D5,0x38B7FBF0,0x38F5257D,
        0x3923520F,0x39599D16,0x3990FA4D,0x39C12C4D,0x3A00B1ED,0x3A2B7A3A,0x3A647B6D,0x3A9837F0,
        0x3ACAD226,0x3B071F62,0x3B340AAF,0x3B6FE4BA,0x3B9FD228,0x3BD4F35B,0x3C0DDF04,0x3C3D08A4,
        0x3C7BDFED,0x3CA7CD94,0x3CDF9613,0x3D14F4F0,0x3D467991,0x3D843A29,0x3DB02F0E,0x3DEAC0C7,
        0x3E1C6573,0x3E506334,0x3E8AD4C6,0x3EB8FBAF,0x3EF67A41,0x3F243516,0x3F5ACB94,0x3F91C3D3,
        0x3FC238D2,0x400164D2,0x402C6897,0x4065B907,0x40990B88,0x40CBEC15,0x4107DB35,0x413504F3,
    };

    static const unsigned int scaleInt[] =
    {
        0x00000000,0x3F2AAAAB,0x3ECCCCCD,0x3E924925,0x3E638E39,0x3E3A2E8C,0x3E1D89D9,0x3E088889,
        0x3D842108,0x3D020821,0x3C810204,0x3C008081,0x3B804020,0x3B002008,0x3A801002,0x3A000801,
    };

    static const float *valueFloat = (float *)valueInt;
    static const float *scaleFloat = (float *)scaleInt;
    int v = data->GetBit(3);

    if(v >= 6)
    {
        for (unsigned int i = 0; i < count; i++)
            value[i] = data->GetBit(6);
    }
    else if(v)
    {
        int v1 = data->GetBit(6), v2 = (1<<v)-1, v3 = v2>>1, v4;

        value[0] = v1;
        for(unsigned int i = 1; i < count; i++)
        {
            v4 = data->GetBit(v);

            if (v4 != v2)
            {
                v1 += v4-v3;
            }
            else
            {
                v1=data->GetBit(6);
            }

            value[i]=v1;
        }
    }
    else
    {
        memset(value, 0, 0x80);
    }

    if(type==2)
    {
        v = data->CheckBit(4); value2[0] = v;

        if(v<15)
            for (int i=0; i<8; i++)
                value2[i]=data->GetBit(4);
    }
    else
    {
        for (unsigned int i = 0; i<a; i++)
            value3[i] = data->GetBit(6);
    }

    for (unsigned int i = 0; i < count; i++)
    {
        v = value[i];

        if (v)
        {
            v = ath[i]+((b+i)>>8)-((v*5)>>1)+1;

            if (v < 0)
                v = 15;
            else if (v >= 0x39)
                v = 1;
            else
                v = scalelist[v];
        }

        scale[i]=v;
    }

    memset(&scale[count], 0, 0x80-count);
    for (unsigned int i = 0; i < count; i++)
        base[i] = valueFloat[(int)value[i]]*scaleFloat[(int)scale[i]];
}

void HcaFile::Channel::Decode2(clData *data)
{
    static const char list1[] =
    {
        0,2,3,3,4,4,4,4,5,6,7,8,9,10,11,12,
    };

    static const char list2[] =
    {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        1,1,2,2,0,0,0,0,0,0,0,0,0,0,0,0,
        2,2,2,2,2,2,3,3,0,0,0,0,0,0,0,0,
        2,2,3,3,3,3,3,3,0,0,0,0,0,0,0,0,
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,
        3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,
        3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,
        3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    };

    static const float list3[] =
    {
        +0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,-1,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,+1,-1,-1,+2,-2,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,-1,+2,-2,+3,-3,+0,+0,+0,+0,+0,+0,+0,+0,
        +0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,+3,-3,-3,+4,-4,
        +0,+0,+1,+1,-1,-1,+2,+2,-2,-2,+3,-3,+4,-4,+5,-5,
        +0,+0,+1,+1,-1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,
        +0,+0,+1,-1,+2,-2,+3,-3,+4,-4,+5,-5,+6,-6,+7,-7,
    };

    for (unsigned int i = 0; i < count; i++)
    {
        float f;
        int s = scale[i];
        int bitSize = list1[s];
        int v = data->GetBit(bitSize);

        if (s < 8)
        {
            v += s<<4;
            data->AddBit(list2[v]-bitSize);
            f = list3[v];
        }
        else
        {
            v = (1-((v&1)<<1))*(v>>1);

            if (!v)
                data->AddBit(-1);

            f=(float)v;
        }

        block[i] = base[i]*f;
    }

    memset(&block[count], 0, sizeof(float)*(0x80-count));
}

void HcaFile::Channel::Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d)
{
    if(type!=2&&b)
    {
        static const unsigned int listInt[2][0x40]=
        {
            {
                0x00000000,0x00000000,0x32A0B051,0x32D61B5E,0x330EA43A,0x333E0F68,0x337D3E0C,0x33A8B6D5,
                0x33E0CCDF,0x3415C3FF,0x34478D75,0x3484F1F6,0x34B123F6,0x34EC0719,0x351D3EDA,0x355184DF,
                0x358B95C2,0x35B9FCD2,0x35F7D0DF,0x36251958,0x365BFBB8,0x36928E72,0x36C346CD,0x370218AF,
                0x372D583F,0x3766F85B,0x3799E046,0x37CD078C,0x3808980F,0x38360094,0x38728177,0x38A18FAF,
                0x38D744FD,0x390F6A81,0x393F179A,0x397E9E11,0x39A9A15B,0x39E2055B,0x3A16942D,0x3A48A2D8,
                0x3A85AAC3,0x3AB21A32,0x3AED4F30,0x3B1E196E,0x3B52A81E,0x3B8C57CA,0x3BBAFF5B,0x3BF9295A,
                0x3C25FED7,0x3C5D2D82,0x3C935A2B,0x3CC4563F,0x3D02CD87,0x3D2E4934,0x3D68396A,0x3D9AB62B,
                0x3DCE248C,0x3E0955EE,0x3E36FD92,0x3E73D290,0x3EA27043,0x3ED87039,0x3F1031DC,0x3F40213B,
            },
            {
                0x3F800000,0x3FAA8D26,0x3FE33F89,0x4017657D,0x4049B9BE,0x40866491,0x40B311C4,0x40EE9910,
                0x411EF532,0x4153CCF1,0x418D1ADF,0x41BC034A,0x41FA83B3,0x4226E595,0x425E60F5,0x429426FF,
                0x42C5672A,0x43038359,0x432F3B79,0x43697C38,0x439B8D3A,0x43CF4319,0x440A14D5,0x4437FBF0,
                0x4475257D,0x44A3520F,0x44D99D16,0x4510FA4D,0x45412C4D,0x4580B1ED,0x45AB7A3A,0x45E47B6D,
                0x461837F0,0x464AD226,0x46871F62,0x46B40AAF,0x46EFE4BA,0x471FD228,0x4754F35B,0x478DDF04,
                0x47BD08A4,0x47FBDFED,0x4827CD94,0x485F9613,0x4894F4F0,0x48C67991,0x49043A29,0x49302F0E,
                0x496AC0C7,0x499C6573,0x49D06334,0x4A0AD4C6,0x4A38FBAF,0x4A767A41,0x4AA43516,0x4ADACB94,
                0x4B11C3D3,0x4B4238D2,0x4B8164D2,0x4BAC6897,0x4BE5B907,0x4C190B88,0x4C4BEC15,0x00000000,
            }
        };

        static const float *listFloat=(float *)listInt[1];

        for (unsigned int i = 0, k = c, l = c-1; i < a; i++)
        {
            for (unsigned int j = 0; j < b && k < d; j++,l--)
            {
                block[k++] = listFloat[value3[i]-value[l]]*block[l];
            }
        }

        block[0x80-1] = 0;
    }
}

void HcaFile::Channel::Decode4(int index, unsigned int a, unsigned int b, unsigned int c, Channel &next)
{
    if(type == 1 && c)
    {
        static const unsigned int listInt[] =
        {
            // v2.0
            0x40000000,0x3FEDB6DB,0x3FDB6DB7,0x3FC92492,0x3FB6DB6E,0x3FA49249,0x3F924925,0x3F800000,
            0x3F5B6DB7,0x3F36DB6E,0x3F124925,0x3EDB6DB7,0x3E924925,0x3E124925,0x00000000,0x00000000,
            0x00000000,0x32A0B051,0x32D61B5E,0x330EA43A,0x333E0F68,0x337D3E0C,0x33A8B6D5,0x33E0CCDF,
            0x3415C3FF,0x34478D75,0x3484F1F6,0x34B123F6,0x34EC0719,0x351D3EDA,0x355184DF,0x358B95C2,
            0x35B9FCD2,0x35F7D0DF,0x36251958,0x365BFBB8,0x36928E72,0x36C346CD,0x370218AF,0x372D583F,
            0x3766F85B,0x3799E046,0x37CD078C,0x3808980F,0x38360094,0x38728177,0x38A18FAF,0x38D744FD,
            0x390F6A81,0x393F179A,0x397E9E11,0x39A9A15B,0x39E2055B,0x3A16942D,0x3A48A2D8,0x3A85AAC3,
            0x3AB21A32,0x3AED4F30,0x3B1E196E,0x3B52A81E,0x3B8C57CA,0x3BBAFF5B,0x3BF9295A,0x3C25FED7,
            //???2??????
            0x3C5D2D82,0x3C935A2B,0x3CC4563F,0x3D02CD87,0x3D2E4934,0x3D68396A,0x3D9AB62B,0x3DCE248C,
            0x3E0955EE,0x3E36FD92,0x3E73D290,0x3EA27043,0x3ED87039,0x3F1031DC,0x3F40213B,0x00000000,
            // v1.3
            //0x40000000,0x3FEDB6DB,0x3FDB6DB7,0x3FC92492,0x3FB6DB6E,0x3FA49249,0x3F924925,0x3F800000,
            //0x3F5B6DB7,0x3F36DB6E,0x3F124925,0x3EDB6DB7,0x3E924925,0x3E124925,0x00000000,0x00000000,
        };

        float f1 = ((float *)listInt)[(int)next.value2[index]];
        float f2 = f1-2.0f;
        float *s = &block[b];
        float *d = &next.block[b];

        for (unsigned int i = 0; i < a; i++)
        {
            *(d++) = *s*f2;
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsequence-point"
#endif
            *(s++) = *s*f1;
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
        }
    }
}

void HcaFile::Channel::Decode5(int index)
{
    static const unsigned int list1Int[7][0x40] =
    {
        {
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
            0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,0x3DA73D75,
        },
        {
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
            0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,0x3F7B14BE,0x3F54DB31,
        },
        {
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
            0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,0x3F7EC46D,0x3F74FA0B,0x3F61C598,0x3F45E403,
        },
        {
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
            0x3F7FB10F,0x3F7D3AAC,0x3F7853F8,0x3F710908,0x3F676BD8,0x3F5B941A,0x3F4D9F02,0x3F3DAEF9,
        },
        {
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
            0x3F7FEC43,0x3F7F4E6D,0x3F7E1324,0x3F7C3B28,0x3F79C79D,0x3F76BA07,0x3F731447,0x3F6ED89E,
            0x3F6A09A7,0x3F64AA59,0x3F5EBE05,0x3F584853,0x3F514D3D,0x3F49D112,0x3F41D870,0x3F396842,
        },
        {
            0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
            0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
            0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
            0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
            0x3F7FFB11,0x3F7FD397,0x3F7F84AB,0x3F7F0E58,0x3F7E70B0,0x3F7DABCC,0x3F7CBFC9,0x3F7BACCD,
            0x3F7A7302,0x3F791298,0x3F778BC5,0x3F75DEC6,0x3F740BDD,0x3F721352,0x3F6FF573,0x3F6DB293,
            0x3F6B4B0C,0x3F68BF3C,0x3F660F88,0x3F633C5A,0x3F604621,0x3F5D2D53,0x3F59F26A,0x3F5695E5,
            0x3F531849,0x3F4F7A1F,0x3F4BBBF8,0x3F47DE65,0x3F43E200,0x3F3FC767,0x3F3B8F3B,0x3F373A23,
        },
        {
            0x3F7FFEC4,0x3F7FF4E6,0x3F7FE129,0x3F7FC38F,0x3F7F9C18,0x3F7F6AC7,0x3F7F2F9D,0x3F7EEA9D,
            0x3F7E9BC9,0x3F7E4323,0x3F7DE0B1,0x3F7D7474,0x3F7CFE73,0x3F7C7EB0,0x3F7BF531,0x3F7B61FC,
            0x3F7AC516,0x3F7A1E84,0x3F796E4E,0x3F78B47B,0x3F77F110,0x3F772417,0x3F764D97,0x3F756D97,
            0x3F748422,0x3F73913F,0x3F7294F8,0x3F718F57,0x3F708066,0x3F6F6830,0x3F6E46BE,0x3F6D1C1D,
            0x3F6BE858,0x3F6AAB7B,0x3F696591,0x3F6816A8,0x3F66BECC,0x3F655E0B,0x3F63F473,0x3F628210,
            0x3F6106F2,0x3F5F8327,0x3F5DF6BE,0x3F5C61C7,0x3F5AC450,0x3F591E6A,0x3F577026,0x3F55B993,
            0x3F53FAC3,0x3F5233C6,0x3F5064AF,0x3F4E8D90,0x3F4CAE79,0x3F4AC77F,0x3F48D8B3,0x3F46E22A,
            0x3F44E3F5,0x3F42DE29,0x3F40D0DA,0x3F3EBC1B,0x3F3CA003,0x3F3A7CA4,0x3F385216,0x3F36206C,
        }
    };

    static const unsigned int list2Int[7][0x40] =
    {
        {
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,
            0x3D0A8BD4,0xBD0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0xBD0A8BD4,0x3D0A8BD4,0x3D0A8BD4,0xBD0A8BD4,
        },
        {
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,
            0x3E47C5C2,0x3F0E39DA,0xBE47C5C2,0xBF0E39DA,0xBE47C5C2,0xBF0E39DA,0x3E47C5C2,0x3F0E39DA,
        },
        {
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,
            0x3DC8BD36,0x3E94A031,0x3EF15AEA,0x3F226799,0xBDC8BD36,0xBE94A031,0xBEF15AEA,0xBF226799,
        },
        {
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0xBD48FB30,0xBE164083,0xBE78CFCC,0xBEAC7CD4,0xBEDAE880,0xBF039C3D,0xBF187FC0,0xBF2BEB4A,
            0x3D48FB30,0x3E164083,0x3E78CFCC,0x3EAC7CD4,0x3EDAE880,0x3F039C3D,0x3F187FC0,0x3F2BEB4A,
        },
        {
            0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
            0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
            0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
            0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
            0x3CC90AB0,0x3D96A905,0x3DFAB273,0x3E2F10A2,0x3E605C13,0x3E888E93,0x3EA09AE5,0x3EB8442A,
            0x3ECF7BCA,0x3EE63375,0x3EFC5D27,0x3F08F59B,0x3F13682A,0x3F1D7FD1,0x3F273656,0x3F3085BB,
            0xBCC90AB0,0xBD96A905,0xBDFAB273,0xBE2F10A2,0xBE605C13,0xBE888E93,0xBEA09AE5,0xBEB8442A,
            0xBECF7BCA,0xBEE63375,0xBEFC5D27,0xBF08F59B,0xBF13682A,0xBF1D7FD1,0xBF273656,0xBF3085BB,
        },
        {
            0xBC490E90,0xBD16C32C,0xBD7B2B74,0xBDAFB680,0xBDE1BC2E,0xBE09CF86,0xBE22ABB6,0xBE3B6ECF,
            0xBE541501,0xBE6C9A7F,0xBE827DC0,0xBE8E9A22,0xBE9AA086,0xBEA68F12,0xBEB263EF,0xBEBE1D4A,
            0xBEC9B953,0xBED53641,0xBEE0924F,0xBEEBCBBB,0xBEF6E0CB,0xBF00E7E4,0xBF064B82,0xBF0B9A6B,
            0xBF10D3CD,0xBF15F6D9,0xBF1B02C6,0xBF1FF6CB,0xBF24D225,0xBF299415,0xBF2E3BDE,0xBF32C8C9,
            0x3C490E90,0x3D16C32C,0x3D7B2B74,0x3DAFB680,0x3DE1BC2E,0x3E09CF86,0x3E22ABB6,0x3E3B6ECF,
            0x3E541501,0x3E6C9A7F,0x3E827DC0,0x3E8E9A22,0x3E9AA086,0x3EA68F12,0x3EB263EF,0x3EBE1D4A,
            0x3EC9B953,0x3ED53641,0x3EE0924F,0x3EEBCBBB,0x3EF6E0CB,0x3F00E7E4,0x3F064B82,0x3F0B9A6B,
            0x3F10D3CD,0x3F15F6D9,0x3F1B02C6,0x3F1FF6CB,0x3F24D225,0x3F299415,0x3F2E3BDE,0x3F32C8C9,
        },
        {
            0xBBC90F88,0xBC96C9B6,0xBCFB49BA,0xBD2FE007,0xBD621469,0xBD8A200A,0xBDA3308C,0xBDBC3AC3,
            0xBDD53DB9,0xBDEE3876,0xBE039502,0xBE1008B7,0xBE1C76DE,0xBE28DEFC,0xBE354098,0xBE419B37,
            0xBE4DEE60,0xBE5A3997,0xBE667C66,0xBE72B651,0xBE7EE6E1,0xBE8586CE,0xBE8B9507,0xBE919DDD,
            0xBE97A117,0xBE9D9E78,0xBEA395C5,0xBEA986C4,0xBEAF713A,0xBEB554EC,0xBEBB31A0,0xBEC1071E,
            0xBEC6D529,0xBECC9B8B,0xBED25A09,0xBED8106B,0xBEDDBE79,0xBEE363FA,0xBEE900B7,0xBEEE9479,
            0xBEF41F07,0xBEF9A02D,0xBEFF17B2,0xBF0242B1,0xBF04F484,0xBF07A136,0xBF0A48AD,0xBF0CEAD0,
            0xBF0F8784,0xBF121EB0,0xBF14B039,0xBF173C07,0xBF19C200,0xBF1C420C,0xBF1EBC12,0xBF212FF9,
            0xBF239DA9,0xBF26050A,0xBF286605,0xBF2AC082,0xBF2D1469,0xBF2F61A5,0xBF31A81D,0xBF33E7BC,
        }
    };

    static const unsigned int list3Int[2][0x40] =
    {
        {
            0x3A3504F0,0x3B0183B8,0x3B70C538,0x3BBB9268,0x3C04A809,0x3C308200,0x3C61284C,0x3C8B3F17,
            0x3CA83992,0x3CC77FBD,0x3CE91110,0x3D0677CD,0x3D198FC4,0x3D2DD35C,0x3D434643,0x3D59ECC1,
            0x3D71CBA8,0x3D85741E,0x3D92A413,0x3DA078B4,0x3DAEF522,0x3DBE1C9E,0x3DCDF27B,0x3DDE7A1D,
            0x3DEFB6ED,0x3E00D62B,0x3E0A2EDA,0x3E13E72A,0x3E1E00B1,0x3E287CF2,0x3E335D55,0x3E3EA321,
            0x3E4A4F75,0x3E56633F,0x3E62DF37,0x3E6FC3D1,0x3E7D1138,0x3E8563A2,0x3E8C72B7,0x3E93B561,
            0x3E9B2AEF,0x3EA2D26F,0x3EAAAAAB,0x3EB2B222,0x3EBAE706,0x3EC34737,0x3ECBD03D,0x3ED47F46,
            0x3EDD5128,0x3EE6425C,0x3EEF4EFF,0x3EF872D7,0x3F00D4A9,0x3F0576CA,0x3F0A1D3B,0x3F0EC548,
            0x3F136C25,0x3F180EF2,0x3F1CAAC2,0x3F213CA2,0x3F25C1A5,0x3F2A36E7,0x3F2E9998,0x3F32E705,
        },
        {
            0xBF371C9E,0xBF3B37FE,0xBF3F36F2,0xBF431780,0xBF46D7E6,0xBF4A76A4,0xBF4DF27C,0xBF514A6F,
            0xBF547DC5,0xBF578C03,0xBF5A74EE,0xBF5D3887,0xBF5FD707,0xBF6250DA,0xBF64A699,0xBF66D908,
            0xBF68E90E,0xBF6AD7B1,0xBF6CA611,0xBF6E5562,0xBF6FE6E7,0xBF715BEF,0xBF72B5D1,0xBF73F5E6,
            0xBF751D89,0xBF762E13,0xBF7728D7,0xBF780F20,0xBF78E234,0xBF79A34C,0xBF7A5397,0xBF7AF439,
            0xBF7B8648,0xBF7C0ACE,0xBF7C82C8,0xBF7CEF26,0xBF7D50CB,0xBF7DA88E,0xBF7DF737,0xBF7E3D86,
            0xBF7E7C2A,0xBF7EB3CC,0xBF7EE507,0xBF7F106C,0xBF7F3683,0xBF7F57CA,0xBF7F74B6,0xBF7F8DB6,
            0xBF7FA32E,0xBF7FB57B,0xBF7FC4F6,0xBF7FD1ED,0xBF7FDCAD,0xBF7FE579,0xBF7FEC90,0xBF7FF22E,
            0xBF7FF688,0xBF7FF9D0,0xBF7FFC32,0xBF7FFDDA,0xBF7FFEED,0xBF7FFF8F,0xBF7FFFDF,0xBF7FFFFC,
        }
    };

    float *s,*d,*s1,*s2;
    s = block; d = wav1;

    for (int i = 0, count1 = 1, count2 = 0x40; i < 7; i++, count1 <<=1, count2 >>= 1)
    {
        float *d1=d;
        float *d2=&d[count2];

        for (int j = 0; j < count1; j++)
        {
            for (int k = 0; k < count2; k++)
            {
                float a = *(s++);
                float b = *(s++);
                *(d1++) = b+a;
                *(d2++) = a-b;
            }

            d1 += count2;
            d2 += count2;
        }

        float *w =&s [-0x80]; s = d; d = w;
    }

    s = wav1; d = block;

    for (int i = 0, count1 = 0x40, count2 = 1; i < 7; i++, count1 >>= 1, count2 <<= 1)
    {
        float *list1Float = (float *)list1Int[i];
        float *list2Float = (float *)list2Int[i];
        float *s1 = s;
        float *s2 = &s1[count2];
        float *d1 = d;
        float *d2 = &d1[count2*2-1];

        for(int j = 0; j < count1; j++)
        {
            for(int k = 0; k < count2; k++)
            {
                float a = *(s1++);
                float b = *(s2++);
                float c = *(list1Float++);
                float d = *(list2Float++);
                *(d1++) = a*c-b*d;
                *(d2--) = a*d+b*c;
            }

            s1 += count2;
            s2 += count2;
            d1 += count2;
            d2 += count2*3;
        }

        float *w=s; s=d; d=w;
    }

    d = wav2;
    for (int i = 0; i < 0x80;i++)
        *(d++)=*(s++);

    s = (float *)list3Int; d = wave[index];
    s1 = &wav2[0x40]; s2 = wav3;

    for (int i = 0; i < 0x40; i++)
        *(d++) = *(s1++)**(s++)+*(s2++);

    for(int i=0;i<0x40;i++)
        *(d++) = *(s++)**(--s1)-*(s2++);

    s1 = &wav2[0x40-1]; s2 = wav3;
    for (int i = 0; i < 0x40; i++)
        *(s2++) = *(s1--)**(--s);

    for (int i = 0; i < 0x40; i++)
        *(s2++) = *(--s)**(++s1);
}

#endif

