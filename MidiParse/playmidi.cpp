// ------ playmidi.cpp
#include <fstream>
#include <iostream>
#include "midifile.h"

class WmTellOverture : public MIDIFile {
	void StartTrack(int trackno);
	static int m_nNotes[][2];
public:
	WmTellOverture(std::ofstream& rfile) : 
					MIDIFile(rfile, 1, 3, 120) { }
};
static const int iv = 100;	// note interval
static const int nt = 60;	// 1st note of song
int WmTellOverture::m_nNotes[][2] = {			// {note, delta}
	{nt,0},{nt, iv/2},{nt,iv/2},{nt,iv},{nt,iv/2},{nt,iv/2},
	{nt,iv},{nt, iv/2},{nt+5,iv/2},{nt+7,iv},{nt+9,iv},{nt,iv},
	{nt,iv/2},{nt,iv/2},{nt,iv},{nt,iv/2},{nt+5,iv/2},{nt+9,iv},
	{nt+9,iv/2},{nt+7,iv/2},{nt+4,iv},{nt,iv},{nt,iv},{nt, iv/2},
	{nt,iv/2},{nt,iv},{nt, iv/2},{nt,iv/2},{nt,iv},{nt,iv/2},
	{nt+5,iv/2},{nt+7,iv},{nt+9,iv},{nt+5,iv},{nt+9,iv/2},
	{nt+12,iv/2},{nt+10,iv*3},{nt+9,iv/2},{nt+7,iv/2},
	{nt+5,iv/2},{nt+9,iv},{nt+5,iv},
	{0,iv}
};
void WmTellOverture::StartTrack(int trackno)
{
	switch (trackno)	{
		case 1:
			// ---- tempo track
			TextEvent(0, META_SEQTRKNAME, 21, "William Tell Overture");
			Tempo(0, 250000);
			break;
		case 2:
			// ---- piano track
			TextEvent(0, META_SEQTRKNAME, 5, "Piano");
			ProgramChange(0, 0, 0);		// channel 0 = acoustic piano
			// --- play the notes
			int i;
			for (i = 0; m_nNotes[i][0] != 0; i++)	{
				if (i > 0)
					NoteOff(m_nNotes[i][1],0,m_nNotes[i-1][0],0);
				NoteOn(0,0,m_nNotes[i][0],64);
			}
			NoteOff(m_nNotes[i][1],0,m_nNotes[i-1][0],0);
			break;
		case 3:
		{
			// ---- drum track
			const int shot = 42;
			const int crash = 49;
			TextEvent(0, META_SEQTRKNAME, 5, "Drums");
			ProgramChange(0, 9, 0);
			for (i = 0; m_nNotes[i][0] != 0; i++)
				NoteOn(m_nNotes[i][1],9,shot,64);
			NoteOn(iv,9,crash,100);
			break;
		}
		default:
			break;
	}
}
int main()
{
	std::ofstream ifile("WTO.mid", std::ios::binary);
	WmTellOverture wto(ifile);
	wto.WriteMIDIFile();
	return 0;
}
