#pragma once
#include <cstdint>
#include <Windows.h>

namespace Main {
    inline uintptr_t krah() {
        return reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    }

    inline uintptr_t Rebase(uintptr_t offset) {
        return krah() + offset;
    }

    namespace Functions {
        inline const uintptr_t Print = Rebase(0x1c68fe0);
        inline const uintptr_t GetGlobalState = Rebase(0x402B3C0);
        inline const uintptr_t LuauExecute = Rebase(0x26bda30);
        inline const uintptr_t LuaDThrow = Rebase(0x26adad0);
        inline const uintptr_t OpcodeLookupTable = Rebase(0x6d45820);
    }

    namespace Miscellaneous {
        inline const uintptr_t FakeDatamodelPOINTER = Rebase(0x8d22868);
        inline const uintptr_t LuaNil = Rebase(0x62f7418);
        inline const uintptr_t LuaDummy = Rebase(0x62f6ec8);
        inline const uintptr_t TargetFPS = Rebase(0x80993c8);
    }

    namespace Offsets {
        inline const uintptr_t DataModel = 0x1f8;
        inline const uintptr_t ScriptContext = 0x440;
        inline const uintptr_t Children = 0x78;
        inline const uintptr_t GameLoaded = 0x5d0;
    }

    namespace Identity1 {
        inline const uintptr_t IdentityPointer = Rebase(0x8051178);
        inline const uintptr_t GetTlsPointer = Rebase(0x4170);
    }

    namespace Identity2 {
        inline const uintptr_t GetCapabilities = Rebase(0x1ca46d0);
        inline const uintptr_t Capabilities = 0x28;
    }
}