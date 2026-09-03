#include "Instances.h"
#include "../Offsets/Funcs.h"

namespace laustate
{
    lua_State* GetLuasState(uintptr_t scriptContext)
    {
        uint64_t v1 = 0;
        uint64_t v2 = 0;
        return SkidBase::GetLuaState(scriptContext, &v1, &v2);
    }
}
