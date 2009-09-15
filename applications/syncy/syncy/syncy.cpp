// syncy.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <iomanip>
#include <exception>
#include <dsound.h>
#include <sstream>
#include <string>

class directsound_buffer
{
};

class directsound_wrapper
{
public:
    directsound_wrapper()
    {
        HRESULT  hr;
        GUID     guID;

        // Clear the GUID
        ::memset(&guID, 0, sizeof(GUID));

        // Create the IDirectSound object with the realiable way (create
        // standard COM object)
        check(
            CoCreateInstance(CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER,
                IID_IDirectSound, (void**)&m_pIDS),
            "could not initialize directx"
            );

        if(m_pIDS == NULL)
        {
            throw std::runtime_error("did not instantiate a sound object");
        }

        // Initialize the IDirectSound object
        check(
            m_pIDS->Initialize(&guID),
            "could not initialize sound"
            };

        check( 
            m_pIDS->SetCooperativeLevel( GetConsoleHwnd(), DSSCL_PRIORITY ),
            "Could not get exclusive access to the soundcard"
            );
    }

    directsound_buffer create_buffer( const size_t size)
    {
        WAVEFORMATEX wfx;
        DSBUFFERDESC desc;

        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = 2;
        wfx.wBitsPerSample = 16;
        wfx.nSamplesPerSec = 44100;
        wfx.nBlockAlign = 4;
        wfx.nAvgBytesPerSec = 44100 * 4; 

        desc.dwSize = sizeof( desc);
        desc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2;
        desc.dwBufferBytes = size;

    }

    ~directsound_wrapper()
    {
        if(m_pIDS)
        {
            m_pIDS->Release();
            m_pIDS = NULL;
        }	
    }


private:
    template< size_t size>
    void check( HRESULT hr, const char (&text)[size])
    {
        if (FAILED(hr))
        {
            throw std::runtime_error( text);
        }
    }

    HWND GetConsoleHwnd()
    {
        const int MY_BUFSIZE=1024; // Buffer size for console window titles.
        std::stringstream s;
        HWND hwndFound;         // This is what is returned to the caller.
        char pszOldWindowTitle[MY_BUFSIZE]; // Contains original
        // WindowTitle.

        // Fetch current window title.

        GetConsoleTitleA(pszOldWindowTitle, MY_BUFSIZE);

        // Format a "unique" NewWindowTitle.

       s    << "title_"
            <<  GetTickCount() << '_'
            <<  GetCurrentProcessId();

        // Change current window title.

        SetConsoleTitleA(s.str().c_str());

        // Ensure window title has been updated.

        Sleep(40);

        // Look for NewWindowTitle.

        hwndFound=FindWindowA(NULL, s.str().c_str());

        // Restore original window title.

        SetConsoleTitleA(pszOldWindowTitle);

        return(hwndFound);
    }

    LPDIRECTSOUND       m_pIDS;
};

int main(int argc, char* argv[])
{
    // this application needs COM to access directsound
    HRESULT hr = ::CoInitialize(NULL);

    if(FAILED(hr))
    {
        return -1;
    }

    try
    {
        directsound_wrapper ds;
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    ::CoUninitialize();
    return 0;
}

