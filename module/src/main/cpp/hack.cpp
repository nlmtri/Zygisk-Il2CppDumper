//
// Created by Perfare on 2020/7/4.
//

#include "hack.h"
#include "il2cpp_dump.h"
#include "lua_api.h"
#include "log.h"
#include "xdl.h"
#include <cstring>
#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <jni.h>
#include <set>
#include <thread>
#include <sys/mman.h>
#include <linux/unistd.h>
#include <array>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <android/input.h>
#include "menu.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <asm-generic/fcntl.h>
#include "pmparser.h"
#include "il2cpp-class.h"
#include <codecvt>

struct lua_State;
extern bool SetupLuaAPI(void* handle);

static int get_prot(const procmaps_struct* procstruct) {
    int prot = 0;
    if (procstruct->is_r) {
        prot |= PROT_READ;
    }
    if (procstruct->is_w) {
        prot |= PROT_WRITE;
    }
    if (procstruct->is_x) {
        prot |= PROT_EXEC;
    }
    return prot;
}

#define FAILURE_RETURN(exp, failure_value) ({   \
    __typeof__(exp) _rc;                    \
    _rc = (exp);                            \
    if (_rc == failure_value) {             \
        LOGW(#exp);                        \
        return 1;                           \
    }                                       \
    _rc; })

static lua_State * g_L = nullptr;
static clock_t start = 0;
const char* print_utils = R"(
local print = hack_lua_print
-- log输出格式化
local function logPrint(str)
    str = os.date("\nLog output date: %Y-%m-%d %H:%M:%S \n", os.time()) .. str
    print(str)
end

-- key值格式化
local function formatKey(key)
    local t = type(key)
    if t == "number" then
        return "["..key.."]"
    elseif t == "string" then
        local n = tonumber(key)
        if n then
            return "["..key.."]"
        end
    end
    return key
end

-- 栈
local function newStack()
    local stack = {
        tableList = {}
    }
    function stack:push(t)
        table.insert(self.tableList, t)
    end
    function stack:pop()
        return table.remove(self.tableList)
    end
    function stack:contains(t)
        for _, v in ipairs(self.tableList) do
            if v == t then
                return true
            end
        end
        return false
    end
    return stack
end

-- 输出打印table表 函数
local function table2String(...)
    local args = {...}
    for k, v in pairs(args) do
        local root = v
        if type(root) == "table" then
            local temp = {
                "------------------------ printTable start ------------------------\n",
                "local tableValue".." = {\n",
            }
            local stack = newStack()
            local function table2String(t, depth)
                stack:push(t)
                if type(depth) == "number" then
                    depth = depth + 1
                else
                    depth = 1
                end
                local indent = ""
                for i=1, depth do
                    indent = indent .. "    "
                end
                for k, v in pairs(t) do
                    local key = tostring(k)
                    local typeV = type(v)
                    if typeV == "table" then
                        if key ~= "__valuePrototype" then
                            if stack:contains(v) then
                                table.insert(temp, indent..formatKey(key).." = {检测到循环引用!},\n")
                            else
                                table.insert(temp, indent..formatKey(key).." = {\n")
                                table2String(v, depth)
                                table.insert(temp, indent.."},\n")
                            end
                        end
                    elseif typeV == "string" then
                        table.insert(temp, string.format("%s%s = \"%s\",\n", indent, formatKey(key), tostring(v)))
                    else
                        table.insert(temp, string.format("%s%s = %s,\n", indent, formatKey(key), tostring(v)))
                    end
                end
                stack:pop()
            end
            table2String(root)
            table.insert(temp, "}\n------------------------- printTable end -------------------------")
            return table.concat(temp)
        else
            return "----------------------- printString start ------------------------\n"
                    .. tostring(root) .. "\n------------------------ printString end -------------------------"
        end
    end
end

local function printTable(...)
    logPrint(table2String(...))
end

return {
    table2String = table2String,
    print_utils = printTable
}
)";

const char* http_server = R"(
local socket = require("socket.core")
local print = hack_lua_print
local print_utils = require "hack_print_utils"
local is_debug = false
local print_r = print_utils.print_utils

local function log_info(info, ...)
	if not is_debug then
		return
	end

	local t = type(info)
	if t == "table" then
		print_r(info)
	else
		print(string.format(tostring(info), ...))
	end
end

local methods = {
	["GET"] = {},
	["POST"] = {},
}

local function split(str,separator)
    local str = tostring(str)
    local separator = tostring(separator)
    local strB, arrayIndex = 1, 1
    local targetArray = {}
    if (separator == nil)
    then
        return false
    end
    local condition = true
    while (condition)
    do
        local si, sd = string.find(str, separator, strB)
        if (si)
        then
            targetArray[arrayIndex] = string.sub(str,strB,si - 1)
            arrayIndex = arrayIndex + 1
            strB = sd + 1
        else
            targetArray[arrayIndex] = string.sub(str,strB,string.len(str))
            condition = false
        end
    end
    return targetArray
end

local function parseGetArgs(getUrl)
    local result

	local v = split(getUrl, "?")
	if #v >= 2 then
	    result = {}
	    local args = split(v[2], "&")
		for _,arg in ipairs(args) do
			local k = split(arg, "=")
			result[k[1]] = k[2]
		end
	end

	return v[1], result
end

local function getMethod(method, url)
    local list = methods[method]
	if list then
		return list[url]
	end
end

local function readHttpData(client)
    local data
    local datas = {}

	local request = client:receive()
	local method, url, httpver = request:match("^(%a+)%s+(.-)%s+HTTP/([%d%.]+)$")
	local args
	url,args = parseGetArgs(url)

    datas["method"] = method
	datas["url"] = url
	if args then
	   datas["params"] = args
	end
	datas["httpver"] = httpver

    repeat
		data = client:receive()
		local l = split(data, ": ")
		if #l == 2 then
		    datas[l[1]] = l[2]
		end
	until (#data == 0)

	local bodyLen = datas["Content-Length"]
	if bodyLen then
	    bodyLen = tonumber(bodyLen)
		local body = client:receive(bodyLen)
		--if datas["Content-Type"] == "application/json" then
		--	local jsonData = json.decode(body)
		--	datas["json"] = jsonData
		--end
		datas["body"] = body
	end

	return datas
end

---@class HttpParams
---@field params table
---@field josn table
---@field url string
---@field body string
---@field httpver string

---@param url string
---@param func fun(request : HttpParams) : string
local function RegisterGetMethod(url, func)
	methods["GET"][url] = func
end

---@param url string
local function UnRegisterGetMethod(url)
	methods["GET"][url] = nil
end

---@param url string
---@param func fun(request : HttpParams) : string
local function RegisterPostMethod(url, func)
	methods["POST"][url] = func
end

---@param url string
local function UnregisterPostMethod(url)
	methods["POST"][url] = nil
end

local errTrace = ""
local function traceback(err)
	errTrace = debug.traceback(err, 2)
end

local function Loop(client)
	local datas = readHttpData(client)
	log_info(datas)

	local method = datas["method"]
	local url = datas["url"]

	local m = getMethod(method, url)
	if m then
		errTrace = ""
		local res, info = xpcall(function() return m(datas) end, traceback)
		if res then
			client:send("HTTP/1.1 200 OK\r\n\r\n")
			client:send(tostring(info))
		else
			client:send("HTTP/1.1 500 Internal Server Error\r\n\r\n")
			client:send(errTrace)
		end
	else
		client:send("HTTP/1.1 403 Forbidden\r\n\r\n")
	end
end

local function poll(server)
	local client = server:accept()
	if not client then
		return
	end

	local res,info = pcall(Loop, client)
	if not res then
		client:send("HTTP/1.1 500 Internal Server Error\r\n\r\n")
		client:send(info)
	end
	client:close()
end

---@param ip string
---@param host number
local function Listener(ip, host, isDebug)
    if not ip then
	   ip = "0.0.0.0"
	end

	if not host then
	   host = 9000
	end
	is_debug = isDebug

	local server = socket.tcp()
	-- 创建一个TCP服务器并监听指定端口
	server:settimeout(0)  -- 设置超时为0，防止阻塞
	server:bind(ip, host)
	server:listen()

	log_info("Server listening at http://%s:%d/", ip, host)

	while true do
		poll(server)
		coroutine.yield()
	end
end

local function Start(ip, host, isDebug)
	local co = coroutine.create(function()
		Listener(ip, host, isDebug)
	end)

	return co
end

local function Loop(co)
	return coroutine.resume(co)
end

return {
    ["Start"] = Start,
	["Loop"] = Loop,

	["RegisterGetMethod"] = RegisterGetMethod,
	["UnRegisterGetMethod"] = UnRegisterGetMethod,

	["RegisterPostMethod"] = RegisterPostMethod,
	["UnregisterPostMethod"] = UnregisterPostMethod,
}
)";

const char* http_server_main_loop = R"(
local http_server = require("hack_http_server")
local load = hack_lua_load

---@param request HttpParams
http_server.RegisterGetMethod("/testerror", function (request)
    error("test error")
end)

---@param request HttpParams
http_server.RegisterPostMethod("/dolua", function (request)
    local code = request.body
    local fun = load(code)

    return fun()
end)

-- 使用协程的示例
local co = http_server.Start("*", 5548, true)
local loop = http_server.Loop

local function mainLoop()
    loop(co)
end

return mainLoop
)";

using namespace ImGui;

static int   g_GlHeight, g_GlWidth;
static bool  g_IsSetup = false;
static std::string          g_IniFileName = "";
static std::string lua_data_dir = "";
static std::string lua_dump_g = "";
static void  Strtrim(char* s) { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }
static char InputBuf[1024];
static std::mutex mutex_;
static volatile int needDump = 0;

void DrawMenu()
{
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    {
        ImGuiIO &io = ImGui::GetIO();

        ImGui::Begin("Cheat");

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Menu")) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Tools")){
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }


        static ImGuiSliderFlags flags = ImGuiSliderFlags_None;
        if(ImGui::Button("God Mod"))
        {
        }

        if(ImGui::Button("Mod Hit 999"))
        {
        }

        ImGui::End();
    }
}

void SetupImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    memset(InputBuf, 0, 1024);

    io.IniFilename = g_IniFileName.c_str();
    io.DisplaySize = ImVec2((float)g_GlWidth, (float)g_GlHeight);

    ImGui_ImplAndroid_Init(nullptr);
    ImGui_ImplOpenGL3_Init("#version 300 es");
    ImGui::StyleColorsLight();
    ImGui::GetStyle().ScaleAllSizes(7.0f);

    io.Fonts->AddFontFromMemoryTTF(Roboto_Regular, 30, 30.0f);
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_GlWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_GlHeight);

    if (!g_IsSetup) {
        SetupImGui();
        g_IsSetup = true;
    }

    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(g_GlWidth, g_GlHeight);
    ImGui::NewFrame();

    DrawMenu();
    //ImGui::ShowDemoWindow();

    ImGui::EndFrame();
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}


std::string replaceAllChars(const std::string& input, char targetChar, const std::string& replacement) {
    std::string result = input;
    size_t pos = result.find(targetChar);
    while (pos != std::string::npos) {
        result.replace(pos, 1, replacement);
        pos = result.find(targetChar, pos + replacement.length());
    }
    return result;
}

HOOK_DEF(lua_State *, luaL_newstate, void) {
    lua_State * L = orig_luaL_newstate();
    LOGI("luaL_newstate is hooked");
    return L;
}

int hack_lua_print(lua_State* L)
{
    int argNum = lua_gettop(L);
    if(argNum == 1)
    {
        const char* info = luaL_checkstring(L, -1);
        LOGI("%s", info);
    }

    return 0;
}

int hack_lua_load(lua_State* L)
{
    size_t size;
    const char* code = luaL_checklstring(L, -1, &size);
    luaL_loadbuffer(L, code, size, "doluaChunk");
    //luaL_loadstring(L, code);

    return 1;
}

const std::string Il2CppString_to_utf8(const Il2CppString * str) {
    const wchar_t* utf16_string = (const wchar_t*)((int32_t*)str + 1);

    // 使用 std::wstring_convert 进行转换
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
    std::string utf8_string = converter.to_bytes(utf16_string);

    // 返回 utf8_string 内部的 const char* 指针
    return utf8_string;
}

HOOK_DEF(int, luaL_loadbuffer_csharp, void *L, const char *buff, int sz, const Il2CppString * name, void* method) {

    LOGI("%s,size:%d", "tolua_loadbuffer_csharp called", sz);
    LOGI("tolua load file:%s", Il2CppString_to_utf8(name).c_str());
    /*
    std::string strName = replaceAllChars(std::string(name), '.', "/");
    strName = replaceAllChars(strName, '\\', "/");

    const char *lua_file = std::string(lua_data_dir).append(strName ).c_str();
    struct stat st;
    if (stat(lua_file, &st) == -1) {
        // Directory does not exist, create it
        if (mkdir(lua_file, 0777) == -1) {
            LOGW("Failed to create directory.\n");
            return 1;
        }
        LOGW("Directory created successfully.\n");
    } else {
        LOGW("Directory already exists.\n");
    }
    auto f = fopen(lua_file, "wb");
    if(f)
    {
        fwrite(buff, 1, sz, f);
        fclose(f);
    }*/

    return orig_luaL_loadbuffer_csharp(L, buff, sz, name, method);
    //std::ofstream outStream(lua_file);
    //outStream <<
}

HOOK_DEF(int, luaL_loadfile, lua_State *L, const char *filename)
{
    LOGI("tolua load file:%s", filename);

    return orig_luaL_loadfile(L, filename);
}

HOOK_DEF(int, lua_pcall, lua_State* L, int nargs, int nresults, int errfunc) {

    if(g_L == nullptr)
    {
        g_L = L;
        LOGI("begin inject lua");
        auto path = lua_data_dir.c_str();
        int oldTop = lua_gettop(L);

        // file路径
        LOGI("begin inject lua ZygiskDir path");
        lua_pushstring(L, path);
        lua_setglobal(L, "ZygiskDir");
        LOGI("ZygiskDir success");

        lua_pushcfunction(L, hack_lua_print);
        lua_setglobal(L, "hack_lua_print");

        lua_pushcfunction(L, hack_lua_load);
        lua_setglobal(L, "hack_lua_load");

        if(luaopen_socket_core != NULL)
        {
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "preload");
            lua_pushcfunction(L, luaopen_socket_core);
            lua_setfield(L, -2, "socket.core");
            lua_pop(L, 2);

            LOGI("begin inject lua hack_print_utils");
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "loaded");

            luaL_loadbuffer(L, print_utils, (size_t)strlen(print_utils), "hack_print_utils");
            lua_call(L, 0, 1);
            lua_setfield(L, -2, "hack_print_utils");

            lua_settop(L, oldTop);
            LOGI("hack_print_utils success");

            LOGI("begin inject lua http_server");
            lua_getglobal(L, "package");
            lua_getfield(L, -1, "loaded");

            tolua_loadbuffer(L, http_server, strlen(http_server), "hack_http_server");
            lua_call(L, 0, 1);
            lua_setfield(L, -2, "hack_http_server");

            lua_settop(L, oldTop);
            LOGI("http_server success");

            LOGI("begin inject lua http_server mainLoop");
            luaL_loadbuffer(L, http_server_main_loop, (size_t)strlen(http_server_main_loop), "hack_http_server_main_loop");
            lua_call(L, 0, 1);
            lua_setglobal(L, "hack_http_server_main_loop");
            LOGI("mainLoop success");
        }

        lua_settop(L, oldTop);
    }

    if(clock() - start >= CLOCKS_PER_SEC / 5)
    {
        int oldTop = lua_gettop(L);
        lua_getglobal(L, "hack_http_server_main_loop");
        lua_call(L, 0, 0);
        lua_settop(L, oldTop);

        start = clock();
    }

    return orig_lua_pcall(L, nargs, nresults, errfunc);
}

HOOK_DEF(int, lua_load, lua_State *L, lua_Reader reader, void *data, const char *chunkname) {
    return orig_lua_load(L, reader, data, chunkname);
}


HOOK_DEF(void, Input, void *thiz, void *ex_ab, void *ex_ac)
{
    orig_Input(thiz, ex_ab, ex_ac);
    ImGui_ImplAndroid_HandleInputEvent((AInputEvent *)thiz);
    return;
}

static void common_handler(RegisterContext *ctx, const HookEntryInfo *info) {
    LOGI("tolua Loader");
}


void hack_start(const char *game_data_dir) {
    bool load = false;
    g_IniFileName = std::string(game_data_dir) + "/files/imgui.ini";
    lua_data_dir = std::string(game_data_dir).append("/files");

    while(true) {
        void *handle = xdl_open("libil2cpp.so", 0);
        if (handle) {
            load = true;
            il2cpp_api_init(handle);
            il2cpp_dump(game_data_dir);
            break;
        } else {
            sleep(0);
        }
    }
    if (!load) {
        LOGI("libil2cpp.so not found in thread %d", gettid());
    }

    sleep(1);
    LOGI("libtolua.so start %d", gettid());
    load = false;
    while(true) {
        void* handle = xdl_open("libtolua.so", 0);

        if (handle) {
            LOGI("libtolua.so start dl_open %ld", (long)handle);
            load = true;
            LOGI("DobbyHooked %ld", (long)handle);
            SetupLuaAPI(handle);

            HOOK(e_lua_pcall, new_lua_pcall, orig_lua_pcall)
            break;
        }
        else {
            sleep(0);
        }
    }
    if (!load) {
        LOGI("libtolua.so not found in thread %d", gettid());
    }

}

std::string GetLibDir(JavaVM *vms) {
    JNIEnv *env = nullptr;
    vms->AttachCurrentThread(&env, nullptr);
    jclass activity_thread_clz = env->FindClass("android/app/ActivityThread");
    if (activity_thread_clz != nullptr) {
        jmethodID currentApplicationId = env->GetStaticMethodID(activity_thread_clz,
                                                                "currentApplication",
                                                                "()Landroid/app/Application;");
        if (currentApplicationId) {
            jobject application = env->CallStaticObjectMethod(activity_thread_clz,
                                                              currentApplicationId);
            jclass application_clazz = env->GetObjectClass(application);
            if (application_clazz) {
                jmethodID get_application_info = env->GetMethodID(application_clazz,
                                                                  "getApplicationInfo",
                                                                  "()Landroid/content/pm/ApplicationInfo;");
                if (get_application_info) {
                    jobject application_info = env->CallObjectMethod(application,
                                                                     get_application_info);
                    jfieldID native_library_dir_id = env->GetFieldID(
                            env->GetObjectClass(application_info), "nativeLibraryDir",
                            "Ljava/lang/String;");
                    if (native_library_dir_id) {
                        auto native_library_dir_jstring = (jstring) env->GetObjectField(
                                application_info, native_library_dir_id);
                        auto path = env->GetStringUTFChars(native_library_dir_jstring, nullptr);
                        LOGI("lib dir %s", path);
                        std::string lib_dir(path);
                        env->ReleaseStringUTFChars(native_library_dir_jstring, path);
                        return lib_dir;
                    } else {
                        LOGE("nativeLibraryDir not found");
                    }
                } else {
                    LOGE("getApplicationInfo not found");
                }
            } else {
                LOGE("application class not found");
            }
        } else {
            LOGE("currentApplication not found");
        }
    } else {
        LOGE("ActivityThread not found");
    }
    return {};
}

static std::string GetNativeBridgeLibrary() {
    auto value = std::array<char, PROP_VALUE_MAX>();
    __system_property_get("ro.dalvik.vm.native.bridge", value.data());
    return {value.data()};
}

struct NativeBridgeCallbacks {
    uint32_t version;
    void *initialize;

    void *(*loadLibrary)(const char *libpath, int flag);

    void *(*getTrampoline)(void *handle, const char *name, const char *shorty, uint32_t len);

    void *isSupported;
    void *getAppEnv;
    void *isCompatibleWith;
    void *getSignalHandler;
    void *unloadLibrary;
    void *getError;
    void *isPathSupported;
    void *initAnonymousNamespace;
    void *createNamespace;
    void *linkNamespaces;

    void *(*loadLibraryExt)(const char *libpath, int flag, void *ns);
};

bool riru_hide(const std::set<std::string_view> &names) {
    procmaps_iterator *maps = pmparser_parse(-1);
    if (maps == nullptr) {
        LOGE("cannot parse the memory map");
        return false;
    }

    procmaps_struct **data = nullptr;
    size_t data_count = 0;
    procmaps_struct *maps_tmp;
    while ((maps_tmp = pmparser_next(maps)) != nullptr) {
        bool matched = false;
        matched = names.count(maps_tmp->pathname);

        if (!matched) continue;

        auto start = (uintptr_t) maps_tmp->addr_start;
        auto end = (uintptr_t) maps_tmp->addr_end;
        if (maps_tmp->is_r) {
            if (data) {
                data = (procmaps_struct **) realloc(data,
                                                    sizeof(procmaps_struct *) * (data_count + 1));
            } else {
                data = (procmaps_struct **) malloc(sizeof(procmaps_struct *));
            }
            data[data_count] = maps_tmp;
            data_count += 1;
        }
        LOGD("%ld-%ld %s %ld %s", (long)start, (long)end, maps_tmp->perm, maps_tmp->offset,maps_tmp->pathname);
    }

    auto fd = open(data[0]->pathname, O_RDONLY);
    struct stat sb;
    if (fstat(fd, &sb) == -1)
        LOGE("fstat");
    auto fileLength = sb.st_size;
    size_t copySize = 0;
    for (int i = 0; i < data_count; ++i) {
        auto procstruct = data[i];
        auto start = (uintptr_t) procstruct->addr_start;
        auto end = (uintptr_t) procstruct->addr_end;
        auto length = end - start;
        copySize += length;
    }
    LOGI("file length : %jd", fileLength);
    LOGI("copySize : %zu", copySize);
    auto backup_address = (uintptr_t) FAILURE_RETURN(
            mmap(nullptr, copySize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, fd, 0),
            MAP_FAILED);
    close(fd);

    for (int i = 0; i < data_count; ++i) {
        auto procstruct = data[i];
        auto start = (uintptr_t) procstruct->addr_start;
        auto end = (uintptr_t) procstruct->addr_end;
        auto length = end - start;
        int prot = get_prot(procstruct);

        // backup
        //LOGD("%" PRIxPTR"-%" PRIxPTR" %s %ld %s is backup to %" PRIxPTR, start, end, procstruct->perm, procstruct->offset, procstruct->pathname, backup_address);

        if (!procstruct->is_r) {
            LOGD("mprotect +r");
            FAILURE_RETURN(mprotect((void *) start, length, prot | PROT_READ), -1);
        }
        LOGD("memcpy -> backup");
        memcpy((void *) backup_address, (void *) start, length);

        // munmap original
        LOGD("munmap original");
        FAILURE_RETURN(munmap((void *) start, length), -1);

        // restore
        LOGD("mmap original");
        FAILURE_RETURN(mmap((void *) start, length, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0),
                       MAP_FAILED);
        LOGD("mprotect +w");
        FAILURE_RETURN(mprotect((void *) start, length, prot | PROT_WRITE), -1);
        LOGD("memcpy -> original");
        memcpy((void *) start, (void *) backup_address, length);
        if (!procstruct->is_w) {
            LOGD("mprotect -w");
            FAILURE_RETURN(mprotect((void *) start, length, prot), -1);
        }

        LOGD("mprotect backup");
        FAILURE_RETURN(mprotect((void *) backup_address, length, prot), -1);
        backup_address += length;
    }

    if (data) free(data);
    pmparser_free(maps);
    return true;
}

HOOK_DEF(void*, dlopen, const char *filename, int flags)
{
    if(strstr(filename, "libtolua.so"))
    {
        LOGW("hide so %s", filename);
        std::set<std::string_view> names = {"libtolua.so"};
        riru_hide(names);
    }
    return orig_dlopen(filename, flags);
}


bool NativeBridgeLoad(const char *game_data_dir, int api_level, void *data, size_t length) {
    //TODO 等待houdini初始化
    sleep(5);

    auto libart = dlopen("libart.so", RTLD_NOW);
    auto JNI_GetCreatedJavaVMs = (jint (*)(JavaVM **, jsize, jsize *)) dlsym(libart,
                                                                             "JNI_GetCreatedJavaVMs");
    LOGI("JNI_GetCreatedJavaVMs %p", JNI_GetCreatedJavaVMs);
    JavaVM *vms_buf[1];
    JavaVM *vms;
    jsize num_vms;
    jint status = JNI_GetCreatedJavaVMs(vms_buf, 1, &num_vms);
    if (status == JNI_OK && num_vms > 0) {
        vms = vms_buf[0];
    } else {
        LOGE("GetCreatedJavaVMs error");
        return false;
    }

    auto lib_dir = GetLibDir(vms);
    if (lib_dir.empty()) {
        LOGE("GetLibDir error");
        return false;
    }
    if (lib_dir.find("/lib/x86") != std::string::npos) {
        LOGI("no need NativeBridge");
        munmap(data, length);
        return false;
    }

    auto nb = dlopen("libhoudini.so", RTLD_NOW);
    if (!nb) {
        auto native_bridge = GetNativeBridgeLibrary();
        LOGI("native bridge: %s", native_bridge.data());
        nb = dlopen(native_bridge.data(), RTLD_NOW);
    }
    if (nb) {
        LOGI("nb %p", nb);
        auto callbacks = (NativeBridgeCallbacks *) dlsym(nb, "NativeBridgeItf");
        if (callbacks) {
            LOGI("NativeBridgeLoadLibrary %p", callbacks->loadLibrary);
            LOGI("NativeBridgeLoadLibraryExt %p", callbacks->loadLibraryExt);
            LOGI("NativeBridgeGetTrampoline %p", callbacks->getTrampoline);

            int fd = syscall(__NR_memfd_create, "anon", MFD_CLOEXEC);
            ftruncate(fd, (off_t) length);
            void *mem = mmap(nullptr, length, PROT_WRITE, MAP_SHARED, fd, 0);
            memcpy(mem, data, length);
            munmap(mem, length);
            munmap(data, length);
            char path[PATH_MAX];
            snprintf(path, PATH_MAX, "/proc/self/fd/%d", fd);
            LOGI("arm path %s", path);

            void *arm_handle;
            if (api_level >= 26) {
                arm_handle = callbacks->loadLibraryExt(path, RTLD_NOW, (void *) 3);
            } else {
                arm_handle = callbacks->loadLibrary(path, RTLD_NOW);
            }
            if (arm_handle) {
                LOGI("arm handle %p", arm_handle);
                auto init = (void (*)(JavaVM *, void *)) callbacks->getTrampoline(arm_handle,
                                                                                  "JNI_OnLoad",
                                                                                  nullptr, 0);
                LOGI("JNI_OnLoad %p", init);
                init(vms, (void *) game_data_dir);
                return true;
            }
            close(fd);
        }
    }
    return false;
}

void hack_prepare(const char *game_data_dir, void *data, size_t length) {

    LOGI("hack thread: %d", gettid());
    int api_level = android_get_device_api_level();
    LOGI("api level: %d", api_level);

#if defined(__i386__) || defined(__x86_64__)
    LOGI("__i386__  __x86_64__");
    if (!NativeBridgeLoad(game_data_dir, api_level, data, length)) {
#endif
        hack_start(game_data_dir);
#if defined(__i386__) || defined(__x86_64__)
    }
    //HOOK(dlopen, new_dlopen, orig_dlopen);

    while(true) {
        LOGI("hook libinput.so");
        void *sym_input = DobbySymbolResolver("/system/lib/libinput.so", "_ZN7android13InputConsumer21initializeMotionEventEPNS_11MotionEventEPKNS_12InputMessageE");
        if (NULL != sym_input){
            DobbyHook((void *)sym_input, (void *) new_Input, (void **)&orig_Input);
            LOGW("input hook %ld\n", (long)orig_Input);
        }
        else
        {
            sleep(1);
            continue;
        }

        auto eglhandle = xdl_open("libEGL.so", 0);
        if (eglhandle) {
            LOGI("libEGL.so start dl_open %ld", (long)eglhandle);
            auto eglSwapBuffers = xdl_sym(eglhandle, "eglSwapBuffers", nullptr);
            LOGI("eglSwapBuffers start %ld", (long)eglSwapBuffers);
            DobbyHook((void *) eglSwapBuffers, (void *) hook_eglSwapBuffers,
                      (void **) &old_eglSwapBuffers);

            xdl_close(eglhandle);
            break;
        }
        else {
            sleep(1);
        }
    }
#endif
}

#if defined(__arm__) || defined(__aarch64__)

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    auto game_data_dir = (const char *) reserved;
    std::thread hack_thread(hack_start, game_data_dir);
    hack_thread.detach();
    return JNI_VERSION_1_6;
}

#endif