#include <iostream>

#include "Process/Process.h"
#include "Injection/Injection.h"
#include <string>

/*
	RenzInjector — simplified loader.
	Roblox offsets are baked into Module.dll, so we skip the runtime
	.byfron integrity patching that volx's Integrity injector did.
*/

#define INJECT "Module.dll"

int WINAPI WinMain(
	HINSTANCE,
	HINSTANCE,
	LPSTR lpCmdLine,
	int
)
{
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	SetConsoleTitleA("RenzBase Injector");

	DWORD pid;
	if (argc == 1) {
		pid = Process::GetProcessId("RobloxPlayerBeta.exe");
		if (!pid) {
			MessageBoxA(0, "rbx aint found ggn", "RenzBase", 0x10);
			std::cin.get();
			return 1;
		}
	}
	else {
		pid = std::stoul(argv[1]);
	}

	// 0x1 removes PROCESS_TERMINATE permission (bypass handle detection)
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS - 0x1, FALSE, pid);


#ifdef INJECT
	if (!Injection::InjectDll(INJECT, hProcess, pid)) {
		MessageBoxA(0, "failed or smth", "RenzBase", 0x10);
		std::cin.get();
	}
#endif


	return 0;
}
