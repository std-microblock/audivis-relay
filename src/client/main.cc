#include "client_context.h"
#include <iostream>
#include <print>
#include <thread>

#include "cpptrace/from_current.hpp"
#include "breeze_ui/ui.h"

int main() {
  CPPTRACE_TRY {
    std::println("Initializing client context...");
    client::ClientContext& context = client::ClientContext::get_instance();

    std::println("Gathering, please wait...");
    context.get_webrtc_service()->start_signaling();

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