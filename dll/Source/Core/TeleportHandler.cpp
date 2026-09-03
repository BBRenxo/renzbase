#include <thread>
#include <chrono>
#include "TeleportHandler.hpp"
#include "../Roblox/Offsets/Offsets.h"
#include "../Roblox/Offsets/Funcs.h"
#include "../Roblox/Environment/Environment.h"
#include "../Roblox/Environment/Instances.h"
#include "Execution/Execution.h"

template<typename T>
static T read(uintptr_t address, uintptr_t offset = 0) {
    return *reinterpret_cast<T*>(address + offset);
}

void connection(lua_State* L);
void flagfix();

TeleportHandler::TeleportHandler() 
{
    isloaded = false;
    LoadedDatamodel = 0;
}

void TeleportHandler::load() 
{
    std::thread(&TeleportHandler::loop, this).detach();
}

void TeleportHandler::loop() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));

        uintptr_t fakeDataModel = read<uintptr_t>(Main::Miscellaneous::FakeDatamodelPOINTER);
        if (!fakeDataModel) {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
            continue;
        }

        uintptr_t DataModel = read<uintptr_t>(fakeDataModel + Main::Offsets::DataModel);
        if (!DataModel) {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
            continue;
        }

        if (DataModel != LoadedDatamodel) {
            isloaded = false;
            LoadedDatamodel = DataModel;
        }

        int State = read<int>(DataModel, Main::Offsets::GameLoaded);

        if (State == 31) {
            if (!isloaded) {
                uintptr_t children = read<uintptr_t>(DataModel, Main::Offsets::Children);
                uintptr_t scriptContext = read<uintptr_t>(read<uintptr_t>(children), Main::Offsets::ScriptContext);

                lua_State* skidstate = laustate::GetLuasState(scriptContext);
                Execution::skidsstate = skidstate;

                flagfix();
                env::Register(skidstate);

                lua_getglobal(skidstate, "print");
                lua_pushstring(skidstate, "(your shitsploit name) has handled teleporting sir");
                lua_pcall(skidstate, 1, 0, 0);

                std::thread(connection, skidstate).detach();

                isloaded = true;
            }
        }
        else {
            if (isloaded) {
                isloaded = false;
                LoadedDatamodel = 0;
            }
        }
    }
}
