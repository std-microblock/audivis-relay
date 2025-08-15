#pragma once
#include "parsec-vusb-api.h"
#include <memory>
#include <stdexcept>
#include <print>

namespace client {

class VirtualUSBHubService {
public:
    VirtualUSBHubService();
    ~VirtualUSBHubService();

    std::shared_ptr<parsec::vusb::VirtualUSBDevice> get_device() const {
        return device_;
    }

private:
    parsec::vusb::VirtualUSBHub hub_;
    std::shared_ptr<parsec::vusb::VirtualUSBDevice> device_;
};

} // namespace client
