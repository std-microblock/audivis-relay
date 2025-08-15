#pragma once
#include "parsec-vusb-api.h"
#include <memory>
#include <stdexcept>
#include <print>

namespace client {

struct VirtualUSBHubService {
    VirtualUSBHubService();
    ~VirtualUSBHubService();

    parsec::vusb::VirtualUSBHub hub_;
    std::shared_ptr<parsec::vusb::VirtualUSBDevice> device_;
};

} // namespace client
