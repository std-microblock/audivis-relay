
#include "parsec-vusb-api.h"
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "cpptrace/from_current.hpp"
#include "httplib.h"
#include <nlohmann/json.hpp>
#include <rtc/rtc.hpp>

using json = nlohmann::json;

// Structure to hold user data for WebSocket connections

// Function to read a file into a string
std::string readFile(const std::filesystem::path &path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return {};
  }
  return {std::istreambuf_iterator<char>(file),
          std::istreambuf_iterator<char>()};
}

int main() {
  CPPTRACE_TRY {
    // Initialize the virtual USB hub and device
    parsec::vusb::VirtualUSBHub hub;

    int orig_device_id = 1;
    auto device =
        hub.device_exists(orig_device_id)
            ? hub.open_device(orig_device_id)
            : hub.create_device(parsec::vusb::DefaultMicrophoneDescriptor);

    if (!device) {
      std::print("Failed to open or create virtual microphone device.\n");
      throw std::runtime_error(
          "Failed to open or create virtual microphone device.");
    }

    device->configure_endpoints({0x81});
    device->configure_endpoint_types({0x02});
    device->plug_in();
    std::println("Virtual microphone plugged in with DeviceID: {}",
                 device->device_id());

    // --- WebRTC Client Implementation ---
    httplib::Client cli("http://audivis-signaling-server.microblock.cc");
    cli.set_connection_timeout(0, 300000); // 5 minutes
    std::shared_ptr<rtc::PeerConnection> pc;

    rtc::Configuration config{
        .iceServers = {{"urls", {"stun:stun.l.google.com:19302"}}}};
    pc = std::make_shared<rtc::PeerConnection>(config);

    pc->onStateChange([](rtc::PeerConnection::State state) {
      // std::cout << "PeerConnection state: " << state << std::endl;
      // if (state == rtc::PeerConnection::State::Closed) {
      //   std::cout << "PeerConnection is closed." << std::endl;
      // }
    });

    pc->onGatheringStateChange([&](rtc::PeerConnection::GatheringState state) {
      if (state == rtc::PeerConnection::GatheringState::Complete) {
        auto description = pc->localDescription();
        json offer = {{"type", description->typeString()},
                      {"sdp", std::string(*description)}};
        auto res = cli.Post("/create", offer.dump(), "application/json");
        if (res) {
          if (res->status == 200) {
            json create_res = json::parse(res->body);
            std::string session_id = create_res["id"];
            std::println("URL: https://microblock.cc/audivis.html?id={}",
                         session_id);

            json wait_req = {{"id", session_id}};
            auto wait_res =
                cli.Post("/wait", wait_req.dump(), "application/json");
            if (wait_res) {
              if (wait_res->status == 200) {
                json answer = json::parse(wait_res->body);
                std::string sdp = answer["sdp"];
                std::string type = answer["type"];
                std::println("Received answer with type: {}", type, sdp);
                try {
                  rtc::Description remote_description(sdp, type);
                  pc->setRemoteDescription(remote_description);
                } catch (const std::exception &e) {
                  std::cerr << "Error parsing answer: " << e.what()
                            << std::endl;
                }
              } else {
                std::println("Failed to wait for answer: {}, {}",
                             wait_res->status, wait_res->body);
              }
            } else {
              auto err = wait_res.error();
              std::println("Failed to wait for answer: {}",
                           httplib::to_string(err));
            }
          } else {
            std::println("Failed to create session: {}, {}", res->status,
                         res->body);
          }
        } else {
          auto err = res.error();
          std::println("Failed to create session: {}", httplib::to_string(err));
        }
      }
    });

    auto dc = pc->createDataChannel("audio");

    dc->onOpen([&]() { std::println("DataChannel opened."); });

    std::deque<uint8_t> audioBuffer;
    dc->onMessage([&](rtc::message_variant message) {
      if (auto binary = std::get_if<rtc::binary>(&message)) {
        std::vector<uint8_t> data;
        data.reserve(binary->size());
        for (const auto &byte : *binary) {
          data.push_back(static_cast<uint8_t>(byte));
        }
        audioBuffer.insert(audioBuffer.end(), data.begin(), data.end());

        while (audioBuffer.size() >= 960) {
          std::vector<uint8_t> buffer(audioBuffer.begin(),
                                      audioBuffer.begin() + 960);
          if (device->submit_audio_data(buffer))
            audioBuffer.erase(audioBuffer.begin(), audioBuffer.begin() + 960);
        }
      }
    });

    pc->setLocalDescription();

    // Keep the application running
    std::this_thread::sleep_for(std::chrono::hours(24));
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