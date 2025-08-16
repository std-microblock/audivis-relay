#include "webrtc_service.h"
#include <iostream>
#include <print>

using json = nlohmann::json;

namespace client {

WebRTCService::WebRTCService(AudioDataCallback audio_data_callback,
                             StatusCallback status_callback)
    : cli_("http://audivis-signaling-server.microblock.cc"),
      audio_data_callback_(std::move(audio_data_callback)),
      status_callback_(std::move(status_callback)) {
  cli_.set_connection_timeout(0, 300000); // 5 minutes
  setup_peer_connection();
  setup_data_channel();
  update_status(ConnectionState::Gathering);
}

WebRTCService::~WebRTCService() {
  if (pc_) {
    pc_->close();
  }
}

void WebRTCService::setup_peer_connection() {
  rtc::Configuration config{};
  pc_ = std::make_shared<rtc::PeerConnection>(config);

  pc_->onStateChange([&](rtc::PeerConnection::State state) {
    if (state == rtc::PeerConnection::State::Connected) {
      update_status(ConnectionState::Connected);
    } else if (state == rtc::PeerConnection::State::Disconnected ||
               state == rtc::PeerConnection::State::Closed) {
      update_status(ConnectionState::Disconnected);
    } else if (state == rtc::PeerConnection::State::Failed) {
      update_status(ConnectionState::Failed);
    }
  });

  pc_->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
    if (state == rtc::PeerConnection::GatheringState::Complete) {
      auto description = pc_->localDescription();
      json offer = {{"type", description->typeString()},
                    {"sdp", std::string(*description)}};
      std::println("Creating offer...");
      auto res = cli_.Post("/create", offer.dump(), "application/json");
      if (res) {
        if (res->status == 200) {
          json create_res = json::parse(res->body);
          session_id_ = create_res["id"];
          std::println("URL: https://microblock.cc/audivis.html?id={}",
                       session_id_);
          update_status(
              ConnectionState::WaitingConnection); // Still waiting for
                                                   // connection, but session_id
                                                   // is available

          json wait_req = {{"id", session_id_}};
          auto wait_res =
              cli_.Post("/wait", wait_req.dump(), "application/json");
          if (wait_res) {
            if (wait_res->status == 200) {
              json answer = json::parse(wait_res->body);
              std::string sdp = answer["sdp"];
              std::string type = answer["type"];
              std::println("Received answer with type: {}", type, sdp);
              try {
                rtc::Description remote_description(sdp, type);
                pc_->setRemoteDescription(remote_description);
              } catch (const std::exception &e) {
                std::cerr << "Error parsing answer: " << e.what() << std::endl;
                update_status(ConnectionState::Failed);
              }
            } else {
              std::println("Failed to wait for answer: {}, {}",
                           wait_res->status, wait_res->body);
              update_status(ConnectionState::Failed);
            }
          } else {
            auto err = wait_res.error();
            std::println("Failed to wait for answer: {}",
                         httplib::to_string(err));
            update_status(ConnectionState::Failed);
          }
        } else {
          std::println("Failed to create session: {}, {}", res->status,
                       res->body);
          update_status(ConnectionState::Failed);
        }
      } else {
        auto err = res.error();
        std::println("Failed to create session: {}", httplib::to_string(err));
        update_status(ConnectionState::Failed);
      }
    }
  });
}

void WebRTCService::setup_data_channel() {
  dc_ = pc_->createDataChannel("audio");

  dc_->onOpen([&]() { std::println("DataChannel opened."); });

  dc_->onMessage([&](rtc::message_variant message) {
    if (auto binary = std::get_if<rtc::binary>(&message)) {
      std::vector<uint8_t> data;
      data.reserve(binary->size());
      for (const auto &byte : *binary) {
        data.push_back(static_cast<uint8_t>(byte));
      }
      audioBuffer_.insert(audioBuffer_.end(), data.begin(), data.end());

      std::println("Received audio data, buffer size: {}", audioBuffer_.size());
      while (audioBuffer_.size() >= 960) {
        std::vector<uint8_t> buffer(audioBuffer_.begin(),
                                    audioBuffer_.begin() + 960);
        if (audio_data_callback_ && audio_data_callback_(buffer)) {
          audioBuffer_.erase(audioBuffer_.begin(), audioBuffer_.begin() + 960);
        }
      }
      std::println("Processed audio data, remaining buffer size: {}",
                   audioBuffer_.size());
    }
  });
}

void WebRTCService::start_signaling() { pc_->setLocalDescription(); }

void WebRTCService::update_status(ConnectionState state) {
  if (status_callback_) {
    status_callback_({state, session_id_});
  }
}

} // namespace client