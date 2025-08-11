/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    speakerwavtable.h

Abstract:

    Declaration of wave miniport tables for the render endpoints.
--*/

#ifndef _VIRTUALAUDIODRIVER_SPEAKERWAVTABLE_H_
#define _VIRTUALAUDIODRIVER_SPEAKERWAVTABLE_H_

//=============================================================================
// To support 16-bit and 24-bit, and sample rates from 48kHz up to 192kHz:
//
#define SPEAKER_DEVICE_MAX_CHANNELS         2    // Max Channels.

#define SPEAKER_HOST_MAX_CHANNELS           2    // Max Channels.

//
// Change bits-per-sample range:
//
#define SPEAKER_HOST_MIN_BITS_PER_SAMPLE    16
#define SPEAKER_HOST_MAX_BITS_PER_SAMPLE    24

//
// Change sample-rate range:
//
#define SPEAKER_HOST_MIN_SAMPLE_RATE        48000
#define SPEAKER_HOST_MAX_SAMPLE_RATE        192000

//
// Max # of pin instances.
//
#define SPEAKER_MAX_INPUT_SYSTEM_STREAMS    1

//=============================================================================
//
// We now list multiple device formats: one for 16-bit and one for 24-bit.
//
// Note: Even though each entry below shows 48kHz as nSamplesPerSec, the
//       data-range below (SpeakerPinDataRangesStream) will expose 
//       48kHz through 192kHz to Windows. Each of these formats can 
//       be validated dynamically within those ranges.
//
static
KSDATAFORMAT_WAVEFORMATEXTENSIBLE SpeakerHostPinSupportedDeviceFormats[] =
{
    //-------------------------------------------------
    // 0) 16-bit, Stereo, 48 kHz (DVD Quality)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,                  // wFormatTag
                2,                                       // nChannels
                48000,                                   // nSamplesPerSec
                48000 * 2 * 16 / 8,                      // nAvgBytesPerSec
                2 * 16 / 8,                              // nBlockAlign
                16,                                      // wBitsPerSample
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,                                         // wValidBitsPerSample
            KSAUDIO_SPEAKER_STEREO,                     // dwChannelMask
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 1) 24-bit, Stereo, 48 kHz
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                48000,
                48000 * 2 * 24 / 8,                      // nAvgBytesPerSec
                2 * 24 / 8,                              // nBlockAlign
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,                                         // wValidBitsPerSample
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 2) 8-bit, Stereo, 8000 Hz
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                8000,
                8000 * 2 * 8 / 8,   // nAvgBytesPerSec = 8000 * 2 * 1
                2 * 8 / 8,         // nBlockAlign     = 2 * 1
                8,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            8,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 3) 8-bit, Stereo, 11025 Hz
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                11025,
                11025 * 2 * 8 / 8,
                2 * 8 / 8,
                8,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            8,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 4) 16-bit, Stereo, 11025 Hz (Dictation)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                11025,
                11025 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 5) 16-bit, Stereo, 16000 Hz (Tape Recorder)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                16000,
                16000 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 6) 16-bit, Stereo, 22050 Hz (AM Radio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                22050,
                22050 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 7) 16-bit, Stereo, 32000 Hz (FM Radio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                32000,
                32000 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 8) 16-bit, Stereo, 44100 Hz (CD Quality)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                44100,
                44100 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 9) 16-bit, Stereo, 88200 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                88200,
                88200 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 10) 16-bit, Stereo, 96000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                96000,
                96000 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 11) 16-bit, Stereo, 176400 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                176400,
                176400 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 12) 16-bit, Stereo, 192000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                192000,
                192000 * 2 * 16 / 8,
                2 * 16 / 8,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 13) 24-bit, Stereo, 44100 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                44100,
                44100 * 2 * 24 / 8,
                2 * 24 / 8,
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 14) 24-bit, Stereo, 88200 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                88200,
                88200 * 2 * 24 / 8,
                2 * 24 / 8,
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 15) 24-bit, Stereo, 96000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                96000,
                96000 * 2 * 24 / 8,
                2 * 24 / 8,
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 16) 24-bit, Stereo, 176400 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                176400,
                176400 * 2 * 24 / 8,
                2 * 24 / 8,
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 17) 24-bit, Stereo, 192000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                192000,
                192000 * 2 * 24 / 8,
                2 * 24 / 8,
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 18) 32-bit, Stereo, 44100 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                44100,
                44100 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 19) 32-bit, Stereo, 48000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                48000,
                48000 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 20) 32-bit, Stereo, 88200 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                88200,
                88200 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 21) 32-bit, Stereo, 96000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                96000,
                96000 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 22) 32-bit, Stereo, 176400 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                176400,
                176400 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //-------------------------------------------------
    // 23) 32-bit, Stereo, 192000 Hz (Studio)
    //-------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0, 0, 0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                192000,
                192000 * 2 * 32 / 8,
                2 * 32 / 8,
                32,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    }
};


//
// Supported modes (only on streaming pins).
// Here we pick one as default, but you could list more.
//
//
// Supported modes (only on streaming pins).
// Here we pick multiple entries for demonstration, each referencing
// a different format from SpeakerHostPinSupportedDeviceFormats[].
//
static
MODE_AND_DEFAULT_FORMAT SpeakerHostPinSupportedDeviceModes[] =
{
    // 0) 16-bit, Stereo, 48000 Hz (DVD Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[0].DataFormat
    },

    // 1) 24-bit, Stereo, 48000 Hz
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[1].DataFormat
    },

    // 2) 8-bit, Stereo, 8000 Hz (Lowest Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[2].DataFormat
    },

    // 3) 8-bit, Stereo, 11025 Hz (Telephone Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[3].DataFormat
    },

    // 4) 16-bit, Stereo, 11025 Hz (Dictation Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[4].DataFormat
    },

    // 5) 16-bit, Stereo, 16000 Hz (Tape Recorder Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[5].DataFormat
    },

    // 6) 16-bit, Stereo, 22050 Hz (AM Radio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[6].DataFormat
    },

    // 7) 16-bit, Stereo, 32000 Hz (FM Radio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[7].DataFormat
    },

    // 8) 16-bit, Stereo, 44100 Hz (CD Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[8].DataFormat
    },

    // 9) 16-bit, Stereo, 88200 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[9].DataFormat
    },

    // 10) 16-bit, Stereo, 96000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[10].DataFormat
    },

    // 11) 16-bit, Stereo, 176400 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[11].DataFormat
    },

    // 12) 16-bit, Stereo, 192000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[12].DataFormat
    },

    // 13) 24-bit, Stereo, 44100 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[13].DataFormat
    },

    // 14) 24-bit, Stereo, 88200 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[14].DataFormat
    },

    // 15) 24-bit, Stereo, 96000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[15].DataFormat
    },

    // 16) 24-bit, Stereo, 176400 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[16].DataFormat
    },

    // 17) 24-bit, Stereo, 192000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[17].DataFormat
    },

    // 18) 32-bit, Stereo, 44100 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[18].DataFormat
    },

    // 19) 32-bit, Stereo, 48000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[19].DataFormat
    },

    // 20) 32-bit, Stereo, 88200 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[20].DataFormat
    },

    // 21) 32-bit, Stereo, 96000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[21].DataFormat
    },

    // 22) 32-bit, Stereo, 176400 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[22].DataFormat
    },

    // 23) 32-bit, Stereo, 192000 Hz (Studio Quality)
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_DEFAULT,
        &SpeakerHostPinSupportedDeviceFormats[23].DataFormat
    }
};


// The entries must follow the same order as the filter's pin descriptor array.
static
PIN_DEVICE_FORMATS_AND_MODES SpeakerPinDeviceFormatsAndModes[] =
{
    {
        SystemRenderPin,
        SpeakerHostPinSupportedDeviceFormats,
        SIZEOF_ARRAY(SpeakerHostPinSupportedDeviceFormats),
        SpeakerHostPinSupportedDeviceModes,
        SIZEOF_ARRAY(SpeakerHostPinSupportedDeviceModes)
    },
    {
        BridgePin,
        NULL,
        0,
        NULL,
        0
    }
};

//=============================================================================
//
// Now we define the data-range for the streaming pin. 
// This single data-range references the macros above (2 channels, 16-24 bits,
// 48k-192k sample rate). The OS will match any requested format in that range.
//
static
KSDATARANGE_AUDIO SpeakerPinDataRangesStream[] =
{
    {
        // --- KSDATARANGE header part (must be first) ---
        {
            sizeof(KSDATARANGE_AUDIO),
            KSDATARANGE_ATTRIBUTES,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },

        // --- KSDATARANGE_AUDIO fields ---
        2,       // MaximumChannels
        8,      // MinimumBitsPerSample
        32,      // MaximumBitsPerSample
        8000,   // MinimumSampleFrequency (48 kHz)
        384000   // MaximumSampleFrequency (192 kHz)
    }
};


static
PKSDATARANGE SpeakerPinDataRangePointersStream[] =
{
    PKSDATARANGE(&SpeakerPinDataRangesStream[0]),
    PKSDATARANGE(&PinDataRangeAttributeList),
};

//=============================================================================
static
KSDATARANGE SpeakerPinDataRangesBridge[] =
{
    {
        sizeof(KSDATARANGE),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_ANALOG),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    }
};

static
PKSDATARANGE SpeakerPinDataRangePointersBridge[] =
{
    &SpeakerPinDataRangesBridge[0]
};

//=============================================================================
static
PCPIN_DESCRIPTOR SpeakerWaveMiniportPins[] =
{
    // Wave Out Streaming Pin (Renderer) KSPIN_WAVE_RENDER3_SINK_SYSTEM
    {
        SPEAKER_MAX_INPUT_SYSTEM_STREAMS,
        SPEAKER_MAX_INPUT_SYSTEM_STREAMS,
        0,
        NULL,        // AutomationTable
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(SpeakerPinDataRangePointersStream),
            SpeakerPinDataRangePointersStream,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_SINK,
            &KSCATEGORY_AUDIO,
            NULL,
            0
        }
    },
    // Wave Out Bridge Pin (Renderer) KSPIN_WAVE_RENDER3_SOURCE
    {
        0,
        0,
        0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(SpeakerPinDataRangePointersBridge),
            SpeakerPinDataRangePointersBridge,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_NONE,
            &KSCATEGORY_AUDIO,
            NULL,
            0
        }
    },
};

//=============================================================================
//
//                   ----------------------------      
//                   |                          |      
//  Host Pin     0-->|                          |--> 1 KSPIN_WAVE_RENDER3_SOURCE
//                   |                          |      
//                   ----------------------------
//
static
PCCONNECTION_DESCRIPTOR SpeakerWaveMiniportConnections[] =
{
    { PCFILTER_NODE, KSPIN_WAVE_RENDER3_SINK_SYSTEM, PCFILTER_NODE, KSPIN_WAVE_RENDER3_SOURCE }
};

//=============================================================================
static
PCPROPERTY_ITEM PropertiesSpeakerWaveFilter[] =
{
    {
        &KSPROPSETID_Pin,
        KSPROPERTY_PIN_PROPOSEDATAFORMAT,
        KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_WaveFilter
    },
    {
        &KSPROPSETID_Pin,
        KSPROPERTY_PIN_PROPOSEDATAFORMAT2,
        KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
        PropertyHandler_WaveFilter
    }
};

DEFINE_PCAUTOMATION_TABLE_PROP(AutomationSpeakerWaveFilter, PropertiesSpeakerWaveFilter);

//=============================================================================
static
PCFILTER_DESCRIPTOR SpeakerWaveMiniportFilterDescriptor =
{
    0,                                              // Version
    &AutomationSpeakerWaveFilter,                   // AutomationTable
    sizeof(PCPIN_DESCRIPTOR),                       // PinSize
    SIZEOF_ARRAY(SpeakerWaveMiniportPins),          // PinCount
    SpeakerWaveMiniportPins,                        // Pins
    sizeof(PCNODE_DESCRIPTOR),                      // NodeSize
    0,                                              // NodeCount
    NULL,                                           // Nodes
    SIZEOF_ARRAY(SpeakerWaveMiniportConnections),   // ConnectionCount
    SpeakerWaveMiniportConnections,                 // Connections
    0,                                              // CategoryCount
    NULL                                            // Categories  - use defaults (audio, render, capture)
};

#endif // _VIRTUALAUDIODRIVER_SPEAKERWAVTABLE_H_
