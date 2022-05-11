#include "AcbFile.h"
#include "Utils.h"
#include "debug.h"

AcbFile::AcbFile() : UtfFile()
{
	StreamAwbTocWorkOld = nullptr;
	AwbFile = nullptr;
    CueLimitWorkTable = nullptr;
    StreamAwbTocWork_Old = nullptr;
	PaddingArea = nullptr;
	StreamAwbTocWork = nullptr;
	StreamAwbAfs2Header = nullptr;
	
	Reset();
}

AcbFile::~AcbFile()
{
    Reset();
}

void AcbFile::Reset()
{
    UtfFile::Reset();

	FileIdentifier = 0;
	Size = 0;
	Version = 0;
	Type = 0;
	Target = 0;
	
	memset(AcfMd5Hash, 0, sizeof(AcfMd5Hash));
	
	CategoryExtension = 0;
	
	CueTable.Reset();
	CueNameTable.Reset();
	WaveformTable.Reset();
	AisacTable.Reset();
	GraphTable.Reset();
	GlobalAisacReferenceTable.Reset();
	AisacNameTable.Reset();
	SynthTable.Reset();
	CommandTable.Reset();
	TrackTable.Reset();
	SequenceTable.Reset();
	AisacControlNameTable.Reset();
	AutoModulationTable.Reset();

    CueTable_modified = false;
    CueNameTable_modified = false;
    WaveformTable_modified = false;
    SynthTable_modified = false;
    CommandTable_modified = false;
    TrackTable_modified = false;
    SequenceTable_modified = false;
	
	if (StreamAwbTocWorkOld)
	{
		delete[] StreamAwbTocWorkOld;
		StreamAwbTocWorkOld = nullptr;
	}
	
	StreamAwbTocWorkOld_Size = 0;
	
	if (AwbFile)
	{
		delete[] AwbFile;
		AwbFile = nullptr;
	}
	
	AwbFile_Size = 0;

    VersionString.clear();

    if (CueLimitWorkTable)
    {
        delete[] CueLimitWorkTable;
        CueLimitWorkTable = nullptr;
    }

    CueLimitWorkTable_Size = 0;
	
	NumCueLimitListWorks = 0;
	NumCueLimitNodeWorks = 0;
	
	memset(AcbGuid, 0, sizeof(AcbGuid));
	memset(StreamAwbHash, 0, sizeof(StreamAwbHash));
	
	if (StreamAwbTocWork_Old)
	{
		delete[] StreamAwbTocWork_Old;
		StreamAwbTocWork_Old = nullptr;
	}
	
	StreamAwbTocWork_Old_Size = 0;
	
	AcbVolume = 0.0f;
	
    StringValueTable.Reset();
	OutsideLinkTable.Reset();
	BlockSequenceTable.Reset();
	BlockTable.Reset();
	
	Name.clear();
	
	CharacterEncodingType = 0;
	
	EventTable.Reset();
	ActionTrackTable.Reset();
	AcfReferenceTable.Reset();
	WaveformExtensionDataTable.Reset();
	
	if (PaddingArea)
	{
		delete[] PaddingArea;
		PaddingArea = nullptr;
	}
	
	PaddingArea_Size = 0;
	
	if (StreamAwbTocWork)
	{
		delete[] StreamAwbTocWork;
		StreamAwbTocWork = nullptr;
	}
	
	StreamAwbTocWork_Size = 0;
	
	if (StreamAwbAfs2Header)
	{
		delete[] StreamAwbAfs2Header;
		StreamAwbAfs2Header = nullptr;
	}
	
	StreamAwbAfs2Header_Size = 0;
	
	AwbFile_Modified = false;
	StreamAwbHash_Modified = false;
	StreamAwbAfs2Header_Modified = false;

    // New, for Xenoverse 2 modified format
    awb_header_in_table = false;
    awb_hash_in_table = false;
}

bool AcbFile::LoadTable(const std::string &name, UtfFile &table)
{
	uint8_t *utf_buf;
	unsigned int utf_buf_size;
	
	utf_buf = GetBlob(name, &utf_buf_size);
	if (!utf_buf)
	{
        //DPRINTF("--- %s not present.\n", name.c_str());
		return false;
	}
	
	if (!table.Load(utf_buf, utf_buf_size))
	{
        //DPRINTF("%s: Failed to load table \"%s\"\n", FUNCNAME, name.c_str());
		return false;
	}
	
    //DPRINTF("+++ %s present.\n", name.c_str());
    return true;
}

bool AcbFile::SaveTable(const std::string &name, UtfFile &table)
{
    uint8_t *utf_buf;
    size_t utf_buf_size;

    utf_buf = table.Save(&utf_buf_size);
    if (!utf_buf)
        return false;

    if (!SetBlob(name, utf_buf, (unsigned int)utf_buf_size, 0, true))
    {
        delete[] utf_buf;
        return false;
    }

    return true;
}

bool AcbFile::Load(const uint8_t *buf, size_t size)
{
	if (!UtfFile::Load(buf, size))
		return false;
	
	if (GetDword("FileIdentifier", &FileIdentifier))
	{
		//DPRINTF("+++ FileIdentifier: %x\n", FileIdentifier);
	}
	else
	{
		//DPRINTF("--- FileIdentifier not found.\n");
	}
	
	if (GetDword("Size", &Size))
	{
		//DPRINTF("+++ Size: %x\n", Size);
	}
	else
	{
		//DPRINTF("--- Size not found.\n");
	}
	
	if (GetDword("Version", &Version))
	{
		//DPRINTF("+++ Version: %08x\n", Version);
	}
	else
	{
		//DPRINTF("--- Version not found.\n");
	}
	
	if (GetByte("Type", &Type))
	{
		//DPRINTF("+++ Type: %x\n", Type);
	}
	else
	{
		//DPRINTF("--- Type not found.\n");
	}
	
	if (GetByte("Target", &Target))
	{
		//DPRINTF("+++ Target: %x\n", Target);
	}
	else
	{
		//DPRINTF("--- Target not found.\n");
	}
	
	if (GetFixedBlob("AcfMd5Hash", AcfMd5Hash, sizeof(AcfMd5Hash)))
	{
		//DPRINTF("+++ AcfMd5Hash is present.\n");
	}
	else
	{
		//DPRINTF("--- AcfMd5Hash is not present or has incorrect size.\n");
	}
	
	if (GetByte("CategoryExtension", &CategoryExtension))
	{
		//DPRINTF("+++ CategoryExtension: %x\n", CategoryExtension);
	}
	else
	{
		//DPRINTF("--- CategoryExtension not found.\n");
	}
	
	LoadTable("CueTable", CueTable);
	LoadTable("CueNameTable", CueNameTable);
	LoadTable("WaveformTable", WaveformTable);
	LoadTable("AisacTable", AisacTable);
	LoadTable("GraphTable", GraphTable);
	LoadTable("GlobalAisacReferenceTable", GlobalAisacReferenceTable);
	LoadTable("AisacNameTable", AisacNameTable);
	LoadTable("SynthTable", SynthTable);
	LoadTable("CommandTable", CommandTable);
	LoadTable("TrackTable", TrackTable);
	LoadTable("SequenceTable", SequenceTable);
	LoadTable("AisacControlNameTable", AisacControlNameTable);
    LoadTable("AutoModulationTable", AutoModulationTable);

	StreamAwbTocWorkOld = GetBlob("StreamAwbTocWorkOld", &StreamAwbTocWorkOld_Size, true);
	if (StreamAwbTocWorkOld)
	{
		//DPRINTF("+++ StreamAwbTocWorkOld present.\n");
	}
	else
	{
		//DPRINTF("--- StreamAwbTocWorkOld not present.\n");
	}
	
	AwbFile = GetBlob("AwbFile", &AwbFile_Size, true);
	if (AwbFile)
	{
		//DPRINTF("+++ AwbFile present (this file is a standalone .acb)\n");
	}
	else
	{
		//DPRINTF("--- AwbFile not present (this file uses a external .awb)\n");
	}
	
    if (UtfFile::GetString("VersionString", &VersionString))
	{
		//DPRINTF("+++ VersionString: \"%s\"\n", VersionString.c_str());
	}
	else
	{
		//DPRINTF("--- VersionString not found.\n");
    }

    CueLimitWorkTable = GetBlob("CueLimitWorkTable", &CueLimitWorkTable_Size, true);
    if (CueLimitWorkTable)
    {
        //DPRINTF("+++ CueLimitWorkTable present.\n");
    }
    else
    {
        //DPRINTF("--- CueLimitWorkTable not present.\n");
    }

    if (GetWord("NumCueLimitListWorks", &NumCueLimitListWorks))
	{
        //DPRINTF("+++ NumCueLimitListWorks: %x\n", NumCueLimitListWorks);
	}
	else
	{
		//DPRINTF("--- NumCueLimitListWorks not found.\n");
	}
	
	if (GetWord("NumCueLimitNodeWorks", &NumCueLimitNodeWorks))
	{
        //DPRINTF("+++ NumCueLimitNodeWorks: %x\n", NumCueLimitNodeWorks);
	}
	else
	{
		//DPRINTF("--- NumCueLimitNodeWorks not found.\n");
	}
	
	if (GetFixedBlob("AcbGuid", AcbGuid, sizeof(AcbGuid)))
	{
		//DPRINTF("+++ AcbGuid present.\n");
	}
	else
	{
		//DPRINTF("--- AcbGuid not present.\n");
	}
	
	if (GetFixedBlob("StreamAwbHash", StreamAwbHash, sizeof(StreamAwbHash)))
	{
        //DPRINTF("+++ StreamAwbHash present.\n");       
	}
	else
	{
        // Xenoverse 2 new format

        UtfFile hash;

        if (LoadTable("StreamAwbHash", hash))
        {
            if (hash.GetFixedBlob("Hash", StreamAwbHash, sizeof(StreamAwbHash)))
            {
                awb_hash_in_table = true;
            }
        }
        //DPRINTF("--- StreamAwbHash not present.\n");
	}
	
	StreamAwbTocWork_Old = GetBlob("StreamAwbTocWork_Old", &StreamAwbTocWork_Old_Size, true);
	if (StreamAwbTocWork_Old)
	{
		//DPRINTF("+++ StreamAwbTocWork_Old present.\n");
	}
	else
	{
		//DPRINTF("--- StreamAwbTocWork_Old not present.\n");
	}
	
	if (GetFloat("AcbVolume", &AcbVolume))
	{
        //DPRINTF("+++ AcbVolume: %f\n", AcbVolume);
	}
	else
	{
		//DPRINTF("--- AcbVolume not present.\n");
	}

    LoadTable("StringValueTable", StringValueTable);
	LoadTable("OutsideLinkTable", OutsideLinkTable);
	LoadTable("BlockSequenceTable", BlockSequenceTable);
	LoadTable("BlockTable", BlockTable);    
	
    if (UtfFile::GetString("Name", &Name))
	{
		//DPRINTF("+++ Name: %s\n", Name.c_str());
	}
	else
	{
		//DPRINTF("--- Name not found.\n");
	}
	
	if (GetByte("CharacterEncodingType", &CharacterEncodingType))
	{
		//DPRINTF("+++ CharacterEncodingType: %x\n", CharacterEncodingType);
	}
	else
	{
		//DPRINTF("--- CharacterEncodingType not found.\n");
	}
	
	LoadTable("EventTable", EventTable);
	LoadTable("ActionTrackTable", ActionTrackTable);
	LoadTable("AcfReferenceTable", AcfReferenceTable);
	LoadTable("WaveformExtensionDataTable", WaveformExtensionDataTable);

    PaddingArea = GetBlob("PaddingArea", &PaddingArea_Size, true);
	if (PaddingArea)
	{
		//DPRINTF("+++ PaddingArea present.\n");
	}
	else
	{
		//DPRINTF("--- PaddingArea not present.\n");
	}
	
	StreamAwbTocWork = GetBlob("StreamAwbTocWork", &StreamAwbTocWork_Size, true);
	if (StreamAwbTocWork)
	{
		//DPRINTF("+++ StreamAwbTocWork present.\n");
	}
	else
	{
		//DPRINTF("--- StreamAwbTocWork not present.\n");
	}
	
	StreamAwbAfs2Header = GetBlob("StreamAwbAfs2Header", &StreamAwbAfs2Header_Size, true);
	if (StreamAwbAfs2Header)
	{
        //DPRINTF("+++ StreamAwbAfs2Header present. (typical of .acb that needs external .awb)\n");
        if (StreamAwbAfs2Header_Size >= sizeof(uint32_t) && *(uint32_t *)StreamAwbAfs2Header == UTF_SIGNATURE)
        {
            // Xenoverse 2 new format;

            awb_header_in_table = true;
            delete[] StreamAwbAfs2Header;
            StreamAwbAfs2Header = nullptr;
            StreamAwbAfs2Header_Size = 0;

            UtfFile header;

            if (!LoadTable("StreamAwbAfs2Header", header))
                return false;

            StreamAwbAfs2Header = header.GetBlob("Header", &StreamAwbAfs2Header_Size, true);
        }
	}
	else
	{
		//DPRINTF("--- StreamAwbAfs2Header not present. (typical of standalone .acb)\n");
    }

    /*CueTable.Debug();
    CueNameTable.Debug();
    WaveformTable.Debug();
    SynthTable.Debug();
    CommandTable.Debug();
    TrackTable.Debug();
    SequenceTable.Debug();*/

    return true;
}

uint8_t *AcbFile::Save(size_t *psize)
{
	if (AwbFile_Modified)
	{
		if (!SetBlob("AwbFile", AwbFile, AwbFile_Size))
		{
			DPRINTF("%s: SetBlob failed.\n", FUNCNAME);
			return nullptr;
		}
		
		AwbFile_Modified = false;
	}
	
    if (StreamAwbHash_Modified && !awb_hash_in_table)
	{
		if (!SetBlob("StreamAwbHash", StreamAwbHash, sizeof(StreamAwbHash)))
		{
			DPRINTF("%s: SetBlob failed.\n", FUNCNAME);
			return nullptr;
		}
		
		StreamAwbHash_Modified = false;
	}
	
    if (StreamAwbAfs2Header_Modified && !awb_header_in_table)
	{
		if (!SetBlob("StreamAwbAfs2Header", StreamAwbAfs2Header, StreamAwbAfs2Header_Size))
		{
			DPRINTF("%s: SetBlob failed.\n", FUNCNAME);
			return nullptr;
		}
		
		StreamAwbAfs2Header_Modified = false;
	}

    if (CueTable_modified && !SaveTable("CueTable", CueTable))
        return nullptr;

    if (CueNameTable_modified && !SaveTable("CueNameTable", CueNameTable))
        return nullptr;

    if (WaveformTable_modified && !SaveTable("WaveformTable", WaveformTable))
        return nullptr;

    if (SynthTable_modified && !SaveTable("SynthTable", SynthTable))
        return nullptr;

    if (CommandTable_modified && !SaveTable("CommandTable", CommandTable))
        return nullptr;

    if (TrackTable_modified && !SaveTable("TrackTable", TrackTable))
        return nullptr;

    if (SequenceTable_modified && !SaveTable("SequenceTable", SequenceTable))
        return nullptr;
	
    return UtfFile::Save(psize);
}

bool AcbFile::LoadFromFile(const std::string &path, bool show_error)
{
    return CriwareAudioContainer::LoadFromFile(path, show_error);
}

bool AcbFile::SaveToFile(const std::string &path, bool show_error, bool build_path)
{
    return CriwareAudioContainer::SaveToFile(path, show_error, build_path);
}

uint8_t *AcbFile::GetAwb(uint32_t *awb_size) const
{
	if (!HasAwb())
		return nullptr;
	
	*awb_size = AwbFile_Size;
	return AwbFile;
}

uint8_t *AcbFile::GetAwbHeader(uint32_t *header_size) const
{
	if (!HasAwbHeader())
		return nullptr;

    *header_size = StreamAwbAfs2Header_Size;
     return StreamAwbAfs2Header;

}

bool AcbFile::SetAwb(void *awb, uint32_t awb_size, bool take_ownership)
{
	if (AwbFile)
		delete[] AwbFile;
		
	AwbFile = nullptr;
	AwbFile_Size = 0;
	
	if (awb)
	{
        if (take_ownership)
        {
            AwbFile = (uint8_t *)awb;
        }
        else
        {
            AwbFile = new uint8_t[awb_size];
            memcpy(AwbFile, awb, awb_size);
        }

		AwbFile_Size = awb_size;
	}	
	
	AwbFile_Modified = true;
    return true;
}

bool AcbFile::SetExternalAwbHash(uint8_t *hash)
{
	memcpy(StreamAwbHash, hash, sizeof(StreamAwbHash));

    if (awb_hash_in_table)
    {
        UtfFile hash;

        if (!LoadTable("StreamAwbHash", hash))
            return false;

        if (!hash.SetBlob("Hash", StreamAwbHash, sizeof(StreamAwbHash)))
            return false;

        size_t size;
        uint8_t *buf = hash.Save(&size);

        if (!buf)
            return false;

        return SetBlob("StreamAwbHash", buf, (unsigned int)size, 0, true);
    }
    else
    {
        StreamAwbHash_Modified = true;
    }

	return true;
}

bool AcbFile::SetAwbHeader(void *header_buf, uint32_t header_size, bool take_ownership)
{
	if (StreamAwbAfs2Header)
		delete[] StreamAwbAfs2Header;
		
	StreamAwbAfs2Header = nullptr;
	StreamAwbAfs2Header_Size = 0;
	
    if (header_buf)
	{
        if (take_ownership)
        {
            StreamAwbAfs2Header = (uint8_t *)header_buf;
        }
        else
        {
            StreamAwbAfs2Header = new uint8_t[header_size];
            memcpy(StreamAwbAfs2Header, header_buf, header_size);
        }

		StreamAwbAfs2Header_Size = header_size;

        if (awb_header_in_table)
        {
            UtfFile header;

            if (!LoadTable("StreamAwbAfs2Header", header) || !header.SetBlob("Header", StreamAwbAfs2Header, StreamAwbAfs2Header_Size))
            {
                if (take_ownership)
                {
                    uint8_t *header_buf8 = (uint8_t *)header_buf;
                    delete[] header_buf8;
                }

                StreamAwbAfs2Header = nullptr;
                StreamAwbAfs2Header_Size = 0;
                return false;
            }

            size_t size;
            uint8_t *buf = header.Save(&size);

            if (!buf)
                return false;

            return SetBlob("StreamAwbAfs2Header", buf, (unsigned int)size, 0, true);
        }
	}	
	
	StreamAwbAfs2Header_Modified = true;
    return true;
}

uint32_t AcbFile::GetNumTracks() const
{
    if (WaveformTable.GetNumColumns() == 0)
        return 0;

    return WaveformTable.GetNumRows();
}

uint32_t AcbFile::GetFilesPerTrack() const
{
    DPRINTF("%s: Depecreated function, remove the call from code.\n", FUNCNAME);

    if (SequenceTable.GetNumColumns() == 0)
        return 1;

    uint16_t num_tracks;

    if (!SequenceTable.GetWord("NumTracks", &num_tracks))
        return 1;

    return num_tracks;
}

bool AcbFile::CanAddTrack() const
{
    if (SequenceTable.GetNumColumns() == 0)
        return false;

    uint16_t num_tracks;

    if (!SequenceTable.IsVariableColumn("NumTracks"))
    {
        if (!SequenceTable.GetWord("NumTracks", &num_tracks))
            return false;

        if (num_tracks != 1)
            return false;
    }
    else
    {
        for (uint32_t i = 0; i < SequenceTable.GetNumRows(); i++)
        {
            if (!SequenceTable.GetWord("NumTracks", &num_tracks, i))
                return false;

            if (num_tracks != 1)
                return false;
        }
    }

    return true;
}

uint32_t AcbFile::AwbIndexToTrackIndex(uint32_t idx, bool external) const
{
    if (WaveformTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    for (uint32_t i = 0; i < WaveformTable.GetNumRows(); i++)
    {
        uint16_t awb_idx;

        if (WaveformTable.GetWord((external) ? "StreamAwbId" : "MemoryAwbId", &awb_idx, i))
        {
            if (idx == awb_idx)
            {
                return i;
            }
        }
        else if (WaveformTable.GetWord("Id", &awb_idx, i))
        {
            if (idx == awb_idx)
            {
                uint8_t streaming;

                if (!WaveformTable.GetByte("Streaming", &streaming, i))
                    return (uint32_t)-1;

                if ((streaming && !external) || (!streaming && external))
                    return (uint32_t)-1;

                return i;
            }
        }
    }

    return (uint32_t)-1;
}

uint32_t AcbFile::TrackIndexToAwbIndex(uint32_t idx, bool *external) const
{
    uint16_t awb_idx;

    if (WaveformTable.GetWord("StreamAwbId", &awb_idx, idx) && awb_idx != 0xFFFF)
    {
        *external = true;
        return awb_idx;
    }
    else if (WaveformTable.GetWord("MemoryAwbId", &awb_idx, idx) && awb_idx != 0xFFFF)
    {
        *external = false;
        return awb_idx;
    }
    else if (WaveformTable.GetWord("Id", &awb_idx, idx) && awb_idx != 0xFFFF)
    {
        uint8_t streaming;

        if (!WaveformTable.GetByte("Streaming", &streaming, idx))
            return (uint32_t)-1;

        *external = (streaming != 0);
        return awb_idx;
    }

    return (uint32_t)-1;
}

uint32_t AcbFile::TrackIndexToCueId(uint32_t idx) const
{
    if (SequenceTable.GetNumColumns() == 0 || TrackTable.GetNumColumns() == 0 || CommandTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    for (unsigned int synth_i = 0; synth_i < SynthTable.GetNumRows(); synth_i++)
    {
        unsigned int ref_size;
        uint8_t *ref = SynthTable.GetBlob("ReferenceItems", &ref_size, false, synth_i);
        if (!ref || ref_size < 4)
            continue;

        if (be16(*(uint16_t *)&ref[2]) == idx)
        {
            for (unsigned int command_i = 0; command_i < CommandTable.GetNumRows(); command_i++)
            {
                unsigned int command_size;
                static const uint8_t hdr[] = { 0x07, 0xD0, 0x04, 0x00 };
                uint8_t *cmd = CommandTable.GetBlob("Command", &command_size, false, command_i);
                if (!cmd || command_size < 10 || memcmp(cmd, hdr, sizeof(hdr)) != 0)
                    continue;

                if (be16(*(uint16_t *)&cmd[5]) == synth_i)
                {
                    for (unsigned int track_i = 0; track_i < TrackTable.GetNumRows(); track_i++)
                    {
                        uint16_t ev;
                        if (!TrackTable.GetWord("EventIndex", &ev, track_i))
                            continue;

                        if (ev == command_i)
                        {
                            for (uint32_t sequence_i = 0; sequence_i < SequenceTable.GetNumRows(); sequence_i++)
                            {
                                unsigned int indexes_size;
                                uint16_t *indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &indexes_size, false, sequence_i);
                                if (!indexes || indexes_size < sizeof(uint16_t))
                                    continue;

                                for (unsigned int i = 0; i< indexes_size / sizeof(uint16_t); i++)
                                {
                                    if (be16(indexes[i]) == track_i)
                                        return sequence_i;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}

size_t AcbFile::TrackIndexToCueIds(uint32_t idx, std::vector<uint32_t> &cue_ids) const
{
    cue_ids.clear();

    if (SequenceTable.GetNumColumns() == 0 || TrackTable.GetNumColumns() == 0 || CommandTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return 0;

    for (unsigned int synth_i = 0; synth_i < SynthTable.GetNumRows(); synth_i++)
    {
        unsigned int ref_size;
        uint8_t *ref = SynthTable.GetBlob("ReferenceItems", &ref_size, false, synth_i);
        if (!ref || ref_size < 4)
            continue;

        if (be16(*(uint16_t *)&ref[2]) == idx)
        {
            for (unsigned int command_i = 0; command_i < CommandTable.GetNumRows(); command_i++)
            {
                unsigned int command_size;
                static const uint8_t hdr[] = { 0x07, 0xD0, 0x04, 0x00 };
                uint8_t *cmd = CommandTable.GetBlob("Command", &command_size, false, command_i);
                if (!cmd || command_size < 10 || memcmp(cmd, hdr, sizeof(hdr)) != 0)
                    continue;

                if (be16(*(uint16_t *)&cmd[5]) == synth_i)
                {
                    for (unsigned int track_i = 0; track_i < TrackTable.GetNumRows(); track_i++)
                    {
                        uint16_t ev;
                        if (!TrackTable.GetWord("EventIndex", &ev, track_i))
                            continue;

                        if (ev == command_i)
                        {
                            for (uint32_t sequence_i = 0; sequence_i < SequenceTable.GetNumRows(); sequence_i++)
                            {
                                unsigned int indexes_size;
                                uint16_t *indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &indexes_size, false, sequence_i);
                                if (!indexes || indexes_size < sizeof(uint16_t))
                                    continue;

                                for (unsigned int i = 0; i< indexes_size / sizeof(uint16_t); i++)
                                {
                                    if (be16(indexes[i]) == track_i)
                                        cue_ids.push_back(sequence_i);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return cue_ids.size();
}

uint32_t AcbFile::CueIdToTrackIndex(uint32_t id) const
{
    if (SequenceTable.GetNumColumns() == 0 || TrackTable.GetNumColumns() == 0 || CommandTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    unsigned int size;
    uint16_t *tt_index = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, id);
    if (!tt_index || size < sizeof(uint16_t))
        return (uint32_t) -1;

    uint16_t ev;
    if (!TrackTable.GetWord("EventIndex", &ev, be16(*tt_index)))
        return (uint32_t)-1;

    unsigned int command_size;
    static const uint8_t hdr[] = { 0x07, 0xD0, 0x04, 0x00 };
    uint8_t *cmd = CommandTable.GetBlob("Command", &command_size, false, ev);
    if (!cmd || command_size < 10 || memcmp(cmd, hdr, sizeof(hdr)) != 0)
        return (uint32_t)-1;

    uint16_t ref_idx = be16(*(uint16_t *)&cmd[5]);
    unsigned int ref_size;
    uint8_t *ref = SynthTable.GetBlob("ReferenceItems", &ref_size, false, ref_idx);
    if (!ref || ref_size < 4)
        return (uint32_t)-1;

    return be16(*(uint16_t *)&ref[2]);
}

size_t AcbFile::CueIdToTrackIndexes(uint32_t id, std::vector<uint32_t> &idxs) const
{
    idxs.clear();

    if (SequenceTable.GetNumColumns() == 0 || TrackTable.GetNumColumns() == 0 || CommandTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return 0;

    unsigned int size;
    uint16_t *tt_indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, id);
    if (!tt_indexes || size < sizeof(uint16_t))
        return 0;

    for (unsigned int i = 0; i < size / sizeof(uint16_t); i++)
    {
        uint16_t ev;
        if (!TrackTable.GetWord("EventIndex", &ev, be16(tt_indexes[i])))
            continue;

        unsigned int command_size;
        static const uint8_t hdr[] = { 0x07, 0xD0, 0x04, 0x00 };
        uint8_t *cmd = CommandTable.GetBlob("Command", &command_size, false, ev);
        if (!cmd || command_size < 10 || memcmp(cmd, hdr, sizeof(hdr)) != 0)
            continue;

        uint16_t ref_idx = be16(*(uint16_t *)&cmd[5]);
        unsigned int ref_size;
        uint8_t *ref = SynthTable.GetBlob("ReferenceItems", &ref_size, false, ref_idx);
        if (!ref || ref_size < 4)
            continue;

        idxs.push_back(be16(*(uint16_t *)&ref[2]));
    }

    return idxs.size();
}

uint32_t AcbFile::AwbIndexToCueId(uint32_t idx, bool external) const
{
    uint32_t track_idx =  AwbIndexToTrackIndex(idx, external);
    if (track_idx == (uint32_t)-1)
        return track_idx;

    return (uint32_t)TrackIndexToCueId(track_idx);
}

uint32_t AcbFile::CueIdToAwbIndex(uint32_t id, bool *external) const
{
    if (WaveformTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    uint32_t track_idx = CueIdToTrackIndex(id);
    if (track_idx == (uint32_t)-1)
        return track_idx;

    return TrackIndexToAwbIndex(track_idx, external);
}

/*uint32_t AcbFile::TrackIndexToCueIndex(uint32_t idx) const
{  
    if (SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;   

    for (uint32_t i = 0; i < SynthTable.GetNumRows(); i++)
    {
        uint32_t index;

        if (SynthTable.GetFixedBlob("ReferenceItems", (uint8_t *)&index, sizeof(uint32_t), i))
        {
            index = be32(index)&0xFFFF;

            if (index == idx)
            {
                //printf("%d to %d\n", idx, i);
                return i;
            }
        }
    }

    return (uint32_t)-1;
}

size_t AcbFile::TrackIndexToCueIndexes(uint32_t idx, std::vector<uint32_t> &cue_idxs) const
{
    cue_idxs.clear();

    if (SynthTable.GetNumColumns() == 0)
        return 0;

    for (uint32_t i = 0; i < SynthTable.GetNumRows(); i++)
    {
        uint32_t index;

        if (SynthTable.GetFixedBlob("ReferenceItems", (uint8_t *)&index, sizeof(uint32_t), i))
        {
            index = be32(index)&0xFFFF;

            if (index == idx)
            {
                //printf("%d to %d\n", idx, i);
                cue_idxs.push_back(i);
            }
        }
    }

    return cue_idxs.size();
}

uint32_t AcbFile::CueIndexToTrackIndex(uint32_t idx) const
{
    if (SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    uint32_t track_idx;

    if (!SynthTable.GetFixedBlob("ReferenceItems", (uint8_t *)&track_idx, sizeof(uint32_t), idx))
        return (uint32_t)-1;

    return be32(track_idx) & 0xFFFF;
}

uint32_t AcbFile::AwbIndexToCueIndex(uint32_t idx, bool external) const
{
    uint32_t track_idx =  AwbIndexToTrackIndex(idx, external);
    if (track_idx == (uint32_t)-1)
        return track_idx;

    return (uint32_t)TrackIndexToCueIndex(track_idx);
}

uint32_t AcbFile::CueIndexToAwbIndex(uint32_t idx, bool *external) const
{
    if (WaveformTable.GetNumColumns() == 0 || SynthTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    uint32_t track_idx = CueIndexToTrackIndex(idx);
    if (track_idx == (uint32_t)-1)
        return track_idx;

    return TrackIndexToAwbIndex(track_idx, external);
}

uint32_t AcbFile::CueIndexToCueId(uint32_t idx) const
{    
    if (SequenceTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    for (uint32_t i = 0; i < SequenceTable.GetNumRows(); i++)
    {
        unsigned int size;
        uint16_t *indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, i);

        if (indexes)
        {
            for (uint32_t j = 0; j < size / 2; j++)
            {
                if (be16(indexes[j]) == idx)
                    return i;
            }
        }
    }

    return (uint32_t)-1;
}

uint32_t AcbFile::CueIdToCueIndex(uint32_t id) const
{
    if (SequenceTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    unsigned int size;
    uint16_t *indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, id);
    if (!indexes)
        return (uint32_t)-1;

    return be16(indexes[0]);
}

size_t AcbFile::CueIdToCueIndexes(uint32_t id, std::vector<uint32_t> &cue_idxs) const
{
    cue_idxs.clear();

    if (SequenceTable.GetNumColumns() == 0)
        return 0;

    unsigned int size;
    uint16_t *indexes = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, id);
    if (!indexes)
        return 0;

    for (uint32_t i = 0; i < size / 2; i++)
    {
        cue_idxs.push_back(be16(indexes[i]));
    }

    return cue_idxs.size();
}*/

bool AcbFile::GetCueName(uint32_t cue_id, std::string *name) const
{
    if (CueNameTable.GetNumColumns() == 0)
        return false;

    for (uint32_t i = 0; i < CueNameTable.GetNumRows(); i++)
    {
        uint16_t index;

        if (CueNameTable.GetWord("CueIndex", &index, i) && index == cue_id)
        {
            return CueNameTable.GetString("CueName", name, i);
        }
    }

    return false;
}

bool AcbFile::SetCueName(uint32_t cue_id, const std::string &name)
{
    if (CueNameTable.GetNumColumns() == 0)
        return false;

    for (uint32_t i = 0; i < CueNameTable.GetNumRows(); i++)
    {
        uint16_t index;

        if (CueNameTable.GetWord("CueIndex", &index, i) && index == cue_id)
        {
            bool ret = CueNameTable.SetString("CueName", name, i, false);
            if (ret)
                CueNameTable_modified = true;

            return ret;
        }
    }

    return false;
}

uint32_t AcbFile::FindCueId(const std::string &name) const
{
    if (CueNameTable.GetNumColumns() == 0)
        return (uint32_t)-1;

    for (uint32_t i = 0; i < CueNameTable.GetNumRows(); i++)
    {
        std::string str;

        if (CueNameTable.GetString("CueName", &str, i) && name == str)
        {
            uint16_t cue_id;

            if (CueNameTable.GetWord("CueIndex", &cue_id, i))
                return cue_id;

            return (uint32_t)-1;
        }
    }

    return (uint32_t)-1;
}

bool AcbFile::AddTrack(const std::string &name, uint16_t awb_idx, bool external, uint32_t track_length_ms, uint32_t track_length_smp, bool loop_flag, uint16_t sample_rate, uint8_t num_channels, uint16_t cmd_idx1, uint16_t cmd_idx2, uint32_t *cue_id, uint32_t *cue_idx, uint32_t *track_idx)
{
    uint32_t _cue_id, _cue_idx, _track_idx;

    /***if (!CanAddTrack())
    {
        DPRINTF("%s: FIXME, not implemented for this kind of acb.\n", FUNCNAME);
        return false;
    }***/

    if (CueTable.GetNumColumns() == 0 || CueNameTable.GetNumColumns() == 0 || WaveformTable.GetNumColumns() == 0 ||
        SynthTable.GetNumColumns() == 0 || TrackTable.GetNumColumns() == 0 || SequenceTable.GetNumColumns() == 0 ||
        CommandTable.GetNumColumns() == 0)
    {
        DPRINTF("%s: Failed because one of the tables we were expecting didn't exist.\n", FUNCNAME);
        return false;
    }

    if (!CueTable.ColumnExists("CueId"))
        return false;

    if (CueTable.GetHighestRow32("CueId", &_cue_id) == (uint32_t)-1)
        return false;

    _cue_id++;
    _cue_idx = (uint32_t) CueTable.GetNumRows();

    //UPRINTF("cue_id = %d, cue_idx = %d.\n", _cue_id, _cue_idx);

    if (!CueTable.CreateRow())
        return false;

    if (!CueTable.SetDword("CueId", _cue_id, _cue_idx, false))
        return false;

    if (CueTable.IsVariableColumn("ReferenceIndex"))
    {
        if (!CueTable.SetWord("ReferenceIndex", (uint16_t)_cue_idx, _cue_idx, false))
            return false;
    }

    if (!CueTable.SetDword("Length", track_length_ms, _cue_idx, false))
        return false;

    if (CueTable.IsVariableColumn("NumAisacControlMaps"))
    {
        // Atm, set to 0
        if (!CueTable.SetByte("NumAisacControlMaps", 0, _cue_idx, false))
        {
            DPRINTF("%s: SetByte failed on CueTable::NumAisacControlMaps.\n", FUNCNAME);
            return false;
        }
    }

    if (!CueNameTable.ColumnExists("CueName") || !CueNameTable.ColumnExists("CueIndex"))
        return false;

    if (CueNameTable.GetNumRows() != _cue_idx)
    {
        DPRINTF("%s: unexpected number of rows in CueNameTable (%d != %d).\n", FUNCNAME, CueNameTable.GetNumRows(), _cue_idx);
        return false;
    }    

    if (!CueNameTable.CreateRow())
        return false;

    if (!CueNameTable.SetString("CueName", name, _cue_idx, false))
        return false;

    if (!CueNameTable.SetWord("CueIndex", (uint16_t)_cue_idx, _cue_idx, false))
        return false;

    bool stream_exists;
    bool memory_exists;

    stream_exists = WaveformTable.ColumnExists("StreamAwbId");
    memory_exists = WaveformTable.ColumnExists("MemoryAwbId");

    if (stream_exists && !memory_exists)
    {
        DPRINTF("%s: StreamAwbId exists but MemoryAwbId doesn't, cannot handle this situation.\n", FUNCNAME);
        return false;
    }
    else if (memory_exists && !stream_exists)
    {
        DPRINTF("%s: MemoryAwbId exists but StreamAwbId doesn't, cannot handle this situation.\n", FUNCNAME);
        return false;
    }

    _track_idx = WaveformTable.GetNumRows();

    if (!WaveformTable.CreateRow())
        return false;    

    if (stream_exists)
    {
        if (external)
        {
            if (!WaveformTable.SetWord("StreamAwbId", (uint16_t)awb_idx, _track_idx, false))
            {
                DPRINTF("%s: Cannot set StreamAwbId. Probably you can only use internal awb here.\n", FUNCNAME);
                return false;
            }

            if (WaveformTable.IsVariableColumn("MemoryAwbId"))
            {
                if (!WaveformTable.SetWord("MemoryAwbId", 0xFFFF, _track_idx, false))
                    return false;
            }
        }
        else
        {
            if (!WaveformTable.SetWord("MemoryAwbId", (uint16_t)awb_idx, _track_idx, false))
                return false;

            if (WaveformTable.IsVariableColumn("StreamAwbId"))
            {
                if (!WaveformTable.SetWord("StreamAwbId", 0xFFFF, _track_idx, false))
                    return false;
            }
        }
    }
    else
    {
        if (!WaveformTable.SetWord("Id", (uint16_t)awb_idx, _track_idx, false))
            return false;
    }

    if (!WaveformTable.IsVariableColumn("Streaming"))
    {
        uint8_t streaming;

        if (WaveformTable.GetByte("Streaming", &streaming))
        {
            if (streaming && !external)
            {
                DPRINTF("%s: We failed because this acb uses only external awb and the function tried to set an internal track.\n", FUNCNAME);
                return false;
            }
            else if (!streaming && external)
            {
                DPRINTF("%s: We failed because this acb uses only internal awb and the function tried to set an external track.\n", FUNCNAME);
                return false;
            }
        }

    }
    else
    {
        if (!WaveformTable.SetByte("Streaming", (external) ? 1 : 0, _track_idx, false))
            return false;
    }

    if (WaveformTable.IsVariableColumn("StreamAwbPortNo"))
    {
        if (!WaveformTable.SetWord("StreamAwbPortNo", (external) ? 0 : 0xFFFF, _track_idx, false))
            return false;
    }

    if (!WaveformTable.SetDword("NumSamples", track_length_smp, _track_idx, false))
        return false;

    if (WaveformTable.IsVariableColumn("LoopFlag"))
    {
        if (!WaveformTable.SetByte("LoopFlag", (loop_flag) ? 1 : 0, _track_idx, false))
            return false;
    }

    if (WaveformTable.IsVariableColumn("SamplingRate"))
    {
        if (!WaveformTable.SetWord("SamplingRate", sample_rate, _track_idx, false))
            return false;
    }

    if (WaveformTable.IsVariableColumn("NumChannels"))
    {
        if (!WaveformTable.SetByte("NumChannels", num_channels, _track_idx, false))
            return false;
    }

    if (WaveformTable.IsVariableColumn("ExtensionData"))
    {
        /***DPRINTF("%s: FIXME: implement ExtensionData.\n", FUNCNAME);
        return false;***/

        // Atm let's set this to 0xFFFF
        if (!WaveformTable.SetWord("ExtensionData", 0xFFFF, WaveformTable.GetNumRows()-1))
        {
            DPRINTF("%s: SetWord failed on WaveformTable::ExtensionData.\n", FUNCNAME);
            return false;
        }
    }

    /***if (SynthTable.GetNumRows() != _cue_idx)
    {
        DPRINTF("%s: unexpected number of rows in SynthTable.\n", FUNCNAME);
        return false;
    }***/

    if (!SynthTable.CreateRow())
    {
        DPRINTF("%s: CreateRow failed on SynthTable.\n", FUNCNAME);
        return false;
    }

    if (SynthTable.IsVariableColumn("CommandIndex"))
    {
        if (!SynthTable.SetWord("CommandIndex", (external) ? 0 : 0xFFFF, SynthTable.GetNumRows()-1, false))
        {
            DPRINTF("%s: SetWord failed on SynthTable::CommandIndex.\n", FUNCNAME);
            return false;
        }
    }

    uint32_t ref_item = be32(0x10000 | _track_idx);
    ///// if (!SynthTable.SetBlob("ReferenceItems", (uint8_t *)&ref_item, sizeof(uint32_t), _cue_idx, false, false))
    if (!SynthTable.SetBlob("ReferenceItems", (uint8_t *)&ref_item, sizeof(uint32_t), SynthTable.GetNumRows()-1, false, false))
    {
        DPRINTF("%s: SetBlob failed on SynthTable::ReferenceItems.\n", FUNCNAME);
        return false;
    }

    if (SynthTable.ColumnExists("ControlWorkArea1"))
    {
        /////if (!SynthTable.SetWord("ControlWorkArea1", (uint16_t)_cue_idx, _cue_idx, false))
        if (!SynthTable.SetWord("ControlWorkArea1", (uint16_t)SynthTable.GetNumRows()-1, SynthTable.GetNumRows()-1, false))
            return false;
    }

    if (SynthTable.ColumnExists("ControlWorkArea2"))
    {
        /////if (!SynthTable.SetWord("ControlWorkArea2", (uint16_t)_cue_idx, _cue_idx, false))
        if (!SynthTable.SetWord("ControlWorkArea2", (uint16_t)SynthTable.GetNumRows()-1, SynthTable.GetNumRows()-1, false))
            return false;
    }

    uint16_t ev_idx = (uint16_t)CommandTable.GetNumRows();

    if (!CommandTable.CreateRow())
        return false;

    uint8_t cmd_data[10];

    cmd_data[0] = 7;
    cmd_data[1] = 0xD0;
    cmd_data[2] = 4;
    cmd_data[3] = 0;
    cmd_data[4] = 2;
    /////*(uint16_t *)&cmd_data[5] = be16((uint16_t)_cue_idx);
    *(uint16_t *)&cmd_data[5] = be16((uint16_t)SynthTable.GetNumRows()-1);
    cmd_data[7] = cmd_data[8] = cmd_data[9] = 0;    

    if (!CommandTable.SetBlob("Command", cmd_data, sizeof(cmd_data), ev_idx, false, false))
        return false;

    /***if (TrackTable.GetNumRows() != _cue_idx)
    {
        DPRINTF("%s: unexpected number of rows in TrackTable.\n", FUNCNAME);
        return false;
    }***/

    if (!TrackTable.CreateRow())
        return false;

    if (TrackTable.IsVariableColumn("CommandIndex"))
    {
        /////if (!TrackTable.SetWord("CommandIndex", cmd_idx1, _cue_idx, false))
        if (!TrackTable.SetWord("CommandIndex", cmd_idx1, TrackTable.GetNumRows()-1, false))
            return false;
    }

    if (TrackTable.IsVariableColumn("EventIndex"))
    {
        /////if (!TrackTable.SetWord("EventIndex", ev_idx, _cue_idx, false))
        if (!TrackTable.SetWord("EventIndex", ev_idx, TrackTable.GetNumRows()-1, false))
            return false;
    }

    if (SequenceTable.GetNumRows() != _cue_idx)
    {
        DPRINTF("%s: unexpected number of rows in SequenceTable.\n", FUNCNAME);
        return false;
    }

    if (!SequenceTable.CreateRow())
        return false;

    //uint16_t track_index = be16((uint16_t)_cue_idx);
    uint16_t track_index = be16((uint16_t)TrackTable.GetNumRows()-1);

    if (!SequenceTable.SetBlob("TrackIndex", (uint8_t *)&track_index, sizeof(uint16_t), _cue_idx, false, false))
        return false;

    if (SequenceTable.IsVariableColumn("CommandIndex"))
    {
        if (!SequenceTable.SetWord("CommandIndex", cmd_idx2, _cue_idx, false))
            return false;
    }

    if (SequenceTable.ColumnExists("ControlWorkArea1"))
    {
        if (!SequenceTable.SetWord("ControlWorkArea1", (uint16_t)_cue_idx, _cue_idx, false))
            return false;
    }

    if (SequenceTable.ColumnExists("ControlWorkArea2"))
    {
        if (!SequenceTable.SetWord("ControlWorkArea2", (uint16_t)_cue_idx, _cue_idx, false))
            return false;
    }

    if (SequenceTable.ColumnExists("NumTracks") && SequenceTable.IsVariableColumn("NumTracks"))
    {
        if (!SequenceTable.SetWord("NumTracks", 1, _cue_idx, false))
            return false;
    }

    CueTable_modified = true;
    CueNameTable_modified = true;
    WaveformTable_modified = true;
    SynthTable_modified = true;
    CommandTable_modified = true;
    TrackTable_modified = true;
    SequenceTable_modified = true;

    if (cue_id)
        *cue_id = _cue_id;

    if (cue_idx)
        *cue_idx = _cue_idx;

    if (track_idx)
        *track_idx = _track_idx;

    return true;
}

bool AcbFile::GetTrackCommandIndex(uint32_t cue_id, uint16_t *cmd_idx) const
{
    if (SequenceTable.GetNumColumns() == 0 && TrackTable.GetNumColumns() == 0)
        return false;

    unsigned int size;
    uint16_t *index = (uint16_t *)SequenceTable.GetBlob("TrackIndex", &size, false, cue_id);
    if (!index || size < sizeof(uint16_t))
        return false;

    return TrackTable.GetWord("CommandIndex", cmd_idx, be16(*index));
}

bool AcbFile::GetSequenceCommandIndex(uint32_t cue_id, uint16_t *cmd_idx) const
{
    if (SequenceTable.GetNumColumns() == 0)
        return false;

    return SequenceTable.GetWord("CommandIndex", cmd_idx, cue_id);
}

bool AcbFile::SetCueLength(uint32_t cue_id, uint32_t length_ms)
{
    if (CueTable.GetNumColumns() == 0)
        return false;

    bool ret = CueTable.SetDword("Length", length_ms, cue_id, false);
    if (ret)
        CueTable_modified = true;

    return ret;
}

bool AcbFile::SetWaveformNumSamples(uint32_t track_idx, uint32_t num_samples)
{
    if (WaveformTable.GetNumColumns() == 0)
        return false;

    bool ret = WaveformTable.SetDword("NumSamples", num_samples, track_idx, false);
    if (ret)
        WaveformTable_modified = true;

    return ret;
}

bool AcbFile::SetWaveformLoopFlag(uint32_t track_idx, bool loop)
{
    if (WaveformTable.GetNumColumns() == 0)
        return false;

    bool ret = WaveformTable.SetByte("LoopFlag", (loop) ? 1 : 0, track_idx, false);
    if (ret)
        WaveformTable_modified = true;

    return ret;
}

bool AcbFile::SetWaveformSamplingRate(uint32_t track_idx, uint16_t sample_rate)
{
    if (WaveformTable.GetNumColumns() == 0)
        return false;

    bool ret = WaveformTable.SetWord("SamplingRate", sample_rate, track_idx, false);
    if (ret)
        WaveformTable_modified = true;

    return ret;
}

bool AcbFile::SetWaveformNumChannels(uint32_t track_idx, uint8_t num_channels)
{
    if (WaveformTable.GetNumColumns() == 0)
        return false;

    bool ret = WaveformTable.SetByte("NumChannels", num_channels, track_idx, false);
    if (ret)
        WaveformTable_modified = true;

    return ret;
}

uint32_t AcbFile::GetHighestCueId() const
{
    if (CueTable.GetNumColumns() == 0)
    {
        //DPRINTF("%s: Failed because one of the tables we were expecting didn't exist.\n", FUNCNAME);
        return (uint32_t)-1;
    }

    if (!CueTable.ColumnExists("CueId"))
        return (uint32_t)-1;

    uint32_t cue_id;

    if (CueTable.GetHighestRow32("CueId", &cue_id) == (uint32_t)-1)
        return (uint32_t)-1;

    return cue_id;
}

bool AcbFile::CanUseExternalAwb() const
{
    if (WaveformTable.ColumnExists("StreamAwbId"))
    {
        return WaveformTable.IsVariableColumn("StreamAwbId");
    }

    return true;
}

void AcbFile::DebugDumpTables() const
{
    DPRINTF("==========CueTable==========\n");
    CueTable.DebugDump();

    DPRINTF("==========CueNameTable==========\n");
    CueNameTable.DebugDump();

    DPRINTF("==========WaveformTable==========\n");
    WaveformTable.DebugDump();

    DPRINTF("==========AisacTable==========\n");
    AisacTable.DebugDump();

    DPRINTF("==========GraphTable==========\n");
    GraphTable.DebugDump();

    DPRINTF("==========GlobalAisacReferenceTable==========\n");
    GlobalAisacReferenceTable.DebugDump();

    DPRINTF("==========AisacNameTable==========\n");
    AisacNameTable.DebugDump();

    DPRINTF("==========SynthTable==========\n");
    SynthTable.DebugDump();

    DPRINTF("==========CommandTable==========\n");
    CommandTable.DebugDump();

    DPRINTF("==========TrackTable==========\n");
    TrackTable.DebugDump();

    DPRINTF("==========SequenceTable==========\n");
    SequenceTable.DebugDump();

    DPRINTF("==========AisacControlNameTable==========\n");
    AisacControlNameTable.DebugDump();

    DPRINTF("==========AutoModulationTable==========\n");
    AutoModulationTable.DebugDump();

    DPRINTF("==========StringValueTable==========\n");
    StringValueTable.DebugDump();

    DPRINTF("==========OutsideLinkTable==========\n");
    OutsideLinkTable.DebugDump();

    DPRINTF("==========BlockSequenceTable==========\n");
    BlockSequenceTable.DebugDump();

    DPRINTF("==========BlockTable==========\n");
    BlockTable.DebugDump();

    DPRINTF("==========EventTable==========\n");
    EventTable.DebugDump();

    DPRINTF("==========ActionTrackTable==========\n");
    ActionTrackTable.DebugDump();

    DPRINTF("==========AcfReferenceTable==========\n");
    AcfReferenceTable.DebugDump();

    DPRINTF("==========WaveformExtensionDataTable==========\n");
    WaveformExtensionDataTable.DebugDump();
}


