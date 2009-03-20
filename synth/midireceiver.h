// MidiReceiver.h: interface for the MidiReceiver class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MIDIRECEIVER_H__9C3D7464_E296_11D1_A5BB_00AA001149F7__INCLUDED_)
#define AFX_MIDIRECEIVER_H__9C3D7464_E296_11D1_A5BB_00AA001149F7__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class MidiReceiver  
{
public:
	virtual void DeltaTime( unsigned long t) = 0;
	virtual void ReceiveMessage( MidiMessage *msg) = 0;
	virtual void ReceiveMessage( NoteOn *msg) = 0;
	virtual void ReceiveMessage( NoteOff *msg) = 0;
	virtual void ReceiveMessage( Aftertouch *msg) = 0;
	virtual void ReceiveMessage( Expression *msg) = 0;
	virtual ~MidiReceiveMessager();
};

#endif // !defined(AFX_MIDIRECEIVER_H__9C3D7464_E296_11D1_A5BB_00AA001149F7__INCLUDED_)
