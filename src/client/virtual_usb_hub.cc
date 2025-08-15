#include "virtual_usb_hub.h"

namespace client {

VirtualUSBHubService::VirtualUSBHubService() {
    int orig_device_id = 1;
    device_ = hub_.device_exists(orig_device_id)
                  ? hub_.open_device(orig_device_id)
                  : hub_.create_device(parsec::vusb::DefaultMicrophoneDescriptor);

    if (!device_) {
        throw std::runtime_error("Failed to open or create virtual microphone device.");
    }

    device_->configure_endpoints({0x81});
    device_->configure_endpoint_types({0x02});
    device_->plug_in();
    std::println("Virtual microphone plugged in with DeviceID: {}", device_->device_id());
}

VirtualUSBHubService::~VirtualUSBHubService() {
    if (device_) {
        device_->unplug();
        std::println("Virtual microphone unplugged.");
    }
}

} // namespace client