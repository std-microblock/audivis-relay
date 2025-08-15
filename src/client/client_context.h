#pragma once
#include "virtual_usb_hub.h"
#include "webrtc_service.h"
#include <memory>

struct audivis_widget;

namespace client {

struct ClientContext {
  static ClientContext &get_instance();

  ClientContext(const ClientContext &) = delete;
  ClientContext &operator=(const ClientContext &) = delete;


  void init_webrtc_service();

  ClientContext(); // Private constructor for singleton
  std::shared_ptr<VirtualUSBHubService> virtual_usb_hub_service;
  std::shared_ptr<WebRTCService> webrtc_service;
  std::shared_ptr<audivis_widget> root_widget;
};

} // namespace client
