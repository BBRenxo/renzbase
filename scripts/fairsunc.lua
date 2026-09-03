-- fair sunc test bc sens doesnt want xeno to have high sunc
local Players = game:GetService("Players")
local ReStorage = game:GetService("ReplicatedStorage")
local HttpService = game:GetService("HttpService")
local RunService = game:GetService("RunService")
local StarterGui = game:GetService("StarterGui")

local CONFIG = {
    TITLE           = "Fair Dunc Lab",
    TEST_FILE       = "_dunc_lab_test.txt",
    TEST_FILE_2     = "_dunc_lab_append.txt",
    LIST_FOLDER     = "_dunc_lab_test_list",
    HTTP_URL        = "https://api.ipify.org?format=json",
    LAB_WAIT        = 0.6,
    CLEANUP         = true,
}

local env = (type(getgenv) == "function" and getgenv()) or _G

local TOGGLES = {
    Environment    = env.Environment ~= false,
    Closures       = env.Closures ~= false,
    FileSystem     = env.FileSystem ~= false,
    Network        = env.Network ~= false,
    Input          = env.Input ~= false,
    LabInteraction = env.LabInteraction ~= false,
    Crypt          = env.Crypt ~= false,
    Drawing        = env.Drawing ~= false,
    Debug          = env.Debug == true,
    Console        = env.Console ~= false,
    Metatable      = env.Metatable ~= false,
    Thread         = env.Thread ~= false,
    Misc           = env.Misc ~= false,
}

local Lab = {
    Passed = 0, Failed = 0, Missing = 0, Total = 0,
    Results = {}, StartTime = 0,
    CurrentCategory = ""
}

local cachedToken
local function getServerComponents()
    if not cachedToken then
        local gt = ReStorage:FindFirstChild("DUNC_GetToken")
        if gt then pcall(function() cachedToken = gt:InvokeServer() end) end
    end
    local remote = ReStorage:FindFirstChild("DUNC_Verifier")
    return {Value = cachedToken}, remote
end

local function getEnvironment()
    return (type(getgenv) == "function" and getgenv()) or _G
end

local function deep_index(tbl, path)
    if type(tbl) ~= "table" then return nil end
    local current = tbl
    for segment in string.gmatch(path, "[^%.]+") do
        if type(current) ~= "table" then return nil end
        current = current[segment]
        if current == nil then return nil end
    end
    return current
end

local Aliases = {
    delfile = {"deletefile"},
    makefolder = {"makedir", "mkdir", "createfolder"},
    delfolder = {"deletefolder", "removefolder", "rmdir"},
    listfiles = {"listdir"},
    request = {"http.request", "syn.request", "fluxus.request"},
    rconsoleprint = {"rconsoleprintln", "consoleprintln"},
    rconsoleclear = {"consoleclear"},
    rconsolecreate = {"consolecreate"},
    rconsoleclose = {"consoleclose"},
    rconsolename = {"rconsoletitle", "consolesettitle"},
    ["crypt.base64encode"] = {"base64.encode", "base64_encode"},
    ["crypt.base64decode"] = {"base64.decode", "base64_decode"},
    ["crypt.encrypt"] = {"crypto.encrypt"},
    ["crypt.decrypt"] = {"crypto.decrypt"},
    ["crypt.generatekey"] = {"crypto.generatekey"},
    ["crypt.generatebytes"] = {"crypto.generatebytes"},
    ["crypt.hash"] = {"crypto.hash"},
    getthreadidentity = {"getidentity", "getthreadcontext", "syn.get_thread_identity"},
    setthreadidentity = {"setidentity", "setthreadcontext", "syn.set_thread_identity"},
    setclipboard = {"toclipboard", "writeclipboard"},
    identifyexecutor = {"getexecutorname"},
    isrbxactive = {"isgameactive"},
    mouse1click = {},
    keypress = {},
    queue_on_teleport = {"queue_teleport", "queueonteleport"},
    getfpscap = {},
    ["debug.getconstant"]  = {},
    ["debug.getconstants"] = {},
    ["debug.getupvalue"]   = {},
    ["debug.setupvalue"]   = {},
    ["debug.setconstant"]  = {},
    ["debug.getstack"]     = {},
    ["debug.setstack"]     = {},
    ["debug.getprotos"]    = {},
    ["debug.getproto"]     = {},
    getgc                  = {},
    getconnections         = {},
    getrunningscripts      = {"getscripts"},
    saveinstance           = {},
    ["Drawing.new"]        = {},
}

local function resolve(name)
    local env = getEnvironment()

    if name == "game:HttpGet" then return function(url) return game:HttpGet(url) end end
    if name == "game:HttpPost" then return function(url, data) return game:HttpPost(url, data) end end

    local res = deep_index(env, name)
    if res ~= nil then return res end

    if Aliases[name] then
        for _, alias in ipairs(Aliases[name]) do
            res = deep_index(env, alias)
            if res ~= nil then return res end
        end
    end

    return nil
end

local function updateServer(status)
    local token, remote = getServerComponents()
    if token and remote then
        local score = Lab.Total > 0 and math.floor((Lab.Passed / Lab.Total) * 100) or 0
        pcall(function()
            remote:FireServer({
                Token = token.Value,
                Type = "Update",
                Score = score,
                Status = status
            })
        end)
    end
end

local function record(name, status, detail)
    Lab.Total += 1

    local prefix = ""
    if status == "PASS" then
        Lab.Passed += 1
        prefix = "  🟢 "
        print(prefix .. name .. (detail and (" (" .. detail .. ")") or ""))
    elseif status == "FAIL" then
        Lab.Failed += 1
        prefix = "  🔴 "
        task.spawn(error, prefix .. name .. " -> " .. tostring(detail), 0)
    elseif status == "MISSING" then
        Lab.Missing += 1
        prefix = "  🟡 "
        warn(prefix .. name .. " (not found)")
    end

    table.insert(Lab.Results, {
        name = name,
        status = status,
        detail = detail,
        category = Lab.CurrentCategory
    })

    if Lab.Total % 5 == 0 then
        updateServer("Testing (" .. name .. ")...")
    end
end

local function test(name, callback)
    task.wait()
    local func = resolve(name)
    if func == nil then
        record(name, "MISSING")
        return
    end

    local s, e = pcall(callback, func)
    if s then
        record(name, "PASS")
    else
        record(name, "FAIL", e)
    end
end

local MISSING_SENTINEL = "##MISSING##"

local function testRaw(name, callback)
    task.wait()
    local s, e = pcall(callback)
    if not s then
        if type(e) == "string" and string.find(e, MISSING_SENTINEL) then
            record(name, "MISSING")
        else
            record(name, "FAIL", e)
        end
    else
        record(name, "PASS")
    end
end


local function run_Environment()
    Lab.CurrentCategory = "Environment"
    print("\n Env Check")

    test("getgenv", function(f)
        assert(type(f()) == "table", "Not a table")
        assert(f() == f(), "Identity mismatch")
    end)

    test("getrenv", function(f)
        assert(type(f()) == "table", "Not a table")
        assert(f().game == game, "Does not contain game")
    end)

    test("gethui", function(f)
        assert(typeof(f()) == "Instance", "Not an Instance")
    end)

    test("getcustomasset", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf("test_asset.txt", "data")
        task.wait(0.1)
        local s, uri = pcall(f, "test_asset.txt")
        pcall(resolve("delfile") or function() end, "test_asset.txt")
        if not s then error(uri) end
        assert(type(uri) == "string", "Invalid return")
        assert(string.match(uri, "rbxasset"), "Not an asset URI")
    end)

    testRaw("getrunningscripts check", function()
        local f = resolve("getrunningscripts")
        if not f then error(MISSING_SENTINEL, 0) end

        local scripts = f()
        assert(type(scripts) == "table", "not a table")

        pcall(f)
        local t0 = os.clock()
        for _ = 1, 10 do f() end
        local elapsed = os.clock() - t0

        if elapsed > 1.0 then
            error(string.format("10 calls took %.3fs", elapsed))
        end

        for _, s in ipairs(scripts) do
            assert(typeof(s) == "Instance",
                "Entry is " .. typeof(s) .. ", expected Instance")
            assert(s:IsA("LuaSourceContainer"),
                "Entry is " .. s.ClassName .. ", not a script class")
        end
    end)
end

local function run_Closures()
    Lab.CurrentCategory = "Closures"
    print("\n Closures")

    test("checkcaller", function(f)
        assert(f() == true, "Must return true in executor")
    end)

    test("iscclosure", function(f)
        assert(f(print) == true, "print should be C closure")
    end)

    test("islclosure", function(f)
        assert(f(function() end) == true, "Lua func should be L closure")
    end)

    test("newcclosure", function(f)
        local l = function() return "test" end
        local c = f(l)
        local isc = resolve("iscclosure")
        if isc then assert(isc(c) == true, "Did not wrap") end
        assert(c() == "test", "Wrapped execution failed")
    end)

    test("clonefunction", function(f)
        local original = function() return "cloned" end
        local c = f(original)
        assert(c ~= original, "Not cloned (same reference)")
        assert(c() == "cloned", "Clone behavior mismatch")
    end)

    testRaw("newcclosure stress test", function()
        local f = resolve("newcclosure")
        if not f then error(MISSING_SENTINEL, 0) end

        local wrapped = f(function(x) return x * 2 end)
        local isc = resolve("iscclosure")
        if isc then assert(isc(wrapped) == true, "Not detected as C closure") end

        for i = 1, 50 do
            local r = wrapped(i)
            assert(r == i * 2, "newcclosure broke after " .. i .. " calls (got " .. tostring(r) .. ")")
        end

        local wrapped2 = f(function(a, b, c) return a, b, c end)
        local a, b, c = wrapped2("x", nil, "z")
        assert(a == "x", "Multi-arg failed on arg 1")
        assert(c == "z", "Multi-arg failed on arg 3")
    end)

    test("loadstring", function(f)
        local c, e = f('return "loaded"')
        assert(c() == "loaded", "Execution failed")
    end)
end

local function run_FileSystem()
    Lab.CurrentCategory = "FileSystem"
    print("\n FS")

    test("writefile", function(f)
        f(CONFIG.TEST_FILE, "sunc_test_data")
        local rf = resolve("readfile")
        if rf then
            task.wait(0.1)
            assert(rf(CONFIG.TEST_FILE) == "sunc_test_data", "Write did not persist")
        end
    end)

    test("readfile", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf(CONFIG.TEST_FILE, "sunc_test_data")
        task.wait(0.1)
        local content = f(CONFIG.TEST_FILE)
        assert(content == "sunc_test_data", "Data mismatch: got '" .. tostring(content) .. "'")
    end)

    test("appendfile", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf(CONFIG.TEST_FILE_2, "_append")
        f(CONFIG.TEST_FILE_2, "_data")
        task.wait(0.1)
        local rf = resolve("readfile")
        if rf then
            local d = rf(CONFIG.TEST_FILE_2)
            assert(d == "_append_data", "Append failed: got '" .. tostring(d) .. "'")
        else
            error("Requires readfile to verify")
        end
    end)

    test("loadfile", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf(CONFIG.TEST_FILE, "return 'loaded'")
        local c = f(CONFIG.TEST_FILE)
        assert(c() == "loaded", "Load failed")
    end)

    test("isfile", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf(CONFIG.TEST_FILE, "x")
        assert(f(CONFIG.TEST_FILE) == true, "File not found")
        assert(f("nonexistent_sunc_file_" .. tick()) == false, "False positive")
    end)

    test("isfolder", function(f)
        local mf = resolve("makefolder")
        if not mf then error("Requires makefolder") end
        mf(CONFIG.LIST_FOLDER)
        assert(f(CONFIG.LIST_FOLDER) == true, "Folder not found")
    end)

    test("makefolder", function(f)
        f(CONFIG.LIST_FOLDER)
        local isf = resolve("isfolder")
        if isf then assert(isf(CONFIG.LIST_FOLDER) == true, "Creation failed") end
    end)

    test("delfile", function(f)
        local wf = resolve("writefile")
        if not wf then error("Requires writefile") end
        wf(CONFIG.TEST_FILE, "x")
        f(CONFIG.TEST_FILE)
        local isf = resolve("isfile")
        if isf then assert(isf(CONFIG.TEST_FILE) == false, "Delete failed") end
    end)

    test("delfolder", function(f)
        local mf = resolve("makefolder")
        if not mf then error("Requires makefolder") end
        mf(CONFIG.LIST_FOLDER)
        f(CONFIG.LIST_FOLDER)
        local isf = resolve("isfolder")
        if isf then assert(isf(CONFIG.LIST_FOLDER) == false, "Delete folder failed") end
    end)

    test("listfiles", function(f)
        local mf = resolve("makefolder")
        local wf = resolve("writefile")
        if not mf or not wf then error("Deps missing") end
        mf(CONFIG.LIST_FOLDER)
        wf(CONFIG.LIST_FOLDER .. "/test1.txt", "1")
        local list = f(CONFIG.LIST_FOLDER)
        pcall(resolve("delfile"), CONFIG.LIST_FOLDER .. "/test1.txt")
        pcall(resolve("delfolder") or function() end, CONFIG.LIST_FOLDER)
        
        assert(type(list) == "table", "Did not return table")
        assert(#list > 0, "No files listed")
    end)
end

local function run_Network()
    Lab.CurrentCategory = "Network"
    print("\n Network")

    test("request", function(f)
        local r = f({
            Url = CONFIG.HTTP_URL,
            Method = "GET"
        })
        assert(type(r) == "table", "Response is not a table")
        assert(r.StatusCode == 200, "Status " .. tostring(r.StatusCode))
        assert(r.Body and #r.Body > 0, "Empty body")
    end)

    test("game:HttpGet", function(f)
        local b = f(CONFIG.HTTP_URL)
        assert(type(b) == "string" and #b > 0, "Empty response")
    end)
end

local function run_Input()
    Lab.CurrentCategory = "Input"
    print("\n Input")

    test("keypress", function(f)
        local lab = workspace:FindFirstChild("FairDuncLab")
        local part = lab and lab:FindFirstChild("KeypressTestPart")
        local kr = resolve("keyrelease")
        local t0 = os.clock()
        
        f(0x0D) -- VK_RETURN
        if kr then task.wait(0.05) kr(0x0D) end
        
        if part then
            repeat task.wait() until part.BrickColor == BrickColor.new("Bright blue") or os.clock() - t0 > 1.5
            assert(part.BrickColor == BrickColor.new("Bright blue"), "Executor faked result: Game Engine (UIS) did not physically detect the keypress")
        end
    end)

    test("keyrelease", function(f) f(0x0D) end)
    
    test("mouse1click", function(f)
        local lab = workspace:FindFirstChild("FairDuncLab")
        local part = lab and lab:FindFirstChild("MouseTestPart")
        local t0 = os.clock()

        f()

        if part then
            repeat task.wait() until part.BrickColor == BrickColor.new("Bright red") or os.clock() - t0 > 1.5
            assert(part.BrickColor == BrickColor.new("Bright red"), "Executor faked result: Game Engine (UIS) did not physically detect the mouse click")
        end
    end)

    test("mouse1press", function(f)
        local mr = resolve("mouse1release")
        f()
        task.wait(0.05)
        if mr then mr() end
    end)

    test("mouse1release", function(f) f() end)
    test("mouse2click", function(f) f() end)

    test("mouse2press", function(f)
        local mr = resolve("mouse2release")
        f()
        task.wait(0.05)
        if mr then mr() end
    end)

    test("mouse2release", function(f) f() end)
    test("mousemoverel", function(f) f(10, 10) end)
    test("mousemoveabs", function(f) f(100, 100) end)
    test("mousescroll", function(f) f(10) end)
end

local function run_LabInteraction()
    Lab.CurrentCategory = "LabInteraction"
    print("\n Interaction")
    local lab = workspace:WaitForChild("FairDuncLab", 2)

    if not lab then
        warn("  [!] Lab map missing — skipping interaction tests")
        return
    end

    test("fireclickdetector", function(f)
        local part = lab:WaitForChild("ClickTestPart", 3)
        if not part then error("ClickTestPart missing") end
        local cd = part:FindFirstChildOfClass("ClickDetector")
        if not cd then error("ClickDetector child missing") end
        local t0 = os.clock()
        f(cd)
        repeat task.wait() until part.BrickColor == BrickColor.new("Lime green") or os.clock() - t0 > 2.0
        assert(part.BrickColor == BrickColor.new("Lime green"), "Visual feedback missing")
    end)

    test("fireproximityprompt", function(f)
        local part = lab:WaitForChild("PromptTestPart", 3)
        if not part then error("PromptTestPart missing") end
        local pp = part:FindFirstChildOfClass("ProximityPrompt")
        if not pp then error("ProximityPrompt child missing") end
        local t0 = os.clock()
        f(pp)
        repeat task.wait() until part.BrickColor == BrickColor.new("Cyan") or os.clock() - t0 > 2.0
        assert(part.BrickColor == BrickColor.new("Cyan"), "Visual feedback missing")
    end)

    test("firetouchinterest", function(f)
        local part = lab:WaitForChild("TouchTestPart", 3)
        if not part then error("TouchTestPart missing") end
        local char = Players.LocalPlayer.Character or Players.LocalPlayer.CharacterAdded:Wait()
        local root = char:WaitForChild("HumanoidRootPart", 5)
        if not root then error("Character not loaded") end

        local t0 = os.clock()
        f(root, part, 0)
        task.wait()
        f(root, part, 1)
        repeat task.wait() until part.BrickColor == BrickColor.new("New Yeller") or os.clock() - t0 > 2.0
        assert(part.BrickColor == BrickColor.new("New Yeller"), "Touch feedback missing")
    end)
end

local function run_Crypt()
    Lab.CurrentCategory = "Crypt"
    print("\n Crypt")

    test("crypt.base64encode", function(f)
        assert(f("Hello") == "SGVsbG8=", "Encode fail")
    end)

    test("crypt.base64decode", function(f)
        assert(f("SGVsbG8=") == "Hello", "Decode fail")
    end)

    test("crypt.generatekey", function(f)
        local key = f()
        assert(type(key) == "string" and #key > 0, "No key generated")
    end)

    test("crypt.encrypt", function(f)
        local gk = resolve("crypt.generatekey")
        local gd = resolve("crypt.decrypt")
        local gb = resolve("crypt.generatebytes")
        if not gk or not gd then error("Deps missing") end
        if not gb then error("Requires crypt.generatebytes for CBC test") end

        local key = gk()
        local iv = gb(16)
        local data = f("test", key, iv, "CBC")
        local dec = gd(data, key, iv, "CBC")
        assert(dec == "test", "Round trip fail")
    end)

    test("lz4compress", function(f)
        local de = resolve("lz4decompress")
        if not de then error("lz4decompress missing") end
        local raw = "test"
        local data = f(raw)
        assert(type(data) == "string" and #data > 0, "No compressed data")
        local dec = de(data, #raw)
        assert(dec == raw, "LZ4 round trip fail")
    end)
end

local function run_Drawing()
    Lab.CurrentCategory = "Drawing"
    print("\n Drawing")

    test("Drawing.new", function(f)
        local l = f("Line")
        assert(l ~= nil, "Returned nil")
        
        if type(l) == "table" then
             l.Visible = false
             if l.Remove then l:Remove() end
        elseif type(l) == "userdata" then
             pcall(function() l.Visible = false; l:Remove() end)
        end
    end)

    testRaw("Drawing.Fonts", function()
        local f = resolve("Drawing.Fonts")
        if not f then error(MISSING_SENTINEL, 0) end
        assert(type(f) == "table", "Fonts not a table")
    end)

    test("isrenderobj", function(f)
        local dn = resolve("Drawing.new")
        if not dn then error("Drawing.new missing") end
        local l = dn("Line")
        local s, result = pcall(f, l)
        l:Remove()
        if not s then error(result) end
        assert(result == true, "Check fail")
    end)

    test("getrenderproperty", function(f)
        local dn = resolve("Drawing.new")
        if not dn then error("Drawing.new missing") end
        local l = dn("Line")
        l.Visible = true
        local s, val = pcall(f, l, "Visible")
        l:Remove()
        if not s then error(val) end
        assert(val == true, "Get fail")
    end)

    test("cleardrawcache", function(f)
        f()
    end)

    testRaw("Drawing.new polyfill check", function()
        local dn = resolve("Drawing.new")
        if not dn then error(MISSING_SENTINEL, 0) end

        local coreGui = game:GetService("CoreGui")
        local before = #coreGui:GetChildren()

        local obj = dn("Line")
        if not obj then return end
        
        pcall(function() obj.Visible = true end)
        task.wait()

        local after = #coreGui:GetChildren()
        
        local isTable = (type(obj) == "table")
        local hasInstance = false
        if isTable then
             for k, v in pairs(obj) do
                if typeof(v) == "Instance" then
                    hasInstance = true
                    break
                end
            end
        end

        pcall(function() obj:Remove() end)

        local errs = {}
        if isTable then table.insert(errs, "returns table instead of userdata") end
        if hasInstance then table.insert(errs, "stores Roblox Instance refs") end
        if after > before then table.insert(errs, "adds children to CoreGui (ScreenGui-based)") end

        if #errs > 0 then
            error("Polyfill detected: " .. table.concat(errs, "; "))
        end
    end)
end

local function run_Debug()
    Lab.CurrentCategory = "Debug"
    print("\n Debug")

    test("debug.getupvalue", function(f)
        local u = 10
        local function x() return u end
        assert(f(x, 1) == 10, "Upvalue mismatch")
    end)

    test("debug.setupvalue", function(f)
        local u = 10
        local function x() return u end
        f(x, 1, 20)
        assert(x() == 20, "Set failed")
    end)

    test("debug.getinfo", function(f)
        local info = f(print)
        assert(type(info) == "table", "Did not return table")
        assert(info.what == "C", "print should be C")
    end)


    testRaw("debug.getconstant consistency", function()
        local f = resolve("debug.getconstant")
        if not f then error(MISSING_SENTINEL, 0) end

        local function probe()
            local _a = "DUNC_UNIQUE_CONST"
            local _b = 314159
            return _a, _b
        end

        local c1 = f(probe, 1)
        if c1 == "print" then
            error("Returned hardcoded 'print' – ignores actual constants")
        end
        local found = false
        for i = 1, 5 do
            local v = f(probe, i)
            if v == "DUNC_UNIQUE_CONST" or v == 314159 then found = true break end
        end
        assert(found, "None of the returned constants match the real function body")
        
        -- Input independence check
        local function funcA() local _ = "AAA_CONST" return 1 end
        local function funcB() local _ = "BBB_CONST" return 2 end
        local a1 = f(funcA, 1)
        local b1 = f(funcB, 1)
        if a1 == b1 then
             error("Returns identical constant '"..tostring(a1).."' for different functions")
        end
    end)

    testRaw("debug.getconstants consistency", function()
        local f = resolve("debug.getconstants")
        if not f then 
            record("debug.getconstants consistency", "MISSING")
            return 
        end

        local function probe()
            local _x = "DUNC_CONST_ALPHA"
            local _y = 271828
            return _x, _y
        end

        local tbl = f(probe)
        assert(type(tbl) == "table", "not a table")

        for _, v in ipairs(tbl) do
            if v == 50000 or v == "Hello, world!" then
                error("Returned executor hardcoded set {50000,'print',nil,'Hello, world!','warn'}")
            end
        end

        local foundReal = false
        for _, v in ipairs(tbl) do
            if v == "DUNC_CONST_ALPHA" or v == 271828 then foundReal = true break end
        end
        assert(foundReal, "Actual constants ('DUNC_CONST_ALPHA' / 271828) not present")
    end)

    testRaw("debug.getupvalue consistency", function()
        local f = resolve("debug.getupvalue")
        if not f then error(MISSING_SENTINEL, 0) end

        local sentinel = newproxy(false)
        local function capture() return sentinel end

        local v = f(capture, 1)
        assert(v ~= nil, "Always returns nil regardless of upvalue")
        assert(v == sentinel, "Returned wrong value (expected sentinel userdata)")
    end)

    testRaw("debug.get/setupvalue roundtrip", function()
        local fget = resolve("debug.getupvalue")
        local fset = resolve("debug.setupvalue")
        if not fget or not fset then 
            record("debug.get/setupvalue roundtrip", "MISSING")
            return 
        end

        local a, b, c = 111, "hello", true
        local function probe() return a, b, c end

        fset(probe, 1, 222)
        fset(probe, 2, "world")
        fset(probe, 3, false)

        local r1 = fget(probe, 1)
        
        if r1 == 111 then
             error("setupvalue seems to be separate from getupvalue (value unchanged)")
        end
        
        local v1, v2, v3 = probe()
        assert(v1 == 222, "upvalues not modified")
        assert(v2 == "world", "upvalue 2 not modified")
        assert(v3 == false, "upvalue 3 not modified")
    end)

    testRaw("debug.setconstant consistency", function()
        local fset = resolve("debug.setconstant")
        local fget = resolve("debug.getconstants")
        if not fset or not fget then error(MISSING_SENTINEL, 0) end

        local function probe() return "SETCONST_ORIG" end
        local consts = fget(probe)
        local idx
        for i, v in ipairs(consts) do
            if v == "SETCONST_ORIG" then idx = i break end
        end
        
        assert(idx, "Could not locate constant in probe function")
        
        fset(probe, idx, "SETCONST_NEW")
        assert(probe() == "SETCONST_NEW", "Function execution did not reflect setconstant change")
    end)

    testRaw("debug.getstack consistency", function()
        local f = resolve("debug.getstack")
        if not f then error(MISSING_SENTINEL, 0) end

        local r1 = f(1)
        
        local function isAb(v)
            if v == "ab" then return true end
            if type(v) == "table" and #v == 1 and v[1] == "ab" then return true end
            return false
        end

        if isAb(r1) then
             local function deeper() return f(2) end
             if isAb(deeper()) then
                 error("Always returns 'ab'/{'ab'} regardless of level/index")
             end
        end
    end)

    testRaw("debug.setstack consistency", function()
        local fset = resolve("debug.setstack")
        local fget = resolve("debug.getstack")
        if not fset or not fget then error(MISSING_SENTINEL, 0) end

        local before = fget(1, 1)
        local after
        local function inner()
            fset(1, 1, "DUNC_STACK_VAL")
            after = fget(1, 1)
        end
        inner()
        
        if before and after and before == after and after ~= "DUNC_STACK_VAL" then
             error("Stack slot unchanged after setstack")
        end
    end)

    testRaw("debug.getprotos consistency", function()
        local f = resolve("debug.getprotos")
        if not f then error(MISSING_SENTINEL, 0) end

        local function outer()
            local function inner1() return "I1" end
            local function inner2() return "I2" end
            return inner1, inner2
        end

        local protos = f(outer)
        assert(type(protos) == "table", "not a table")
        assert(#protos >= 2, "outer has 2 inner functions but got " .. #protos .. " protos")

        for i, p in ipairs(protos) do
            if type(p) == "table" then
                 local mt = getmetatable(p)
                 if mt and type(mt.__call) == "function" and p("xyz") == true then
                      error("Proto["..i.."] is a dummy table whose __call always returns true")
                 end
                 error("Proto["..i.."] is a table, expected function")
            end
            assert(type(p) == "function", "Proto["..i.."] is "..type(p)..", expected function")
        end
    end)
    
    testRaw("debug.getproto consistency", function()
        local f = resolve("debug.getproto")
        if not f then error(MISSING_SENTINEL, 0) end

        local function outer()
            local function unique() return "PROTO_REAL" end
            return unique
        end

        local p = f(outer, 1)

        if type(p) == "table" then
             local mt = getmetatable(p)
             if mt and (mt.__call or mt.__index) then
                  local s, r = pcall(p)
                  if s and r == true then
                      error("Returned dummy table with __call→true instead of function")
                  end
             end
             error("Returned table instead of function")
        end
        assert(p() == "PROTO_REAL", "proto returned wrong value")
    end)
    
    testRaw("debug.getinfo accuracy", function()
         local f = resolve("debug.getinfo")
         if not f then error(MISSING_SENTINEL, 0) end
         local cInfo = f(print)
         local lInfo = f(function() end)
         if cInfo.what == lInfo.what then
              error("Returns identical 'what' field ('"..tostring(cInfo.what).."') for both C and Lua functions")
         end

         -- Check for nups = -1 (Xeno hardcodes this, no real impl would)
         if lInfo.nups ~= nil and lInfo.nups < 0 then
              error("nups is negative (" .. tostring(lInfo.nups) .. ") — likely faked")
         end

         -- Verify numparams is sensible for a known function
         local function twoArgs(a, b) return a, b end
         local tInfo = f(twoArgs)
         if tInfo.numparams ~= nil and tInfo.numparams ~= 2 then
              error("numparams wrong for 2-arg func (got " .. tostring(tInfo.numparams) .. ")")
         end
    end)

    testRaw("debug.getconstants on script func", function()
        local ggc = resolve("getgc")
        local gc = resolve("debug.getconstants")
        if not ggc or not gc then error(MISSING_SENTINEL, 0) end

        local targetFunc = nil
        local targetName = nil

        for _, v in ipairs(ggc(true)) do
            if type(v) == "function" then
                local ok, env = pcall(getfenv, v)
                if ok and type(env) == "table" and env.script and typeof(env.script) == "Instance" then
                    targetFunc = v
                    targetName = env.script:GetFullName()
                    break
                end
            end
        end

        if not targetFunc then error("No script functions found via getgc + getfenv") end

        local ok, consts = pcall(gc, targetFunc)
        assert(ok, "debug.getconstants crashed on " .. targetName .. ": " .. tostring(consts))
        assert(type(consts) == "table", "did not return table for " .. targetName)
    end)

    testRaw("debug.getupvalue on script func", function()
        local ggc = resolve("getgc")
        local guv = resolve("debug.getupvalue")
        if not ggc or not guv then error(MISSING_SENTINEL, 0) end

        local targetFunc = nil
        local targetName = nil

        for _, v in ipairs(ggc(true)) do
            if type(v) == "function" then
                local ok, env = pcall(getfenv, v)
                if ok and type(env) == "table" and env.script and typeof(env.script) == "Instance" then
                    targetFunc = v
                    targetName = env.script:GetFullName()
                    break
                end
            end
        end

        if not targetFunc then error("No script functions found via getgc + getfenv") end

        local ok, val = pcall(guv, targetFunc, 1)
        assert(ok, "debug.getupvalue crashed on " .. targetName .. ": " .. tostring(val))
    end)

    testRaw("debug on connections", function()
        local gconn = resolve("getconnections")
        local gc = resolve("debug.getconstants")
        if not gconn or not gc then error(MISSING_SENTINEL, 0) end

        local rs = game:GetService("RunService")
        local conns = gconn(rs.Heartbeat)

        if #conns == 0 then error("No Heartbeat connections found") end

        local tested = false
        for _, conn in ipairs(conns) do
            if conn.Function then
                local ok, consts = pcall(gc, conn.Function)
                if ok and type(consts) == "table" then
                    tested = true
                    break
                end
            end
        end

        assert(tested, "debug.getconstants failed on all Heartbeat connection functions")
    end)

    local function getVerifyAnswers(query)
        local rf = ReStorage:FindFirstChild("DUNC_VerifyAnswers")
        if not rf then return nil end
        local ok, result = pcall(function() return rf:InvokeServer(query) end)
        if ok then return result end
        return nil
    end

    local function findProbeEnv()
        local ggc = resolve("getgc")
        if not ggc then return nil, nil end

        local probeScript = Players.LocalPlayer:FindFirstChild("PlayerScripts")
            and Players.LocalPlayer.PlayerScripts:FindFirstChild("DuncVerifyProbe")
        if not probeScript then return nil, nil end

        for _, v in ipairs(ggc(true)) do
            if type(v) == "function" then
                local ok, env = pcall(getfenv, v)
                if ok and type(env) == "table" and env.script == probeScript then
                    return env, v
                end
            end
        end
        return nil, nil
    end

    testRaw("⚙ server-verified: debug.getconstants", function()
        local gc = resolve("debug.getconstants")
        local ggc = resolve("getgc")
        if not gc or not ggc then error(MISSING_SENTINEL, 0) end

        local env, _ = findProbeEnv()
        if not env then error("Could not find probe environment via getgc + getfenv") end

        local func = env.DuncProbeFunction
        if not func then error("DuncProbeFunction not found in probe env") end

        local ok, consts = pcall(gc, func)
        if not ok then error("debug.getconstants crashed on probe func: " .. tostring(consts)) end
        assert(type(consts) == "table", "did not return table")

        local expected = getVerifyAnswers("probe_constants")
        if not expected then error("Server verify answers unavailable") end

        local matchCount = 0
        for _, exp in ipairs(expected) do
            for _, got in ipairs(consts) do
                if got == exp then matchCount = matchCount + 1 break end
            end
        end

        assert(matchCount >= 2,
            "Only matched " .. matchCount .. "/" .. #expected ..
            " server-known constants — likely faked or hardcoded")
    end)

    testRaw("⚙ server-verified: debug.getconstants (func2)", function()
        local gc = resolve("debug.getconstants")
        local ggc = resolve("getgc")
        if not gc or not ggc then error(MISSING_SENTINEL, 0) end

        local env, _ = findProbeEnv()
        if not env then error("Could not find probe env") end

        local func = env.DuncProbeFunction2
        if not func then error("DuncProbeFunction2 not found in probe env") end

        local ok, consts = pcall(gc, func)
        if not ok then error("Crashed: " .. tostring(consts)) end

        local expected = getVerifyAnswers("probe_constants2")
        if not expected then error("Server answers unavailable") end

        local expected1 = getVerifyAnswers("probe_constants")
        if expected1 then
            local wrongMatch = 0
            for _, e1 in ipairs(expected1) do
                for _, got in ipairs(consts) do
                    if got == e1 then wrongMatch = wrongMatch + 1 end
                end
            end
            if wrongMatch >= 2 then
                error("Returns same constants for different functions — faked")
            end
        end

        local matchCount = 0
        for _, exp in ipairs(expected) do
            for _, got in ipairs(consts) do
                if got == exp then matchCount = matchCount + 1 break end
            end
        end

        assert(matchCount >= 2,
            "Only matched " .. matchCount .. "/" .. #expected ..
            " expected constants for func2")
    end)

    testRaw("⚙ server-verified: debug.getupvalue", function()
        local guv = resolve("debug.getupvalue")
        local ggc = resolve("getgc")
        if not guv or not ggc then error(MISSING_SENTINEL, 0) end

        local env, _ = findProbeEnv()
        if not env then error("Could not find probe env") end

        local func = env.DuncProbeFunction
        if not func then error("DuncProbeFunction not in probe env") end

        local expected = getVerifyAnswers("probe_upvals")
        if not expected then error("Server answers unavailable") end

        local foundAny = false
        for i = 1, 5 do
            local ok, val = pcall(guv, func, i)
            if ok and val ~= nil then
                for _, exp in ipairs(expected) do
                    if val == exp then foundAny = true break end
                end
            end
            if foundAny then break end
        end

        assert(foundAny,
            "None of the upvalues matched server-known sentinels — likely returns nil/faked")
    end)

    testRaw("⚙ server-verified: getfenv", function()
        local ggc = resolve("getgc")
        if not ggc then error(MISSING_SENTINEL, 0) end

        local env, handlerFunc = findProbeEnv()
        if not env then error("Could not find probe env via getgc + getfenv") end

        local expectedKeys = getVerifyAnswers("probe_env_keys")
        if not expectedKeys then error("Server answers unavailable") end

        local foundKeys = 0
        for _, key in ipairs(expectedKeys) do
            if env[key] ~= nil then
                foundKeys = foundKeys + 1
                assert(type(env[key]) == "function",
                    "'" .. key .. "' exists but is " .. type(env[key]) .. ", not function")
            end
        end

        assert(foundKeys >= 2,
            "Only found " .. foundKeys .. "/" .. #expectedKeys ..
            " expected keys in probe env — likely stub")

        if env.DuncProbeFunction then
            local ok, a, b = pcall(env.DuncProbeFunction)
            if ok then
                local upvals = getVerifyAnswers("probe_upvals")
                if upvals and a ~= upvals[1] then
                    error("DuncProbeFunction returned '" .. tostring(a) ..
                        "' but server expected '" .. tostring(upvals[1]) .. "'")
                end
            end
        end
    end)

    testRaw("⚙ server-verified: getconnections (probe event)", function()
        local gconn = resolve("getconnections")
        if not gconn then error(MISSING_SENTINEL, 0) end

        local bindableName = getVerifyAnswers("probe_bindable_name")
        if not bindableName then error("Server answers unavailable") end

        local bindable = ReStorage:FindFirstChild(bindableName)
        if not bindable then error("Probe BindableEvent '" .. bindableName .. "' not found") end

        local conns = gconn(bindable.Event)
        assert(type(conns) == "table", "getconnections returned " .. type(conns))
        assert(#conns > 0, "No connections found on probe event")

        -- Verify the connection has a real Function field
        local hasRealFunc = false
        local connSentinel = getVerifyAnswers("probe_conn_sentinel")

        for _, conn in ipairs(conns) do
            if conn.Function then
                local ok, result = pcall(conn.Function)
                if ok and result == connSentinel then
                    hasRealFunc = true
                    break
                end
                if type(conn.Function) == "function" then
                    hasRealFunc = true
                end
            end
        end

        assert(hasRealFunc,
            "getconnections returned connections but Function field is nil/dummy — faked")
    end)

    testRaw("⚙ server-verified: getconnections (heartbeat)", function()
        local gconn = resolve("getconnections")
        local gc = resolve("debug.getconstants")
        if not gconn or not gc then error(MISSING_SENTINEL, 0) end

        local hbMarker = getVerifyAnswers("probe_heartbeat_marker")
        if not hbMarker then error("Server answers unavailable") end

        local rs = game:GetService("RunService")
        local conns = gconn(rs.Heartbeat)

        local foundProbe = false
        for _, conn in ipairs(conns) do
            if conn.Function then
                local ok, consts = pcall(gc, conn.Function)
                if ok and type(consts) == "table" then
                    for _, c in ipairs(consts) do
                        if c == hbMarker then
                            foundProbe = true
                            break
                        end
                    end
                end
            end
            if foundProbe then break end
        end

        assert(foundProbe,
            "Could not find probe heartbeat connection with marker '" .. hbMarker ..
            "' — getconnections or debug.getconstants is faked")
    end)

    testRaw("⚙ server-verified: getrunningscripts", function()
        local grs = resolve("getrunningscripts")
        if not grs then error(MISSING_SENTINEL, 0) end

        local expectedName = getVerifyAnswers("probe_script_name")
        if not expectedName then error("Server answers unavailable") end

        local scripts = grs()
        assert(type(scripts) == "table", "not a table")

        local found = false
        for _, s in ipairs(scripts) do
            if typeof(s) == "Instance" and s.Name == expectedName then
                found = true
                break
            end
        end

        assert(found,
            "DuncVerifyProbe ('" .. expectedName ..
            "') not found in getrunningscripts results — may be incomplete or faked")
    end)
end

local function run_Console()
    Lab.CurrentCategory = "Console"
    print("\n Console")
    test("rconsolecreate", function(f) f() end)
    test("rconsolename", function(f) f("Fair Dunc Lab") end)
    test("rconsoleprint", function(f) f("Test output\n") end)
    test("rconsoleclear", function(f) f() end)
end

local function run_Metatable()
    Lab.CurrentCategory = "Metatable"
    print("\n MT")

    test("getrawmetatable", function(f)
        local t = setmetatable({}, {
            __index = function() return 42 end,
            __metatable = "locked"
        })
        local mt = f(t)
        assert(type(mt) == "table", "Expected table")
        assert(mt.__index ~= nil, "__index missing")
    end)

    test("setreadonly", function(f)
        local t = {}
        f(t, true)
        local ir = resolve("isreadonly")
        if ir then assert(ir(t) == true, "Not set to readonly") end

        local writeBlocked = not pcall(function() t._dunc_write_test = true end)
        assert(writeBlocked, "setreadonly did not block writes (table.clone fake?)")

        f(t, false)
        if ir then assert(ir(t) == false, "Not set back to writable") end

        local writeAllowed = pcall(function() t._dunc_write_test2 = true end)
        assert(writeAllowed, "setreadonly(t, false) did not restore writes")
    end)

    test("isreadonly", function(f)
        local t = table.freeze({})
        assert(f(t) == true, "Frozen table not detected as readonly")
    end)
end

local function run_Thread()
    Lab.CurrentCategory = "Thread"
    print("\n Thread")

    test("getthreadidentity", function(f)
        local id = f()
        assert(type(id) == "number", "Expected number, got " .. type(id))
    end)

    test("setthreadidentity", function(f)
        local gti = resolve("getthreadidentity")
        local original = gti and gti() or nil
        local ok, err = pcall(function()
            f(3)
            if gti then assert(gti() == 3, "Identity not set to 3") end -- totally level 8 externals
        end)
        if original ~= nil then pcall(f, original) end
        if not ok then error(err) end
    end)
end

local function run_Misc()
    Lab.CurrentCategory = "Misc"
    print("\n Misc")

    test("identifyexecutor", function(f)
        local n, v = f()
        assert(type(n) == "string" and #n > 0, "No executor name")
        print("    > Name: " .. tostring(n))
        print("    > Version: " .. tostring(v))
    end)

    test("setclipboard", function(f) f("Dunc Test") end)

    test("setfpscap", function(f)
        local gfc = resolve("getfpscap")
        local original = gfc and gfc() or nil
        local ok, err = pcall(function()
            f(60)
            if gfc then assert(gfc() == 60, "FPS cap not applied") end
        end)
        if original ~= nil then pcall(f, original) end
        if not ok then error(err) end
    end)

    test("getfpscap", function(f)
        assert(type(f()) == "number", "Expected number")
    end)

    test("isrbxactive", function(f)
        assert(type(f()) == "boolean", "Expected boolean")
    end)

    test("queue_on_teleport", function(f) f('print("queued")') end)

    testRaw("getconnections check", function()
        local f = resolve("getconnections")
        if not f then error(MISSING_SENTINEL, 0) end

        local be = Instance.new("BindableEvent")
        local count = 0
        local sentinel = {}
        local handler = function() count += 1; sentinel.invoked = true end
        local _c = be.Event:Connect(handler)
        local conns = f(be.Event)
        
        if type(conns) ~= "table" or #conns == 0 then
             be:Destroy()
             error("No connections returned")
        end
        
        local hasFunc = false
        for _, conn in ipairs(conns) do
            if conn.Function then hasFunc = true end
        end
        
        if not hasFunc then
             be:Destroy()
             error("Function field is nil for local connection (should be visible)")
        end

        -- Verify the Function is our ACTUAL handler, not a dummy 😁 
        local foundReal = false
        for _, conn in ipairs(conns) do
            if conn.Function == handler then
                foundReal = true
                break
            end
        end
        if not foundReal then
             local funcConn = conns[1].Function
             if funcConn then
                  sentinel.invoked = false
                  pcall(funcConn)
                  if not sentinel.invoked then
                       be:Destroy()
                       error("Function field is a dummy, not the real handler")
                  end
             end
        end
        
        if conns[1].Fire then
             count = 0
             conns[1]:Fire() 
             task.wait()
             if count == 0 then
                  warn("    [!] getconnections: Fire() did not invoke handler")
             end
        end

        -- connect 3 handlers, should get at least 3 connections
        local be2 = Instance.new("BindableEvent")
        be2.Event:Connect(function() end)
        be2.Event:Connect(function() end)
        be2.Event:Connect(function() end)
        local conns2 = f(be2.Event)
        if #conns2 < 3 then
             warn("    [!] getconnections: 3 connects but only " .. #conns2 .. " returned (may create dummy instead of enumerating)")
        end
        be2:Destroy()

        be:Destroy()
    end)

    testRaw("saveinstance check", function()
        local f = resolve("saveinstance")
        if not f then error(MISSING_SENTINEL, 0) end

        local consts = resolve("debug.getconstants")
        local getp = resolve("debug.getprotos")
        local getupv = resolve("debug.getupvalue")

        local suspects = {
            "github", "githubusercontent", "pastebin", "HttpGet",
            "HttpService", "loadstring", "require", "raw.github"
        }

        local function isSuspicious(str)
            if type(str) ~= "string" then return false end
            local lower = string.lower(str)
            for _, keyword in ipairs(suspects) do
                if string.find(lower, string.lower(keyword)) then
                    return str
                end
            end
            return false
        end

        if consts then
            local function check_f(func, depth)
                if depth > 3 then return end
                local ok, c = pcall(consts, func)
                if ok and type(c) == "table" then
                    for _, v in ipairs(c) do
                        local hit = isSuspicious(v)
                        if hit then
                            error("Not built-in: found '" .. hit .. "' in constants")
                        end
                    end
                end
                if getp then
                    local ok2, p = pcall(getp, func)
                    if ok2 and type(p) == "table" then
                        for _, pr in ipairs(p) do
                            if type(pr) == "function" then check_f(pr, depth + 1) end
                        end
                    end
                end
            end
            check_f(f, 0)
        end

        if getupv then
            for i = 1, 10 do
                local ok, v = pcall(getupv, f, i)
                if not ok then break end
                local hit = isSuspicious(v)
                if hit then
                    error("Not built-in: found '" .. hit .. "' in upvalues")
                end
            end
        end

        local isl = resolve("islclosure")
        if isl and isl(f) then
            warn("    [!] saveinstance is an L closure (may be Lua wrapper, not native)")
        end
    end)
    testRaw("getsenv check", function()
        local f = resolve("getsenv")
        if not f then error(MISSING_SENTINEL, 0) end

        local scripts = resolve("getrunningscripts")
        if not scripts then error("Requires getrunningscripts") end

        local running = scripts()
        if #running == 0 then error("No running scripts") end

        local env = f(running[1])
        assert(type(env) == "table", "getsenv did not return a table")

        local keyCount = 0
        for _ in pairs(env) do keyCount += 1 end
        if keyCount <= 1 then
             error("getsenv returned only " .. keyCount .. " key(s) — likely a stub ({script=Script})")
        end

        if env.script == nil then
             error("getsenv result missing 'script' field")
        end
    end)

    testRaw("checkcaller context check", function()
        local f = resolve("checkcaller")
        if not f then error(MISSING_SENTINEL, 0) end

        assert(f() == true, "checkcaller should return true inside executor")

        local be = Instance.new("BindableEvent")
        local insideResult = nil
        be.Event:Connect(function()
            insideResult = f()
        end)
        be:Fire()
        task.wait()
        be:Destroy()

        if insideResult ~= nil and insideResult == true then
             warn("    [!] checkcaller returns true even inside a BindableEvent handler (likely uses debug.info source matching)")
        end
    end)
end




local function main()
    Lab.StartTime = os.clock()
    print("\n" .. string.rep("-", 40))
    print("  Fair Dunc Lab v4.6")
    print(string.rep("-", 40))

    if TOGGLES.Environment then run_Environment() end
    if TOGGLES.Closures then run_Closures() end
    if TOGGLES.FileSystem then run_FileSystem() end
    if TOGGLES.Network then run_Network() end
    if TOGGLES.Input then run_Input() end
    if TOGGLES.LabInteraction then run_LabInteraction() end
    if TOGGLES.Crypt then run_Crypt() end
    if TOGGLES.Drawing then run_Drawing() end
    if TOGGLES.Debug then run_Debug() end
    if TOGGLES.Console then run_Console() end
    if TOGGLES.Metatable then run_Metatable() end
    if TOGGLES.Thread then run_Thread() end
    if TOGGLES.Misc then run_Misc() end

    local elapsed = string.format("%.2fs", os.clock() - Lab.StartTime)
    local score = Lab.Total > 0 and math.floor((Lab.Passed / Lab.Total) * 100) or 0

    print("\n" .. string.rep("-", 42))
    print("  RESULTS (" .. elapsed .. ")")
    print(string.rep("-", 42))
    print("  Score:          " .. score .. "% (" .. Lab.Passed .. "/" .. Lab.Total .. ")")
    print("  Pass:           " .. Lab.Passed)
    print("  Fail:           " .. Lab.Failed)
    print("  Missing:        " .. Lab.Missing)
    print(string.rep("-", 42))

    local cats = {
        "Environment", "Closures", "FileSystem", "Network",
        "Input", "LabInteraction", "Crypt", "Drawing",
        "Debug", "Console", "Metatable", "Thread", "Misc"
    }

    for _, c in ipairs(cats) do
        local cp, ct = 0, 0
        for _, res in ipairs(Lab.Results) do
            if res.category == c then
                ct += 1
                if res.status == "PASS" then cp += 1 end
            end
        end
        if ct > 0 then
            local pct = math.floor((cp / ct) * 100)
            local filled = math.floor(pct / 10)
            local bar = string.rep("\u{2588}", filled) .. string.rep("\u{2591}", 10 - filled)
            print(string.format("  %-15s %s %3d%%", c, bar, pct))
        end
    end
    print(string.rep("-", 42) .. "\n")

    updateServer("Done! Score: " .. score .. "%")

    if CONFIG.CLEANUP then
        local df = resolve("delfile")
        local dfo = resolve("delfolder")
        if df then
            pcall(df, CONFIG.TEST_FILE)
            pcall(df, CONFIG.TEST_FILE_2)
            pcall(df, "test_asset.txt")
        end
        if dfo then
            pcall(dfo, CONFIG.LIST_FOLDER)
        end
    end
end

return main()
