// --- midifile.h
#ifndef MIDIFILE_H
#define MIDIFILE_H
#include <fstream>
#include <stdexcept>
#include <string>
#include <cassert>

typedef short int Short;
typedef long int Long;

// ---- macro for defining MIDIFile exception classes
#define rte std::runtime_error
#define MFX(id, text) class id:public rte{public:id():rte(text){}};
// ------- MIDIFile exception classes
MFX(MFBadEof,"unexpected end of file")
MFX(MFBadHdrLen,"invalid header length")
MFX(MFBadSig,"MThd not found in header")
MFX(MFBadFmt,"invalid format in header")
MFX(MFBadTrks,"invalid number of tracks in header")
MFX(MFBadDiv,"invalid division in header")
MFX(MFBadMetaEvLen,"invalid meta event length")
MFX(MFBadDeltaTime,"invalid delta time")
MFX(MFBadTrkData,"track data length error")
MFX(MFBadMetaEvVal,"invalid meta event value")
MFX(MFBadMetaType,"invalid meta event type")
MFX(MFBadTrkSig,"MTrk not found in track header")
MFX(MFBadTrkLen,"invalid track length")
MFX(MFNoEot,"no EOT event in track")
MFX(MFBadStatus,"invalid status")
MFX(MFIFileFail,"cannot read file")

// ---- MIDI events
const Short MIDI_NOTEOFF	= 0x80;	// Note off
const Short MIDI_NOTEON		= 0x90;	// Note on
const Short MIDI_PRESSURE	= 0xa0;	// Polyphonic key pressure
const Short MIDI_CONTROL	= 0xb0;	// Control change
const Short MIDI_PROGRAM	= 0xc0;	// Program change
const Short MIDI_CHANPRES	= 0xd0;	// Channel pressure
const Short MIDI_PITCHBEND	= 0xe0;	// Pitch wheel change
// ---- System exclusive or meta event
const Short SYSEX_META		= 0xf0;
// ---- Meta-events
const Short META_SEQNUM		= 0x00;	// Sequence number
const Short META_TEXT		= 0x01;	// Text event
const Short META_COPYRIGHT	= 0x02;	// Copyright notice
const Short META_SEQTRKNAME	= 0x03;	// Sequence/track name 
const Short META_INSTNAME	= 0x04;	// Instrument name
const Short META_LYRIC		= 0x05;	// Lyric
const Short META_MARKER		= 0x06;	// Marker
const Short META_CUEPT		= 0x07;	// Cue point
const Short META_CHANPFX	= 0x20;	// Channel prefix
const Short META_EOT		= 0x2f;	// End of track
const Short META_TEMPO		= 0x51;	// Set tempo
const Short META_SMPTE		= 0x54;	// SMPTE offset
const Short META_TIMESIG	= 0x58;	// Time signature
const Short META_KEYSIG		= 0x59;	// Key signature
const Short META_SEQSPEC	= 0x7f;	// Sequencer-specific event
// ------ event buffer class
class EventBufferHolder	{
	char* bf;
	int length;
	EventBufferHolder() : bf(0), length(0) { }
	~EventBufferHolder() { delete [] bf; }
	friend class MIDIFile;
	void EventBuffer(int len)
	{
		if (len > length)	{
			delete [] bf;
			bf = new char[len];
		}
	}
};
class MIDIFile {
	// stream files
	std::ifstream* m_pIfile;
	std::ofstream* m_pOfile;
	Long m_nDeltaTime;		// delta time since previous event
	Long m_nTrackLength;	// track length
	// --- header data
	Short m_nFormat;		// File format
	Short m_nTracks;		// Number of tracks in the file
	Short m_nDivision;		// Quarter-note division
	// --- data members for reading a MIDI file
	bool m_bEot;			// true if eot function was called
	bool m_bOverride;		// true if event function was overriden
	bool m_bBypassSysEx;	// true to bypass system exclusive messages
	EventBufferHolder m_data;
	// --- data members for writing a MIDI file
	bool m_bWritingTrack;
	// ---- read/write data members
	int m_nTrackNo;			// track number
	// ---- private member functions
	void InitializeMembers();
	Short GetMIDIChar() throw (MFBadEof);
	Short GetMIDITrackChar() throw (MFBadTrkData);
	Long AtoInt(const char* str, int nLen);
	Long ReadInt(int nCount);
	Long Read32()
		{ return ReadInt(4); }
	Short Read16()
		{ return static_cast<Short>(ReadInt(2)); }
	void WriteInt(int nCount, Long value);
	void Write16(Long value)
		{ WriteInt(2, value); }
	void Write24(Long value)
		{ WriteInt(3, value); }
	void Write32(Long value)
		{ WriteInt(4, value); }
	Long ReadVarLength(std::runtime_error& exc) throw (std::runtime_error);
	int WriteVarLength(Long value);
	void ReadHeader() throw (std::runtime_error);
	void ReadSysex();
	void ReadSysexPacket();
	void ReadMeta() throw (std::runtime_error);
	void ReadEvent() throw (std::runtime_error);
	void ReadTrack() throw (std::runtime_error);
	int ParamCount(Short status);
	void WriteMetaEvent(Long delta, Short event, Short length);
	void WriteMIDIEvent(Long delta, Short event, Short length);
	friend class EventBuffer;
protected:
	explicit MIDIFile(std::ifstream& rFile, bool bBypassSysEx = true);
	explicit MIDIFile(std::ofstream& rFile, Short format,Short tracks,Short division);
	virtual ~MIDIFile() { }
	//
	// --- overridable functions to read, callable functions to write
	//
	// override to read (not used for writing)
	//
	virtual void Header(Short format,Short tracks,Short division)
		{ assert(m_pOfile == 0); m_bOverride = false; }
	virtual void EndOfTrack(Long delta = 0)
		{ assert(m_pOfile == 0); m_bOverride = false; }
	virtual void UnknownMetaType(Long delta,Short type)
		{ assert(m_pOfile == 0); m_bOverride = false; }
	virtual void UnknownStatus(Long delta,Short channel,Short event)
		{ assert(m_pOfile == 0); m_bOverride = false; }
	//
	// override to read or write
	//
	virtual void StartTrack(int trackno)
		{ m_bOverride = false; }
	//
	// override to read, call to write
	//
	// --- meta events
	virtual void SequenceNum(Long delta,Short seqno);
	virtual void TextEvent(Long delta,Short event,Long length,const char* text);
	virtual void Tempo(Long delta,Long tempo);
	virtual void SMPTE(Long delta,Short hr,Short min,Short sec,Short frame,Short fraction);
	virtual void TimeSignature(Long delta,Short numer,Short denom,Short clocks,Short qnote);
	virtual void KeySignature(Long delta,Short sharpsflats,bool isminor);
	virtual void SequencerSpecific(Long delta,Short length,const char* text);
	virtual void ChannelPrefix(Long delta,Short channel);
	// --- MIDI events
	virtual void NoteOn(Long delta,Short channel,Short note,Short velocity);
	virtual void NoteOff(Long delta,Short channel,Short note,Short velocity);
	virtual void Pressure(Long delta,Short channel,Short note,Short aftertouch);
	virtual void Controller(Long delta,Short channel,Short controller,Short value);
	virtual void ProgramChange(Long delta,Short channel,Short program);
	virtual void ChannelPressure(Long delta,Short channel,Short aftertouch);
	virtual void PitchBend(Long delta,Short channel,Short pitch);
	// --- system exclusive events
	virtual void SystemExclusive(Long delta,Long length,const char* data);
	virtual void SystemExclusivePacket(Long delta,Long length,const char* data);
public:
	void ReadMIDIFile();
	void WriteMIDIFile();
};
#endif
