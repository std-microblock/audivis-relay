/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    micarraywavtable.h

Abstract:-

    Declaration of wave miniport tables for the mic array.

--*/

#ifndef _VIRTUALAUDIODRIVER_MICARRAYWAVTABLE_H_
#define _VIRTUALAUDIODRIVER_MICARRAYWAVTABLE_H_

//
// Mic array range.
//
#define MICARRAY_RAW_CHANNELS                   2       // Channels for raw mode
#define MICARRAY_DEVICE_MAX_CHANNELS            2       // Max channels overall
#define MICARRAY_32_BITS_PER_SAMPLE_PCM         32      // 32 Bits Per Sample
#define MICARRAY_RAW_SAMPLE_RATE                48000   // Raw sample rate

//
// Max # of pin instances.
//
#define MICARRAY_MAX_INPUT_STREAMS              1

//=============================================================================
// Extended array of supported device formats.
//=============================================================================
static
KSDATAFORMAT_WAVEFORMATEXTENSIBLE MicArrayPinSupportedDeviceFormats[] =
{
    //------------------------------------------------------------------------
    // 48 KHz, 32-bit, 2 channels (Stereo)
    //------------------------------------------------------------------------
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
                WAVE_FORMAT_EXTENSIBLE,       // wFormatTag
                2,                           // nChannels
                48000,                       // nSamplesPerSec
                48000 * 2 * 4,               // nAvgBytesPerSec = 48000 * channels * bytes_per_sample
                2 * 4,                       // nBlockAlign = channels * bytes_per_sample
                32,                          // wBitsPerSample
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,                              // Samples (wValidBitsPerSample)
            KSAUDIO_SPEAKER_STEREO,          // dwChannelMask
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //------------------------------------------------------------------------
    // 44.1 KHz, 16-bit, 2 channels (Stereo)
    //------------------------------------------------------------------------
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
                44100,
                44100 * 2 * 2,  // nAvgBytesPerSec = 44100 * channels * bytes_per_sample
                2 * 2,          // nBlockAlign = channels * bytes_per_sample
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,                             // wValidBitsPerSample
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //------------------------------------------------------------------------
    // 48 KHz, 16-bit, 2 channels (Stereo)
    //------------------------------------------------------------------------
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
                48000 * 2 * 2, // nAvgBytesPerSec = 48000 * channels * bytes_per_sample
                2 * 2,         // nBlockAlign = channels * bytes_per_sample
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //------------------------------------------------------------------------
    // 96 KHz, 24-bit, 2 channels (Stereo)
    //------------------------------------------------------------------------
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
                96000,
                96000 * 2 * 3, // nAvgBytesPerSec = 96000 * channels * bytes_per_sample
                2 * 3,         // nBlockAlign = channels * bytes_per_sample
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //------------------------------------------------------------------------
    // 192 KHz, 24-bit, 2 channels (Stereo)
    //------------------------------------------------------------------------
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
                192000,
                192000 * 2 * 3, // nAvgBytesPerSec = 192000 * channels * bytes_per_sample
                2 * 3,          // nBlockAlign = channels * bytes_per_sample
                24,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            24,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },

    //------------------------------------------------------------------------
    // Example: 48 KHz, 32-bit float, 2 channels (Stereo)
    //          Uses FLOAT subtype instead of PCM.
    //------------------------------------------------------------------------
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                48000,
                48000 * 2 * 4, // 4 bytes per float sample
                2 * 4,         // nBlockAlign
                32,            // wBitsPerSample
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            32,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
        }
    },
};

//=============================================================================
// Supported modes (only on streaming pins).
// Add references to each format you want to expose in a particular mode.
//=============================================================================
static
MODE_AND_DEFAULT_FORMAT MicArrayPinSupportedDeviceModes[] =
{
    // If you only have one signal processing mode (e.g., RAW),
    // you can list each supported format under it.
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[0].DataFormat
    },
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[1].DataFormat
    },
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[2].DataFormat
    },
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[3].DataFormat
    },
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[4].DataFormat
    },
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[5].DataFormat
    },
};

//=============================================================================
// Notes on extension:
//
// 1. Adjust nChannels, nSamplesPerSec, wBitsPerSample, and the SUBTYPE_GUID
//    as appropriate. For example, floating-point (IEEE_FLOAT) is common for
//    advanced processing or higher-fidelity capture.
//
// 2. If you have multiple signal processing modes (like RAW, Speech, etc.),
//    you may add new MODE_AND_DEFAULT_FORMAT entries pointing to different
//    subsets of the MicArrayPinSupportedDeviceFormats array.
//
// 3. Make sure the size of KSDATAFORMAT_WAVEFORMATEXTENSIBLE (DataFormat.FormatSize)
//    matches the actual structure size, and that WAVEFORMATEXTENSIBLE is
//    used properly (e.g., wValidBitsPerSample must align with wBitsPerSample).
//
// 4. The GUID for KSDATAFORMAT_SUBTYPE_PCM or KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
//    must be used correctly if you are advertising PCM or float formats.
//
// 5. KSAUDIO_SPEAKER_STEREO is a standard channel mask for 2 channels.
//    If you have more channels (e.g., 4, 5.1, 7.1), use the appropriate
//    speaker channel masks (e.g. KSAUDIO_SPEAKER_5POINT1).
//=============================================================================


//
// The entries here must follow the same order as the filter's pin
// descriptor array.
//
static
PIN_DEVICE_FORMATS_AND_MODES MicArrayPinDeviceFormatsAndModes[] =
{
    {
        BridgePin,
        NULL,
        0,
        NULL,
        0
    },
    {
        SystemCapturePin,
        MicArrayPinSupportedDeviceFormats,
        SIZEOF_ARRAY(MicArrayPinSupportedDeviceFormats),
        MicArrayPinSupportedDeviceModes,
        SIZEOF_ARRAY(MicArrayPinSupportedDeviceModes)
    }
};

//=============================================================================
// Data ranges
//
// See CMiniportWaveRT::DataRangeIntersection.
//
static
KSDATARANGE_AUDIO MicArrayPinDataRangesRawStream[] =
{
    {
        {
            sizeof(KSDATARANGE_AUDIO),
            KSDATARANGE_ATTRIBUTES,         // An attributes list follows this data range
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        MICARRAY_RAW_CHANNELS,
        MICARRAY_32_BITS_PER_SAMPLE_PCM,
        MICARRAY_32_BITS_PER_SAMPLE_PCM,
        MICARRAY_RAW_SAMPLE_RATE,
        MICARRAY_RAW_SAMPLE_RATE
    },
};

static
PKSDATARANGE MicArrayPinDataRangePointersStream[] =
{
    // All supported device formats should be listed in the DataRange.
    PKSDATARANGE(&MicArrayPinDataRangesRawStream[0]),
    PKSDATARANGE(&PinDataRangeAttributeList),
};

//=============================================================================
static
KSDATARANGE MicArrayPinDataRangesBridge[] =
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
PKSDATARANGE MicArrayPinDataRangePointersBridge[] =
{
    &MicArrayPinDataRangesBridge[0]
};

//=============================================================================
static
PCPIN_DESCRIPTOR MicArrayWaveMiniportPins[] =
{
    // Wave In Bridge Pin (Capture - From Topology) KSPIN_WAVE_BRIDGE
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
            SIZEOF_ARRAY(MicArrayPinDataRangePointersBridge),
            MicArrayPinDataRangePointersBridge,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_NONE,
            &KSCATEGORY_AUDIO,
            NULL,
            0
        }
    },
    // Wave In Streaming Pin (Capture) KSPIN_WAVE_HOST
    {
        MICARRAY_MAX_INPUT_STREAMS,
        MICARRAY_MAX_INPUT_STREAMS,
        0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(MicArrayPinDataRangePointersStream),
            MicArrayPinDataRangePointersStream,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_SINK,
            &KSCATEGORY_AUDIO,
            &KSAUDFNAME_RECORDING_CONTROL,
            0
        }
    }
};

//=============================================================================
static
PCNODE_DESCRIPTOR MicArrayWaveMiniportNodes[] =
{
    // KSNODE_WAVE_ADC
    {
        0,                      // Flags
        NULL,                   // AutomationTable
        &KSNODETYPE_ADC,        // Type
        NULL                    // Name
    }
};

//=============================================================================
static
PCCONNECTION_DESCRIPTOR MicArrayWaveMiniportConnections[] =
{
    { PCFILTER_NODE,        KSPIN_WAVE_BRIDGE,      KSNODE_WAVE_ADC,     1 },
    { KSNODE_WAVE_ADC,      0,                      PCFILTER_NODE,       KSPIN_WAVEIN_HOST },
};

//=============================================================================

static
VIRTUALAUDIODRIVER_PROPERTY_ITEM PropertiesMicArrayWaveFilter[] =
{
    {
        {
            &KSPROPSETID_General,
            KSPROPERTY_GENERAL_COMPONENTID,
            KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    },
    {
        {
            &KSPROPSETID_Pin,
            KSPROPERTY_PIN_PROPOSEDATAFORMAT,
            KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    },
    {
        {
            &KSPROPSETID_Pin,
            KSPROPERTY_PIN_PROPOSEDATAFORMAT2,
            KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL,
        NULL,
        0
    },
};

DEFINE_PCAUTOMATION_TABLE_PROP(AutomationMicArrayWaveFilter, PropertiesMicArrayWaveFilter);

//=============================================================================
static
PCFILTER_DESCRIPTOR MicArrayWaveMiniportFilterDescriptor =
{
    0,                                              // Version
    &AutomationMicArrayWaveFilter,                  // AutomationTable
    sizeof(PCPIN_DESCRIPTOR),                       // PinSize
    SIZEOF_ARRAY(MicArrayWaveMiniportPins),         // PinCount
    MicArrayWaveMiniportPins,                       // Pins
    sizeof(PCNODE_DESCRIPTOR),                      // NodeSize
    SIZEOF_ARRAY(MicArrayWaveMiniportNodes),        // NodeCount
    MicArrayWaveMiniportNodes,                      // Nodes
    SIZEOF_ARRAY(MicArrayWaveMiniportConnections),  // ConnectionCount
    MicArrayWaveMiniportConnections,                // Connections
    0,                                              // CategoryCount
    NULL                                            // Categories  - use defaults (audio, render, capture)
};

#endif // _VIRTUALAUDIODRIVER_MICARRAYWAVTABLE_H_
