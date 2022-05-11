#ifndef __HCAFILE_H__
#define __HCAFILE_H__

#include "WavFile.h"

#define HCA_SIGNATURE 	0x00414348
#define FMT_SIGNATURE 	0x00746D66
#define COMP_SIGNATURE	0x706D6F63
#define DEC_SIGNATURE	0x00636564
#define VBR_SIGNATURE	0x00726276
#define ATH_SIGNATURE	0x00687461
#define LOOP_SIGNATURE	0x706F6F6C
#define CIPH_SIGNATURE	0x68706963
#define RVA_SIGNATURE	0x00617672
#define PAD_SIGNATURE	0x00646170
#define COMM_SIGNATURE	0x6D6D6F63

#ifdef _MSC_VER
#pragma pack(push,1)
#endif

typedef struct
{
	uint32_t signature; // 0 
	uint16_t version; // 4
	uint16_t data_offset; // 6
} PACKED HCAHeader;

static_assert(sizeof(HCAHeader) == 8, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint32_t num_channels:8; // 4
	uint32_t sample_rate:24; // 5
	uint32_t block_count; // 8
	uint16_t r01; // 0xC
	uint16_t r02; // 0xE
} PACKED HCAFmt;

static_assert(sizeof(HCAFmt) == 0x10, "Incorrect structure size.");

typedef struct
{ 
	uint32_t signature; // 0
	uint16_t block_size; // 4
	uint8_t r01; // 6
	uint8_t r02; // 7
	uint8_t r03; // 8
	uint8_t r04; // 9
	uint8_t r05; // 0xA
	uint8_t r06; // 0xB
	uint8_t r07; // 0xC
	uint8_t r08; // 0xD
	uint16_t reserved; // 0xE
} PACKED HCAComp;

static_assert(sizeof(HCAComp) == 0x10, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint16_t block_size; // 4
	uint8_t r01; // 6
	uint8_t r02; // 7
	uint8_t count1; // 8
	uint8_t count2; // 9
	uint8_t r03:4; // 0xA
	uint8_t r04:4; // 0xA
	uint8_t enable_count2; // 0xB
} PACKED HCADec;

static_assert(sizeof(HCADec) == 0xC, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint16_t r01; // 4
	uint16_t r02; // 6
} PACKED HCAVbr;

static_assert(sizeof(HCAVbr) == 8, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint16_t type; // 4
} PACKED HCAAth;

static_assert(sizeof(HCAAth) == 6, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint32_t loop_start; // 4
	uint32_t loop_end; // 8
	uint16_t r01; // 0xC
	uint16_t r02; // 0xE
} PACKED HCALoop;

static_assert(sizeof(HCALoop) == 0x10, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint16_t type; // 4
} PACKED HCACiph;

static_assert(sizeof(HCACiph) == 6, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	float volume; // 4
} PACKED HCARva;

static_assert(sizeof(HCARva) == 8, "Incorrect structure size.");

typedef struct
{
	uint32_t signature; // 0
	uint8_t len; // 4 It is supossed to include null char or not?
} PACKED HCAComm;

static_assert(sizeof(HCAComm) == 5, "Incorrect structure size.");

#ifdef _MSC_VER
#pragma pack(pop)
#endif

struct HcaFmt
{
	uint8_t num_channels;
	uint32_t sample_rate;
	uint32_t block_count;
	uint16_t r01;
	uint16_t r02;
};

struct HcaComp
{
	uint16_t block_size; 
	uint8_t r01; 
	uint8_t r02;
	uint8_t r03; 
	uint8_t r04; 
	uint8_t r05; 
	uint8_t r06; 
	uint8_t r07; 
	uint8_t r08;
	uint16_t reserved; 
};

struct HcaDec
{
	uint16_t block_size; 
	uint8_t r01;
	uint8_t r02; 
	uint8_t count1;
	uint8_t count2; 
	uint8_t r03; 
	uint8_t r04; 
	uint8_t enable_count2; 
};

struct HcaVbr
{
	uint16_t r01; 
	uint16_t r02; 
};

struct HcaLoop
{
	uint32_t loop_start; 
	uint32_t loop_end; 
	uint16_t r01;
	uint16_t r02; 
};

class HcaFile : public AudioFile
{
private:

	uint16_t version;	
	
	HcaFmt fmt;
	HcaComp comp;
	HcaDec dec;
	HcaVbr vbr;
	uint16_t ath_type;
	HcaLoop loop;
	uint16_t ciph_type;
	float volume;
	std::string comm;
	
	bool has_comp;
	bool has_dec;
	bool has_vbr;
	bool has_ath;
	bool has_loop;
	bool has_ciph;
	bool has_rva;
	bool has_comm;
	bool has_pad;
	
	uint32_t pad_size;
	uint8_t *raw_data;
	
	uint32_t block_size;	
	uint8_t comp_r01;
	uint8_t comp_r02;
	uint8_t comp_r03;
	uint8_t comp_r04;
	uint8_t comp_r05;
	uint8_t comp_r06;
	uint8_t comp_r07;
	uint8_t comp_r08;
	uint8_t comp_r09;
	
	uint8_t cipher_table[0x100];
	unsigned int cipher56_key1, cipher56_key2;
    bool cipher56_inited;
	
	uint8_t ath_table[0x80];

    static int default_quality;
    static int default_cutoff;
	
	static uint16_t CheckSum(const void *data, unsigned int size, uint16_t sum=0);
	
	unsigned int CalculateHeaderSize() const;
	
	void InitCipher1();
	void InitCipher56();
	
	void InitCipher1Rev();
	void InitCipher56Rev();
	
	void Mask(uint8_t *buf, unsigned int size);
	
	void InitAth1(uint32_t key);
	
	bool InitDecoder();
	bool DecodeBlock(const uint8_t *buf, unsigned int size);
	
	class clData
	{
		public:
		
			clData(void *data, int size) : _data((uint8_t *)data), _size(size*8-16), _bit(0) {}
			int CheckBit(int bitSize);
			int GetBit(int bitSize);
			void AddBit(int bitSize);
			
		private:
		
			uint8_t *_data;
			int _size;
			int _bit;
	};
	
	struct Channel
	{
		float block[0x80];
		float base[0x80];
		char value[0x80];
		char scale[0x80];
		char value2[8];
		int type;
		char *value3;
		unsigned int count;
		float wav1[0x80];
		float wav2[0x80];
		float wav3[0x80];
		float wave[8][0x80];
		
		void Decode1(clData *data, unsigned int a, int b, uint8_t *ath);
		void Decode2(clData *data);
		void Decode3(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
		void Decode4(int index, unsigned int a, unsigned int b, unsigned int c, Channel &next);
		void Decode5(int index);
	} channels[0x10];
	
protected:

	void Reset();
	
public:

	HcaFile();
	virtual ~HcaFile();
	
    virtual bool Load(const uint8_t *buf, size_t size) override;
    virtual uint8_t *Save(size_t *psize) override;
	
    virtual uint16_t GetNumChannels() const override { return fmt.num_channels; }
    virtual bool SetNumChannels(uint16_t num_channels) override{ UNUSED(num_channels); return false; }

    virtual uint32_t GetNumSamples() const override
    {
        return 0x80*8*fmt.block_count;
    }
	
    virtual uint32_t GetSampleRate() const override { return fmt.sample_rate; }
    virtual bool SetSampleRate(uint32_t sample_rate) override { UNUSED(sample_rate); return false; }

    virtual float GetDuration() const override
    {
        return (0x80*8*fmt.block_count) / (float)fmt.sample_rate;
    }

    virtual uint8_t *Decode(int *format, size_t *psize) override;
    virtual bool Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership=true) override;

    virtual bool HasLoop() const override
    {
        return has_loop;
    }

    virtual bool GetLoop(float *start, float *end, int *count) const
    {
        if (!has_loop)
            return false;

        *start = (loop.loop_start*0x80*8) / (float)fmt.sample_rate;
        *end = (loop.loop_end*0x80*8) / (float)fmt.sample_rate;
        *count = (loop.r01 < 0x80) ? loop.r01 : 0;

        return true;
    }

    virtual bool SetLoop(float start, float end, int count) override;

    virtual bool GetLoopSample(uint32_t *sample_start, uint32_t *sample_end, int *count) const override
    {
        if (!has_loop)
            return false;

        *sample_start = (loop.loop_start*0x80*8);
        *sample_end = (loop.loop_start*0x80*8);
        *count = (loop.r01 < 0x80) ? loop.r01 : 0;

        return true;
    }

    virtual bool SetLoopSample(uint32_t sample_start, uint32_t sample_end, int count) override;

    virtual void SetMaxLoop() override
    {
        has_loop = true;
        loop.loop_start = 0;
        loop.loop_end = fmt.block_count-1;
        loop.r01 = 0x80;
        loop.r02 = 0x400;
    }

    virtual void RemoveLoop() override
    {
        has_loop = false;
    }

    // Unimplemented
    virtual bool FromFiles(const std::vector<AudioFile *> &, int , bool, int) { return false; }
    virtual bool ToFiles(const std::vector<AudioFile *> &, uint16_t, bool, bool, int) const { return false; }

    inline uint16_t GetCiphType() const { return ciph_type; }
	bool SetCiphType(uint16_t new_ciph_type);
	
    bool DecodeToWav(const std::string &file, bool preserve_loop=true, int format=AUDIO_FORMAT_16BITS);
    bool EncodeFromWav(const std::string &file, int quality=0, int cutoff_freq=0, bool preserve_loop=true);

    static bool CheckHeader(const uint8_t *buf, size_t size, uint32_t *num_channels=nullptr, uint32_t *sample_rate=nullptr, uint32_t *block_count=nullptr);
    static bool CheckHeader(const std::string &file, uint32_t *num_channels=nullptr, uint32_t *sample_rate=nullptr, uint32_t *block_count=nullptr);

    static inline int GetDefaultQuality() { return default_quality; }
    static inline int GetDefaultCutoff() { return default_cutoff; }

    static inline void SetDefaultQuality(int quality) { default_quality = quality; }
    static inline void SetDefaultCutoff(int cutoff) { default_cutoff = cutoff; }
};

#endif /* __HCAFILE_H__ */
