#include "parsec-vusb-api.h"
#include <print>
#include <string>
#include <system_error>

#include <setupapi.h>
#include <windows.h>

namespace parsec::vusb {

// --- Constants ---
constexpr DWORD IOCTL_VIRTUAL_USB_DEVICE_CREATE = 0x2ae804;
constexpr DWORD IOCTL_CONFIGURE_ENDPOINTS = 0x2aa808;
constexpr DWORD IOCTL_CONFIGURE_ENDPOINT_TYPES = 0x2aa81c;
constexpr DWORD IOCTL_PLUG_IN_DEVICE = 0x2aac04;
constexpr DWORD IOCTL_QUERY_MEDIA = 0x2af014;
constexpr DWORD IOCTL_SUBMIT_AUDIO = 0x2ab018;

// --- Helper Functions ---

template <typename T>
void write_to_buffer(std::vector<uint8_t> &buffer, size_t offset,
                     const T &value) {
  if (offset + sizeof(T) > buffer.size()) {
    throw std::out_of_range("Write operation out of buffer bounds.");
  }
  std::memcpy(buffer.data() + offset, &value, sizeof(T));
}

static void vusb_ioctl(HANDLE handle, DWORD code,
                       std::vector<uint8_t> &buffer) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    throw VUSBError("Invalid handle.", 0);
  }

  DWORD bytes_returned = 0;
  OVERLAPPED overlapped{};
  overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!overlapped.hEvent) {
    throw VUSBError("Failed to create event for DeviceIoControl.",
                    GetLastError());
  }

  if (!DeviceIoControl(handle, code, buffer.data(),
                       static_cast<DWORD>(buffer.size()), nullptr, 0,
                       &bytes_returned, &overlapped)) {
    if (GetLastError() != ERROR_IO_PENDING) {
      CloseHandle(overlapped.hEvent);
      throw VUSBError("DeviceIoControl failed.", GetLastError());
    }
  }

  if (!GetOverlappedResult(handle, &overlapped, &bytes_returned, TRUE)) {
    CloseHandle(overlapped.hEvent);
    throw VUSBError("GetOverlappedResult failed.", GetLastError());
  }

  CloseHandle(overlapped.hEvent);
}

static void vusb_ioctl_in_out(HANDLE handle, DWORD code,
                              std::vector<uint8_t> &buffer) {
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    throw VUSBError("Invalid handle.", 0);
  }

  DWORD bytes_returned = 0;
  OVERLAPPED overlapped{};
  overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!overlapped.hEvent) {
    throw VUSBError("Failed to create event for DeviceIoControl.",
                    GetLastError());
  }

  if (!DeviceIoControl(handle, code, buffer.data(),
                       static_cast<DWORD>(buffer.size()), buffer.data(),
                       static_cast<DWORD>(buffer.size()), &bytes_returned,
                       &overlapped)) {
    if (GetLastError() != ERROR_IO_PENDING) {
      CloseHandle(overlapped.hEvent);
      throw VUSBError("DeviceIoControl failed.", GetLastError());
    }
  }

  if (!GetOverlappedResult(handle, &overlapped, &bytes_returned, TRUE)) {
    DWORD error = GetLastError();
    CloseHandle(overlapped.hEvent);
    throw VUSBError("GetOverlappedResult failed.", error);
  }

  CloseHandle(overlapped.hEvent);
}

// --- VUSBError ---

VUSBError::VUSBError(const std::string &message, DWORD error_code)
    : std::runtime_error(message +
                         " (Error code: " + std::to_string(error_code) + ")"),
      _error_code(error_code) {}

DWORD VUSBError::error_code() const { return _error_code; }

// --- VirtualUSBHub ---

static HANDLE OpenDeviceHandle(const GUID *interfaceGuid) {
  HANDLE handle = INVALID_HANDLE_VALUE;
  HDEVINFO devInfo = SetupDiGetClassDevsA(
      interfaceGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

  if (devInfo != INVALID_HANDLE_VALUE) {
    SP_DEVICE_INTERFACE_DATA devInterface;
    ZeroMemory(&devInterface, sizeof(SP_DEVICE_INTERFACE_DATA));
    devInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (DWORD i = 0; SetupDiEnumDeviceInterfaces(devInfo, NULL, interfaceGuid,
                                                  i, &devInterface);
         ++i) {
      DWORD detailSize = 0;
      SetupDiGetDeviceInterfaceDetailA(devInfo, &devInterface, NULL, 0,
                                       &detailSize, NULL);

      SP_DEVICE_INTERFACE_DETAIL_DATA_A *detail =
          (SP_DEVICE_INTERFACE_DETAIL_DATA_A *)calloc(1, detailSize);
      detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);

      if (SetupDiGetDeviceInterfaceDetailA(devInfo, &devInterface, detail,
                                           detailSize, &detailSize, NULL)) {
        handle =
            CreateFileA(detail->DevicePath, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING |
                            FILE_FLAG_OVERLAPPED | FILE_FLAG_WRITE_THROUGH,
                        NULL);

        if (handle != NULL && handle != INVALID_HANDLE_VALUE)
          break;
      }

      free(detail);
    }

    SetupDiDestroyDeviceInfoList(devInfo);
  }

  return handle;
}

static const GUID VUSB_ADAPTER_GUID = {
    0x25efc209,
    0x91fe,
    0x4460,
    {0xa4, 0xb7, 0x6a, 0x9e, 0x31, 0xc0, 0xd0, 0xf1}};

VirtualUSBHub::VirtualUSBHub() {
  _handle = OpenDeviceHandle(&VUSB_ADAPTER_GUID);
  if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) {
    throw VUSBError("Failed to open virtual USB hub.", GetLastError());
  }
}

VirtualUSBHub::~VirtualUSBHub() { close(); }

VirtualUSBHub::VirtualUSBHub(VirtualUSBHub &&other) noexcept
    : _handle(other._handle) {
  other._handle = INVALID_HANDLE_VALUE;
}

VirtualUSBHub &VirtualUSBHub::operator=(VirtualUSBHub &&other) noexcept {
  if (this != &other) {
    close();
    _handle = other._handle;
    other._handle = INVALID_HANDLE_VALUE;
  }
  return *this;
}

void VirtualUSBHub::close() {
  if (_handle != INVALID_HANDLE_VALUE) {
    CloseHandle(_handle);
    _handle = INVALID_HANDLE_VALUE;
  }
}

std::unique_ptr<VirtualUSBDevice>
VirtualUSBHub::create_device(const std::vector<uint8_t> &device_descriptor) {
  return std::unique_ptr<VirtualUSBDevice>(
      new VirtualUSBDevice(_handle, device_descriptor));
}

// --- VirtualUSBDevice ---

VirtualUSBDevice::VirtualUSBDevice(
    HANDLE hub_handle, const std::vector<uint8_t> &device_descriptor)
    : _hub_handle(hub_handle) {
  if (_hub_handle == INVALID_HANDLE_VALUE) {
    throw VUSBError("Hub handle is invalid.");
  }

  std::vector<uint8_t> buffer = device_descriptor;
  vusb_ioctl_in_out(_hub_handle, IOCTL_VIRTUAL_USB_DEVICE_CREATE, buffer);

  _device_id = *reinterpret_cast<uint32_t *>(buffer.data() + 4);

  wchar_t event_name[MAX_PATH];
  swprintf_s(event_name, MAX_PATH,
             L"Global\\PcvudhcControlEndpointRequestAvailableNotification%04X",
             _device_id);
  _control_wait_event = CreateEventW(nullptr, FALSE, FALSE, event_name);
  if (!_control_wait_event) {
    throw VUSBError("Failed to create control wait event.", GetLastError());
  }

  swprintf_s(event_name, MAX_PATH,
             L"Global\\PcvudhcInEndpointRequestAvailableNotification%04X",
             _device_id);
  _cancelled_wait_event = CreateEventW(nullptr, FALSE, FALSE, event_name);
  if (!_cancelled_wait_event) {
    CloseHandle(_control_wait_event);
    throw VUSBError("Failed to create cancelled wait event.", GetLastError());
  }
}

VirtualUSBDevice::~VirtualUSBDevice() { close(); }

VirtualUSBDevice::VirtualUSBDevice(VirtualUSBDevice &&other) noexcept
    : _hub_handle(other._hub_handle), _device_id(other._device_id),
      _control_wait_event(other._control_wait_event),
      _cancelled_wait_event(other._cancelled_wait_event) {
  other._hub_handle = INVALID_HANDLE_VALUE;
  other._device_id = 0;
  other._control_wait_event = nullptr;
  other._cancelled_wait_event = nullptr;
}

VirtualUSBDevice &
VirtualUSBDevice::operator=(VirtualUSBDevice &&other) noexcept {
  if (this != &other) {
    close();
    _hub_handle = other._hub_handle;
    _device_id = other._device_id;
    _control_wait_event = other._control_wait_event;
    _cancelled_wait_event = other._cancelled_wait_event;

    other._hub_handle = INVALID_HANDLE_VALUE;
    other._device_id = 0;
    other._control_wait_event = nullptr;
    other._cancelled_wait_event = nullptr;
  }
  return *this;
}

void VirtualUSBDevice::close() {
  if (_control_wait_event) {
    CloseHandle(_control_wait_event);
    _control_wait_event = nullptr;
  }
  if (_cancelled_wait_event) {
    CloseHandle(_cancelled_wait_event);
    _cancelled_wait_event = nullptr;
  }
  // The hub handle is owned by the hub, so we don't close it here.
  _hub_handle = INVALID_HANDLE_VALUE;
}

void VirtualUSBDevice::configure_endpoints(
    const std::vector<uint8_t> &endpointIds) {
  if (endpointIds.empty()) {
    throw VUSBError("Endpoint ID list cannot be empty.");
  }

#pragma pack(push, 1)
  struct EndpointRequest {
    uint32_t TotalSize;
    uint32_t DeviceId;
    uint8_t EndpointCount;
    uint8_t EndpointIds;
  };
#pragma pack(pop)

  size_t endpointDataSize = endpointIds.size();
  size_t totalBufferSize =
      sizeof(EndpointRequest) - sizeof(uint8_t) + endpointDataSize;

  std::vector<uint8_t> buffer(totalBufferSize);
  auto *request = reinterpret_cast<EndpointRequest *>(buffer.data());

  request->TotalSize = static_cast<uint32_t>(totalBufferSize);
  request->DeviceId = _device_id;
  request->EndpointCount = static_cast<uint8_t>(endpointIds.size());
  memcpy(&request->EndpointIds, endpointIds.data(), endpointDataSize);

  vusb_ioctl(_hub_handle, IOCTL_CONFIGURE_ENDPOINTS, buffer);

  for (uint8_t epId : endpointIds) {
    wchar_t eventName[MAX_PATH];
    const wchar_t *format =
        (epId & 0x80)
            ? L"Global\\PcvudhcInEndpointRequestAvailableNotification%04X%02X"
            : L"Global\\PcvudhcOutEndpointRequestAvailableNotification%04X%02X";
    swprintf_s(eventName, MAX_PATH, format, _device_id, epId & 0xFF);
    HANDLE hEvent = CreateEventW(nullptr, FALSE, FALSE, eventName);
    if (!hEvent) {
      // Don't throw, just log, as the original code does.
      std::println(stderr, "Failed to create endpoint event: {}",
                   GetLastError());
    } else {
      // The original code doesn't store these handles, so we just close them.
      CloseHandle(hEvent);
    }
  }
}

void VirtualUSBDevice::configure_endpoint_types(
    const std::vector<int32_t> &endpointTypes) {
  if (endpointTypes.empty()) {
    throw VUSBError("Endpoint type list cannot be empty.");
  }

#pragma pack(push, 1)
  struct EndpointTypeRequest {
    uint32_t TotalSize;
    uint32_t DeviceId;
    uint8_t TypeCount;
    int32_t Types;
  };
#pragma pack(pop)

  size_t typeDataSize = endpointTypes.size() * sizeof(int32_t);
  size_t totalBufferSize =
      sizeof(EndpointTypeRequest) - sizeof(int32_t) + typeDataSize;

  std::vector<uint8_t> buffer(totalBufferSize);
  auto *request = reinterpret_cast<EndpointTypeRequest *>(buffer.data());

  request->TotalSize = static_cast<uint32_t>(totalBufferSize);
  request->DeviceId = _device_id;
  request->TypeCount = static_cast<uint8_t>(endpointTypes.size());
  memcpy(&request->Types, endpointTypes.data(), typeDataSize);

  vusb_ioctl(_hub_handle, IOCTL_CONFIGURE_ENDPOINT_TYPES, buffer);
}

void VirtualUSBDevice::plug_in() {
#pragma pack(push, 1)
  struct PlugInRequest {
    int32_t TotalSize;
    int32_t DeviceId;
    char PlugIn;
    char Reserved1;
    int16_t Reserved2;
    int32_t Reserved3;
  };
#pragma pack(pop)

  PlugInRequest request{};
  request.TotalSize = sizeof(PlugInRequest);
  request.DeviceId = _device_id;
  request.PlugIn = 1; // Plug in
  request.Reserved3 = 3;

  std::vector<uint8_t> buffer(sizeof(request));
  memcpy(buffer.data(), &request, sizeof(request));

  vusb_ioctl(_hub_handle, IOCTL_PLUG_IN_DEVICE, buffer);
}

void VirtualUSBDevice::unplug() {
#pragma pack(push, 1)
  struct PlugInRequest {
    int32_t TotalSize;
    int32_t DeviceId;
    char PlugIn;
    char Reserved1;
    int16_t Reserved2;
    int32_t Reserved3;
  };
#pragma pack(pop)

  PlugInRequest request{};
  request.TotalSize = sizeof(PlugInRequest);
  request.DeviceId = _device_id;
  request.PlugIn = 0; // Unplug
  request.Reserved3 = 3;

  std::vector<uint8_t> buffer(sizeof(request));
  memcpy(buffer.data(), &request, sizeof(request));

  vusb_ioctl(_hub_handle, IOCTL_PLUG_IN_DEVICE, buffer);
}

bool VirtualUSBDevice::submit_audio_data(const std::vector<uint8_t> &data) {
  if (data.size() != 960) {
    throw VUSBError("Audio data must be exactly 960 bytes");
  }

  try {
    std::vector<uint8_t> queryBuffer(149, 0);
    write_to_buffer(queryBuffer, 0, static_cast<uint32_t>(149));
    write_to_buffer(queryBuffer, 4, _device_id);
    write_to_buffer(queryBuffer, 8, static_cast<uint8_t>(0x81)); // endpoint
    write_to_buffer(queryBuffer, 25, static_cast<uint32_t>(10));

    vusb_ioctl_in_out(_hub_handle, IOCTL_QUERY_MEDIA, queryBuffer);

    uint32_t field_9 = *reinterpret_cast<uint32_t *>(queryBuffer.data() + 9);
    uint32_t field_19 = *reinterpret_cast<uint32_t *>(queryBuffer.data() + 25);

    size_t bufferSize = 12 * field_19 + 1001;
    std::vector<uint8_t> audioBuffer(bufferSize, 0);

    write_to_buffer(audioBuffer, 0, static_cast<uint32_t>(bufferSize));
    write_to_buffer(audioBuffer, 4, _device_id);
    write_to_buffer(audioBuffer, 8, static_cast<uint8_t>(0x81));
    write_to_buffer(audioBuffer, 9, field_9);
    write_to_buffer(audioBuffer, 21, static_cast<uint64_t>(10000)); // timing
    write_to_buffer(audioBuffer, 33, field_19);
    write_to_buffer(audioBuffer, 37, static_cast<uint32_t>(960));

    if (field_19 >= 10) {
      uint32_t remaining = 960;
      for (uint32_t i = 0; i < 10; ++i) {
        size_t offset = 41 + i * 12;
        write_to_buffer(audioBuffer, offset, static_cast<uint32_t>(i * 96));

        if (i == 9 && remaining < 96) {
          write_to_buffer(audioBuffer, offset + 4, remaining);
          break;
        }
        write_to_buffer(audioBuffer, offset + 4, static_cast<uint32_t>(96));
        remaining -= 96;
      }

      size_t audioDataOffset = 41 + 12 * field_19;
      std::memcpy(audioBuffer.data() + audioDataOffset, data.data(), 960);
    } else {
      write_to_buffer(audioBuffer, 13, static_cast<uint32_t>(0xC0000001));
      write_to_buffer(audioBuffer, 17, static_cast<uint32_t>(0xC0000004));
    }

    // For submit audio, the output buffer is null
    DWORD bytes_returned = 0;
    OVERLAPPED overlapped{};
    overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    if (!overlapped.hEvent) {
      throw VUSBError("Failed to create event for submit audio.",
                      GetLastError());
    }

    if (!DeviceIoControl(_hub_handle, IOCTL_SUBMIT_AUDIO, audioBuffer.data(),
                         static_cast<DWORD>(bufferSize), nullptr, 0,
                         &bytes_returned, &overlapped)) {
      if (GetLastError() != ERROR_IO_PENDING) {
        CloseHandle(overlapped.hEvent);
        throw VUSBError("Submit audio DeviceIoControl failed.", GetLastError());
      }
    }

    if (!GetOverlappedResult(_hub_handle, &overlapped, &bytes_returned, TRUE)) {
      CloseHandle(overlapped.hEvent);
      throw VUSBError("Submit audio GetOverlappedResult failed.",
                      GetLastError());
    }
    CloseHandle(overlapped.hEvent);

  } catch (const VUSBError &e) {
    if (e.error_code() == ERROR_NO_MORE_ITEMS) {
      return false;
    }
    throw;
  }

  return true;
}

VirtualUSBDevice::VirtualUSBDevice(HANDLE hub_handle, uint32_t device_id) {
  _hub_handle = hub_handle;
  _device_id = device_id;
  _control_wait_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  _cancelled_wait_event = CreateEventW(nullptr, TRUE, FALSE, nullptr);
}
std::unique_ptr<VirtualUSBDevice> VirtualUSBHub::open_device(int device_id) {
  return std::unique_ptr<VirtualUSBDevice>(
      new VirtualUSBDevice(_handle, device_id));
}
bool VirtualUSBHub::device_exists(int device_id) {
  auto dev = open_device(device_id);
  try {
    dev->configure_endpoints({0x81});
  } catch (const std::exception &e) {
    return false;
  }
  return true;
}
} // namespace parsec::vusb