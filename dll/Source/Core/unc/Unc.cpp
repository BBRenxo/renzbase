#include "Unc.h"

namespace unc {
    void apply(lua_State* L, Mode mode) {
        if (!L) return;
        switch (mode) {
            case Mode::Off:
                // Let counter tick normally (leave alone)
                set_sunc_flag(L, false);
                break;
            case Mode::Unc:
                // SAFE: only zero yield counter if lua_State signature looks right
                // Otherwise leave alone (yield limit applies but no crash)
                if (safe_apply_unc(L)) {
                    set_sunc_flag(L, false);
                }
                break;
            case Mode::Sunc:
                // SAFE: same guard — only patch if signature is valid
                if (safe_apply_unc(L)) {
                    set_sunc_flag(L, true);
                }
                break;
        }
    }
}
