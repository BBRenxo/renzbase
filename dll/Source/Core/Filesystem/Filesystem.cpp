#include "Filesystem.h"
#include <random>
#include <chrono>
#include <mutex>

namespace fs {
    static std::string g_root;
    static std::once_flag g_init;

    static std::string make_session_id() {
        // Generate a short random hex string from current time + random.
        auto t = std::chrono::steady_clock::now().time_since_epoch().count();
        std::mt19937_64 rng((uint64_t)t);
        char buf[17];
        uint64_t r = rng();
        for (int i = 0; i < 16; i++) {
            int nib = (r >> ((15 - i) * 4)) & 0xF;
            buf[i] = nib < 10 ? '0' + nib : 'a' + nib - 10;
        }
        buf[16] = 0;
        return std::string(buf);
    }

    std::string root() {
        std::call_once(g_init, []() {
            char temp[MAX_PATH];
            GetTempPathA(MAX_PATH, temp);
            g_root = std::string(temp) + "SkidBase_" + make_session_id();
            CreateDirectoryA(g_root.c_str(), nullptr);
        });
        return g_root;
    }
}
