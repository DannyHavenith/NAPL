//
// MidiMessages.cpp - implementation of Midi Messages
//
#include "MidiMessages.h"

namespace MidiMessages
{
	//
	// note messages are messages that contain 2 parameters:
	// a note number and a velocity value
	//
	struct NoteMessage : public MidiMessage
	{
		unsigned char m_note;
		unsigned char m_velocity;

		virtual bool Parse (istream &input)
		{
			m_note = input.get();
			m_velocity = input.get();
			return 0 == (m_note & 0x80 | m_velocity & 0x80);
		}
	};

	struct NoteOn : public NoteMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct NoteOff : public NoteMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct Aftertouch : public NoteMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct ControlChange : public MidiMessage
	{
		unsigned char m_control;
		unsigned char m_value;

		virtual bool Parse (istream &input)
		{
			m_control = input.get();
			m_value = input.get();
			return 0 == (m_control & 0x80 | m_value & 0x80);
		}
	};

	//
	// Single value messages contain one extra 7-bit value
	//
	struct SingleValueMessage : public MidiMessage
	{
		unsigned char m_value;

		virtual bool Parse( istream &input)
		{
			m_value = input.get();
			return 0 == (m_value & 0x80);
		}
	};

	struct ProgramChange : public SingleValueMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct Expression: public SingleValueMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct SongNumber: public SingleValueMessage
	{
		virtual bool CallReceiver( MidiReciever * pReceiver)
		{
			return true; // do nothing
		}
	};



	//
	// Large Value Messages contain one 14-bit value
	//

	struct LargeValueMessage : public MidiMessage
	{
		unsigned short m_value;

		virtual bool Parse( istream &input)
		{
			unsigned char ch;

			m_value = input.get();
			if (m_value & 0x80) return false;

			m_value <<= 7;
			ch = input.get();

			if (ch & 0x80) return false;

			m_value |= ch;
		}

	};

	struct PitchWheel : public LargeValueMessage
	{
		virtual bool CallReceiver( MidiReceiver * pReceiver)
		{
			pReceiver->ReceiveMessage( this);
		}
	};

	struct SongPosition : public LargeValueMessage
	{
		virtual bool CallReceiver( MidiReceiver * pReceiver)
		{
			return true; // do nothing.
		}
	};

	struct SysExSkipper : public MidiMessage
	{
		unsigned long m_len;

		virtual bool Parse( istream &input)
		{
			m_len = MidiUtils::ParseVarLen( input);
			input.seekg( m_len, ios_base::cur);
		}

		virtual bool CallReceiver( MidiReceiver * pReceiver)
		{
			return true; // do nothing.
		}
	};

	struct TextMessageSkipper : public MidiMessage
	{
		unsigned char m_type;
		unsigned long m_len;

		virtual bool Parse( istream &input)
		{
			m_type = input.get();
			m_len = MidiUtils::ParseVarLen( input);
			input.seekg( m_len, ios_base::cur);
			return true;
		}

		virtual bool CallReceiver( MidiReceiver * pReceiver)
		{
			return true; // do nothing.
		}
	};

	struct SimpleMessageSkipper : public MidiMessage
	{
		virtual bool Parse( istream &input)
		{
			return true; // do nothing.
		}

		virtual bool CallReceiver( MidiReceiver * pReceiver)
		{
			return true; // do nothing.
		}
	};

	//
	// This is a struct containing all message types. It's more
	// thread-safe than using singletons...
	//
	struct MessageSet
	{
		Aftertouch aftertouch;
		ControlChange contralchange;
		Expression expression;
		NoteOff noteoff;
		NoteOn noteon;
		PitchWheel pitchwheel;
		ProgramChange programchange;
		SimpleMessageSkipper simplemessageskipper;
		SongNumber songnumber;
		SongPosition songposition;
		SysExSkipper sysexskipper;
		TextMessageSkipper textmessageskipper;

		inline MidiMessage *GetMessage( const unsigned char id)
		{
			MidiMessage *pMsg;

			switch (id & 0xf0)
			{
			case 0x80:
				pMsg = &noteoff;
				break;
			case 0x90:
				pMsg = &noteon;
				break;
			case 0xa0:
				pMsg = &aftertouch;
				break;
			case 0xb0:
				pMsg = &controlchange;
				break;
			case 0xc0:
				pMsg = &programchange;
				break;
			case 0xd0:
				pMsg = &expression;
				break;
			case 0xe0:
				pMsg = &pitchwheel;
				break;
			case 0xf0:
				switch (id)
				{
				case 0xf0:
				case 0xf7:
					pMsg = &sysexskipper;
					break;
				case 0xf2:
					pMsg = &songposition;
					break;
				case 0xf3:
					pMsg = &songnumber;
					break;
				case 0xff:
					pMsg = &textmessageskipper;
					break;
				default:
					pMsg = &simplemessageskipper;
					break;
				}
				break;
			default:
				return 0;
			}

			pMsg->messageID = id;
			return pMsg;
		}
	};

	MidiMessage *GetMessage( const unsigned char id)
	{
		static MessageSet ms;
		return ms.GetMessage( id);
	}

}