// MidiScanner.cpp: implementation of the MidiScanner class.
//
//
//

#include "MidiMessages.h"
#include "MidiUtils.h"
#include "MidiScanner.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MidiScanner::MidiScanner() : m_receivers(16)
{

}

MidiScanner::~MidiScanner()
{

}

bool MidiScanner::ParseStream(istream & input)
{
	unsigned long DeltaTime = 0;
	unsigned char lastChannel = 0;
	MidiMessage *lastMessage = 0;

	DeltaTime = MidiUtils::ParseVarLen( input);
	TimeShift( DeltaTime);

	while (input)
	{
		input.get( ch);
		if ( ch & 0x80)
		{
			lastMessage = MidiMessages::GetMessage( ch);
			lastChannel = ch & 0x0f;
		}
		else
		{
			input.putback( ch);
		}

		if (!lastMessage) 
		{
			return false;
		}

		if (lastMessage->Parse( input))
		{
			if ((lastMessage->m_messageID & 0xf0) != 0xf0)
			{
				lastMessage->CallReceiver( m_receivers[ lastChannel]);
			}
		}
		else 
		{
			return false;
		}
	}
}

MidiReceiver * MidiScanner::SetReceiverOnChannel(MidiReceiver * pReceiver, int channel)
{
	if (channel >= m_receivers.size() || channel < 0) return 0;
	else
	{
		MidiReceiver * ret = m_receivers[ channel];
		m_receivers[channel] = pReceiver;
		return ret;
	}
}
