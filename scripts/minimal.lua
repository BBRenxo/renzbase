-- Minimal test - just identifyexecutor
print("[renzbase] minimal test start")
local ok, name = pcall(function() return identifyexecutor() end)
print("[renzbase] identifyexecutor: " .. tostring(ok) .. " -> " .. tostring(name))
print("[renzbase] minimal test end")
