// ---- miditest.cpp
#include <iostream>
#include <iomanip>
#include <fstream>
#include "midifile.h"
// ---- application class for displaying contents of SMF file.
class MIDITest : public MIDIFile	{
	void Header(Short format, Short tracks, Short division);
	void StartTrack(int trackno);
	void SequenceNum(Long delta,Short seqnum);
	void TextEvent(Long delta,Short event,Long length,const char* text);
	void Tempo(Long delta,Long tempo);
	void SMPTE(Long delta,Short hour,Short min,Short sec,Short frame,Short fract);
	void TimeSignature(Long delta,Short numer,Short denom,Short clocks,Short qnotes);
	void KeySignature(Long delta,Short sharpflat,bool isminor);
	void SequencerSpecific(Long delta,Short length, const char* text);
	void NoteOn(Long delta,Short channel,Short note, Short velocity);
	void NoteOff(Long delta,Short channel,Short note, Short velocity);
	void Pressure(Long delta,Short channel,Short note, Short aftertouch);
	void Controller(Long delta,Short channel,Short controller, Short value);
	void ProgramChange(Long delta,Short channel,Short program);
	void ChannelPressure(Long delta,Short channel,Short aftertouch);
	void PitchBend(Long delta,Short channel,Short pitch);
	void ChannelPrefix(Long delta,Short channel);
	void UnknownMetaType(Long delta,Short type);
	void UnknownStatus(Long delta,Short channel,Short event);
	void SystemExclusive(Long delta,Long length,const char* data);
	void SystemExclusivePacket(Long delta,Long length,const char* data);
	void EndOfTrack(Long delta);
	void DisplayTime(Long delta)
		{ std::cout << std::setw(6) << delta << ' '; }
public:
	MIDITest(std::ifstream& rFile) : MIDIFile (rFile, false) { }
};
void MIDITest::Header(Short format,Short tracks,Short division)
{
	std::cout << "Header: \n";
	std::cout << "  Format:   " << format   << '\n';
	std::cout << "  Tracks:   " << tracks   << '\n';
	std::cout << "  Division: " << division << '\n';
}
void MIDITest::SequenceNum(Long delta,Short seqnum)
{
	std::cout << std::setw(6) << delta << ' ';
	std::cout << "  Sequence: " << seqnum << '\n';
}
void MIDITest::TextEvent(Long delta,Short event,Long len,const char* text)
{
	DisplayTime(delta);
	std::cout << "Text: " << event << ' ';
	for (int i = 0; i < len; i++)
		std::cout << *(text+i);
	std::cout << '\n';
}
void MIDITest::StartTrack(int trackno)
{
	std::cout << "Start of Track " << trackno << '\n';
}
void MIDITest::EndOfTrack(Long delta)
{
	DisplayTime(delta);
	std::cout << "End of Track\n";
}
void MIDITest::Tempo(Long delta,Long tempo)
{
	DisplayTime(delta);
	std::cout << "Tempo: " << tempo << '\n';
}
void MIDITest::SMPTE(Long delta,Short hour,Short min,Short sec,Short frame,Short fract)
{
	DisplayTime(delta);
	std::cout << "SMPTE: " << hour << ':' << min << ':' << sec << ':' << frame << ':' << fract << '\n';
}
void MIDITest::TimeSignature(Long delta,Short numer,Short denom,Short clocks,Short qnotes)
{
	DisplayTime(delta);
	std::cout << "Time signature: " << numer << '/' << denom << '/' << clocks << '/' << qnotes << '\n';
}
void MIDITest::KeySignature(Long delta,Short sharpflat,bool isminor)
{
	DisplayTime(delta);
	std::cout << "Key signature: " << sharpflat << ", " << isminor << '\n';
}
void MIDITest::SequencerSpecific(Long delta,Short length,const char* text)
{
	DisplayTime(delta);
	std::cout << "Sequencer specific: ";
	for (int i = 0; i < length; i++)
		std::cout << *(text+i);
	std::cout << '\n';
}
void MIDITest::NoteOn(Long delta,Short channel,Short note,Short velocity)
{
	DisplayTime(delta);
	std::cout << "Note On " << channel << ' ' << note << ' ' << velocity << '\n';
}
void MIDITest::NoteOff(Long delta,Short channel,Short note,Short velocity)
{
	DisplayTime(delta);
	std::cout << "Note Off " << channel << ' ' << note << ' ' << velocity << '\n';
}
void MIDITest::Pressure(Long delta,Short channel,Short note,Short aftertouch)
{
	DisplayTime(delta);
	std::cout << "Pressure " << channel << ' ' << note << ' ' << aftertouch << '\n';
}
void MIDITest::Controller(Long delta,Short channel,Short controller,Short value)
{
	DisplayTime(delta);
	std::cout << "Controller " << channel << ' ' << controller << ' ' << value << '\n';
}
void MIDITest::ProgramChange(Long delta,Short channel,Short program)
{
	DisplayTime(delta);
	std::cout << "Program " << channel << ' ' << program << '\n';
}
void MIDITest::ChannelPressure(Long delta,Short channel,Short aftertouch)
{
	DisplayTime(delta);
	std::cout << "Channel Pressure " << channel << ' ' << aftertouch << '\n';
}
void MIDITest::PitchBend(Long delta,Short channel,Short pitch)
{
	DisplayTime(delta);
	std::cout << "Pitch bend " << channel << ' ' << pitch << '\n';
}
void MIDITest::ChannelPrefix(Long delta,Short channel)
{
	DisplayTime(delta);
	std::cout << "Channel Prefix " << channel << '\n';
}
void MIDITest::UnknownMetaType(Long delta,Short type)
{
	DisplayTime(delta);
	std::cout << "Unknown meta type: " << type << '\n';
}
void MIDITest::UnknownStatus(Long delta,Short channel,Short event)
{
	DisplayTime(delta);
	std::cout << "Unknown status: " << channel << ' ' << event << '\n';
}
void MIDITest::SystemExclusive(Long delta,Long length,const char* data)
{
	DisplayTime(delta);
	std::cout << "System exclusive: ";
	for (int i = 0; i < length; i++)
		std::cout << std::hex << ((int)*(data+i)) << ' ';
	std::cout << '\n';
}
void MIDITest::SystemExclusivePacket(Long delta,Long length,const char* data)
{
	DisplayTime(delta);
	std::cout << "System exclusive packet: ";
	for (int i = 0; i < length; i++)
		std::cout << std::hex << ((int)*(data+i)) << ' ';
	std::cout << '\n';
}
int main(int argc,char* argv[])
{
	if (argc > 1)	{
		try	{
			std::ifstream ifile(argv[1],std::ios::binary);
			MIDITest mt(ifile);
			mt.ReadMIDIFile();
		}
		catch (std::exception& exc)	{
			std::cout << "\nError: " << exc.what() << '\n';
		}
	}
	else
		std::cout << "\nUsage: miditest filename.mid\n";
	return 0;
}
