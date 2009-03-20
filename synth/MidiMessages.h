//
// MidiMessages.h definition of Midi-Messages
//

namespace MidiMessages
{

	struct MidiMessage
	{
	public:
		unsigned char m_messageID;

	public:
		virtual bool Parse( istream &input) = 0;
		virtual bool CallReceiver( MidiReceiver *pReceiver) = 0;
	};

	MidiMessage *GetMessage( const unsigned char id);
}