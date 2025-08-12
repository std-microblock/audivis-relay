#include "parsec-vdd.h"
#include <iostream>
#include <stdio.h>
#include <windows.h>

#include "parsec-vdd.h"
#include <chrono>
#include <conio.h>
#include <fstream>
#include <print>
#include <stdio.h>
#include <thread>
#include <vector>

using namespace std::chrono_literals;
using namespace parsec_vdd;


template <typename T>
void write_to_buffer(std::vector<uint8_t> &buffer, size_t offset,
                     const T &value) {
  if (offset + sizeof(T) > buffer.size()) {
    throw std::out_of_range("Write operation out of buffer bounds.");
  }
  std::memcpy(buffer.data() + offset, &value, sizeof(T));
}


void write_wstring_to_buffer(std::vector<uint8_t> &buffer, size_t offset,
                             const std::wstring &str) {
  
  size_t len_bytes = (str.length() + 1) * sizeof(wchar_t);
  if (offset + len_bytes > buffer.size()) {
    throw std::out_of_range("WString write operation out of buffer bounds.");
  }
  std::memcpy(buffer.data() + offset, str.c_str(), len_bytes);
}




struct DeviceContext {
  HANDLE hDevice;  
  UINT32 DeviceId; 
  HANDLE hControlWaitEvent;   
                              
  HANDLE hCancelledWaitEvent; 
                              
  
  BYTE Padding[0x258 - (sizeof(HANDLE) * 3 + sizeof(UINT32))];
};


#define IOCTL_VIRTUAL_USB_DEVICE_CREATE 0x2AE804

DWORD VusbIoCtl(HANDLE vdd, int code, std::vector<uint8_t> &buffer) {
  if (vdd == NULL || vdd == INVALID_HANDLE_VALUE)
    return -1;

  DWORD bytesReturned;
  OVERLAPPED overlapped{};
  overlapped.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
  DeviceIoControl(vdd, code, buffer.data(), static_cast<DWORD>(buffer.size()),
                  nullptr, 0, &bytesReturned, &overlapped);
  auto res = GetOverlappedResult(vdd, &overlapped, &bytesReturned, TRUE);
  if (!res) {
    std::println("failed {:0x}: {}", code, GetLastError());
    CloseHandle(overlapped.hEvent);
    return -1;
  }

  CloseHandle(overlapped.hEvent);
  return bytesReturned;
}
/**
 * @brief Sends the virtual USB device creation request via DeviceIoControl.
 *
 * This function replicates the asynchronous DeviceIoControl call seen in the
 * VirtualUsbDeviceCreate function. It assumes the caller has already
 * constructed the necessary data buffer.
 *
 * @param hDevice A handle to the target USB device, obtained via CreateFile.
 * @param pIoBuffer A pointer to the data buffer to be used for both input and
 * output.
 * @param nIoBufferSize The size of the data buffer. The disassembly shows this
 *                      is also the first 4 bytes of the buffer itself.
 * @param ppDeviceContext A pointer to a pointer that will receive the newly
 * allocated DeviceContext structure on success. The caller is responsible for
 * freeing this memory with `delete`.
 * @return A Win32 error code. ERROR_SUCCESS (0) on success.
 */
DWORD SendVirtualUsbDeviceCreateRequest(HANDLE hDevice, PVOID pIoBuffer,
                                        DWORD nIoBufferSize,
                                        DeviceContext **ppDeviceContext) {
  if (!hDevice || hDevice == INVALID_HANDLE_VALUE || !pIoBuffer ||
      !ppDeviceContext) {
    return ERROR_INVALID_PARAMETER;
  }

  *ppDeviceContext = nullptr;
  DWORD bytesReturned = 0;
  OVERLAPPED overlapped = {0};
  DWORD lastError = ERROR_SUCCESS;

  
  overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
  if (!overlapped.hEvent) {
    return GetLastError();
  }

  
  
  BOOL bResult = DeviceIoControl(hDevice, IOCTL_VIRTUAL_USB_DEVICE_CREATE,
                                 pIoBuffer, nIoBufferSize, pIoBuffer,
                                 nIoBufferSize, &bytesReturned, &overlapped);

  if (!bResult) {
    lastError = GetLastError();
    if (lastError != ERROR_IO_PENDING) {
      std::cerr << "DeviceIoControl failed immediately with error: "
                << lastError << std::endl;
      CloseHandle(overlapped.hEvent);
      return lastError;
    }

    
    
    bResult = GetOverlappedResult(hDevice, &overlapped, &bytesReturned, TRUE);
    if (!bResult) {
      lastError = GetLastError();
      std::cerr << "GetOverlappedResult failed with error: " << lastError
                << std::endl;
      CloseHandle(overlapped.hEvent);
      return lastError;
    }
    lastError = ERROR_SUCCESS;
  }

  std::cout << "DeviceIoControl and GetOverlappedResult succeeded."
            << std::endl;

  
  
  std::unique_ptr<DeviceContext> context = std::make_unique<DeviceContext>();
  if (!context) {
    CloseHandle(overlapped.hEvent);
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  
  memset(context.get(), 0, sizeof(DeviceContext));

  
  context->hDevice = hDevice;

  
  context->DeviceId =
      *(reinterpret_cast<UINT32 *>(static_cast<BYTE *>(pIoBuffer) + 4));

  
  wchar_t eventName[MAX_PATH];

  
  swprintf_s(eventName, MAX_PATH,
             L"Global\\PcvudhcControlEndpointRequestAvailableNotification%04X",
             context->DeviceId);
  context->hControlWaitEvent = CreateEventW(nullptr, FALSE, FALSE, eventName);
  if (!context->hControlWaitEvent) {
    lastError = GetLastError();
    std::cerr << "Failed to create ControlWaitEvent: " << lastError
              << std::endl;
    CloseHandle(overlapped.hEvent);
    return lastError;
  }

  
  swprintf_s(eventName, MAX_PATH,
             L"Global\\PcvudhcInEndpointRequestAvailableNotification%04X",
             context->DeviceId);
  context->hCancelledWaitEvent = CreateEventW(nullptr, FALSE, FALSE, eventName);
  if (!context->hCancelledWaitEvent) {
    lastError = GetLastError();
    std::cerr << "Failed to create CancelledWaitEvent: " << lastError
              << std::endl;
    CloseHandle(overlapped.hEvent);
    CloseHandle(context->hControlWaitEvent);
    return lastError;
  }

  std::cout << "Successfully created device context with DeviceId: "
            << context->DeviceId << std::endl;

  
  CloseHandle(overlapped.hEvent);

  
  *ppDeviceContext = context.release();

  return ERROR_SUCCESS;
}

void printhex(const std::vector<uint8_t> v) {
  for (const auto &byte : v) {
    printf("%02X ", byte);
  }
  printf("\n");
}

/**
 * @brief 配置虚拟设备的端点。对应于 sub_1801877d0。
 * @param vdd 从 AddVirtualDevice 获取的设备句柄 (context->hDevice)。
 * @param deviceId 设备的唯一ID (context->DeviceId)。
 * @param endpointIds 一个包含端点ID的向量，例如 {0x81}。
 * @return 如果成功返回-1，否则返回Win32错误码。
 */
int ConfigureDeviceEndpoints(HANDLE vdd, uint32_t deviceId,
                             const std::vector<uint8_t> &endpointIds) {
  if (endpointIds.empty()) {
    std::cerr << "Endpoint ID list cannot be empty." << std::endl;
    return ERROR_INVALID_PARAMETER;
  }

  const DWORD IOCTL_CONFIGURE_ENDPOINTS = 0x2aa808;

#pragma pack(push, 1)
  
  struct EndpointRequest {
    uint32_t TotalSize;
    uint32_t DeviceId;
    uint8_t EndpointCount;
    uint8_t EndpointIds[0];
  };
#pragma pack(pop)

  
  size_t endpointDataSize = endpointIds.size();
  size_t totalBufferSize = sizeof(EndpointRequest) + endpointDataSize;

  std::vector<uint8_t> buffer(totalBufferSize);
  EndpointRequest *request = reinterpret_cast<EndpointRequest *>(buffer.data());

  
  request->TotalSize = static_cast<uint32_t>(totalBufferSize);
  request->DeviceId = deviceId;
  request->EndpointCount = static_cast<uint8_t>(endpointIds.size());
  memcpy(request->EndpointIds, endpointIds.data(), endpointDataSize);
  printhex(buffer);
  
  std::cout << "Configuring device endpoints..." << std::endl;

  VusbIoCtl(vdd, IOCTL_CONFIGURE_ENDPOINTS, buffer);
  std::cout << "Successfully ioctl\n";

  
  
  std::vector<int32_t> outEndpoints;
  std::vector<int32_t> inEndpoints;

  for (int32_t epId : endpointIds) {
    if (epId & 0x80) {
      inEndpoints.push_back(epId);
    } else {
      outEndpoints.push_back(epId);
    }
  }

  
  if (!outEndpoints.empty()) {
    for (size_t i = 0; i < outEndpoints.size(); ++i) {
      wchar_t eventName[MAX_PATH];
      swprintf_s(
          eventName, MAX_PATH,
          L"Global\\PcvudhcOutEndpointRequestAvailableNotification%04X%02X",
          deviceId, outEndpoints[i] & 0xFF);

      HANDLE hEvent = CreateEventW(nullptr, FALSE, FALSE, eventName);
      if (!hEvent) {
        std::cerr << "Failed to create OUT endpoint event: " << GetLastError()
                  << std::endl;
        return GetLastError();
      }

      if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::cout << "OUT endpoint event already exists: " << std::endl;
        SetLastError(NO_ERROR);
      }

      std::wcout << L"Created OUT endpoint event: " << eventName << std::endl;
      
      
    }
  }

  
  if (!inEndpoints.empty()) {
    for (size_t i = 0; i < inEndpoints.size(); ++i) {
      wchar_t eventName[MAX_PATH];
      swprintf_s(
          eventName, MAX_PATH,
          L"Global\\PcvudhcInEndpointRequestAvailableNotification%04X%02X",
          deviceId, inEndpoints[i] & 0xFF);

      HANDLE hEvent = CreateEventW(nullptr, FALSE, FALSE, eventName);
      if (!hEvent) {
        std::cerr << "Failed to create IN endpoint event: " << GetLastError()
                  << std::endl;
        return GetLastError();
      }

      if (GetLastError() == ERROR_ALREADY_EXISTS) {
        std::wcout << L"IN endpoint event already exists: " << eventName
                   << std::endl;
        SetLastError(NO_ERROR);
      }

      std::wcout << L"Created IN endpoint event: " << eventName << std::endl;
      
      
    }
  }

  std::cout << "ConfigureDeviceEndpoints completed successfully." << std::endl;
  return -1; 
}

/**
 * @brief 配置虚拟设备端点的类型。对应于 sub_180187630。
 * @param vdd 从 AddVirtualDevice 获取的设备句柄 (context->hDevice)。
 * @param deviceId 设备的唯一ID (context->DeviceId)。
 * @param endpointTypes 一个包含端点类型的向量，例如 {2} (Bulk)。
 * @return 如果成功返回-1，否则返回Win32错误码。
 */
int ConfigureEndpointTypes(HANDLE vdd, uint32_t deviceId,
                           const std::vector<int32_t> &endpointTypes) {
  if (endpointTypes.empty()) {
    std::cerr << "Endpoint type list cannot be empty." << std::endl;
    return ERROR_INVALID_PARAMETER;
  }

  
  const DWORD IOCTL_CONFIGURE_ENDPOINT_TYPES = 0x2aa81c;


#pragma pack(push, 1)
  struct EndpointTypeRequest {
    uint32_t TotalSize;
    uint32_t DeviceId;
    uint8_t TypeCount;
    int32_t Types[1]; 
  };
#pragma pack(pop)

  
  size_t typeDataSize = endpointTypes.size() * sizeof(int32_t);
  size_t totalBufferSize =
      sizeof(EndpointTypeRequest) - sizeof(int32_t) + typeDataSize;

  std::vector<uint8_t> buffer(totalBufferSize);
  EndpointTypeRequest *request =
      reinterpret_cast<EndpointTypeRequest *>(buffer.data());

  
  request->TotalSize = static_cast<uint32_t>(totalBufferSize);
  request->DeviceId = deviceId;
  request->TypeCount = static_cast<uint8_t>(endpointTypes.size());
  memcpy(request->Types, endpointTypes.data(), typeDataSize);

  
  std::cout << "Configuring endpoint types..." << std::endl;
  DWORD result = VusbIoCtl(vdd, IOCTL_CONFIGURE_ENDPOINT_TYPES, buffer);
  if (result == -1) {
    std::cerr << "Failed to configure endpoint types." << std::endl;
    return GetLastError();
  }
  std::cout << "Configuring endpoint types completed successfully."
            << std::endl;
  return -1; 
}

/**
 * @brief 
 * @param device 设备句柄
 * @param deviceId 设备ID
 * @param data 音频数据（PCM格式，960字节）
 */
bool SubmitAudioData(HANDLE device, int deviceId, const std::vector<uint8_t>& data) {
  const DWORD IOCTL_QUERY_MEDIA = 0x2AF014;
  const DWORD IOCTL_SUBMIT_AUDIO = 0x2AB018;

  // Check if data size is exactly 960 bytes (PCM audio frame)
  if (data.size() != 960) {
    throw std::runtime_error("Audio data must be exactly 960 bytes");
  }

  DWORD bytesReturned = 0;
  OVERLAPPED overlapped = {0};
  overlapped.hEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!overlapped.hEvent) {
    throw std::runtime_error("Failed to create event: " + std::to_string(GetLastError()));
  }

  try {
    // First query media to get endpoint info
    std::vector<uint8_t> queryBuffer(149, 0);
    write_to_buffer(queryBuffer, 0, static_cast<uint32_t>(149));
    write_to_buffer(queryBuffer, 4, static_cast<uint32_t>(deviceId));
    write_to_buffer(queryBuffer, 8, static_cast<uint8_t>(0x81)); // endpoint
    write_to_buffer(queryBuffer, 25, static_cast<uint32_t>(10));

    if (!DeviceIoControl(device, IOCTL_QUERY_MEDIA, queryBuffer.data(), 149,
              queryBuffer.data(), 149, &bytesReturned, &overlapped)) {
      if (GetLastError() != ERROR_IO_PENDING) {
        throw std::runtime_error("Query media DeviceIoControl failed: " + std::to_string(GetLastError()));
      }
    }

    if (!GetOverlappedResult(device, &overlapped, &bytesReturned, TRUE)) {
      DWORD error = GetLastError();
      if (error == ERROR_NO_MORE_ITEMS) {
        return false;
      }
      throw std::runtime_error("Query media GetOverlappedResult failed: " + std::to_string(error));
    }

    // Extract field_9 and field_19 from query response
    uint32_t field_9 = *reinterpret_cast<uint32_t*>(queryBuffer.data() + 9);
    uint32_t field_19 = *reinterpret_cast<uint32_t*>(queryBuffer.data() + 25);

    // Create audio data buffer
    size_t bufferSize = 12 * field_19 + 1001;
    std::vector<uint8_t> audioBuffer(bufferSize, 0);

    // Fill buffer header
    write_to_buffer(audioBuffer, 0, static_cast<uint32_t>(bufferSize));
    write_to_buffer(audioBuffer, 4, static_cast<uint32_t>(deviceId));
    write_to_buffer(audioBuffer, 8, static_cast<uint8_t>(0x81));
    write_to_buffer(audioBuffer, 9, field_9);
    write_to_buffer(audioBuffer, 21, static_cast<uint64_t>(10000)); // timing
    write_to_buffer(audioBuffer, 33, field_19);
    write_to_buffer(audioBuffer, 37, static_cast<uint32_t>(960));

    if (field_19 >= 10) {
      // Fill packet descriptors
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

      // Copy audio data
      size_t audioDataOffset = 41 + 12 * field_19;
      std::memcpy(audioBuffer.data() + audioDataOffset, data.data(), 960);
    } else {
      // Error case
      write_to_buffer(audioBuffer, 13, static_cast<uint32_t>(0xC0000001));
      write_to_buffer(audioBuffer, 17, static_cast<uint32_t>(0xC0000004));
    }

    // Submit audio data
    if (!DeviceIoControl(device, IOCTL_SUBMIT_AUDIO, audioBuffer.data(),
              static_cast<DWORD>(bufferSize), nullptr, 0,
              &bytesReturned, &overlapped)) {
      if (GetLastError() != ERROR_IO_PENDING) {
        throw std::runtime_error("Submit audio DeviceIoControl failed: " + std::to_string(GetLastError()));
      }
    }

    if (!GetOverlappedResult(device, &overlapped, &bytesReturned, TRUE)) {
      throw std::runtime_error("Submit audio GetOverlappedResult failed: " + std::to_string(GetLastError()));
    }

    CloseHandle(overlapped.hEvent);
  } catch (...) {
    CloseHandle(overlapped.hEvent);
    throw;
  }

  return true;
}
int main() {
  
  
  
  
  
  
  

  
  HANDLE vdd = OpenDeviceHandle(&VDD_ADAPTER_GUID);
  if (vdd == NULL || vdd == INVALID_HANDLE_VALUE) {
    printf("Failed to obtain the device handle.\n");
    return 1;
  }

  std::println("Device handle: {}", (void *)vdd);

  DeviceContext *context = nullptr;

  std::vector<uint8_t> q = {
      0x90, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x12, 0x01, 0x10, 0x01, 0x00, 0x00, 0x00, 0x08,
      0x55, 0xF0, 0x55, 0xF0, 0x00, 0x01, 0x01, 0x02, 0x00, 0x01, 0x09, 0x04,
      0x04, 0x03, 0x09, 0x04, 0x00, 0x04, 0x00, 0x00, 0x50, 0x00, 0x61, 0x00,
      0x72, 0x00, 0x73, 0x00, 0x65, 0x00, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0E, 0x00, 0x01, 0x73,
      0x00, 0x74, 0x00, 0x64, 0x00, 0x20, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x63,
      0x00, 0x72, 0x00, 0x6F, 0x00, 0x70, 0x00, 0x68, 0x00, 0x6F, 0x00, 0x6E,
      0x00, 0x65, 0x00, 0x20, 0x00, 0x41, 0x00, 0x75, 0x00, 0x64, 0x00, 0x69,
      0x00, 0x6F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A,
      0x00, 0x02, 0x64, 0x00, 0x09, 0x02, 0x64, 0x00, 0x02, 0x01, 0x00, 0x80,
      0x0A, 0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x09, 0x24,
      0x01, 0x00, 0x01, 0x1E, 0x00, 0x01, 0x01, 0x0C, 0x24, 0x02, 0x01, 0x01,
      0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x09, 0x24, 0x03, 0x02, 0x01,
      0x01, 0x00, 0x01, 0x00, 0x09, 0x04, 0x01, 0x00, 0x00, 0x01, 0x02, 0x00,
      0x00, 0x09, 0x04, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00, 0x07, 0x24,
      0x01, 0x02, 0x01, 0x01, 0x00, 0x0B, 0x24, 0x02, 0x01, 0x01, 0x02, 0x10,
      0x01, 0x80, 0xBB, 0x00, 0x09, 0x05, 0x81, 0x05, 0x60, 0x00, 0x04, 0x00,
      0x00, 0x07, 0x25, 0x01, 0x00, 0x00, 0x00, 0x00,
  };

  
  
  
  
  std::vector<uint8_t> endpoints = {0x81};

  auto deviceId = 4; 
  int res1 = ConfigureDeviceEndpoints(vdd, deviceId, endpoints);
  if (res1 != -1) {
    std::cerr << "ConfigureDeviceEndpoints failed. Error: " << res1
              << std::endl;
    return 1;
  }
  std::cout << "ConfigureDeviceEndpoints succeeded." << std::endl;
  

  std::vector<int32_t> types = {2}; 
  int res2 = ConfigureEndpointTypes(vdd, deviceId, types);
  if (res2 != -1) {
    std::cerr << "ConfigureEndpointTypes failed. Error: " << res2 << std::endl;
    return 1;
  }
  std::cout << "ConfigureEndpointTypes succeeded." << std::endl;

#pragma pack(push, 1)
  struct struct_1 {
    int32_t field_0;
    int32_t field_4;
    char field_8;
    char field_9;
    int16_t field_a;
    int32_t field_c;
  } buffer2;
#pragma pack(pop)

  
  buffer2.field_0 = 0x10;
  buffer2.field_4 = deviceId;
  bool plugIn = 0;
  buffer2.field_8 = plugIn;
  buffer2.field_9 = plugIn;
  buffer2.field_a = 0;
  buffer2.field_c = 3;
  std::vector<uint8_t> buffer3(sizeof(buffer2));
  memcpy(buffer3.data(), &buffer2, sizeof(buffer2));
  DWORD res3 = VusbIoCtl(vdd, 0x2aac04, buffer3);

  if (res3 != -1) {
    std::cerr << "VddIoControl failed. Error: " << res3 << std::endl;
  }

  std::println("ok");


  std::vector<uint8_t> randomPCM = {};

  for (size_t i = 0; i < 960; i++) {
    randomPCM.push_back(static_cast<uint8_t>(rand() % 256));
  }
  Sleep(500);
  while(1) {
    try {
      SubmitAudioData(vdd, deviceId, randomPCM);
    } catch (const std::runtime_error &e) {
      std::cerr << "Error submitting audio data: " << e.what() << std::endl;
      Sleep(100);
    }
  }
  return 0;
}