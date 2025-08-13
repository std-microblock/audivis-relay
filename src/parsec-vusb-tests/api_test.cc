#include "parsec-vusb-api.h"
#include <gtest/gtest.h>
#include <print>

TEST(ParsecVUSB, HubCreation) {
  ASSERT_NO_THROW({ parsec::vusb::VirtualUSBHub hub; });
}

TEST(ParsecVUSB, DeviceCreation) {
  parsec::vusb::VirtualUSBHub hub;
  ASSERT_NO_THROW({
    auto device = hub.create_device(parsec::vusb::DefaultMicrophoneDescriptor);
  });
}

TEST(ParsecVUSB, FullDeviceLifecycle) {
  parsec::vusb::VirtualUSBHub hub;
  auto device = hub.create_device(parsec::vusb::DefaultMicrophoneDescriptor);
  ASSERT_NE(device, nullptr);

  std::vector<uint8_t> endpoints = {0x81};
  ASSERT_NO_THROW(device->configure_endpoints(endpoints));
  std::vector<int32_t> types = {2};
  ASSERT_NO_THROW(device->configure_endpoint_types(types));
  ASSERT_NO_THROW(device->plug_in());
  std::vector<uint8_t> randomPCM(960);
  for (size_t i = 0; i < 960; i++) {
    randomPCM[i] = static_cast<uint8_t>(rand() % 256);
  }

  // It may not be ready to submit immediately
  Sleep(1000);

  bool submitted = false;
  for (int i = 0; i < 10; ++i) {
    if (device->submit_audio_data(randomPCM)) {
      submitted = true;
      break;
    }
    Sleep(100);
  }
  ASSERT_TRUE(submitted);

  ASSERT_NO_THROW(device->unplug());
}
