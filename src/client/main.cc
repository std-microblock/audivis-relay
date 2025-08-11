#include <windows.h>
#include <stdio.h>
#include "..\common\interface.h"

#define AUDIO_BUFFER_SIZE (44100 * 2 / 10)

int main() {
    HANDLE hDevice = CreateFileW(
        L"\\\\.\\VirtualBeepMic",
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error: %d\n", GetLastError());
        return 1;
    }

    printf("Device opened successfully.\n");

    char buffer[AUDIO_BUFFER_SIZE];
    DWORD bytesRead;

    while (1) {
        if (DeviceIoControl(hDevice, IOCTL_GET_AUDIO_BUFFER, NULL, 0, buffer, sizeof(buffer), &bytesRead, NULL)) {
            printf("Read %d bytes of audio data.\n", bytesRead);
            // Here you would typically play the audio data.
            // For this example, we just print a message.
        }
        else {
            printf("Failed to read from device. Error: %d\n", GetLastError());
        }
        Sleep(100); // Poll every 100ms
    }


    CloseHandle(hDevice);
    return 0;
}