#include "CsbFile.h"

CsbFile::CsbFile() : AcbFile()
{
    ResetCSB();
}

CsbFile::~CsbFile()
{
    ResetCSB();
}

void CsbFile::ResetCSB()
{
    se_row = -1;
    data_modified = false;
    tracks_data.clear();
    AcbFile::Reset();
}

bool CsbFile::Load(const uint8_t *buf, size_t size)
{
    ResetCSB();

    if (!UtfFile::Load(buf, size))
        return false;

    for (unsigned int i = 0; i < GetNumRows(); i++)
    {
        std::string name;

        if (!UtfFile::GetString("name", &name, i))
            return false;

        if (name == "SOUND_ELEMENT")
        {
            se_row = i;
            break;
        }
    }

    if (se_row == -1)
        return false;

    UtfFile se;
    uint8_t *se_data;
    unsigned int se_size;
    uint8_t stmflg;

    se_data = GetBlob("utf", &se_size, false, se_row);
    if (!se_data)
        return false;

    if (!se.Load(se_data, se_size))
        return false;

    if (!se.GetByte("stmflg", &stmflg))
        return false;

    if (!stmflg)
    {
        // Internal
        if (!SetAwb(se_data, se_size))
            return false;
    }

    tracks_data.resize(se.GetNumRows());

    for (unsigned int i = 0; i < se.GetNumRows(); i++)
    {
        if (!se.GetByte("nch", &tracks_data[i].nch, i))
            return false;

        if (!se.GetDword("sfreq", &tracks_data[i].sfreq, i))
            return false;

        if (!se.GetDword("nsmpl", &tracks_data[i].nsmpl, i))
            return false;
    }

    return true;
}

uint8_t *CsbFile::Save(size_t *psize)
{
    if (se_row == -1)
        return nullptr;

    bool modified = (data_modified || (HasAwb() && IsAwbModified()));

    if (modified)
    {
        UtfFile se;
        uint8_t *se_data;
        uint32_t se_size;
        size_t se_size_s;

        if (HasAwb())
        {
            se_data = GetAwb(&se_size);
        }
        else
        {
            se_data = GetBlob("utf", &se_size, false, se_row);
        }

        if (!se_data)
            return nullptr;

        if (!se.Load(se_data, se_size))
            return nullptr;

        for (unsigned int i = 0; i < (unsigned int)tracks_data.size(); i++)
        {
            if (!se.SetByte("nch", tracks_data[i].nch, i))
                return nullptr;

            if (!se.SetDword("sfreq", tracks_data[i].sfreq, i))
                return nullptr;

            if (!se.SetDword("nsmpl", tracks_data[i].nsmpl, i))
                return nullptr;
        }

        se_data = se.Save(&se_size_s);
        if (!se_data)
            return nullptr;

        if (!SetBlob("utf", se_data, (unsigned int)se_size_s, se_row))
            return nullptr;
    }

    return UtfFile::Save(psize);
}

bool CsbFile::SetTrackData(size_t track, uint8_t num_channels, uint32_t sample_rate, uint32_t num_samples)
{
    if (track >= tracks_data.size())
        return false;

    TrackData &data = tracks_data[track];
    data.nch = num_channels;
    data.sfreq = sample_rate;
    data.nsmpl = num_samples;

    data_modified = true;
    return true;
}
