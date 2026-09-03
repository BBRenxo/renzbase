#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include "lua.h"
#include "lualib.h"
#include "../Roblox/Offsets/Offsets.h"
#include "../Roblox/Offsets/Funcs.h"
#include "../Roblox/Environment/Instances.h"
#include "../Roblox/Environment/Environment.h"
#include "../Core/Execution/Execution.h"

#include "TeleportHandler.hpp"

// Todo
// waitinghybirdscriptsjob
template<typename T>
T read(uintptr_t address, uintptr_t offset = 0) {
    return *reinterpret_cast<T*>(address + offset);
}

#pragma comment(lib, "ws2_32.lib")

void connection(lua_State* L) {
    // Two listeners:
    //   1. Named pipe \\\\.\\pipe\\RenzBase (modern, invisible to network AC)
    //   2. Winsock port 9002 (legacy — original SkidBase Loader.exe uses this)
    // Either accepts script source and dispatches to Execution::execute.

    char pipeName[] = "\\\\.\\pipe\\RenzBase";

    // === Thread 1: named pipe loop ===
    // PIPE_TYPE_BYTE mode so clients don't need to set message mode.
    auto pipe_thread = std::thread([L, pipeName]() {
        while (true) {
            HANDLE hPipe = CreateNamedPipeA(
                pipeName,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                4096, 4096, 0, nullptr);

            if (hPipe == INVALID_HANDLE_VALUE) {
                Sleep(1000);
                continue;
            }

            BOOL connected = ConnectNamedPipe(hPipe, nullptr) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
            if (!connected) {
                CloseHandle(hPipe);
                continue;
            }

            std::string script;
            char buffer[4096];
            DWORD bytesRead = 0;

            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                script += buffer;
            }

            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);

            if (!script.empty()) {
                Execution::execute(L, script);
            }

            Sleep(10);
        }
    });
    pipe_thread.detach();

    // === Thread 2: legacy port 9002 loop (for original Loader.exe compat) ===
    WSADATA sss;
    if (WSAStartup(MAKEWORD(2, 2), &sss) != 0) return;

    SOCKET ss = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ss == INVALID_SOCKET) {
        WSACleanup();
        return;
    }

    sockaddr_in s;
    s.sin_family = AF_INET;
    s.sin_addr.s_addr = inet_addr("127.0.0.1");
    s.sin_port = htons(9002);

    if (bind(ss, (SOCKADDR*)&s, sizeof(s)) == SOCKET_ERROR) {
        closesocket(ss);
        WSACleanup();
        return;
    }

    if (listen(ss, 10) == SOCKET_ERROR) {
        closesocket(ss);
        WSACleanup();
        return;
    }

    while (true) {
        SOCKET client = accept(ss, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            std::string script;
            char buffer[4096];
            int ssss;

            while ((ssss = recv(client, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[ssss] = '\0';
                script += buffer;
            }

            closesocket(client);

            if (!script.empty()) {
                Execution::execute(L, script);
            }
        }
        Sleep(10);
    }

    closesocket(ss);
    WSACleanup();
}

void main_thread() {
    Teleport->load();
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        std::thread(main_thread).detach();
    }
    return TRUE;
}
