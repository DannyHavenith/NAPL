// MidiFile.cpp
#include "midifile.h"
// --- construct a MIDIFile object to read and parse an SMF file
MIDIFile::MIDIFile(std::ifstream& rFile,bool bBypassSysEx) : 
						m_pIfile(&rFile),
						m_bBypassSysEx(bBypassSysEx)
{
	InitializeMembers();
	m_pOfile = 0;
}
// --- construct a MIDIFile object to write an SMF file
MIDIFile::MIDIFile(std::ofstream& rFile, Short format,Short tracks,Short division) : 
						m_pOfile(&rFile)
{
	assert(tracks == 1 || format != 0);
	assert(format >= 0 && format <= 2);
	InitializeMembers();
	m_nFormat = format;
	m_nTracks = tracks;
	m_nDivision = division;
	m_pIfile = 0;
}
// ---- called by constructors to initialize the data members
void MIDIFile::InitializeMembers()
{
	// --- ensure that typedefs are properly set
	assert(sizeof(Short) == 2 && sizeof(Long) == 4);
	m_nFormat = 0;
	m_nTracks = 0;
	m_nDivision = 0;
	m_nDeltaTime = 0;
	m_bEot = false;
	m_bOverride = false;
	m_bBypassSysEx = false;
	m_bWritingTrack = false;
	m_nTrackNo = 0;
	m_nTrackLength = 0;
}
// ------ get a byte from the SMF file
Short MIDIFile::GetMIDIChar() throw (MFBadEof)
{
	if (m_pIfile->eof())			// eof should not occur because length values 
		throw MFBadEof();			// in file control when to call for characters
	if (m_pIfile->fail())			// if file does not exist or other input problem
		throw MFIFileFail();
	char ch;
	m_pIfile->get(ch);
	return static_cast<Short>(ch) & 0x00ff;
}
// ------- get a byte from the SMF file's track chunk
Short MIDIFile::GetMIDITrackChar() throw (MFBadTrkData)
{
	if (--m_nTrackLength < 0)		// if the track length is exhausted, we
		throw MFBadTrkData();		// should not be asking for more bytes
	return GetMIDIChar();
}
// ---- convert byte stream to integer (Endian conversion from MIDI to X86)
Long MIDIFile::AtoInt(const char* str,int nLen)
{
	Long rtn = 0;
	for (int i = 0; i < nLen; i++)	{
		rtn <<= 8;
		rtn |= (static_cast<Long>(str[i] & 0x000000ff));
	}
	return rtn;
}
// ---- read a fixed-length integer
Long MIDIFile::ReadInt(int nCount)
{
	char* str = new char[nCount];
	// --- read the bytes into a buffer
	for (int i = 0; i < nCount; i++)
		str[i] = GetMIDIChar();
	// --- convert the buffer to an integer
	Long rtn = AtoInt(str,nCount);
	delete [] str;
	return rtn;
}

void MIDIFile::WriteInt(int nCount,Long value)
{
	char str[4];
	int i;
	for (i = 3; i >= 0; --i)	{
		str[i] = static_cast<unsigned char>(value & 0xff);
		value >>= 8;
	}
	for (i = 4 - nCount; i < 4; i++)
		m_pOfile->put(str[i]);
}
// ---- read the SMF header chunk
void MIDIFile::ReadHeader() throw (std::runtime_error)
{
	char buff[5];	// Buffer for signature "MThd"
	// ----- read the signature
	for (int i = 0; i < 4; i++)
		buff[i] = GetMIDIChar();
	buff[i] = '\0';
	// ----- validate the signature
	if (std::string(buff) != "MThd")
		throw MFBadSig();
	// Read and validate the header length (must be 6)
	if (Read32() != 6)
		throw MFBadHdrLen();
	// Read the file format (0, 1, 2)
	if ((m_nFormat = Read16()) > 2)
		throw MFBadFmt();
	// Read the number of tracks
	if ((m_nTracks = Read16()) < 0)
		throw MFBadTrks();
	// Read the quarter-note division
	if ((m_nDivision = Read16()) < 0)
		throw MFBadDiv();
	// --- notify the user of the header and its contents
	Header(m_nFormat,m_nTracks,m_nDivision);
}
// ---- read an SMF variable length integer value (event lengths and delta times)
//      encoded as a byte stream
//      the last byte has the msb == 0
//      all others have msb == 1
//      bits 0-7 contain the parts of the integer
Long MIDIFile::ReadVarLength(std::runtime_error& exc) throw (std::runtime_error)
{
	Long value = 0;
	int c;
	do	{
		c = GetMIDITrackChar();
		value = (value << 7) + (c & 0x7f);
	} while (c & 0x80);
	if (value < 0)
		throw exc;
	return value;
}
// ---- write an SMF variable length integer value
//      return the number of bytes written
int MIDIFile::WriteVarLength(Long value)
{
	unsigned char cBytes[4];
	int nCt = 1;
	for (int i = 3; i >= 0; --i)	{
		*(cBytes + i) = (unsigned char)(value & 0x7f);
		value >>= 7;
	}
	for (i = 0; i < 3; i++)	{
		if (*(cBytes + i) != 0)	{
			while (i < 3)	{
				*(cBytes + i) |= 0x80;
				m_pOfile->put(cBytes[i++]);
				nCt++;
			}
		}
	}
	m_pOfile->put(cBytes[3]);
	return nCt;
}
// --- return number of parameters based on channel status code
int MIDIFile::ParamCount(Short status)
{
	if (status == 0xf2)	// not sure whether f2 & f3 would ever
		return 2;
	if (status == 0xf3)	// be in an SMF file, but better be safe
		return 1;
	if (status >= 0xf0)
		return 0;
	if (status >= 0xe0)
		return 2;
	if (status < 0xc0)
		return 2;
	return 1;
}
// ---- Read a system exclusive message
void MIDIFile::ReadSysex()
{
	// Read length of system exclusive event
	Long length = ReadVarLength(MFBadMetaEvLen());
	if (m_bBypassSysEx)	{
		// ---- bypass the system exclusive event
		for (int i = 0; i < length; i++)
			GetMIDITrackChar();
		SystemExclusive(m_nDeltaTime,length,0);
	}
	else	{
		m_data.EventBuffer(length);
		// Read in the system exclusive event
		for (int i = 0; i < length; i++)
			m_data.bf[i] = GetMIDITrackChar();
		SystemExclusive(m_nDeltaTime,length,m_data.bf);
	}
}
// ---- Read a system exclusive message packet
void MIDIFile::ReadSysexPacket()
{
	// Read length of system exclusive event packet
	Long length = ReadVarLength(MFBadMetaEvLen());
	if (m_bBypassSysEx)	{
		// ---- bypass the system exclusive event packet
		for (int i = 0; i < length; i++)
			GetMIDITrackChar();
		SystemExclusivePacket(m_nDeltaTime,length,0);
	}
	else	{
		m_data.EventBuffer(length);
		// Read in the system exclusive event packet
		for (int i = 0; i < length; i++)
			m_data.bf[i] = GetMIDITrackChar();
		SystemExclusivePacket(m_nDeltaTime,length,m_data.bf);
	}
}
// ---- read a meta event
void MIDIFile::ReadMeta() throw (std::runtime_error)
{
	// --------- Meta message
	int msg_type = GetMIDITrackChar();	// Type of meta-event
	// Read length of meta event
	Long length = ReadVarLength(MFBadMetaEvLen());
	m_data.EventBuffer(length);
	// Read in the meta event
	for (int i = 0; i < length; i++)
		m_data.bf[i] = GetMIDITrackChar();
	switch(msg_type) {
		case META_SEQNUM:	// Sequence number
			if (length < 2)
				throw MFBadMetaEvLen();
			SequenceNum(m_nDeltaTime,static_cast<Short>(AtoInt(m_data.bf,2)));
			break;
		case META_TEXT:			// Text event
		case META_COPYRIGHT:	// Copyright notice
		case META_SEQTRKNAME:	// Sequence name
		case META_INSTNAME:		// Instrument name
		case META_LYRIC:		// Lyric
		case META_MARKER:		// Marker
		case META_CUEPT:		// Cue point
			TextEvent(m_nDeltaTime,msg_type,length,m_data.bf);
			break;
		case META_TEMPO:	// Set tempo
			if (length < 3)
				throw MFBadMetaEvLen();
			Tempo(m_nDeltaTime,AtoInt(m_data.bf,3));
			break;
		case META_SMPTE:	// SMPTE offset
		{
			if (length < 5)
				throw MFBadMetaEvLen();
			int hour  = static_cast<int>(m_data.bf[0]);
			int min   = static_cast<int>(m_data.bf[1]);
			int sec   = static_cast<int>(m_data.bf[2]);
			int frame = static_cast<int>(m_data.bf[3]);
			int fract = static_cast<int>(m_data.bf[4]);
			SMPTE(m_nDeltaTime,hour,min,sec,frame,fract);
			break;
		}
		case META_TIMESIG:	// Time signature
		{
			if (length < 4)
				throw MFBadMetaEvLen();
			int numer  = static_cast<int>(m_data.bf[0]);
			int denom  = static_cast<int>(m_data.bf[1]);
			int clocks = static_cast<int>(m_data.bf[2]);
			int qnotes = static_cast<int>(m_data.bf[3]);
			TimeSignature(m_nDeltaTime,numer,denom,clocks,qnotes);
			break;
		}
		case META_KEYSIG:	// Key signature
		{
			if (length < 2)
				throw MFBadMetaEvLen();
			int sharpflat = static_cast<int>(m_data.bf[0]);
			int minor    = (static_cast<int>(m_data.bf[1]) == 1);
			if (sharpflat > 7 || sharpflat < -7 || 
					(minor != 0 && minor != 1))
				throw MFBadMetaEvVal();
			KeySignature(m_nDeltaTime,sharpflat,minor == 1);
			break;
		}
		case META_SEQSPEC:	// Sequencer-specific
			SequencerSpecific(m_nDeltaTime,length,m_data.bf);
			break;
		case META_CHANPFX:	// Channel prefix
			if (length < 1)
				throw MFBadMetaEvLen();
			ChannelPrefix(m_nDeltaTime,m_data.bf[0]);
			break;
		case META_EOT:		// End of track
			if (length != 0)
				throw MFBadMetaEvLen();
			EndOfTrack(m_nDeltaTime);
			m_bEot = true;
			break;
		default:
			UnknownMetaType(m_nDeltaTime,msg_type);
			break;
	}
}
// ----- read an event chunk
void MIDIFile::ReadEvent() throw (std::runtime_error)
{
	static unsigned char running_status = 0x00;	// Running status
	static unsigned char params[2];				// Event parameters
	int cur_param;			// Parameter being currently read
	m_bOverride = true;		// false if user does not override event function
	// -------- Read delta-time
	Long delta_time = ReadVarLength(MFBadDeltaTime());
	m_nDeltaTime += delta_time;
	// -------- Read event type
	unsigned char stat = GetMIDITrackChar();
	if (stat & 0x80)	{	// Is it a new event type?
		running_status = stat;	// Set new running status
		cur_param = 0;		// Start reading at 0th param
	} else {
		// --- continuation of running status
		params[0] = stat;	// Record 1st parameter
		cur_param = 1;		// Start reading at 1st param
	}
	// -------- Read the parameters corresponding to the status byte
	for (int i = ParamCount(running_status)-cur_param; i > 0; i--,cur_param++)
		params[cur_param] = GetMIDITrackChar();
	// ---- break status into its two parts
	int channel = running_status & 0x0f;
	int event = running_status & 0xf0;
	switch (event) {
		case MIDI_NOTEOFF:	// Note off
			NoteOff(m_nDeltaTime,channel,params[0],params[1]);
			break;
		case MIDI_NOTEON:	// Note on
			NoteOn(m_nDeltaTime,channel,params[0],params[1]);
			break;
		case MIDI_PRESSURE:	// Polyphonic key pressure
			Pressure(m_nDeltaTime,channel,params[0],params[1]);
			break;
		case MIDI_CONTROL:	// Control change
			Controller(m_nDeltaTime,channel,params[0],params[1]);
			break;
		case MIDI_PROGRAM:	// Program change
			ProgramChange(m_nDeltaTime,channel,params[0]);
			break;
		case MIDI_CHANPRES:	// Channel pressure
			ChannelPressure(m_nDeltaTime,channel,params[0]);
			break;
		case MIDI_PITCHBEND:// Pitch wheel change
		{
			Short pitch = (static_cast<Short>(AtoInt(reinterpret_cast<const char*>(params),2)));
			PitchBend(m_nDeltaTime,channel,pitch);
			break;
		}
		case SYSEX_META:	// System-exclusive or meta event
			switch (running_status)	{
				case 0xf0:
					ReadSysex();
					break;
				case 0xf7:
					ReadSysexPacket();
					break;
				case 0xff:
					ReadMeta();
					break;
				// ------- ( there are several realtime events, but they
				//      would not be found in an SMF file -------------
				default:
					UnknownStatus(m_nDeltaTime,channel,event);
					break;
			}
			break;
		default:
			UnknownStatus(m_nDeltaTime,channel,event);
			break;
	}
	// --- If the user overrides the event, set delta time to 0.
	//     Otherwise allow it to accumulate so that the variable always
	//     represents elapsed time since the last user-intercepted event.
	//	   How do we know? The overridable functions set m_bOverride to
	//     false. Overriding functions do not.
	if (m_bOverride)
		m_nDeltaTime = 0;
}
// ------ read and process a track
void MIDIFile::ReadTrack() throw (std::runtime_error)
{
	char buff[5];	// Buffer for signature "MThd"
	// ------ read signature
	for (int i = 0; i < 4; i++)
		buff[i] = GetMIDIChar();
	buff[i] = '\0';
	// ------ validate signature
	if (std::string(buff) != "MTrk")
		throw MFBadTrkSig();
	// ---- initialize time variable for the track
	m_nDeltaTime = 0;
	m_bEot = false;	// Indicate that EOT event hasn't been sensed
	// ---- notify user that new track is found
	StartTrack(++m_nTrackNo);
	// -------- Read the length of the track
	if ((m_nTrackLength = Read32()) < 0)
		throw MFBadTrkLen();
	// --------- Read the data in the track
	while (m_nTrackLength > 0)
		ReadEvent();
	if (!m_bEot)	// Has EOT been sensed?
		throw MFNoEot();
}
// ------- read and process a Standard MIDI Format (SMF) file
void MIDIFile::ReadMIDIFile()
{
	assert(m_pIfile != 0);
	ReadHeader();		// read the header
	for (int i = 0; i < m_nTracks; i++)
		ReadTrack();	// read the tracks
}
// ------- write a Standard MIDI Format (SMF) file
void MIDIFile::WriteMIDIFile()
{
	assert(m_pOfile != 0);
	// ---- write the header
	m_pOfile->write("MThd",4);
	// --- header length
	Write32(6);
	// --- format
	Write16(m_nFormat);
	// --- tracks
	Write16(m_nTracks);
	// --- ticks/quarter note
	Write16(m_nDivision);
	m_bWritingTrack = true;
	for (m_nTrackNo = 1; m_nTrackNo <= m_nTracks; m_nTrackNo++)	{
		// --- write the track header
		m_nTrackLength = 0;
		m_pOfile->write("MTrk",4);
		long position = m_pOfile->tellp();	// where length is written at end of track
		Write32(0);							// to be overwritten at end of track
		m_bOverride = true;
		StartTrack(m_nTrackNo);
		assert(m_bOverride == true);		// ensure that application overrides StartTrack
		// ---- write end of track
		WriteMetaEvent(0,META_EOT,0);
		long here = m_pOfile->tellp();
		// --- seek back and write track length
		m_pOfile->seekp(position);
		Write32(m_nTrackLength);
		// --- seek to write next track
		m_pOfile->seekp(here);
	}
}
void MIDIFile::WriteMetaEvent(Long delta,Short event,Short length)
{
	m_nTrackLength += WriteVarLength(delta);
	m_pOfile->put('\xff');
	m_pOfile->put(static_cast<char>(event));
	m_nTrackLength += 2;
	m_nTrackLength += WriteVarLength(length);
	m_nTrackLength += length;
}
void MIDIFile::SequenceNum(Long delta,Short seqno)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_SEQNUM,2);
		Write16(seqno);
	}
	else
		m_bOverride = false;
}
void MIDIFile::TextEvent(Long delta,Short event,Long length,const char* text)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		assert(event >= META_COPYRIGHT && event <= META_CUEPT);
		WriteMetaEvent(delta,event,length);
		m_pOfile->write(text,length);
	}
	else
		m_bOverride = false;
}
void MIDIFile::Tempo(Long delta,Long tempo)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_TEMPO,3);
		Write24(tempo);
	}
	else
		m_bOverride = false;
}
void MIDIFile::SMPTE(Long delta,Short hr,Short min,Short sec,Short frame,Short fraction)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_SMPTE,5);
		m_pOfile->put(static_cast<char>(hr));
		m_pOfile->put(static_cast<char>(min));
		m_pOfile->put(static_cast<char>(sec));
		m_pOfile->put(static_cast<char>(frame));
		m_pOfile->put(static_cast<char>(fraction));
	}
	else
		m_bOverride = false;
}
void MIDIFile::TimeSignature(Long delta,Short numer,Short denom,Short clocks,Short qnote)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_TIMESIG,4);
		m_pOfile->put(static_cast<char>(numer));
		m_pOfile->put(static_cast<char>(denom));
		m_pOfile->put(static_cast<char>(clocks));
		m_pOfile->put(static_cast<char>(qnote));
	}
	else
		m_bOverride = false;
}
void MIDIFile::KeySignature(Long delta,Short sharpsflats,bool isminor)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_KEYSIG,2);
		m_pOfile->put(static_cast<char>(sharpsflats));
		m_pOfile->put(isminor ? '\x01' : '\x00');
	}
	else
		m_bOverride = false;
}
void MIDIFile::SequencerSpecific(Long delta,Short length,const char* text)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_SEQSPEC,length);
		m_pOfile->write(text,length);
	}
	else
		m_bOverride = false;
}
void MIDIFile::ChannelPrefix(Long delta,Short channel)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMetaEvent(delta,META_CHANPFX,1);
		m_pOfile->put(static_cast<char>(channel));
	}
	else
		m_bOverride = false;
}
void MIDIFile::WriteMIDIEvent(Long delta,Short event,Short length)
{
	m_nTrackLength += WriteVarLength(delta);
	m_pOfile->put(static_cast<char>(event));
	m_nTrackLength++;
	m_nTrackLength += length;
}
void MIDIFile::NoteOn(Long delta,Short channel,Short note,Short velocity)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_NOTEON | channel,2);
		m_pOfile->put(static_cast<char>(note & 0x7f));
		m_pOfile->put(static_cast<char>(velocity & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::NoteOff(Long delta,Short channel,Short note,Short velocity)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_NOTEOFF | channel,2);
		m_pOfile->put(static_cast<char>(note & 0x7f));
		m_pOfile->put(static_cast<char>(velocity & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::Pressure(Long delta,Short channel,Short note,Short aftertouch)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_PRESSURE | channel,2);
		m_pOfile->put(static_cast<char>(note & 0x7f));
		m_pOfile->put(static_cast<char>(aftertouch & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::Controller(Long delta,Short channel,Short controller,Short value)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_CONTROL | channel,2);
		m_pOfile->put(static_cast<char>(controller & 0x7f));
		m_pOfile->put(static_cast<char>(value & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::ProgramChange(Long delta,Short channel,Short program)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_PROGRAM | channel,1);
		m_pOfile->put(static_cast<char>(program & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::ChannelPressure(Long delta,Short channel,Short aftertouch)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_CHANPRES | channel,1);
		m_pOfile->put(static_cast<char>(aftertouch & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::PitchBend(Long delta,Short channel,Short pitch)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		WriteMIDIEvent(delta,MIDI_PITCHBEND | channel,1);
		m_pOfile->put(static_cast<char>(pitch & 0x7f));
	}
	else
		m_bOverride = false;
}
void MIDIFile::SystemExclusive(Long delta,Long length,const char* data)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		m_nTrackLength += WriteVarLength(delta);
		m_pOfile->put('\xf0');
		m_nTrackLength++;
		m_nTrackLength += WriteVarLength(length);
		m_pOfile->write(data,length);
	}
	else
		m_bOverride = false;
}
void MIDIFile::SystemExclusivePacket(Long delta,Long length,const char* data)
{
	if (m_pOfile != 0)	{
		// --- writing an SMF file
		assert(m_bWritingTrack);
		m_nTrackLength += WriteVarLength(delta);
		m_pOfile->put('\xf7');
		m_nTrackLength++;
		m_nTrackLength += WriteVarLength(length);
		m_pOfile->write(data,length);
	}
	else
		m_bOverride = false;
}
