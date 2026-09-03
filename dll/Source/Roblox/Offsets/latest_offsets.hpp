/*                  ____
 *   ___ __ _____  / __ \
 *  (_-</ // / _ \/ /_/ /
 * /___/\_,_/ .__/\___\_\
 *         /_/
 *
 *  Roblox Version : version-e7d81637d42c4b23
 *  Dumped         : 2026-09-02 21:33
 *  Dumper         : supQ Runtime Dumper v2
 *  Dumped in      : 25.53s
 */
#pragma once
#include <cstdint>
#include <Windows.h>

inline std::uintptr_t rbx_rebase(std::uintptr_t rva) {
    static std::uintptr_t base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
    return rva ? (rva + base) : 0;
}
#define REBASE(x) (rbx_rebase(x))

namespace Offsets
{
    inline const uintptr_t Print                      = REBASE(0x1C8A050);
    inline const uintptr_t ScriptContextResume        = REBASE(0x4115190); //?
    inline const uintptr_t GetLuaStateForInstance     = REBASE(0x0); //?
    inline const uintptr_t LuaVMLoad                  = REBASE(0x406E600);
    inline const uintptr_t LuaD_Call                  = REBASE(0x26D4770);
    inline const uintptr_t luaG_runerror              = REBASE(0x2707380);
    inline const uintptr_t luaD_throw                 = REBASE(0x26D4690);
    inline const uintptr_t TaskDefer                  = REBASE(0x41D3960);
    inline const uintptr_t OpcodeLookupTable          = REBASE(0x61D1700);

    namespace Luau
    {
        inline const uintptr_t Luau_Execute               = REBASE(0x0); //?
        inline const uintptr_t LuaO_NilObject             = REBASE(0x63516D8);
        inline const uintptr_t LuaH_DummyNode             = REBASE(0x6351188);
    }

    namespace reflections
    {
        inline const uintptr_t CastArgs                   = REBASE(0x3FDE600);
    }

    namespace InstanceBridge
    {
        inline const uintptr_t Push                       = REBASE(0x40608A0);
    }

    namespace Capabilities
    {
        inline const uintptr_t GetCapabilities            = REBASE(0x1CC5820);
        inline const uintptr_t Capabilities               = 0x0; //?
    }

    namespace ClassDescriptor
    {
        inline const uintptr_t GetProperty                = REBASE(0x1CC3C50);
        inline const uintptr_t HashLookup                 = REBASE(0x0); //?
        inline const uintptr_t ClassName                  = 0x8; //?
        inline const uintptr_t Properties                 = 0x3C0; //?
    }

    namespace Extras
    {
        inline const uintptr_t k_table                    = REBASE(0x801F560);
    }

    namespace FFlags
    {
        inline const uintptr_t GetFFlag                   = REBASE(0x47FFCB0);
        inline const uintptr_t FFlagPointer               = REBASE(0x87CE5E0);
        inline const uintptr_t BooleanType                = REBASE(0x6C2F110);
        inline const uintptr_t IntegerType                = REBASE(0x6C2EB98);
    }

    namespace Flags
    {
        inline const uintptr_t FireMouseClick             = REBASE(0x0); //?
        inline const uintptr_t FireRightMouseClick        = REBASE(0x3B07820);
        inline const uintptr_t FireMouseHoverEnter        = REBASE(0x3B08E10);
        inline const uintptr_t FireMouseHoverLeave        = REBASE(0x3B08FB0);
        inline const uintptr_t FireProximityPrompt        = REBASE(0x30CA950);
        inline const uintptr_t TaskSchedulerTargetFps     = REBASE(0x8109DE8);
    }

    namespace Signals
    {
        inline const uintptr_t FireTouchInterest          = REBASE(0x0); //?
        inline const uintptr_t DisconnectConnect          = REBASE(0x40410A0);
    }

    namespace RobloxThread
    {
        inline const uintptr_t IdentityPtr                = REBASE(0x80C10C8);
        inline const uintptr_t GetTlsPointer              = REBASE(0x39A0);
    }

    namespace Scheduler
    {
        inline const uintptr_t TaskScheduler              = REBASE(0x8ABD728);
        inline const uintptr_t JobStart                   = 0xC8;
        inline const uintptr_t JobEnd                     = 0xD0;
        inline const uintptr_t JobTypeName                = 0x108; //?
        inline const uintptr_t Fps                        = 0xB0;
    }

    namespace DataModel
    {
        inline const uintptr_t FakeDataModelPointer       = REBASE(0x8D22868);
        inline const uintptr_t FakeDataModelToDataModel   = 0x1F8;
        inline const uintptr_t ScriptContext              = 0x440; //?
        inline const uintptr_t GameLoaded                 = 0x630;
        inline const uintptr_t PlaceId                    = 0x98;
        inline const uintptr_t Children                   = 0x78;
        inline const uintptr_t CreatorId                  = 0x188; //?
        inline const uintptr_t GameId                     = 0x190; //?
        inline const uintptr_t JobId                      = 0x138; //?
        inline const uintptr_t PlaceVersion               = 0x1A0; //?
        inline const uintptr_t Workspace                  = 0x170; //?
        inline const uintptr_t Lighting                   = 0x178; //?
        inline const uintptr_t Players                    = 0x180; //?
    }

    namespace Instance
    {
        inline const uintptr_t Name                       = 0x70;
        inline const uintptr_t Parent                     = 0x68;
        inline const uintptr_t ClassDescriptor            = 0x18;
        inline const uintptr_t Primitive                  = 0x128; //?
        inline const uintptr_t Overlap                    = 0x1F0; //?
        inline const uintptr_t Properties                 = 0x28;
        inline const uintptr_t PropertyDescriptorBitFlags = 0x8C;
        inline const uintptr_t ScriptableMask             = 0x10; //?
        inline const uintptr_t Root                       = 0x50; //?
        inline const uintptr_t Archivable                 = 0x18; //?
        inline const uintptr_t IsA                        = REBASE(0x5AB9B0); //?
        inline const uintptr_t FindFirstChild             = REBASE(0x5A9AB0); //?
        inline const uintptr_t WaitForChild               = REBASE(0x5A8BD0); //?
    }

    namespace PropertyDescriptor
    {
        inline const uintptr_t Name                       = 0x98; //?
        inline const uintptr_t BitFlags                   = 0x8C; //?
        inline const uintptr_t ScriptableMask             = 0x10; //?
    }

    namespace ExtraSpace
    {
        inline const uintptr_t RequireBypass              = 0xB01;
        inline const uintptr_t ScriptContextToResume      = 0x0; //?
    }

    namespace Actor
    {
        inline const uintptr_t Parent                     = 0x68;
        inline const uintptr_t ParallelFlag               = 0x100; //?
        inline const uintptr_t WriteLock                  = 0x720; //?
        inline const uintptr_t ResumeFacet                = 0x0; //?
        inline const uintptr_t TaskSynchronize            = REBASE(0x41D2980);
        inline const uintptr_t TaskDesynchronize          = REBASE(0x41D2D90);
    }

    namespace Connection
    {
        inline const uintptr_t enabled                    = 0x20;
        inline const uintptr_t next                       = 0x10;
        inline const uintptr_t slot_wrapper               = 0x30;
        inline const uintptr_t slot_wrapper_self          = 0x38;
    }

    namespace SlotWrapper
    {
        inline const uintptr_t function_id                = 0x30;
        inline const uintptr_t thread                     = 0x28;
        inline const uintptr_t weak_object_ref            = 0x90;
    }

    namespace WaitSlot
    {
        inline const uintptr_t checkPointer               = 0x10; //?
        inline const uintptr_t connectionRefs             = 0x38; //?
        inline const uintptr_t connectionRefs2            = 0x40; //?
        inline const uintptr_t connectionRefs3            = 0x50; //?
        inline const uintptr_t threadRefContainer         = 0x18; //?
        inline const uintptr_t threadRefContainer2        = 0x8; //?
        inline const uintptr_t threadRefContainer3        = 0x10; //?
        inline const uintptr_t threadRef                  = 0x20; //?
    }

    namespace Signal
    {
        inline const uintptr_t signalSlot                 = 0x30;
        inline const uintptr_t signalSlotWrapper          = 0x38;
    }

    namespace Signal::SignalSlotWrapper
    {
        inline const uintptr_t Ptr                        = 0x10; //?
        inline const uintptr_t secondPtr                  = 0x18; //?
        inline const uintptr_t thirdPtr                   = 0x38; //?
    }

    namespace Signal::SignalSlot
    {
        inline const uintptr_t isOnce                     = 0x95; //?
        inline const uintptr_t SignalRefs                 = 0x70; //?
    }

    namespace Signal::SignalSlot::Refs
    {
        inline const uintptr_t luaThread                  = 0x8; //?
        inline const uintptr_t luaThreadRefId             = 0x10; //?
        inline const uintptr_t functionRefId              = 0x14; //?
    }

    namespace signal_slot
    {
        inline const uintptr_t is_once                    = 0x95; //?
    }

    namespace Scripts
    {
        inline const uintptr_t LocalScriptByteCode        = 0x190;
        inline const uintptr_t ModuleScriptByteCode       = 0x138;
    }

    namespace Bytecode
    {
        inline const uintptr_t Pointer                    = 0x10; //?
        inline const uintptr_t Size                       = 0x20; //?
    }

    namespace VisualEngine
    {
        inline const uintptr_t Dimensions                 = 0x730; //?
        inline const uintptr_t ViewMatrix                 = 0x490; //?
        inline const uintptr_t Camera                     = 0x480; //?
    }

    namespace Camera
    {
        inline const uintptr_t CFrame                     = 0x2C; //?
        inline const uintptr_t Focus                      = 0x5C; //?
        inline const uintptr_t FieldOfView                = 0x88; //?
        inline const uintptr_t NearPlane                  = 0x8C; //?
        inline const uintptr_t FarPlane                   = 0x90; //?
        inline const uintptr_t Viewport                   = 0x94; //?
    }

    namespace Player
    {
        inline const uintptr_t LocalPlayer                = 0x118; //?
        inline const uintptr_t Character                  = 0x140; //?
        inline const uintptr_t UserId                     = 0x1F0; //?
        inline const uintptr_t DisplayName                = 0x100; //?
        inline const uintptr_t Team                       = 0x1D8; //?
    }

    namespace Humanoid
    {
        inline const uintptr_t Health                     = 0x18C; //?
        inline const uintptr_t MaxHealth                  = 0x190; //?
        inline const uintptr_t WalkSpeed                  = 0x194; //?
        inline const uintptr_t JumpPower                  = 0x198; //?
        inline const uintptr_t JumpHeight                 = 0x19C; //?
        inline const uintptr_t HipHeight                  = 0x1A0; //?
        inline const uintptr_t RigType                    = 0x1A4; //?
        inline const uintptr_t MoveDirection              = 0x140; //?
        inline const uintptr_t RootPart                   = 0x488; //?
    }
}
