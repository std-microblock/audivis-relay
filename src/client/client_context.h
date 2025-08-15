#pragma once
#include "virtual_usb_hub.h"
#include "webrtc_service.h"
#include <memory>

namespace client {

class ClientContext {
public:
  static ClientContext &get_instance();

  // Delete copy constructor and assignment operator to prevent copying
  ClientContext(const ClientContext &) = delete;
  ClientContext &operator=(const ClientContext &) = delete;

  std::shared_ptr<VirtualUSBHubService> get_virtual_usb_hub_service() const {
    return virtual_usb_hub_service_;
  }

  std::shared_ptr<WebRTCService> get_webrtc_service() const {
    return webrtc_service_;
  }

private:
  ClientContext(); // Private constructor for singleton
  std::shared_ptr<VirtualUSBHubService> virtual_usb_hub_service_;
  std::shared_ptr<WebRTCService> webrtc_service_;
};

} // namespace client
