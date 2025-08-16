#include "breeze_ui/animator.h"
#include "breeze_ui/nanovg_wrapper.h"
#include "breeze_ui/widget.h"
#include "client_context.h"
#include <codecvt>
#include <iostream>
#include <print>
#include <thread>

#include "breeze_ui/ui.h"
#include "cpptrace/from_current.hpp"

#include "parsec-vusb-api.h"
#include "qrcodegen.hpp"
using namespace qrcodegen;

inline constexpr NVGcolor parse_color(std::string_view str) {
  // allowed:
  // #RRGGBB
  // #RRGGBBAA
  // #RGB
  // #RGBA
  // RRGGBB
  // RRGGBBAA
  // RGB
  // RGBA
  // r, g,b
  // r,g, b,a

  std::string s(str);
  if (s.empty())
    return nvgRGBA(0, 0, 0, 255);

  // Remove leading '#' if present
  if (s[0] == '#')
    s = s.substr(1);

  // Handle comma-separated values
  if (s.find(',') != std::string::npos) {
    std::vector<int> components;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, ',')) {
      components.push_back(std::stoi(item));
    }

    if (components.size() == 3) {
      return nvgRGB(components[0], components[1], components[2]);
    }
    if (components.size() == 4) {
      return nvgRGBA(components[0], components[1], components[2],
                     components[3]);
    }
  }

  // Handle hex values
  switch (s.length()) {
  case 3: // RGB
    return nvgRGB(17 * std::stoi(s.substr(0, 1), nullptr, 16),
                  17 * std::stoi(s.substr(1, 1), nullptr, 16),
                  17 * std::stoi(s.substr(2, 1), nullptr, 16));
  case 4: // RGBA
    return nvgRGBA(17 * std::stoi(s.substr(0, 1), nullptr, 16),
                   17 * std::stoi(s.substr(1, 1), nullptr, 16),
                   17 * std::stoi(s.substr(2, 1), nullptr, 16),
                   17 * std::stoi(s.substr(3, 1), nullptr, 16));
  case 6: // RRGGBB
    return nvgRGB(std::stoi(s.substr(0, 2), nullptr, 16),
                  std::stoi(s.substr(2, 2), nullptr, 16),
                  std::stoi(s.substr(4, 2), nullptr, 16));
  case 8: // RRGGBBAA
    return nvgRGBA(std::stoi(s.substr(0, 2), nullptr, 16),
                   std::stoi(s.substr(2, 2), nullptr, 16),
                   std::stoi(s.substr(4, 2), nullptr, 16),
                   std::stoi(s.substr(6, 2), nullptr, 16));
  }

  return nvgRGBA(0, 0, 0, 255); // Default black
}

static void copy_to_clipboard(const std::string &text) {
  OpenClipboard(nullptr);
  EmptyClipboard();
  HGLOBAL hGlob = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
  memcpy(GlobalLock(hGlob), text.c_str(), text.size() + 1);
  GlobalUnlock(hGlob);
  SetClipboardData(CF_TEXT, hGlob);
  CloseClipboard();
}

struct page_error_widget;
struct page_connected_widget;
struct page_connect_widget;
struct page_gathering_widget;

struct audivis_widget : public ui::widget_flex {
  audivis_widget() {
    auto_size = false;
    justify_content = ui::widget_flex::justify::center;
    align_items = ui::widget_flex::align::center;
  }

  void render(ui::nanovg_context vg) override {
    vg.fillColor(parse_color("#1C1B1F"));
    vg.fillRect(0, 0, *width, *height);
    ui::widget_flex::render(vg);
  }

  void switch_to_connect_page(std::string id) {
    children.clear();
    children_dirty = true;
    emplace_child<page_connect_widget>(std::move(id));
  }

  void switch_to_gathering_page() {
    children.clear();
    children_dirty = true;
    emplace_child<page_gathering_widget>();
  }

  void switch_to_connected_page() {
    children.clear();
    children_dirty = true;
    emplace_child<page_connected_widget>();
  }

  void switch_to_error_page(std::string reason) {
    children.clear();
    children_dirty = true;
    emplace_child<page_error_widget>(std::move(reason));
  }
};

struct page_gathering_widget : public ui::widget_flex {
  page_gathering_widget() {
    align_items = ui::widget_flex::align::center;
    justify_content = ui::widget_flex::justify::center;
    gap = 20;

    struct loading_bar_widget : public ui::widget {
      ui::sp_anim_float progress = anim_float();

      loading_bar_widget() {
        width->reset_to(200);
        height->reset_to(6);

        progress->set_duration(3000);
        progress->set_easing(ui::easing_type::ease_in_out);
        progress->after_animate = [progress = this->progress](float) {
          progress->reset_to(0);
          progress->animate_to(1.0f);
        };
        progress->animate_to(1.0f);
      }

      void render(ui::nanovg_context vg) override {
        vg.fillColor(parse_color("#49454F"));
        vg.fillRoundedRect(*x, *y, *width, *height, *height / 2);

        float indicator_width =
            *width * 0.3f * (-4 * (*progress * *progress) + 4 * *progress);
        float travel_distance = *width - indicator_width;
        float indicator_x =
            *x + travel_distance * *progress - indicator_width / 2;

        NVGpaint gradient = vg.linearGradient(
            indicator_x, *y, indicator_x + indicator_width, *y,
            nvgRGBA(103, 80, 164, 0), nvgRGBA(103, 80, 164, 255));

        vg.fillPaint(gradient);
        vg.fillRoundedRect(indicator_x, *y, indicator_width, *height,
                           *height / 2);
      }

      void update(ui::update_context &ctx) override {
        needs_repaint = true; // Always repaint for animation
        ui::widget::update(ctx);
      }
    };

    emplace_child<loading_bar_widget>();

    auto txt_loading = emplace_child<ui::text_widget>();
    txt_loading->text = "加载中...";
    txt_loading->font_size = 14;
    txt_loading->color.animate_to(1, 1, 1, 1);
  }
};

struct page_connect_widget : public ui::widget_flex {
  std::string id;
  page_connect_widget(std::string idx) : id(std::move(idx)) {
    align_items = ui::widget_flex::align::center;
    justify_content = ui::widget_flex::justify::center;
    gap = 10;

    struct qrcode_widget : public ui::widget {
      qrcodegen::QrCode qr;

      qrcode_widget(std::string &id)
          : qr(qrcodegen::QrCode::encodeText(
                ("https://microblock.cc/audivis.html?id=" + id).c_str(),
                qrcodegen::QrCode::Ecc::HIGH)) {
        width->reset_to(200);
        height->reset_to(200);
      }

      void render(ui::nanovg_context vg) override {
        auto s = vg.transaction();
        vg.fillColor(parse_color("#49454F"));
        vg.fillRoundedRect(*x, *y, *width, *height, 16);

        constexpr auto padding = 6;

        vg.fillColor(parse_color("#1C1B1F"));
        vg.fillRoundedRect(*x + padding, *y + padding, *width - padding * 2,
                           *height - padding * 2, 10);

        auto size = qr.getSize();
        auto qrcodeSize = 200 - padding * 2 - 10;
        auto scale = qrcodeSize / static_cast<float>(size);
        for (int dy = 0; dy < size; dy++) {
          for (int dx = 0; dx < size; dx++) {
            if (qr.getModule(dx, dy)) {
              vg.fillColor(parse_color("#D0BCFF"));
              vg.fillRect(5 + dx * scale + padding + *x,
                          5 + dy * scale + padding + *y, scale, scale);
            }
          }
        }

        vg.strokeColor(parse_color("#938F99"));
        vg.strokeWidth(1);
        vg.strokeRoundedRect(*x + padding, *y + padding, *width - padding * 2,
                             *height - padding * 2, 10);
      }
    };

    emplace_child<qrcode_widget>(id);

    auto spacer = emplace_child<ui::widget>();
    spacer->height->reset_to(15);

    auto txt_conn = emplace_child<ui::text_widget>();
    txt_conn->text = "扫描二维码或点击复制下方链接进行连接";
    txt_conn->font_size = 14;
    txt_conn->color.animate_to(1, 1, 1, 1);

    struct link_widget : public ui::widget {
      std::string &id;
      link_widget(std::string &id) : id(id) {
        height->reset_to(30);
        width->reset_to(250);

        hover_prg->set_duration(100);
        hover_prg->set_easing(ui::easing_type::ease_in_out);
      }

      bool pasted = false;
      ui::sp_anim_float hover_prg = anim_float();
      void render(ui::nanovg_context vg) override {
        auto scope = vg.transaction();
        // Background
        vg.fillColor(parse_color("#49454F"));
        vg.fillRoundedRect(*x, *y, *width, *height, 15);

        // Text
        vg.globalAlpha(1 - *hover_prg);
        vg.fontSize(12);
        vg.fontFace("mono");
        vg.fillColor(parse_color("#FFFFFF"));
        vg.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        vg.text(*x + *width / 2, *y + *height / 2, id.c_str(), nullptr);

        vg.globalAlpha(*hover_prg);
        vg.fontSize(12);
        vg.fontFace("main");
        vg.fillColor(parse_color("#FFFFFF"));
        vg.text(*x + *width / 2, *y + *height / 2, pasted ? "已复制" : "复制",
                nullptr);
      }

      void update(ui::update_context &ctx) override {
        hover_prg->animate_to(ctx.hovered(this));
        if (*hover_prg == 0 && pasted) {
          pasted = false;
          needs_repaint = true;
        }

        if (ctx.mouse_clicked_on(this)) {
          pasted = true;
          needs_repaint = true;
          copy_to_clipboard(
              std::format("https://microblock.cc/audivis.html?id={}", id));
        }

        ui::widget::update(ctx);
      }
    };

    emplace_child<link_widget>(id);
  }
};

struct page_connected_widget : public ui::widget_flex {
  page_connected_widget() {
    align_items = ui::widget_flex::align::center;
    justify_content = ui::widget_flex::justify::center;
    gap = 20;

    struct status_header : public ui::widget_flex {
      status_header() {
        align_items = ui::widget_flex::align::center;
        gap = 8;

        auto txt = emplace_child<ui::text_widget>();
        txt->text = "● 已连接";
        txt->font_size = 14;
        txt->color.animate_to(0.7f, 0.6f, 0.9f, 1);
      }
    };

    emplace_child<status_header>();

    struct spectrum_widget : public ui::widget {
      std::vector<ui::sp_anim_float> bars;

      spectrum_widget() {
        width->reset_to(200);
        height->reset_to(80);

        for (int i = 0; i < 9; i++) {
          bars.push_back(anim_float());
          bars[i]->set_duration(800 + i * 100);
          bars[i]->set_easing(ui::easing_type::ease_in_out);
          bars[i]->after_animate = [bar = bars[i]](float) {
            bar->animate_to(0.2f + (rand() % 80) / 100.0f);
          };
          bars[i]->animate_to(0.2f + (rand() % 80) / 100.0f);
        }
      }

      void render(ui::nanovg_context vg) override {
        vg.fillColor(parse_color("#49454F"));
        vg.fillRoundedRect(*x, *y, *width, *height, 12);

        float bar_width = 8;
        float gap = 4;
        float total_width = 9 * bar_width + 8 * gap;
        float start_x = *x + (*width - total_width) / 2;

        for (int i = 0; i < 9; i++) {
          float bar_height = *height * 0.7f * *bars[i];
          float bar_x = start_x + i * (bar_width + gap);
          float bar_y = *y + *height - 15 - bar_height;

          vg.fillColor(parse_color("#D0BCFF"));
          vg.fillRoundedRect(bar_x, bar_y, bar_width, bar_height, 2);
        }
      }

      void update(ui::update_context &ctx) override {
        needs_repaint = true;
        ui::widget::update(ctx);
      }
    };

    emplace_child<spectrum_widget>();

    struct stats_widget : public ui::widget_flex {
      stats_widget() {
        gap = 30;
        horizontal = true;

        auto create_stat = [this](const std::string &value,
                                  const std::string &label) {
          auto stat = emplace_child<ui::widget_flex>();
          stat->align_items = ui::widget_flex::align::center;
          stat->gap = 2;

          auto val_txt = stat->emplace_child<ui::text_widget>();
          val_txt->text = value;
          val_txt->font_size = 18;
          val_txt->color.animate_to(1, 1, 1, 1);

          auto lbl_txt = stat->emplace_child<ui::text_widget>();
          lbl_txt->text = label;
          lbl_txt->font_size = 12;
          lbl_txt->color.animate_to(0.7f, 0.7f, 0.7f, 1);

          return val_txt;
        };

        rtt_text = create_stat("-ms", "延迟");

        create_stat("48kHz", "采样率");
        create_stat("16-bit", "位深");
      }

      std::shared_ptr<ui::text_widget> rtt_text;

      size_t last_update_rtt = 0;
      void update(ui::update_context &ctx) override {
        auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        if (last_update_rtt + 1000000000 < now) {
          last_update_rtt = now;
          rtt_text->text =
              std::format("{}ms", client::ClientContext::get_instance()
                                      .webrtc_service->pc_->rtt()
                                      ->count());
        }

        ui::widget_flex::update(ctx);
      }
    };

    emplace_child<stats_widget>();

    struct disconnect_button : public ui::widget {
      ui::sp_anim_float hover_prg = anim_float();

      disconnect_button() {
        width->reset_to(100);
        height->reset_to(36);

        hover_prg->set_duration(150);
        hover_prg->set_easing(ui::easing_type::ease_in_out);
      }

      void render(ui::nanovg_context vg) override {
        NVGcolor bg_color = nvgLerpRGBA(parse_color("#6750A4"),
                                        parse_color("#7C4DFF"), *hover_prg);
        vg.fillColor(bg_color);
        vg.fillRoundedRect(*x, *y, *width, *height, 18);

        vg.fontSize(14);
        vg.fontFace("main");
        vg.fillColor(parse_color("#FFFFFF"));
        vg.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        vg.text(*x + *width / 2, *y + *height / 2, "断开连接", nullptr);
      }

      void update(ui::update_context &ctx) override {
        hover_prg->animate_to(ctx.hovered(this) ? 1.0f : 0.0f);

        ui::widget::update(ctx);
        if (ctx.mouse_clicked_on(this)) {
          owner_rt->post_loop_thread_task([]() {
            client::ClientContext::get_instance()
                .root_widget->switch_to_gathering_page();
            client::ClientContext::get_instance().init_webrtc_service();
          });
        }
      }
    };

    emplace_child<disconnect_button>();
  }
};

struct page_error_widget : public ui::widget_flex {
  std::string error_message;

  page_error_widget(std::string error_message)
      : error_message(std::move(error_message)) {
    align_items = ui::widget_flex::align::center;
    justify_content = ui::widget_flex::justify::center;
    gap = 25;

    struct error_icon : public ui::widget {
      error_icon() {
        width->reset_to(80);
        height->reset_to(80);
      }

      void render(ui::nanovg_context vg) override {
        float center_x = *x + *width / 2;
        float center_y = *y + *height / 2;
        float radius = 30;

        vg.strokeColor(parse_color("#6750A4"));
        vg.strokeWidth(4);
        vg.strokeCircle(center_x, center_y, radius);

        float cross_size = 15;
        vg.beginPath();
        vg.moveTo(center_x - cross_size, center_y - cross_size);
        vg.lineTo(center_x + cross_size, center_y + cross_size);
        vg.moveTo(center_x + cross_size, center_y - cross_size);
        vg.lineTo(center_x - cross_size, center_y + cross_size);
        vg.stroke();
      }
    };

    emplace_child<error_icon>();

    auto error_txt = emplace_child<ui::text_widget>();
    error_txt->text = error_message;
    error_txt->font_size = 16;
    error_txt->color.animate_to(1, 1, 1, 1);

    struct retry_button : public ui::widget {
      ui::sp_anim_float hover_prg = anim_float();
      ui::sp_anim_float active_prg = anim_float();

      retry_button() {
        width->reset_to(120);
        height->reset_to(40);

        hover_prg->set_duration(150);
        hover_prg->set_easing(ui::easing_type::ease_in_out);
        active_prg->set_duration(100);
        active_prg->set_easing(ui::easing_type::ease_in_out);
      }

      void render(ui::nanovg_context vg) override {
        auto scope = vg.transaction();

        NVGcolor bg_color = nvgLerpRGBA(parse_color("#6750A4"),
                                        parse_color("#7C4DFF"), *hover_prg);
        vg.fillColor(bg_color);
        vg.fillRoundedRect(*x, *y, *width, *height, 20);

        vg.fontSize(14);
        vg.fontFace("main");
        vg.fillColor(parse_color("#FFFFFF"));
        vg.textAlign(NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        vg.text(*x + *width / 2, *y + *height / 2, "重新连接", nullptr);
      }

      void update(ui::update_context &ctx) override {
        bool hovered = ctx.hovered(this);
        bool pressed = ctx.mouse_down_on(this);

        hover_prg->animate_to(hovered ? 1.0f : 0.0f);
        active_prg->animate_to(pressed ? 1.0f : 0.0f);

        if (ctx.mouse_clicked_on(this)) {
          owner_rt->post_loop_thread_task(
              []() {
                client::ClientContext::get_instance().webrtc_service = nullptr;
                client::ClientContext::get_instance()
                    .root_widget->switch_to_gathering_page();
                Sleep(50);
                client::ClientContext::get_instance().init_webrtc_service();
              },
              true);
        }

        ui::widget::update(ctx);
      }
    };

    emplace_child<retry_button>();
  }
};

std::wstring utf8_to_wstring(std::string const &str) {
  std::wstring_convert<
      std::conditional_t<sizeof(wchar_t) == 4, std::codecvt_utf8<wchar_t>,
                         std::codecvt_utf8_utf16<wchar_t>>>
      converter;
  return converter.from_bytes(str);
}
std::string wstring_to_utf8(std::wstring const &str) {
  std::wstring_convert<
      std::conditional_t<sizeof(wchar_t) == 4, std::codecvt_utf8<wchar_t>,
                         std::codecvt_utf8_utf16<wchar_t>>>
      converter;
  return converter.to_bytes(str);
}

std::optional<std::string> env(const std::string &name) {
  wchar_t buffer[32767];
  GetEnvironmentVariableW(utf8_to_wstring(name).c_str(), buffer, 32767);
  if (buffer[0] == 0) {
    return std::nullopt;
  }
  return wstring_to_utf8(buffer);
}

int main() {
  if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);
  }

  auto &inst = client::ClientContext::get_instance();
  CPPTRACE_TRY {
    std::println("Initializing client context...");

    ui::render_target rt;
    if (auto res = rt.init_global(); !res) {
      throw std::runtime_error("Failed to initialize global render target");
    }
    rt.title = "Audivis Relay";
    if (auto res = rt.init(); !res) {
      throw std::runtime_error("Failed to initialize render target");
    }

    rt.resize(320 * rt.dpi_scale, 500 * rt.dpi_scale);

    auto font = std::filesystem::path(env("WINDIR").value()) / "Fonts";
    nvgCreateFont(rt.nvg, "main", (font / "msyh.ttc").string().c_str());
    nvgCreateFont(rt.nvg, "mono", (font / "consola.ttf").string().c_str());

    inst.root_widget = std::make_shared<audivis_widget>();
    rt.root = inst.root_widget;
    inst.root_widget->owner_rt = &rt;
    if (!parsec::vusb::VirtualUSBHub::is_driver_installed()) {
      inst.root_widget->switch_to_error_page("虚拟 USB 驱动未安装");
    } else {
      inst.init_webrtc_service();
      inst.webrtc_service->start_signaling();
    }
    rt.root->width->reset_to(320);
    rt.root->height->reset_to(500);
    rt.show();

    auto hWnd = (HWND)rt.hwnd();
    LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
    style &= ~(WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    SetWindowLongPtr(hWnd, GWL_EXSTYLE, style);
    SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

    rt.start_loop();
    TerminateProcess(GetCurrentProcess(), 0);
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

namespace client {

ClientContext &ClientContext::get_instance() {
  static ClientContext instance;
  return instance;
}

ClientContext::ClientContext() {
  virtual_usb_hub_service = std::make_shared<VirtualUSBHubService>();
}

void ClientContext::init_webrtc_service() {
  webrtc_service = std::make_shared<WebRTCService>(
      [&](const std::vector<uint8_t> &data) {
        if (virtual_usb_hub_service && virtual_usb_hub_service->device_) {
          virtual_usb_hub_service->device_->submit_audio_data(data);
        }
      },
      [&](const WebRTCService::WebRTCStatus &status) {
        root_widget->owner_rt->post_loop_thread_task([this, status]() {
          if (status.state == WebRTCService::ConnectionState::Gathering) {
            root_widget->switch_to_gathering_page();
          } else if (status.state ==
                     WebRTCService::ConnectionState::WaitingConnection) {
            root_widget->switch_to_connect_page(status.session_id);
          } else if (status.state ==
                     WebRTCService::ConnectionState::Connected) {
            root_widget->switch_to_connected_page();
          } else if (status.state == WebRTCService::ConnectionState::Failed) {
            root_widget->switch_to_error_page("WebRTC 连接失败");
          } else if (status.state ==
                     WebRTCService::ConnectionState::Disconnected) {
            root_widget->switch_to_error_page("连接已断开");
          }
        });
      });
}

} // namespace client