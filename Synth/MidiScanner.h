// MidiScanner.h: interface for the MidiScanner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MIDISCANNER_H__9C3D7463_E296_11D1_A5BB_00AA001149F7__INCLUDED_)
#define AFX_MIDISCANNER_H__9C3D7463_E296_11D1_A5BB_00AA001149F7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MidiReceiver.h"

class MidiScanner  
{
public:
	MidiReceiver * SetReceiverOnChannel( MidiReceiver *receiver, int channel);
	bool ParseStream( istream &input);
	MidiScanner();
	virtual ~MidiScanner();

protected:
	const static int NUMCHANNELS = 16;

private:
	vector< MidiReceiver *> m_receivers;

};

#endif // !defined(AFX_MIDISCANNER_H__9C3D7463_E296_11D1_A5BB_00AA001149F7__INCLUDED_)
