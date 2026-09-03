#pragma once
#include <string>
#include <vector>
#include <utility>
#include <windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace net {
    // HTTP response structure
    struct Response {
        bool success = false;
        int status_code = 0;
        std::string body;
        std::vector<std::pair<std::string, std::string>> headers;
    };

    // get(url, headers) — HTTP GET. Returns Response.
    inline Response get(const std::string& url,
                       const std::vector<std::pair<std::string, std::string>>& headers = {}) {
        Response r;
        // Parse URL
        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS uc = { sizeof(uc) };
        uc.dwStructSize = sizeof(uc);
        uc.dwHostNameLength = 1;
        uc.dwUrlPathLength = 1;
        uc.dwSchemeLength = 1;
        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc)) return r;
        std::wstring host(uc.lpszHostName, uc.dwHostNameLength);
        std::wstring path(uc.lpszUrlPath, uc.dwUrlPathLength);
        bool https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hSession = WinHttpOpen(L"SkidBase/1.0",
                                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME,
                                        WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return r;
        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), uc.nPort, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return r; }
        DWORD flags = https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
                                                nullptr, WINHTTP_NO_REFERER,
                                                WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return r; }

        // Add headers
        for (auto& h : headers) {
            std::wstring wh(h.first.begin(), h.first.end());
            wh += L": ";
            wh += std::wstring(h.second.begin(), h.second.end());
            WinHttpAddRequestHeaders(hRequest, wh.c_str(), (DWORD)wh.size(),
                                    WINHTTP_ADDREQ_FLAG_ADD);
        }

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return r;
        }
        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return r;
        }

        // Read status code
        DWORD status = 0; DWORD len = sizeof(status);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);
        r.status_code = (int)status;
        r.success = (status >= 200 && status < 400);

        // Read body
        std::string body;
        for (;;) {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &avail)) break;
            if (avail == 0) break;
            std::string chunk(avail, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, chunk.data(), avail, &read)) break;
            chunk.resize(read);
            body += chunk;
        }
        r.body = std::move(body);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return r;
    }

    // post(url, body, headers) — HTTP POST.
    inline Response post(const std::string& url, const std::string& body,
                        const std::vector<std::pair<std::string, std::string>>& headers = {}) {
        Response r;
        std::wstring wurl(url.begin(), url.end());
        URL_COMPONENTS uc = { sizeof(uc) };
        uc.dwStructSize = sizeof(uc);
        uc.dwHostNameLength = 1;
        uc.dwUrlPathLength = 1;
        uc.dwSchemeLength = 1;
        if (!WinHttpCrackUrl(wurl.c_str(), (DWORD)wurl.size(), 0, &uc)) return r;
        std::wstring host(uc.lpszHostName, uc.dwHostNameLength);
        std::wstring path(uc.lpszUrlPath, uc.dwUrlPathLength);
        bool https = (uc.nScheme == INTERNET_SCHEME_HTTPS);

        HINTERNET hSession = WinHttpOpen(L"SkidBase/1.0",
                                        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                        WINHTTP_NO_PROXY_NAME,
                                        WINHTTP_NO_PROXY_BYPASS, 0);
        if (!hSession) return r;
        HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), uc.nPort, 0);
        if (!hConnect) { WinHttpCloseHandle(hSession); return r; }
        DWORD flags = https ? WINHTTP_FLAG_SECURE : 0;
        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", path.c_str(),
                                                nullptr, WINHTTP_NO_REFERER,
                                                WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
        if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return r; }

        for (auto& h : headers) {
            std::wstring wh(h.first.begin(), h.first.end());
            wh += L": ";
            wh += std::wstring(h.second.begin(), h.second.end());
            WinHttpAddRequestHeaders(hRequest, wh.c_str(), (DWORD)wh.size(),
                                    WINHTTP_ADDREQ_FLAG_ADD);
        }

        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                (LPVOID)body.data(), (DWORD)body.size(),
                                (DWORD)body.size(), 0)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return r;
        }
        if (!WinHttpReceiveResponse(hRequest, nullptr)) {
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            return r;
        }
        DWORD status = 0; DWORD len = sizeof(status);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                            WINHTTP_HEADER_NAME_BY_INDEX, &status, &len, WINHTTP_NO_HEADER_INDEX);
        r.status_code = (int)status;
        r.success = (status >= 200 && status < 400);
        std::string out;
        for (;;) {
            DWORD avail = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &avail)) break;
            if (avail == 0) break;
            std::string chunk(avail, '\0');
            DWORD read = 0;
            if (!WinHttpReadData(hRequest, chunk.data(), avail, &read)) break;
            chunk.resize(read);
            out += chunk;
        }
        r.body = std::move(out);

        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return r;
    }
}
