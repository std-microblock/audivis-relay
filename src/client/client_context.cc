#include "client_context.h"
#include <stdexcept>

namespace client {

ClientContext& ClientContext::get_instance() {
    static ClientContext instance;
    return instance;
}

ClientContext::ClientContext() {
    virtual_usb_hub_service_ = std::make_shared<VirtualUSBHubService>();
    webrtc_service_ = std::make_shared<WebRTCService>(
        [&](const std::vector<uint8_t>& data) {
            if (virtual_usb_hub_service_ && virtual_usb_hub_service_->get_device()) {
                virtual_usb_hub_service_->get_device()->submit_audio_data(data);
            }
        },
        [&](const WebRTCService::WebRTCStatus& status) {
            // Handle WebRTC status updates here, e.g., log them or update UI
            // For now, just print
            std::println("WebRTC Status: State={}, SessionID={}", static_cast<int>(status.state), status.session_id);
        }
    );
}

} // namespace client