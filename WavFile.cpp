#include <math.h>
#include "WavFile.h"
#include "MemoryStream.h"
#include "FileStream.h"
#include "Thread.h"
#include "debug.h"

static uint8_t pcm_guid[16] =
{
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};

static uint8_t pcm_float_guid[16] =
{
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};

static uint8_t adpcm_guid[16] =
{
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
};


WavFile::WavFile()
{
	big_endian = false;
    samples = nullptr;
    Reset();
}

WavFile::~WavFile()
{
	Reset();
}

void WavFile::Reset()
{
    if (samples)
        delete samples;

    samples = new MemoryStream();

	loop_start = 0;
	loop_end = 0;
    loop_play_count = 0;    
}

bool WavFile::Load(const uint8_t *buf, size_t size)
{
	Reset();
	
	RIFFHeader *rhdr;
	WAVHeader *whdr;
	
	if (size < (sizeof(RIFFHeader) + sizeof(WAVHeader)))
		return false;
	
	rhdr = (RIFFHeader *)buf;
	whdr = (WAVHeader *)(buf + sizeof(RIFFHeader));
	
	if (rhdr->signature != RIFF_SIGNATURE || whdr->signature != WAV_SIGNATURE)
		return false;
	
    if (whdr->fmt_size < 0x10 || (whdr->type != WAV_PCM && whdr->type != WAV_PCM_FLOAT && whdr->type != WAV_ADPCM && whdr->type != 0xFFFE))
	{
        DPRINTF("%s: only PCM and MS-ADPCM wavs are supported.\n", FUNCNAME);
		return false;
	}

    if (whdr->type == 0xFFFE)
    {
        if (whdr->fmt_size+0x14 < sizeof(WAVExtensibleHeader))
        {
            DPRINTF("%s: Cannot handle this header size (wav extensible).\n", FUNCNAME);
            return false;
        }

        WAVExtensibleHeader *ehdr = (WAVExtensibleHeader *)whdr;

        if (memcmp(ehdr->sub_format, pcm_guid, sizeof(ehdr->sub_format)) == 0)
        {
            format = WAV_PCM;
        }
        else if (memcmp(ehdr->sub_format, pcm_float_guid, sizeof(ehdr->sub_format)) == 0)
        {
            format = WAV_PCM_FLOAT;
        }
        else if (memcmp(ehdr->sub_format, adpcm_guid, sizeof(ehdr->sub_format)) == 0)
        {
            format = WAV_ADPCM;
        }
        else
        {
            DPRINTF("%s: only PCM and MS-ADPCM wavs are supported.\n", FUNCNAME);
            return false;
        }
    }
    else
    {
        format = whdr->type;
    }

	num_channels = whdr->num_channels;
	sample_rate = whdr->sample_rate;
	bit_depth = whdr->bit_depth;
    block_align = whdr->block_align;
	
    if (format != WAV_ADPCM)
    {
        if (bit_depth != 8 && bit_depth != 16 && bit_depth != 24 && bit_depth != 32)
        {
            DPRINTF("%s: Unsupported bit_depth: %d\n", FUNCNAME, bit_depth);
            return false;
        }
    }
    else
    {
        if (bit_depth != 4)
        {
            DPRINTF("%s: Unsupported bit_depth: %d\n", FUNCNAME, bit_depth);
            return false;
        }
    }
	
    if (format != WAV_ADPCM && whdr->block_align < (num_channels*(bit_depth/8)))
	{
		DPRINTF("%s: Invalid value for block align (0x%x)\n", FUNCNAME, whdr->block_align);
		return false;
	}	
	
	const uint8_t *bottom = buf + size;
    const uint8_t *ptr;

    if (whdr->fmt_size == 0x10)
    {
        ptr = buf + sizeof(RIFFHeader) + sizeof(WAVHeader);
    }
    else
    {
        ptr = buf + sizeof(RIFFHeader) + whdr->fmt_size + 0xC;
    }
	
	while ((ptr+8) <= bottom)
	{
		uint32_t chunk_id = *(uint32_t *)ptr;
		uint32_t chunk_size = *(uint32_t *)(ptr+4);
		
		if (chunk_id == SMPL_SIGNATURE)
		{
			if (chunk_size >= sizeof(SMPLHeader) && (ptr+8+sizeof(SMPLHeader)) <= bottom)
			{
				SMPLHeader *shdr = (SMPLHeader *)ptr;
				
				if (shdr->num_sample_loops > 0 && chunk_size >= (sizeof(SMPLHeader)+sizeof(SMPLLoop)-8) && shdr->sampler_size >= sizeof(SMPLLoop))
				{
                    //DPRINTF("here.\n");
					
					if ((ptr+8+sizeof(SMPLHeader)+sizeof(SMPLLoop)) <= bottom)
					{
						SMPLLoop *loop = (SMPLLoop *)(ptr + sizeof(SMPLHeader));
						
						if (loop->type == 0)
						{						
							loop_start = loop->loop_start;
							loop_end = loop->loop_end;
							loop_play_count = loop->play_count;
						}
					}
				}
			}
		}
		else if (chunk_id == DATA_SIGNATURE)
		{
			if (ptr+8+chunk_size <= bottom)
			{
				const uint8_t *data = ptr+8;
				const uint8_t *data_bottom = data+chunk_size;

                if ((chunk_size % whdr->block_align) != 0)
				{
					DPRINTF("%s: Warning, data size not multiple of block_align.\n", FUNCNAME);
					return false;
				}
				
				while (data < data_bottom)
				{
                    if (!samples->Write(data, whdr->block_align))
						return false;
					
					data += whdr->block_align;
				}
			}
		}
		
		ptr += 8 + chunk_size;
	}
	
    if (samples->GetSize() == 0)
	{
		DPRINTF("%s: Wav file is empty or corrupted (or bug in this function).\n", FUNCNAME);
		return false;
	}
	
	return true;
}

bool WavFile::LoadFromFile(const std::string &path, bool show_error)
{
    FileStream *stream = new FileStream();
    if (!stream->LoadFromFile(path, show_error))
    {
        delete stream;
        return true;
    }

    RIFFHeader rhdr;
    WAVHeader whdr;

    if (!stream->Read(&rhdr, sizeof(rhdr)) || !stream->Read(&whdr, sizeof(whdr)))
    {
        delete stream;
        return false;
    }

    if (rhdr.signature != RIFF_SIGNATURE || whdr.signature != WAV_SIGNATURE)
    {
        if (show_error)
            DPRINTF("%s: Not a wav file: \"%s\".\n", FUNCNAME, path.c_str());

        delete stream;
        return false;
    }

    if (whdr.fmt_size < 0x10 || (whdr.type != WAV_PCM && whdr.type != WAV_PCM_FLOAT && whdr.type != WAV_ADPCM && whdr.type != 0xFFFE))
    {
        if (show_error)
            DPRINTF("%s: only PCM and MS-ADPCM wavs are supported.\n", FUNCNAME);

        delete stream;
        return false;
    }

    if (whdr.type == 0xFFFE)
    {
        if (whdr.fmt_size+0x14 < sizeof(WAVExtensibleHeader))
        {
            DPRINTF("%s: Cannot handle this header size (wav extensible).\n", FUNCNAME);
            delete stream;
            return false;
        }

        WAVExtensibleHeader ehdr;
        uint64_t saved_pos;

        saved_pos = stream->Tell();
        stream->Seek(-(int64_t)sizeof(whdr), SEEK_CUR);
        if (!stream->Read(&ehdr, sizeof(WAVExtensibleHeader)))
        {
            delete stream;
            return false;
        }

        if (memcmp(ehdr.sub_format, pcm_guid, sizeof(ehdr.sub_format)) == 0)
        {
            format = WAV_PCM;
        }
        else if (memcmp(ehdr.sub_format, pcm_float_guid, sizeof(ehdr.sub_format)) == 0)
        {
            format = WAV_PCM_FLOAT;
        }
        else if (memcmp(ehdr.sub_format, adpcm_guid, sizeof(ehdr.sub_format)) == 0)
        {
            format = WAV_ADPCM;
        }
        else
        {
            DPRINTF("%s: only PCM and MS-ADPCM wavs are supported.\n", FUNCNAME);
            delete stream;
            return false;
        }

        stream->Seek(saved_pos, SEEK_SET);
    }
    else
    {
        format = whdr.type;
    }

    if (whdr.fmt_size != 0x10)
    {
        stream->Seek(whdr.fmt_size - 0x10, SEEK_CUR);
    }

    num_channels = whdr.num_channels;
    sample_rate = whdr.sample_rate;
    bit_depth = whdr.bit_depth;
    block_align = whdr.block_align;

    if (format != WAV_ADPCM)
    {
        if (bit_depth != 8 && bit_depth != 16 && bit_depth != 24 && bit_depth != 32)
        {
            DPRINTF("%s: Unsupported bit_depth: %d\n", FUNCNAME, bit_depth);
            delete stream;
            return false;
        }
    }
    else
    {
        if (bit_depth != 4)
        {
            DPRINTF("%s: Unsupported bit_depth: %d\n", FUNCNAME, bit_depth);
            delete stream;
            return false;
        }
    }

    if (format != WAV_ADPCM && whdr.block_align < (num_channels*(bit_depth/8)))
    {
        DPRINTF("%s: Invalid value for block align (0x%x)\n", FUNCNAME, whdr.block_align);
        delete stream;
        return false;
    }

    uint32_t chunk_id;
    uint32_t chunk_size;
    uint32_t data_start = 0;
    uint32_t data_size = 0;

    while (stream->Read32(&chunk_id) && stream->Read32(&chunk_size))
    {
        uint64_t save_pos = stream->Tell();

        if (chunk_id == SMPL_SIGNATURE)
        {
            if (chunk_size >= sizeof(SMPLHeader))
            {
                SMPLHeader shdr;

                if (!stream->Read((uint8_t *)(&shdr)+8, sizeof(shdr)-8))
                {
                    delete stream;
                    return false;
                }

                if (shdr.num_sample_loops > 0 && chunk_size >= (sizeof(SMPLHeader)+sizeof(SMPLLoop)-8) && shdr.sampler_size >= sizeof(SMPLLoop))
                {  
                    SMPLLoop loop;

                    if (!stream->Read(&loop, sizeof(loop)))
                    {
                        delete stream;
                        return false;
                    }

                    if (loop.type == 0)
                    {
                        loop_start = loop.loop_start;
                        loop_end = loop.loop_end;
                        loop_play_count = loop.play_count;
                    }
                }
            }            
        }
		else if (chunk_id == DATA_SIGNATURE)
		{
			data_start = (uint32_t)stream->Tell();
			data_size = chunk_size;
		}
		
        stream->Seek(save_pos, SEEK_SET);
        stream->Seek(chunk_size, SEEK_CUR);
    }

    if (data_start == 0 || data_size == 0 || !stream->SetRegion(data_start, data_size))
    {
        if (show_error)
            DPRINTF("%s: Wav file is empty or corrupted (or bug in this function).\n", FUNCNAME);

        delete stream;
        return false;
    }

    delete samples;
    samples = stream;

    return true;
}

uint32_t WavFile::CalculateFileSize()
{
    uint32_t size = sizeof(RIFFHeader) + sizeof(WAVHeader);
	
	if (loop_start != loop_end)
		size += sizeof(SMPLHeader) + sizeof(SMPLLoop);
	
    size += 8 + (uint32_t)samples->GetSize();
	return size;
}

void WavFile::CreateHeader(uint8_t *header, uint32_t file_size, uint32_t samples_size)
{
    uint8_t *ptr = header;


    RIFFHeader *rhdr = (RIFFHeader *)ptr;
    rhdr->signature = RIFF_SIGNATURE;
    rhdr->chunk_size = file_size-sizeof(RIFFHeader);
    ptr += sizeof(RIFFHeader);

    WAVHeader *whdr = (WAVHeader *)ptr;
    whdr->signature = WAV_SIGNATURE;
    whdr->fmt_size = 0x10;
    whdr->type = format;
    whdr->num_channels = num_channels;
    whdr->sample_rate = sample_rate;
    whdr->block_align = num_channels * (bit_depth/8);
    whdr->byte_rate = sample_rate * whdr->block_align;
    whdr->bit_depth = bit_depth;
    ptr += sizeof(WAVHeader);

    if (loop_start != loop_end)
    {
        SMPLHeader *shdr = (SMPLHeader *)ptr;

        shdr->signature = SMPL_SIGNATURE;
        shdr->smpl_size = sizeof(SMPLHeader) + sizeof(SMPLLoop) - 8;
        shdr->manufacturer = 0;
        shdr->product = 0;
        shdr->sample_period = (uint32_t)(1/(double)sample_rate*1000000000);
        shdr->midi_unity_note = 0x3C;
        shdr->midi_pitch_fraction = 0;
        shdr->smpte_format = 0;
        shdr->smpte_offset = 0;
        shdr->num_sample_loops = 1;
        shdr->sampler_size = sizeof(SMPLLoop);

        ptr += sizeof(SMPLHeader);

        SMPLLoop *loop = (SMPLLoop *)ptr;

        loop->cue_point_id = 0;
        loop->type = 0;
        loop->loop_start = loop_start;
        loop->loop_end = loop_end;
        loop->fraction = 0;
        loop->play_count = loop_play_count;

        ptr += sizeof(SMPLLoop);
    }

    *(uint32_t *)ptr = DATA_SIGNATURE;
    *(uint32_t *)(ptr+4) = (samples_size == 0) ? (uint32_t)samples->GetSize() : samples_size;
}


uint8_t *WavFile::Save(size_t *psize)
{
    uint32_t size = CalculateFileSize();
    uint32_t header_size = size - (uint32_t)samples->GetSize();

    uint8_t *buf = new uint8_t[size];

	memset(buf, 0, size);
    CreateHeader(buf, size);
	
    samples->Seek(0, SEEK_SET);
	
    if (!samples->Read(buf+header_size, samples->GetSize()))
	{
		delete[] buf;
		return nullptr;
	}
	
	*psize = size;
    return buf;
}

uint8_t *WavFile::CreateHeader(size_t *psize, uint32_t samples_size)
{
    uint32_t file_size = CalculateFileSize();
    uint32_t size = file_size - (uint32_t)samples->GetSize();

    uint8_t *buf = new uint8_t[size];

    if (samples_size != 0)
    {
        file_size -= (uint32_t)samples->GetSize();
        file_size += samples_size;
    }

    memset(buf, 0, size);
    CreateHeader(buf, file_size, samples_size);

    *psize = size;
    return buf;
}

uint8_t *WavFile::Decode(int *format, size_t *psize)
{
    if (this->format == WAV_ADPCM)
    {
        DPRINTF("%s: not implemented for ADPCM.\n", FUNCNAME);
        return nullptr;
    }

    size_t size = samples->GetSize();
    if (size == 0)
        return nullptr;

    if (this->format == 1)
    {
        *format = GetBitDepth();
    }
    else
    {
        *format = AUDIO_FORMAT_FLOAT;
    }

    uint8_t *buf = new uint8_t[size];

    samples->Seek(0, SEEK_SET);
    if (!samples->Read(buf, size))
    {
        delete[] buf;
        return nullptr;
    }

    *psize = size;
    return buf;
}

bool WavFile::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    Reset();

    if (format != 0 && format != 8 && format != 16 && format != 24 && format != 32)
    {
        if (take_ownership)
            delete[] buf;

        return false;
    }

    MemoryStream *memory = dynamic_cast<MemoryStream *>(samples);

    if (take_ownership)
    {
        memory->SetMemory(buf, size);
    }
    else
    {
        if (!memory->Resize(size))
            return false;

        memory->Seek(0, SEEK_SET);

        if (!memory->Write(buf, size))
            return false;
    }

    this->num_channels = num_channels;
    this->sample_rate = sample_rate;
    this->format = (format == AUDIO_FORMAT_FLOAT) ? 3 : 1;
    this->bit_depth = (format == AUDIO_FORMAT_FLOAT) ? 32 : (uint16_t)format;

    return true;
}

bool WavFile::SetLoop(float start, float end, int count)
{
    if (start >= end)
        return false;

    uint32_t temp_start = lrint((start*(float)sample_rate));
    uint32_t temp_end = lrint((end*(float)sample_rate));

    if (temp_end > GetNumSamples())
        temp_end = GetNumSamples();

    if (temp_start >= temp_end)
        return false;

    loop_start = temp_start;
    loop_end = temp_end;
    loop_play_count = count;

    return true;
}

bool WavFile::MoveToMemory() const
{
    MemoryStream *stream = dynamic_cast<MemoryStream *>(samples);
    if (stream)
        return true; // already a memory stream;

    stream = new MemoryStream();
    if (!stream->Resize(samples->GetSize()))
    {
        delete stream;
        return false;
    }

    samples->Seek(0, SEEK_SET);

    if (!samples->Read(stream->GetMemory(false), stream->GetSize()))
    {
        delete stream;
        return false;
    }

    delete samples;
    samples = stream;

    return true;
}

bool WavFile::FromFiles(const std::vector<AudioFile *> &files, int format, bool preserve_loop, int max_threads)
{
    Reset();

    if (format != 0 && format != 8 && format != 16 && format != 24 && format != 32)
        return false;

    if (files.size() == 0)
        return true;

    uint32_t num_samples = files[0]->GetNumSamples();
    uint32_t sample_rate = files[0]->GetSampleRate();
    uint16_t num_channels = files[0]->GetNumChannels();

    for (size_t i = 1; i < files.size(); i++)
    {
        const AudioFile *audio = files[i];

        if (audio->GetNumSamples() != num_samples)
            return false;

        if (audio->GetSampleRate() != sample_rate)
            return false;

        if (audio->GetNumChannels() != num_channels)
            return false;
    }

    uint32_t num_samples_all = num_samples*(uint32_t)files.size();
    uint32_t total_samples = num_samples_all*num_channels;

    if (format == 0 || format == 32)
        samples->Resize(total_samples*4);
    else if (format == 8)
        samples->Resize(total_samples);
    else if (format == 16)
        samples->Resize(total_samples*2);
    else
        samples->Resize(total_samples*3);

    if (max_threads <= 0)
    {
        int cores_count = Thread::LogicalCoresCount();
        max_threads = (files.size() > (unsigned)cores_count) ? cores_count : (int)files.size();
    }

    this->sample_rate = sample_rate;
    this->num_channels = num_channels*(uint16_t)files.size();
    this->format = (format == AUDIO_FORMAT_FLOAT) ? 3 : 1;
    this->bit_depth = (format == AUDIO_FORMAT_FLOAT) ? 32 : format;

    ThreadPool pool(max_threads);
    bool error = false;

    MemoryStream *memory = dynamic_cast<MemoryStream *>(samples);
    uint8_t *buf = memory->GetMemory(false);

    for (size_t i = 0; i < files.size(); i++)
    {
        pool.AddWork(new MultipleAudioDecoder(files[i], buf, (int)i, (int)files.size(), format, &error));
    }

    pool.Wait();

    if (error)
    {
        Reset();
    }
    else if (preserve_loop)
    {
        uint32_t loop_start, loop_end;
        int loop_count;

        if (files[0]->GetLoopSample(&loop_start, &loop_end, &loop_count))
        {
            SetLoopSample(loop_start, loop_end, loop_count);
        }
    }

    return !error;
}

bool WavFile::ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop, int max_threads) const
{
    if (this->format == WAV_ADPCM)
    {
        DPRINTF("%s: not implemented for ADPCM.\n", FUNCNAME);
        return false;
    }

    uint16_t total_channels = (uint16_t)files.size() * split_channels;

    if (total_channels > num_channels && !allow_silence)
        return false;

    if (total_channels < num_channels)
        return false;

    if ((num_channels % split_channels) != 0)
        return false;

    if (files.size() == 0)
        return true;

    if (!MoveToMemory())
        return false;

    if (max_threads <= 0)
    {
        int cores_count = Thread::LogicalCoresCount();
        max_threads = (files.size() > (unsigned)cores_count) ? cores_count : (int)files.size();
    }

    ThreadPool pool(max_threads);
    bool error = false;

    MemoryStream *memory = dynamic_cast<MemoryStream *>(samples);
    const uint8_t *buf = memory->GetMemory(false);

    int format = (this->format == 1) ? bit_depth : (int)AUDIO_FORMAT_FLOAT;

    for (uint16_t i = 0; i < (uint16_t)files.size(); i++)
    {
        pool.AddWork(new MultipleAudioEncoder(files[i], i*split_channels >= num_channels ? nullptr : buf, i, format,
                                         split_channels, sample_rate, GetNumSamples(), num_channels, &error));
    }

    pool.Wait();

    if (preserve_loop && HasLoop() && !error)
    {
        uint32_t loop_start, loop_end;
        int loop_count;

        if (GetLoopSample(&loop_start, &loop_end, &loop_count))
        {
            for (AudioFile * const &file : files)
            {
                file->SetLoopSample(loop_start, loop_end, loop_count);
            }
        }
    }

    return !error;
}
