#if !defined(DIRECTSOUND_PLAYER_HPP)
#define DIRECTSOUND_PLAYER_HPP

#include <dsound.h>
#include <exception>
#include <boost/function.hpp>
#include "napl.h"

#undef min

namespace
{
    template< size_t size>
    void check_hr( HRESULT hr, const char (&text)[size])
    {
        if (FAILED(hr))
        {
            throw std::runtime_error( text);
        }
    }
}

/// a NAPL consumer that can play sound to an audio device
class directsound_player : public block_consumer
{
public:
    typedef boost::function< void(size_t)>      signal_type;

    static const size_t buffer_seconds = 2;
    static const int unique_event_count = 4;
    static const int events_per_second = 100;

    directsound_player( 
        IDirectSoundBuffer *buffer_, 
        const DSBUFFERDESC &desc_,
        const stream_header &h)
        : buffer( buffer_), desc( desc_), header(h)
    {
    }

    directsound_player()
        : buffer(0)
    {
    }

    void register_position_handler( signal_type signal)
    {
        position_signal = signal;
    }

    /// receive a NAPL block of wave data
	virtual void ReceiveBlock( const sample_block &b)
    {
        std::copy( b.m_start, b.m_end, current_buffer_ptr);
        current_buffer_ptr += (b.m_end - b.m_start);
    }

    /// start playing
    void start( bool do_events = true)
    {
        setup_events();
        stream_header h;
        m_pProducer->GetStreamHeader( h);
        samples_to_go = header.numframes;

        // get the first bunch of samples
        issue_request( true);
        expected_position = 0;

        // set the whole circus in motion.
        // playing the buffer will generate events
        buffer->SetCurrentPosition(0);
        buffer->Play( 0, 0, DSBPLAY_LOOPING);

        if (do_events)
        {
            handle_events();
        }

    }

    /// handle events that are generated during sound play
    void handle_events()
    {
        bool go_on = true;
        while (go_on)
        {
            unsigned int index = ::WaitForMultipleObjects( unique_event_count + 2, &events[0], FALSE, INFINITE);
            if (index >= WAIT_OBJECT_0 && index < WAIT_OBJECT_0 + unique_event_count + 2)
            {
                index -= WAIT_OBJECT_0;
                ::ResetEvent( events[index]);
                if (index < unique_event_count )
                {
                    position_event( index);
                }
                else 
                {
                    index -= (unique_event_count);
                    go_on = refill_event( index);
                }
            }

        }
    }

    bool refill_event( unsigned int index)
    {
        issue_request( index != 0);
        return samples_to_go != 0;
    }
    
    void position_event( unsigned int index)
    {
        // catch up on any missed events
        while (expected_position%unique_event_count != index)
        {
            position_signal( expected_position++);
        }

        // send out the event
        position_signal( expected_position++);
    }

    ~directsound_player()
    {
        if (buffer)
        {
            buffer->Release();
        }
        free_events();
    }


    
private:
    typedef std::vector<HANDLE>         event_vector; 

    /// send out a request for more samples
    void issue_request( 
        bool is_first_half ///< whether to fill the first half of the buffer or the second
        )
    {
        lock( is_first_half); 
        const size_t samples_per_half_buffer = desc.dwBufferBytes / header.frame_size() / 2;
        const size_t requested_samples = std::min(samples_per_half_buffer, samples_to_go);
        m_pProducer->RequestBlock( *this, header.numframes - samples_to_go, requested_samples);
        unlock();
        samples_to_go -= requested_samples;
    }

    /// Create a limited amount of events (say, 4) and create notification positions at every
    /// 1/Nth of a second (say 1/100th).
    void setup_events()
    {
        size_t bytes_per_event = header.samplerate * header.frame_size() / events_per_second;
        int position_count = events_per_second * buffer_seconds;

        // create 2 extra events for the 'refill buffer' events
        free_events();
        for (int counter = 0; counter < unique_event_count + 2; ++ counter)
        {
            events.push_back( CreateEvent( NULL, TRUE, FALSE, NULL));
        }

        LPDIRECTSOUNDNOTIFY8 lpDsNotify;
        std::vector<DSBPOSITIONNOTIFY> notifications(position_count + 2);

        check_hr( 
            buffer->QueryInterface(IID_IDirectSoundNotify8,(LPVOID*)&lpDsNotify),
            "could not access events of soundbuffer"
            );

        size_t pos = 0;
        for (; pos != position_count; ++pos)
        {
            notifications[pos].hEventNotify = events[ pos % unique_event_count];
            notifications[pos].dwOffset = bytes_per_event * pos;
        }

        // add 2 extra notifications for the 'refill buffer' events.

        notifications[pos].hEventNotify = events[ unique_event_count];
        notifications[pos].dwOffset = bytes_per_event / 2;
        ++pos;
        notifications[pos].hEventNotify = events[unique_event_count + 1];
        notifications[pos].dwOffset = desc.dwBufferBytes/2 + bytes_per_event/2;

        check_hr(
            lpDsNotify->SetNotificationPositions( notifications.size(), &notifications[0]),
            "could not set up timing events"
            );

        lpDsNotify->Release();
    }

    void free_events()
    {
        std::for_each( events.begin(), events.end(), &::CloseHandle);
        events.clear();
    }

    void lock( bool first_half)
    {
        if (first_half)
        {
            lock( 0, desc.dwBufferBytes/2);
        }
        else
        {
            lock( desc.dwBufferBytes/2, desc.dwBufferBytes/2);
        }
    }

    void lock( size_t offset, size_t bytes)
    {
        HRESULT hr = buffer->Lock( offset, bytes, &buffer_data, &buffer_length, 0, 0, 0);
        if (hr == DSERR_BUFFERLOST)
        {
            buffer->Restore();
            hr = buffer->Lock( offset, bytes, &buffer_data, &buffer_length, 0, 0, 0);
        }
        check_hr( hr, "could not lock buffer");
        current_buffer_ptr = static_cast<char*>( buffer_data);
    }

    void unlock()
    {
        buffer->Unlock( buffer_data, buffer_length, 0, 0);
    }

    /// events that are used to trigger on offsets in the buffer
    event_vector    events;

    /// pointer to the start of the complete buffer
    void            *buffer_data;

    /// pointer to the location that will receive data during a refill.
    char            *current_buffer_ptr;

    /// length of the full buffer
    DWORD           buffer_length;

    /// description of the buffer
    const DSBUFFERDESC desc;

    /// pointer to the DirectSound buffer.
    IDirectSoundBuffer *buffer;

    /// information about the sample size and expected number of samples, etc
    stream_header   header;

    /// the number of samples that we need to request from our buffer provider
    size_t          samples_to_go;

    /// the next position we're supposed to reach. In units of 1/100th of a second.
    size_t          expected_position;

    /// signal that is given when we've reached a new position in the buffer
    signal_type     position_signal;
};

class directsound_wrapper
{
public:
    directsound_wrapper()
    {
        GUID     guID;

        // Clear the GUID
        ::memset(&guID, 0, sizeof(GUID));

       check_hr(
            CoCreateInstance(CLSID_DirectSound, NULL, CLSCTX_INPROC_SERVER,
                IID_IDirectSound, (void**)&m_pIDS),
            "could not initialize directx"
            );

        if(m_pIDS == NULL)
        {
            throw std::runtime_error("did not instantiate a sound object");
        }

        // Initialize the IDirectSound object
        check_hr(
            m_pIDS->Initialize(&guID),
            "could not initialize sound"
            );

        check_hr( 
            m_pIDS->SetCooperativeLevel( GetConsoleHwnd(), DSSCL_PRIORITY ),
            "Could not get exclusive access to the soundcard"
            );
    }

    directsound_player create_player( const stream_header &h)
    {
        WAVEFORMATEX wfx;
        DSBUFFERDESC desc;

        memset( &wfx, 0, sizeof( wfx));
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = h.numchannels;
        wfx.wBitsPerSample = h.get_sample_size();
        wfx.nSamplesPerSec = h.samplerate;
        wfx.nBlockAlign = h.frame_size();
        wfx.nAvgBytesPerSec = h.samplerate * h.frame_size(); 

        memset( &desc, 0, sizeof(desc));
        desc.dwSize = sizeof( desc);
        desc.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
        desc.dwBufferBytes = directsound_player::buffer_seconds * h.frame_size() * h.samplerate;
        desc.lpwfxFormat = &wfx;

        IDirectSoundBuffer *buffer_ptr = 0;
        check_hr(
            m_pIDS->CreateSoundBuffer( &desc, &buffer_ptr,NULL),
            "could not create a sound buffer"
            );

        return directsound_player( buffer_ptr, desc, h);

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

#endif //DIRECTSOUND_PLAYER_HPP