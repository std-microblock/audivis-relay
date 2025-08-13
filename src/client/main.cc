#include "parsec-vusb-api.h"
#include <print>

#include <cmath>
#include <vector>

#include "libnyquist/Decoders.h"
#include <samplerate.h>

#include "cpptrace/from_current.hpp"

std::vector<int16_t> generate_sine_wave(float frequency, int sample_rate,
                                        float duration) {
  std::vector<int16_t> samples;
  int total_samples = static_cast<int>(sample_rate * duration);
  for (int i = 0; i < total_samples; ++i) {
    float t = static_cast<float>(i) / sample_rate;
    samples.push_back(
        static_cast<int16_t>(std::sin(2 * 3.1415 * frequency * t) * 32767));
  }
  return samples;
}

int main() {
  CPPTRACE_TRY {
    parsec::vusb::VirtualUSBHub hub;
    auto device =
        hub.device_exists(1)
            ? hub.open_device(1)
            : hub.create_device(parsec::vusb::DefaultMicrophoneDescriptor);

    if (!device) {
      std::print("Failed to open device.\n");
      return 1;
    }

    std::println("DeviceID: {}", device->device_id());

    device->configure_endpoints({0x81});
    device->configure_endpoint_types({0x02});
    device->plug_in();
    nqr::NyquistIO loader;
    auto fileData = std::make_shared<nqr::AudioData>();
    loader.Load(fileData.get(), "D:\\audivis-relay\\resources\\test.ogg");

    std::println("Audio info: {} {} {}", fileData->channelCount,
                 fileData->frameSize, fileData->sampleRate);

    constexpr auto deviceChannelCount = 1;
    constexpr auto deviceSampleRate = 48000;

    std::vector<float> convertedSamples;

    if (fileData->channelCount != deviceChannelCount ||
        fileData->sampleRate != deviceSampleRate) {
      if (fileData->sampleRate != deviceSampleRate) {
        SRC_DATA src_data;
        src_data.data_in = fileData->samples.data();
        src_data.input_frames =
            fileData->samples.size() / fileData->channelCount;
        src_data.src_ratio =
            static_cast<double>(deviceSampleRate) / fileData->sampleRate;

        std::vector<float> resampled(
            static_cast<size_t>(src_data.input_frames * src_data.src_ratio *
                                fileData->channelCount));
        src_data.data_out = resampled.data();
        src_data.output_frames = resampled.size() / fileData->channelCount;

        int error =
            src_simple(&src_data, SRC_SINC_FASTEST, fileData->channelCount);
        if (error != 0) {
          std::print("Resampling error: {}\n", src_strerror(error));
          return 1;
        }

        convertedSamples.assign(resampled.begin(),
                                resampled.begin() + src_data.output_frames_gen *
                                                        fileData->channelCount);
      } else {
        convertedSamples = fileData->samples;
      }

      if (fileData->channelCount != deviceChannelCount) {
        std::vector<float> channelConverted;
        size_t frameCount = convertedSamples.size() / fileData->channelCount;

        if (fileData->channelCount > deviceChannelCount) {
          channelConverted.reserve(frameCount);
          for (size_t i = 0; i < frameCount; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < fileData->channelCount; ++ch) {
              sum += convertedSamples[i * fileData->channelCount + ch];
            }
            channelConverted.push_back(sum / fileData->channelCount);
          }
        }
        convertedSamples = std::move(channelConverted);
      }
    } else {
      convertedSamples = fileData->samples;
    }

    size_t pData = 0;
    const size_t samplesPerPacket = 480; // 480 samples for 48kHz mono (10ms)

    while (pData < convertedSamples.size()) {
      try {
        size_t remainingSamples = convertedSamples.size() - pData;
        size_t currentPacketSize = std::min(samplesPerPacket, remainingSamples);

        std::vector<uint8_t> audio_data(currentPacketSize * sizeof(int16_t));
        int16_t *int16Data = reinterpret_cast<int16_t *>(audio_data.data());

        for (size_t i = 0; i < currentPacketSize; ++i) {
          float sample = std::clamp(convertedSamples[pData + i], -1.0f, 1.0f);
          int16Data[i] = static_cast<int16_t>(sample * 32767.0f);
        }

        if (device->submit_audio_data(audio_data)) {
          pData += currentPacketSize;
        }
      } catch (const parsec::vusb::VUSBError &e) {
        std::print("VUSB Error: {}\n", e.what());
        Sleep(100);
      }
    }
  }
  CPPTRACE_CATCH(std::exception & e) {
    std::print("Error: {}\n", e.what());
    cpptrace::from_current_exception().print();
  }
  catch (...) {
    std::print("Unknown error occurred\n");
    cpptrace::from_current_exception().print();
  }
}