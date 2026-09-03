-- Test script for RenzBase
print("[renzbase] DLL loaded successfully")

-- Executor identification
local name = "unknown"
local ver = "unknown"
pcall(function()
    name = identifyexecutor()
end)
pcall(function()
    ver = "1.0.0"
end)
print("[renzbase] executor: " .. tostring(name) .. " v" .. tostring(ver))

-- sUNC environment checks
local hui = false
pcall(function() hui = gethui() end)
print("[renzbase] gethui: " .. tostring(hui))

local same_env = false
pcall(function() same_env = (getgenv() == getfenv()) end)
print("[renzbase] getgenv == getfenv: " .. tostring(same_env))

-- sUNC state
local sunc = "off"
pcall(function() sunc = getsunc() end)
print("[renzbase] getsunc: " .. tostring(sunc))

-- File system test
local fs_ok = false
pcall(function()
    writefile("renz_test.txt", "hello from renzbase")
    local content = readfile("renz_test.txt")
    fs_ok = (content == "hello from renzbase")
end)
print("[renzbase] writefile/readfile: " .. tostring(fs_ok))

-- Crypt test
local b64 = ""
pcall(function() b64 = crypt.base64encode("hi") end)
print("[renzbase] base64('hi'): " .. tostring(b64))

local md5 = ""
pcall(function() md5 = crypt.hash("x", "md5") end)
print("[renzbase] md5('x'): " .. tostring(md5))

-- Thread identity
local id_ok = false
pcall(function() id_ok = setthreadidentity(8) end)
print("[renzbase] setthreadidentity(8): " .. tostring(id_ok))

print("[renzbase] ===== ALL TESTS COMPLETE =====")
