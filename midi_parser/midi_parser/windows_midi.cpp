#include <windows.h>
#include <stdexcept>

struct midi_exception : public std::runtime_error
{
	midi_exception( const std::string &what)
		:std::runtime_error( what)
	{}

};

void windows_midi()
{
	unsigned long result; 
	HMIDIOUT      outHandle;

	/* Open the MIDI Mapper. */
	result = midiOutOpen(&outHandle, (UINT)-1, 0, 0, CALLBACK_WINDOW);
	if (result)
	{
		throw midi_exception( "could not open midi device");
	}


    midiOutShortMsg( outHandle, 0x007f4090);
    midiOutShortMsg( outHandle, 0x007f4590);

    midiOutShortMsg( outHandle, 0x007f4990);
    
    ::Sleep( 1000);
}

