#include "parsec-vusb-api.h"
#include <deque>
#include <filesystem>
#include <fstream>
#include <print>
#include <queue>
#include <thread>
#include <uwebsockets/App.h>
#include <vector>

#include <rtc/rtc.hpp>

// Structure to hold user data for WebSocket connections
struct PerSocketData {
  // We can add user-specific data here later, e.g., user ID
};

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
  // Initialize the virtual USB hub and device
  parsec::vusb::VirtualUSBHub hub;
  auto device =
      hub.device_exists(1)
          ? hub.open_device(1)
          : hub.create_device(parsec::vusb::DefaultMicrophoneDescriptor);

  if (!device) {
    std::print("Failed to open or create virtual microphone device.\n");
    return 1;
  }

  device->configure_endpoints({0x81});
  device->configure_endpoint_types({0x02});
  device->plug_in();
  std::println("Virtual microphone plugged in with DeviceID: {}",
               device->device_id());

  // --- uWebSockets Server Implementation ---

  // Path to the HTML file
  const std::filesystem::path html_path = "src/client/index.html";
  const std::string html_content = readFile(html_path);

  if (html_content.empty()) {
    std::println(stderr, "Error: Could not read {}", html_path.string());
    return 1;
  }

  std::mutex audio_queue_mutex;
  std::deque<uint8_t> audio_queue;

  std::thread audio_thread([&]() {
    static constexpr auto SLICE_SIZE = 960;
    std::vector<uint8_t> audio_data(SLICE_SIZE);
    while (true) {
      {
        std::lock_guard<std::mutex> lock(audio_queue_mutex);
        if (audio_queue.size() > SLICE_SIZE) {
          std::copy_n(audio_queue.begin(), SLICE_SIZE, audio_data.begin());
        } else {
          continue;
        }
      }

      if (device->submit_audio_data(audio_data)) {
        std::lock_guard<std::mutex> lock(audio_queue_mutex);
        audio_queue.erase(audio_queue.begin(), audio_queue.begin() + SLICE_SIZE);
      }

      std::this_thread::yield();
    }
  });

  uWS::App()
      .ws<PerSocketData>(
          "/",
          {/* Settings */
           .compression = uWS::SHARED_COMPRESSOR,
           .maxPayloadLength = 16 * 1024 * 1024,
           .idleTimeout = 60,
           /* Handlers */
           .upgrade = nullptr,
           .open =
               [](auto *ws) {
                 std::println("WebSocket connection established.");
               },
           .message =
               [&](auto *ws, std::string_view message, uWS::OpCode opCode) {
                 if (opCode == uWS::OpCode::BINARY) {
                   std::lock_guard<std::mutex> lock(audio_queue_mutex);
                   audio_queue.append_range(std::span<const uint8_t>(
                       (uint8_t *)message.data(), message.size()));
                 }
               },
           .close =
               [](auto *ws, int code, std::string_view message) {
                 std::println("WebSocket connection closed.");
               }})
      .get("/*",
           [&html_content](auto *res, auto *req) { res->end(html_content); })
      .listen("0.0.0.0", 9001,
              [](auto *listen_socket) {
                if (listen_socket) {
                  std::println(
                      "HTTP and WebSocket server listening on port 9001");
                } else {
                  std::println(stderr, "Failed to listen on port 9001");
                }
              })
      .run();
}