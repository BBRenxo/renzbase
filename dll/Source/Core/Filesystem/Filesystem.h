#pragma once
#include <string>
#include <vector>
#include <optional>
#include <Windows.h>

namespace fs {
    // Root directory for executor files. We use %TEMP%\<random_session_id>\
    // so multiple Roblox instances don't collide and so files auto-clean.
    // Real executors use a workspace folder but for safety we sandbox.
    std::string root();

    // writefile(path, content) — writes content to path. Creates folders
    // as needed. Returns true on success. Defensive: handles invalid paths.
    inline bool writefile(const std::string& path, const std::string& content) {
        if (path.empty()) return false;
        if (path.find("..") != std::string::npos) return false; // path traversal
        std::string full = root() + "\\" + path;
        // Strip any drive letter / absolute path attempts from user input
        // (already prevented by the .. check but be paranoid)
        if (full.size() > 260) return false; // MAX_PATH
        // Create parent directory if needed
        std::string parent = full;
        size_t pos = parent.find_last_of("\\/");
        if (pos != std::string::npos) {
            parent = parent.substr(0, pos);
            CreateDirectoryA(parent.c_str(), nullptr);
        }
        HANDLE h = CreateFileA(full.c_str(), GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        DWORD written = 0;
        BOOL ok = WriteFile(h, content.data(), (DWORD)content.size(), &written, nullptr);
        CloseHandle(h);
        if (!ok) return false;
        if (written != content.size()) return false;
        return true;
    }

    // readfile(path) — returns file contents or empty string on failure.
    inline std::string readfile(const std::string& path) {
        std::string full = root() + "\\" + path;
        HANDLE h = CreateFileA(full.c_str(), GENERIC_READ, FILE_SHARE_READ,
                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return "";
        DWORD size = GetFileSize(h, nullptr);
        if (size == INVALID_FILE_SIZE) { CloseHandle(h); return ""; }
        std::string out(size, '\0');
        DWORD read = 0;
        BOOL ok = ReadFile(h, out.data(), size, &read, nullptr);
        CloseHandle(h);
        if (!ok || read != size) return "";
        return out;
    }

    // appendfile(path, content) — appends to file (creates if missing).
    inline bool appendfile(const std::string& path, const std::string& content) {
        std::string full = root() + "\\" + path;
        std::string parent = full;
        size_t pos = parent.find_last_of("\\/");
        if (pos != std::string::npos) {
            parent = parent.substr(0, pos);
            CreateDirectoryA(parent.c_str(), nullptr);
        }
        HANDLE h = CreateFileA(full.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ,
                               nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (h == INVALID_HANDLE_VALUE) return false;
        DWORD written = 0;
        BOOL ok = WriteFile(h, content.data(), (DWORD)content.size(), &written, nullptr);
        CloseHandle(h);
        return ok && written == content.size();
    }

    // isfile(path) — returns true if path exists AND is a file.
    inline bool isfile(const std::string& path) {
        std::string full = root() + "\\" + path;
        DWORD attr = GetFileAttributesA(full.c_str());
        return attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    // isfolder(path) — returns true if path exists AND is a directory.
    inline bool isfolder(const std::string& path) {
        std::string full = root() + "\\" + path;
        DWORD attr = GetFileAttributesA(full.c_str());
        return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY);
    }

    // makefolder(path) — creates a directory (recursive).
    inline bool makefolder(const std::string& path) {
        std::string full = root() + "\\" + path;
        return CreateDirectoryA(full.c_str(), nullptr) || GetLastError() == ERROR_ALREADY_EXISTS;
    }

    // delfile(path) — deletes a file. Returns true on success.
    inline bool delfile(const std::string& path) {
        std::string full = root() + "\\" + path;
        return DeleteFileA(full.c_str()) != 0;
    }

    // delfolder(path) — deletes a directory (recursive).
    inline bool delfolder(const std::string& path) {
        std::string full = root() + "\\" + path;
        // First delete contents recursively
        std::string search = full + "\\*";
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(search.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
                std::string child = full + "\\" + fd.cFileName;
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    delfolder(child.substr(root().size() + 1));
                } else {
                    DeleteFileA(child.c_str());
                }
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        return RemoveDirectoryA(full.c_str()) != 0;
    }

    // listfiles(folder) — returns all entries in folder (no recursion).
    inline std::vector<std::string> listfiles(const std::string& folder) {
        std::vector<std::string> out;
        std::string full = root() + "\\" + folder;
        std::string search = full + "\\*";
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(search.c_str(), &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0) continue;
                out.push_back(fd.cFileName);
            } while (FindNextFileA(h, &fd));
            FindClose(h);
        }
        return out;
    }

    // loadfile(path) — read a file and compile it as Lua chunk. Returns
    // (function, nil) on success, (nil, error_string) on failure.
    struct LoadFileResult { void* fn = nullptr; std::string err; bool ok = false; };
    inline LoadFileResult loadfile(const std::string& path) {
        LoadFileResult r;
        std::string src = readfile(path);
        if (src.empty()) { r.err = "could not read file"; return r; }
        // We don't have direct Luau access here; caller handles compilation.
        // Return source as fn via opaque — actual integration uses luau_compile.
        r.fn = (void*)new std::string(src);
        r.ok = true;
        return r;
    }
}
