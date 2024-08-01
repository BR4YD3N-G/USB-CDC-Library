#include <windows.h>
#include <setupapi.h>
#include <winusb.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")

typedef bool (*SetLineCodingFn)(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits);
typedef bool (*GetLineCodingFn)(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits);

extern "C" {
    __declspec(dllexport) bool OpenCDCDevice(const char* devicePath);
    __declspec(dllexport) void CloseCDCDevice();
    __declspec(dllexport) bool StartWriteToCDCDevice(const char* data, size_t length);
    __declspec(dllexport) bool StartReadFromCDCDevice(size_t bufferSize);
    __declspec(dllexport) bool CheckWriteStatus();
    __declspec(dllexport) bool CheckReadStatus(char* buffer, size_t* bytesRead);
    __declspec(dllexport) bool SetCDCDeviceTimeouts(DWORD readTimeout, DWORD writeTimeout);
    __declspec(dllexport) void CancelOperations();
    __declspec(dllexport) bool SetSerialParams(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits, BOOL xonXoffInput, BOOL xonXoffOutput, BOOL dtrControl, BOOL rtsControl);
    __declspec(dllexport) bool GetSerialParams(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits, BOOL* xonXoffInput, BOOL* xonXoffOutput, BOOL* dtrControl, BOOL* rtsControl);
    __declspec(dllexport) bool SetDtr(bool enable);
    __declspec(dllexport) bool SetRts(bool enable);
    __declspec(dllexport) void SetLineCodingHandlers(SetLineCodingFn setFn, GetLineCodingFn getFn);
    __declspec(dllexport) bool SetControlLineState(BOOL dtr, BOOL rts);
    __declspec(dllexport) bool GetControlLineState(BOOL* dtr, BOOL* rts);
    __declspec(dllexport) bool SendBreak();
    __declspec(dllexport) bool SetFlowControl(BOOL xonXoffInput, BOOL xonXoffOutput, BOOL rtsCts, BOOL dtrDsr);
}

HANDLE hDevice = INVALID_HANDLE_VALUE;
WINUSB_INTERFACE_HANDLE winusbHandle = NULL;
OVERLAPPED readOverlapped = {0};
OVERLAPPED writeOverlapped = {0};
HANDLE readEvent = NULL;
HANDLE writeEvent = NULL;
std::vector<BYTE> readBuffer;
bool readPending = false;
bool writePending = false;

SetLineCodingFn customSetLineCoding = nullptr;
GetLineCodingFn customGetLineCoding = nullptr;

bool OpenCDCDevice(const char* devicePath) {
    hDevice = CreateFileA(
        devicePath,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (!WinUsb_Initialize(hDevice, &winusbHandle)) {
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
        return false;
    }

    readEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    writeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (!readEvent || !writeEvent) {
        CloseCDCDevice();
        return false;
    }

    return true;
}

void CloseCDCDevice() {
    if (winusbHandle) {
        WinUsb_Free(winusbHandle);
        winusbHandle = NULL;
    }
    if (hDevice != INVALID_HANDLE_VALUE) {
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
    }
    if (readEvent) {
        CloseHandle(readEvent);
        readEvent = NULL;
    }
    if (writeEvent) {
        CloseHandle(writeEvent);
        writeEvent = NULL;
    }
}

bool StartWriteToCDCDevice(const char* data, size_t length) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    writeOverlapped.hEvent = writeEvent;
    BOOL result = WriteFile(hDevice, data, static_cast<DWORD>(length), NULL, &writeOverlapped);
    if (!result && GetLastError() != ERROR_IO_PENDING) {
        return false;
    }
    writePending = true;
    return true;
}

bool StartReadFromCDCDevice(size_t bufferSize) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    readBuffer.resize(bufferSize);
    readOverlapped.hEvent = readEvent;
    BOOL result = ReadFile(hDevice, readBuffer.data(), static_cast<DWORD>(bufferSize), NULL, &readOverlapped);
    if (!result && GetLastError() != ERROR_IO_PENDING) {
        return false;
    }
    readPending = true;
    return true;
}

bool CheckWriteStatus() {
    if (!writePending) {
        return false;
    }

    DWORD result = WaitForSingleObject(writeEvent, 0);
    if (result == WAIT_OBJECT_0) {
        writePending = false;
        return true;
    }
    return false;
}

bool CheckReadStatus(char* buffer, size_t* bytesRead) {
    if (!readPending) {
        return false;
    }

    DWORD result = WaitForSingleObject(readEvent, 0);
    if (result == WAIT_OBJECT_0) {
        readPending = false;
        DWORD bytesReadTemp;
        BOOL success = GetOverlappedResult(hDevice, &readOverlapped, &bytesReadTemp, FALSE);
        if (success) {
            if (buffer && bytesRead) {
                memcpy(buffer, readBuffer.data(), bytesReadTemp);
                *bytesRead = bytesReadTemp;
            }
            return true;
        }
    }
    return false;
}

bool SetCDCDeviceTimeouts(DWORD readTimeout, DWORD writeTimeout) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = readTimeout;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = writeTimeout;
    timeouts.WriteTotalTimeoutConstant = 0;

    return SetCommTimeouts(hDevice, &timeouts) != 0;
}

void CancelOperations() {
    if (writePending) {
        CancelIoEx(hDevice, &writeOverlapped);
        writePending = false;
    }
    if (readPending) {
        CancelIoEx(hDevice, &readOverlapped);
        readPending = false;
    }
}

bool SetSerialParams(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits, BOOL xonXoffInput, BOOL xonXoffOutput, BOOL dtrControl, BOOL rtsControl) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    DCB dcb = {0};
    if (!GetCommState(hDevice, &dcb)) {
        return false;
    }

    dcb.BaudRate = baudRate;
    dcb.ByteSize = dataBits;
    dcb.StopBits = stopBits;
    dcb.Parity = parity;
    dcb.fOutX = xonXoffOutput; // XON/XOFF flow control for output
    dcb.fInX = xonXoffInput;  // XON/XOFF flow control for input
    dcb.fDtrControl = dtrControl ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE; // DTR control
    dcb.fRtsControl = rtsControl ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE; // RTS control

    return SetCommState(hDevice, &dcb) != 0;
}

bool GetSerialParams(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits, BOOL* xonXoffInput, BOOL* xonXoffOutput, BOOL* dtrControl, BOOL* rtsControl) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    DCB dcb = {0};
    if (!GetCommState(hDevice, &dcb)) {
        return false;
    }

    if (baudRate) {
        *baudRate = dcb.BaudRate;
    }
    if (parity) {
        *parity = dcb.Parity;
    }
    if (dataBits) {
        *dataBits = dcb.ByteSize;
    }
    if (stopBits) {
        *stopBits = dcb.StopBits;
    }
    if (xonXoffInput) {
        *xonXoffInput = dcb.fInX;
    }
    if (xonXoffOutput) {
        *xonXoffOutput = dcb.fOutX;
    }
    if (dtrControl) {
        *dtrControl = (dcb.fDtrControl == DTR_CONTROL_ENABLE);
    }
    if (rtsControl) {
        *rtsControl = (dcb.fRtsControl == RTS_CONTROL_ENABLE);
    }
    return true;
}

bool SetDtr(bool enable) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    return EscapeCommFunction(hDevice, enable ? SETDTR : CLRDTR) != 0;
}

bool SetRts(bool enable) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    return EscapeCommFunction(hDevice, enable ? SETRTS : CLRRTS) != 0;
}

void SetLineCodingHandlers(SetLineCodingFn setFn, GetLineCodingFn getFn) {
    customSetLineCoding = setFn;
    customGetLineCoding = getFn;
}

bool SetLineCoding(DWORD baudRate, BYTE parity, BYTE dataBits, BYTE stopBits) {
    if (customSetLineCoding) {
        return customSetLineCoding(baudRate, parity, dataBits, stopBits);
    }
    return false; // No custom function provided
}

bool GetLineCoding(DWORD* baudRate, BYTE* parity, BYTE* dataBits, BYTE* stopBits) {
    if (customGetLineCoding) {
        return customGetLineCoding(baudRate, parity, dataBits, stopBits);
    }
    return false; // No custom function provided
}

bool SetControlLineState(BOOL dtr, BOOL rts) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD state = 0;
    if (dtr) {
        state |= SETDTR;
    }
    if (rts) {
        state |= SETRTS;
    }
    return EscapeCommFunction(hDevice, state) != 0;
}

bool GetControlLineState(BOOL* dtr, BOOL* rts) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    DWORD state = 0;
    BOOL result = EscapeCommFunction(hDevice, GETCOMMSTATE) != 0;
    if (result) {
        *dtr = (state & DTR_CONTROL_ENABLE) != 0;
        *rts = (state & RTS_CONTROL_ENABLE) != 0;
    }
    return result;
}

bool SendBreak() {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }
    return EscapeCommFunction(hDevice, SETBREAK) != 0;
}

bool SetFlowControl(BOOL xonXoffInput, BOOL xonXoffOutput, BOOL rtsCts, BOOL dtrDsr) {
    if (hDevice == INVALID_HANDLE_VALUE) {
        return false;
    }

    DCB dcb = {0};
    if (!GetCommState(hDevice, &dcb)) {
        return false;
    }

    dcb.fOutX = xonXoffOutput; // XON/XOFF flow control for output
    dcb.fInX = xonXoffInput;  // XON/XOFF flow control for input
    dcb.fRtsControl = rtsCts ? RTS_CONTROL_ENABLE : RTS_CONTROL_DISABLE; // RTS/CTS flow control
    dcb.fDtrControl = dtrDsr ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE; // DTR/DSR flow control

    return SetCommState(hDevice, &dcb) != 0;
}
