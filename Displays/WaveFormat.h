

class WaveFormat: public WAVEFORMATEX
{
public:
    WaveFormat (
        WORD    nCh, // number of channels (mono, stereo)
        DWORD   nSampleRate, // sample rate
        WORD    BitsPerSample)
    {
        wFormatTag = WAVE_FORMAT_PCM;
        nChannels = nCh;
        nSamplesPerSec = nSampleRate;
        nAvgBytesPerSec = nSampleRate * nCh * BitsPerSample/8;
        nBlockAlign = nChannels * BitsPerSample/8;
        wBitsPerSample = BitsPerSample;
        cbSize = 0;
    }

    BOOL isInSupported (UINT idDev)
    {
        MMRESULT result = waveInOpen
            (0, idDev, this, 0, 0, WAVE_FORMAT_QUERY);
        return result == MMSYSERR_NOERROR;
    }
};


class WaveHeader: public WAVEHDR
{
public:
    BOOL IsDone () const { return dwFlags & WHDR_DONE; }
};
