# USB CDC DLL

## Overview

This DLL provides an interface for controlling USB CDC (Communications Device Class) devices on Windows. It supports a range of functionalities, including reading from and writing to CDC devices, configuring serial parameters, and managing control line states. 

## Features

- Open and close connections to USB CDC devices
- Read from and write to CDC devices asynchronously
- Configure and retrieve serial communication parameters (baud rate, parity, data bits, stop bits)
- Manage control lines (DTR, RTS)
- Set and get line coding
- Set flow control parameters (XON/XOFF, RTS/CTS, DTR/DSR)
- Send break signals

## Functions

### `bool OpenCDCDevice(const char* devicePath);`

Opens a connection to a USB CDC device.

**Parameters:**
- `devicePath`: Path to the device (e.g., `\\.\COM3`).

**Returns:**
- `true` on success, `false` otherwise.

### `void CloseCDCDevice();`

Closes the connection to the USB CDC device.

### `bool StartWriteToCDCDevice(const char* data, size_t length);`

Starts an asynchronous write operation to the CDC device.

**Parameters:**
- `data`: Pointer to the data buffer to write.
- `length`: Length of the data to write.

**Returns:**
- `true` if the operation was started successfully, `false` otherwise.

### `bool StartReadFromCDCDevice(size_t bufferSize);`

Starts an asynchronous read operation from the CDC device.

**Parameters:**
- `bufferSize`: Size of the buffer to allocate for reading.

**Returns:**
- `true` if the operation was started successfully, `false` otherwise.

### `bool CheckWriteStatus();`

Checks if the asynchronous write operation has completed.

**Returns:**
- `true` if the write operation is complete, `false` otherwise.

### `bool CheckReadStatus(char* buffer, size_t* bytesRead);`

Checks if the asynchronous read operation has completed and retrieves the read data.

**Parameters:**
- `buffer`: Pointer to the buffer to store read data.
- `bytesRead`: Pointer to the variable to store the number of bytes read.

**Returns:**
- `true` if the read operation is complete and data is available, `false` otherwise.

### `bool SetCDCDeviceTimeouts(DWORD readTimeout, DWORD writeTimeout);`

Sets timeouts for read and write operations.

**Parameters:**
- `readTimeout`: Read timeout in milliseconds.
- `writeTimeout`: Write timeout in milliseconds.

**Returns:**
- `true` if timeouts were set successfully, `false` otherwise.

### `void CancelOperations();`

Cancels any ongoing read or write operations.

### `bool SetSerialParams(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits, BOOL xonXoffInput, BOOL xonXoffOutput, BOOL dtrControl, BOOL rtsControl);`

Sets serial communication parameters.

**Parameters:**
- `baudRate`: Baud rate.
- `parity`: Parity (e.g., `NOPARITY`).
- `dataBits`: Number of data bits (5, 6, 7, or 8).
- `stopBits`: Number of stop bits (e.g., `ONESTOPBIT`).
- `xonXoffInput`: Enable XON/XOFF input flow control.
- `xonXoffOutput`: Enable XON/XOFF output flow control.
- `dtrControl`: Enable DTR control.
- `rtsControl`: Enable RTS control.

**Returns:**
- `true` if parameters were set successfully, `false` otherwise.

### `bool GetSerialParams(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits, BOOL* xonXoffInput, BOOL* xonXoffOutput, BOOL* dtrControl, BOOL* rtsControl);`

Gets current serial communication parameters.

**Parameters:**
- `baudRate`: Pointer to store the baud rate.
- `parity`: Pointer to store parity.
- `dataBits`: Pointer to store the number of data bits.
- `stopBits`: Pointer to store the number of stop bits.
- `xonXoffInput`: Pointer to store XON/XOFF input flow control state.
- `xonXoffOutput`: Pointer to store XON/XOFF output flow control state.
- `dtrControl`: Pointer to store DTR control state.
- `rtsControl`: Pointer to store RTS control state.

**Returns:**
- `true` if parameters were retrieved successfully, `false` otherwise.

### `bool SetDtr(bool enable);`

Sets or clears the DTR (Data Terminal Ready) control line.

**Parameters:**
- `enable`: `true` to set DTR, `false` to clear DTR.

**Returns:**
- `true` if the control line state was set successfully, `false` otherwise.

### `bool SetRts(bool enable);`

Sets or clears the RTS (Request to Send) control line.

**Parameters:**
- `enable`: `true` to set RTS, `false` to clear RTS.

**Returns:**
- `true` if the control line state was set successfully, `false` otherwise.

### `void SetLineCodingHandlers(SetLineCodingFn setFn, GetLineCodingFn getFn);`

Sets custom handlers for line coding operations.

**Parameters:**
- `setFn`: Function pointer for setting line coding.
- `getFn`: Function pointer for getting line coding.

### `bool SetLineCoding(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits);`

Sets the line coding parameters using the custom handler if provided.

**Parameters:**
- `baudRate`: Baud rate.
- `parity`: Parity.
- `dataBits`: Number of data bits.
- `stopBits`: Number of stop bits.

**Returns:**
- `true` if the line coding was set successfully, `false` otherwise.

### `bool GetLineCoding(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits);`

Gets the line coding parameters using the custom handler if provided.

**Parameters:**
- `baudRate`: Pointer to store the baud rate.
- `parity`: Pointer to store parity.
- `dataBits`: Pointer to store the number of data bits.
- `stopBits`: Pointer to store the number of stop bits.

**Returns:**
- `true` if the line coding was retrieved successfully, `false` otherwise.

### `bool SetControlLineState(BOOL dtr, BOOL rts);`

Sets the control line state.

**Parameters:**
- `dtr`: Enable or disable DTR.
- `rts`: Enable or disable RTS.

**Returns:**
- `true` if the control line state was set successfully, `false` otherwise.

### `bool GetControlLineState(BOOL* dtr, BOOL* rts);`

Gets the current state of the control lines.

**Parameters:**
- `dtr`: Pointer to store the DTR state.
- `rts`: Pointer to store the RTS state.

**Returns:**
- `true` if the control line state was retrieved successfully, `false` otherwise.

### `bool SendBreak();`

Sends a break signal on the communication line.

**Returns:**
- `true` if the break signal was sent successfully, `false` otherwise.

### `bool SetFlowControl(BOOL xonXoffInput, BOOL xonXoffOutput, BOOL rtsCts, BOOL dtrDsr);`

Sets flow control parameters.

**Parameters:**
- `xonXoffInput`: Enable XON/XOFF input flow control.
- `xonXoffOutput`: Enable XON/XOFF output flow control.
- `rtsCts`: Enable RTS/CTS flow control.
- `dtrDsr`: Enable DTR/DSR flow control.

**Returns:**
- `true` if flow control parameters were set successfully, `false` otherwise.

## Future support
- Mac
- Linux
- Android
- IOS

## Example

Here is a basic example demonstrating how to use the DLL in a C++ application.

**Example Usage**

```cpp
#include <iostream>
#include <windows.h>
#include "usb_cdc_dll.h" // Include the DLL header file

// Define custom line coding handlers
bool CustomSetLineCoding(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits) {
    // Implement custom logic here
    std::cout << "Custom setting line coding: BaudRate=" << baudRate << ", Parity=" << (int)parity << ", DataBits=" << (int)dataBits << ", StopBits=" << (int)stopBits << std::endl;
    return true; // Return success
}

bool CustomGetLineCoding(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits) {
    // Implement custom logic here
    if (baudRate) *baudRate = 9600;
    if (parity) *parity = NOPARITY;
    if (dataBits) *dataBits = 8;
    if (stopBits) *stopBits = ONESTOPBIT;
    return true; // Return success
}

int main() {
    HINSTANCE hDll = LoadLibrary("usb_cdc_dll.dll");
    if (!hDll) {
        std::cerr << "Failed to load DLL." << std::endl;
        return 1;
    }

    // Define function pointers
    typedef bool (*OpenCDCDeviceFn)(const char*);
    typedef void (*CloseCDCDeviceFn)();
    typedef bool (*SetLineCodingHandlersFn)(SetLineCodingFn, GetLineCodingFn);

    // Get function pointers
    OpenCDCDeviceFn OpenCDCDevice = (OpenCDCDeviceFn)GetProcAddress(hDll, "OpenCDCDevice");
    CloseCDCDeviceFn CloseCDCDevice = (CloseCDCDeviceFn)GetProcAddress(hDll, "CloseCDCDevice");
    SetLineCodingHandlersFn SetLineCodingHandlers = (SetLineCodingHandlersFn)GetProcAddress(hDll, "SetLineCodingHandlers");

    if (OpenCDCDevice && CloseCDCDevice && SetLineCodingHandlers) {
        // Set custom line coding handlers
        SetLineCodingHandlers(CustomSetLineCoding, CustomGetLineCoding);

        // Open a CDC device (example device path)
        if (OpenCDCDevice("\\\\.\\COM3")) {
            std::cout << "Device opened successfully." << std::endl;

            // Set serial parameters
            if (SetSerialParams(9600, NOPARITY, 8, ONESTOPBIT, TRUE, TRUE, TRUE, TRUE)) {
                std::cout << "Serial parameters set successfully." << std::endl;
            } else {
                std::cerr << "Failed to set serial parameters." << std::endl;
            }

            // Close the device
            CloseCDCDevice();
            std::cout << "Device closed successfully." << std::endl;
        } else {
            std::cerr << "Failed to open device." << std::endl;
        }

        // Free the DLL
        FreeLibrary(hDll);
    } else {
        std::cerr << "Failed to get function pointers from DLL." << std::endl;
    }

    return 0;
}
