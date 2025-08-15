#ifndef CLIENT_WEBRTC_SERVICE_H
#define CLIENT_WEBRTC_SERVICE_H

#include "httplib.h"
#include "rtc/configuration.hpp"
#include <deque>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <rtc/rtc.hpp>
#include <string>
#include <vector>

namespace client {

class WebRTCService {
public:
    enum class ConnectionState {
        Gathering,
        WaitingConnection,
        Connected,
        Disconnected,
        Failed
    };

    struct WebRTCStatus {
        ConnectionState state;
        std::string session_id;
    };

    using AudioDataCallback = std::function<void(const std::vector<uint8_t>&)>;
    using StatusCallback = std::function<void(const WebRTCStatus&)>;

    WebRTCService(AudioDataCallback audio_data_callback, StatusCallback status_callback);
    ~WebRTCService();

    void start_signaling();
    inline std::string session_id() const { return session_id_; }

private:
    httplib::Client cli_;
    std::shared_ptr<rtc::PeerConnection> pc_;
    std::shared_ptr<rtc::DataChannel> dc_;
    std::deque<uint8_t> audioBuffer_;
    AudioDataCallback audio_data_callback_;
    StatusCallback status_callback_;
    std::string session_id_;

    void setup_peer_connection();
    void setup_data_channel();
    void update_status(ConnectionState state);
};

} // namespace client

#endif // CLIENT_WEBRTC_SERVICE_H