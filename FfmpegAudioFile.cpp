#include "FfmpegAudioFile.h"
#include "debug.h"

#define MAX_AUDIO_FRAME_SIZE 192000

#ifndef FF_INPUT_BUFFER_PADDING_SIZE
#define FF_INPUT_BUFFER_PADDING_SIZE AV_INPUT_BUFFER_PADDING_SIZE
#endif

FfmpegAudioFile::FfmpegAudioFile()
{
    decoded = nullptr;
    Reset();
}

FfmpegAudioFile::~FfmpegAudioFile()
{
    Reset();
}

void FfmpegAudioFile::Reset()
{
    if (decoded)
    {
        delete decoded;
        decoded = nullptr;
    }

    num_channels = 0;
    sample_rate = 1;
    num_samples = 0;
    format = -1;
}

bool FfmpegAudioFile::LoadFromFile(const std::string &path, bool show_error)
{
    Reset();

    AVFormatContext* format_ctx = avformat_alloc_context();

    if (avformat_open_input(&format_ctx, path.c_str(), nullptr, nullptr) < 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Failed to open file.\n", FUNCNAME);
        }

        avformat_free_context(format_ctx);
        return false;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        avformat_close_input(&format_ctx);
        return false;
    }

    int stream_id = -1;

    for (unsigned int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            stream_id = i;
            break;
        }
    }

    if (stream_id == -1)
    {
        if (show_error)
        {
            DPRINTF("%s: That file doesn't have an audio stream.\n", FUNCNAME);
        }

        avformat_close_input(&format_ctx);
        return false;
    }

    AVCodecContext *codec_ctx = format_ctx->streams[stream_id]->codec;
    AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);

    if (!codec)
    {
        if (show_error)
        {
            DPRINTF("%s: Codec not supported.\n", FUNCNAME);
        }

        avformat_close_input(&format_ctx);
        return false;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        if (show_error)
        {
            DPRINTF("%s: Codec not supported.\n", FUNCNAME);
        }

        avformat_close_input(&format_ctx);
        return false;
    }

    num_channels = codec_ctx->channels;
    sample_rate = codec_ctx->sample_rate;

    decoded = new MemoryStream();

    AVPacket packet;
    av_init_packet(&packet);

    AVFrame *frame = av_frame_alloc();
    int buf_size = MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;

    uint8_t *buf = new uint8_t[buf_size];
    packet.data = buf;
    packet.size = buf_size;

    int sformat = codec_ctx->sample_fmt;

    bool ret = false;

    while (av_read_frame(format_ctx, &packet) >= 0)
    {
        if (packet.stream_index == stream_id)
        {
            int got_frame;
            int plane_size;

            avcodec_decode_audio4(codec_ctx, frame, &got_frame, &packet);
            av_samples_get_buffer_size(&plane_size, codec_ctx->channels, frame->nb_samples, codec_ctx->sample_fmt, 1);

            if (got_frame)
            {
                switch (sformat)
                {
                    case AV_SAMPLE_FMT_U8: case AV_SAMPLE_FMT_S16: case AV_SAMPLE_FMT_S32: case AV_SAMPLE_FMT_FLT:
                    {
                        if (!decoded->Write(frame->extended_data[0], frame->linesize[0]))
                        {
                            goto cleanup;
                        }
                    }

                    break;

                    case AV_SAMPLE_FMT_U8P:
                    {

                        int size = num_channels*plane_size;
                        uint8_t *samples = new uint8_t[size];
                        uint8_t *ptr = samples;

                        for (int nb = 0; nb < plane_size; nb++)
                        {
                            for (int ch = 0; ch < num_channels; ch++)
                            {
                                *(ptr++) = ((uint8_t *) frame->extended_data[ch])[nb];
                            }
                        }

                        if (!decoded->Write(samples, size))
                        {
                            delete[] samples;
                            goto cleanup;
                        }

                        delete[] samples;

                    }
                    break;

                    case AV_SAMPLE_FMT_S16P:
                    {

                        int size = num_channels*plane_size;
                        uint16_t *samples = new uint16_t[size / sizeof(uint16_t)];
                        uint16_t *ptr = samples;

                        for (int nb = 0; nb < plane_size/(int)sizeof(uint16_t); nb++)
                        {
                            for (int ch = 0; ch < num_channels; ch++)
                            {
                                *(ptr++) = ((uint16_t *) frame->extended_data[ch])[nb];
                            }
                        }

                        if (!decoded->Write(samples, size))
                        {
                            delete[] samples;
                            goto cleanup;
                        }

                        delete[] samples;

                    }
                    break;

                    case AV_SAMPLE_FMT_S32P: case AV_SAMPLE_FMT_FLTP:
                    {

                        int size = num_channels*plane_size;
                        uint32_t *samples = new uint32_t[size / sizeof(uint32_t)];
                        uint32_t *ptr = samples;

                        for (int nb = 0; nb < plane_size/(int)sizeof(uint32_t); nb++)
                        {
                            for (int ch = 0; ch < num_channels; ch++)
                            {
                                *(ptr++) = ((uint32_t *) frame->extended_data[ch])[nb];
                            }
                        }

                        if (!decoded->Write(samples, size))
                        {
                            delete[] samples;
                            goto cleanup;
                        }

                        delete[] samples;

                    }
                    break;

                    default:
                        DPRINTF("%s: Unsupported sample format: %d\n", FUNCNAME, sformat);
                        goto cleanup;
                }
            }
        }

        av_free_packet(&packet);
    }

    switch (sformat)
    {
        case AV_SAMPLE_FMT_U8: case AV_SAMPLE_FMT_U8P:
            format = AUDIO_FORMAT_8BITS;
            num_samples = (uint32_t)decoded->GetSize();
        break;

        case AV_SAMPLE_FMT_S16: case AV_SAMPLE_FMT_S16P:
            format = AUDIO_FORMAT_16BITS;
            num_samples = (uint32_t)(decoded->GetSize() / sizeof(int16_t));
        break;

        case AV_SAMPLE_FMT_S32: case AV_SAMPLE_FMT_S32P:
            format = AUDIO_FORMAT_32BITS;
            num_samples = (uint32_t)(decoded->GetSize() / sizeof(int32_t));
        break;

        case AV_SAMPLE_FMT_FLT: case AV_SAMPLE_FMT_FLTP:
            format = AUDIO_FORMAT_FLOAT;
            num_samples = (uint32_t)(decoded->GetSize() / sizeof(float));
        break;
    }

    num_samples /= num_channels;
    ret = true;

cleanup:

    avcodec_close(codec_ctx);
    avformat_close_input(&format_ctx);
    av_frame_free(&frame);

    return ret;
}

uint8_t *FfmpegAudioFile::Decode(int *format, size_t *psize)
{
    if (!decoded || this->format < 0)
        return nullptr;

    *psize = decoded->GetSize();
    uint8_t *buf = new uint8_t[*psize];

    memcpy(buf, decoded->GetMemory(false), *psize);
    *format = this->format;
    return buf;
}

bool FfmpegAudioFile::Encode(uint8_t *buf, size_t size, int format, uint16_t num_channels, uint32_t sample_rate, bool take_ownership)
{
    UNUSED(buf); UNUSED(size); UNUSED(format); UNUSED(num_channels); UNUSED(sample_rate); UNUSED(take_ownership);
    return false;

    // TODO: complete this

    /*AVCodecID codec_id;
    AVSampleFormat sformat;
    uint8_t *dec = buf;
    bool should_delete = take_ownership;

    if (format == AUDIO_FORMAT_FLOAT)
    {
        sformat = AV_SAMPLE_FMT_FLTP;
        codec_id = AV_CODEC_ID_WAVPACK;
    }
    else if (format == AUDIO_FORMAT_8BITS)
    {
        sformat = AV_SAMPLE_FMT_U8P;
        codec_id = AV_CODEC_ID_WAVPACK;
    }
    else if (format == AUDIO_FORMAT_16BITS)
    {
        sformat = AV_SAMPLE_FMT_S16;
        codec_id = AV_CODEC_ID_FLAC;
    }
    else if (format == AUDIO_FORMAT_24BITS)
    {
        sformat = AV_SAMPLE_FMT_S32;
        codec_id = AV_CODEC_ID_FLAC;
    }
    else if (format == AUDIO_FORMAT_32BITS)
    {
        sformat = AV_SAMPLE_FMT_S32P;
        codec_id = AV_CODEC_ID_WAVPACK;
    }

    AVCodec *codec = avcodec_find_encoder(codec_id);
    if (!codec)
    {
        DPRINTF("%s: Cannot find flac encoder.\n", FUNCNAME);
        return false;
    }

    if (num_channels > 1)
    {
        // Convert to panar for wavpack encoder

        if (sformat == AV_SAMPLE_FMT_U8P)
        {
            uint8_t *buf_panar = new uint8_t[size];
            size_t plane_size = size / num_channels;

            for (size_t i = 0; i < plane_size; i++)
            {
                for (uint16_t j = 0; j < num_channels; j++)
                {
                    buf_panar[i+j*plane_size] = buf[i*plane_size+j];
                }
            }

            if (take_ownership)
                delete[] buf;

            dec = buf_panar;
            should_delete = true;
        }
        else if (sformat == AV_SAMPLE_FMT_S32P || sformat == AV_SAMPLE_FMT_FLTP)
        {
            uint32_t *buf_panar = new uint32_t[size/4];
            size_t plane_size = (size/4) / num_channels;

            for (size_t i = 0; i < plane_size; i++)
            {
                for (uint16_t j = 0; j < num_channels; j++)
                {
                    buf_panar[i+j*plane_size] = buf[i*plane_size+j];
                }
            }

            if (take_ownership)
                delete[] buf;

            dec = (uint8_t *)buf_panar;
            should_delete = true;
        }
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
        return false;

    codec_ctx->sample_fmt = sformat;
    codec_ctx->channels = num_channels;
    codec_ctx->sample_rate = sample_rate;
    codec_ctx->compression_level = (codec_id == AV_CODEC_ID_FLAC) ? 12 : 8;

    if (format == AUDIO_FORMAT_24BITS)
    {
        codec_ctx->bits_per_raw_sample = 24;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        if (should_delete)
            delete[] dec;

        av_free(codec_ctx);
        return false;
    }

    AVFrame *frame = av_frame_alloc();
    if (!frame)
    {
        if (should_delete)
            delete[] dec;

        avcodec_close(codec_ctx);
        av_free(codec_ctx);
        return false;
    }

    frame->nb_samples     = codec_ctx->frame_size;
    frame->format         = codec_ctx->sample_fmt;
    frame->channel_layout = codec_ctx->channel_layout;

    int buffer_size = av_samples_get_buffer_size(nullptr, codec_ctx->channels, codec_ctx->frame_size, codec_ctx->sample_fmt, 0);

    return true;*/
}

bool FfmpegAudioFile::ToFiles(const std::vector<AudioFile *> &files, uint16_t split_channels, bool allow_silence, bool preserve_loop, int max_threads) const
{
    if (!decoded || format == -1)
        return false;

    uint16_t total_channels = (uint16_t)files.size() * split_channels;

    if (total_channels > num_channels && !allow_silence)
        return false;

    if (total_channels < num_channels)
        return false;

    if ((num_channels % split_channels) != 0)
        return false;

    if (files.size() == 0)
        return true;

    if (max_threads <= 0)
    {
        int cores_count = Thread::LogicalCoresCount();
        max_threads = (files.size() > (size_t)cores_count) ? cores_count : (int)files.size();
    }

    ThreadPool pool(max_threads);
    bool error = false;

    const uint8_t *buf = decoded->GetMemory(false);

    for (uint16_t i = 0; i < (uint16_t)files.size(); i++)
    {
        pool.AddWork(new MultipleAudioEncoder(files[i], i*split_channels >= num_channels ? nullptr : buf, i, format,
                                         split_channels, sample_rate, GetNumSamples(), num_channels, &error));
    }

    pool.Wait();

    // TODO: enable this when loop implemented
    UNUSED(preserve_loop);
    /*if (preserve_loop && HasLoop() && !error)
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
    }*/

    return !error;
}

bool FfmpegAudioFile::CheckHeader(const std::string &path, uint16_t *num_channels, uint32_t *sample_rate)
{
    AVFormatContext* format_ctx = avformat_alloc_context();

    if (avformat_open_input(&format_ctx, path.c_str(), nullptr, nullptr) < 0)
    {
        avformat_free_context(format_ctx);
        return false;
    }

    if (avformat_find_stream_info(format_ctx, nullptr) < 0)
    {
        avformat_close_input(&format_ctx);
        return false;
    }

    int stream_id = -1;

    for (unsigned int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            stream_id = i;
            break;
        }
    }

    if (stream_id == -1)
    {
        avformat_close_input(&format_ctx);
        return false;
    }

    AVCodecContext *codec_ctx = format_ctx->streams[stream_id]->codec;
    AVCodec *codec = avcodec_find_decoder(codec_ctx->codec_id);

    if (!codec)
    {
        avformat_close_input(&format_ctx);
        return false;
    }

    if (avcodec_open2(codec_ctx, codec, nullptr) < 0)
    {
        avformat_close_input(&format_ctx);
        return false;
    }

    if (num_channels)
        *num_channels = codec_ctx->channels;

    if (sample_rate)
        *sample_rate = codec_ctx->sample_rate;

    avcodec_close(codec_ctx);
    avformat_close_input(&format_ctx);

    return true;
}
