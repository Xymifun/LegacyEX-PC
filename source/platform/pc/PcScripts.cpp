#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcScriptsInternal.hpp"

#include "platform/pc/PcContent.hpp"
#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcOs.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

namespace PcScripts
{
namespace
{
const size_t MEMORY_LIMIT = 64 * 1024 * 1024;
const unsigned int HANDLER_TIME_MS = 200;
const unsigned int LOAD_TIME_MS = 2000;
const unsigned int HOOK_EVERY = 10000;
const unsigned int MAX_ERRORS = 10;
const size_t MAX_SCRIPT_SIZE = 4 * 1024 * 1024;
const size_t MAX_COMMAND_LENGTH = 32;

const char* const sideNames[eSide_Count] = {"client", "host", "content"};
const char* const scriptFiles[eSide_Count] = {"client.lua", "host.lua", "content.lua"};

char handlersKey;
char commandsKey;
char modulesKey;

const char* const* sideEvents[eSide_Count];
int sideEventCounts[eSide_Count];

struct CommandEntry
{
    State* state;
    std::string help;
};
std::map<std::string, CommandEntry> registeredCommands[eSide_Count];

const char PRELUDE_LUA[] = R"LUA(
local traceback, report = ...
local on = events.on
local timers, count, nextId, ticking = {}, 0, 0, false

local function tick()
    if count == 0 then
        return
    end
    local due = {}
    for id, entry in pairs(timers) do
        entry.left = entry.left - 1
        if entry.left <= 0 then
            due[#due + 1] = id
        end
    end
    table.sort(due)
    for _, id in ipairs(due) do
        local entry = timers[id]
        if entry then
            if entry.every then
                entry.left = entry.every
            else
                timers[id] = nil
                count = count - 1
            end
            local ok, message = xpcall(entry.fn, traceback)
            if not ok then
                report("timer", message)
            end
        end
    end
end

local function add(ticks, fn, every)
    if type(ticks) ~= "number" or ticks < 1 then
        error("a timer needs a number of ticks from 1", 3)
    end
    if type(fn) ~= "function" then
        error("a timer needs a function", 3)
    end
    ticks = math.floor(ticks)
    if not ticking then
        ticking = true
        on("tick", tick)
    end
    nextId = nextId + 1
    timers[nextId] = {left = ticks, every = every and ticks or nil, fn = fn}
    count = count + 1
    return nextId
end

timer = {}
function timer.after(ticks, fn)
    return add(ticks, fn, false)
end
function timer.every(ticks, fn)
    return add(ticks, fn, true)
end
function timer.cancel(id)
    if timers[id] then
        timers[id] = nil
        count = count - 1
    end
end

local rawLoad = load
function load(chunk, name, mode, ...)
    if select("#", ...) > 0 then
        return rawLoad(chunk, name, "t", ...)
    end
    return rawLoad(chunk, name, "t")
end

local rawCollect = collectgarbage
function collectgarbage(option, ...)
    if option == nil or option == "collect" or option == "count" or option == "step" then
        return rawCollect(option, ...)
    end
    error("collectgarbage(\"" .. tostring(option) .. "\") is not available to mods", 2)
end
)LUA";

void* Allocate(void* ud, void* ptr, size_t oldSize, size_t newSize)
{
    State* state = static_cast<State*>(ud);
    const size_t old = ptr != 0 ? oldSize : 0;
    if (newSize == 0)
    {
        free(ptr);
        state->memory -= old;
        return 0;
    }
    if (newSize > old && state->memory - old + newSize > MEMORY_LIMIT)
        return 0;
    void* p = realloc(ptr, newSize);
    if (p != 0)
        state->memory = state->memory - old + newSize;
    return p;
}

void Hook(lua_State* L, lua_Debug*)
{
    State* s = StateOf(L);
    if (s->deadline != 0 && PcOsMonotonicNanoseconds() > s->deadline)
        luaL_error(L, "the script ran for longer than %d ms", (int)s->budgetMilliseconds);
}

void StartBudget(State* state, unsigned int ms)
{
    state->budgetMilliseconds = ms;
    state->deadline = PcOsMonotonicNanoseconds() + (unsigned long long)ms * 1000000ULL;
}

int Traceback(lua_State* L)
{
    const char* msg = lua_tostring(L, 1);
    if (msg == 0)
    {
        if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
            return 1;
        msg = lua_pushfstring(L, "(an error object of type %s)", luaL_typename(L, 1));
    }
    luaL_traceback(L, L, msg, 1);
    return 1;
}

int Report(lua_State* L)
{
    const char* where = luaL_optstring(L, 1, "script");
    const char* msg = luaL_optstring(L, 2, "?");
    ReportError(StateOf(L), where, msg);
    return 0;
}

int Print(lua_State* L)
{
    const int n = lua_gettop(L);
    luaL_Buffer buf;
    luaL_buffinit(L, &buf);
    for (int i = 1; i <= n; ++i)
    {
        if (i > 1)
            luaL_addchar(&buf, '\t');
        luaL_tolstring(L, i, 0);
        luaL_addvalue(&buf);
    }
    luaL_pushresult(&buf);
    State* s = StateOf(L);
    printf("[pc] mod %s (%s): %s\n", s->modId.c_str(), sideNames[s->side], lua_tostring(L, -1));
    fflush(stdout);
    return 0;
}

int EventId(State* s, const char* name)
{
    for (int i = 0; i < sideEventCounts[s->side]; ++i)
    {
        if (strcmp(sideEvents[s->side][i], name) == 0)
            return i;
    }
    return -1;
}

int EventsOn(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    State* s = StateOf(L);
    const int id = EventId(s, name);
    if (id < 0)
    {
        luaL_Buffer buf;
        luaL_buffinit(L, &buf);
        for (int i = 0; i < sideEventCounts[s->side]; ++i)
        {
            if (i > 0)
                luaL_addstring(&buf, ", ");
            luaL_addstring(&buf, sideEvents[s->side][i]);
        }
        luaL_pushresult(&buf);
        return luaL_error(L, "there is no %s event \"%s\" (there are: %s)", sideNames[s->side], name,
                          lua_tostring(L, -1));
    }
    lua_rawgetp(L, LUA_REGISTRYINDEX, &handlersKey);
    if (lua_rawgeti(L, -1, id + 1) != LUA_TTABLE)
    {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_rawseti(L, -3, id + 1);
    }
    lua_pushvalue(L, 2);
    lua_rawseti(L, -2, (lua_Integer)lua_rawlen(L, -2) + 1);
    s->handlerMask |= 1u << id;
    return 0;
}

bool ValidCommandName(const char* name)
{
    const size_t len = strlen(name);
    if (len == 0 || len > MAX_COMMAND_LENGTH)
        return false;
    for (size_t i = 0; i < len; ++i)
    {
        const char c = name[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-'))
            return false;
    }
    return true;
}

int CommandsRegister(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    const char* help = luaL_optstring(L, 3, "");
    if (!ValidCommandName(name))
        return luaL_error(L, "a command name is 1 to 32 lowercase letters, digits, '_' or '-' (not \"%s\")", name);
    State* s = StateOf(L);
    if (strcmp(name, "help") == 0)
        return luaL_error(L, "/help is the game's own command");
    std::map<std::string, CommandEntry>::iterator it = registeredCommands[s->side].find(name);
    if (it != registeredCommands[s->side].end() && it->second.state != s)
    {
        printf("[pc] scripts: %s: /%s is already %s's command; this one is not added\n", s->modId.c_str(), name,
               it->second.state->modId.c_str());
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_rawgetp(L, LUA_REGISTRYINDEX, &commandsKey);
    lua_pushvalue(L, 2);
    lua_setfield(L, -2, name);
    CommandEntry& e = registeredCommands[s->side][name];
    e.state = s;
    e.help = help;
    lua_pushboolean(L, 1);
    return 1;
}

bool ValidModuleName(const char* name)
{
    bool atStart = true;
    for (const char* c = name; *c != 0; ++c)
    {
        if (*c == '.')
        {
            if (atStart)
                return false;
            atStart = true;
            continue;
        }
        if (!((*c >= 'a' && *c <= 'z') || (*c >= 'A' && *c <= 'Z') || (*c >= '0' && *c <= '9') || *c == '_'
              || *c == '-'))
            return false;
        atStart = false;
    }
    return !atStart;
}

bool ReadFile(const std::string& path, std::string* text)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    char buf[16384];
    size_t n;
    text->clear();
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0 && text->size() <= MAX_SCRIPT_SIZE)
        text->append(buf, n);
    const bool ok = ferror(fp) == 0 && text->size() <= MAX_SCRIPT_SIZE;
    fclose(fp);
    return ok;
}

int LoadScriptFile(lua_State* L, State* state, const std::string& rel)
{
    std::string text;
    const std::string path = state->directory + "/scripts/" + rel;
    if (!ReadFile(path, &text))
    {
        lua_pushfstring(L, "cannot read scripts/%s (missing, or over 4 MB)", rel.c_str());
        return LUA_ERRFILE;
    }
    const std::string chunk = "@" + state->modId + "/scripts/" + rel;
    return luaL_loadbufferx(L, text.data(), text.size(), chunk.c_str(), "t");
}

int LoadModule(lua_State* L, State* state, const char* name)
{
    std::string rel = name;
    for (size_t i = 0; i < rel.size(); ++i)
    {
        if (rel[i] == '.')
            rel[i] = '/';
    }
    return LoadScriptFile(L, state, rel + ".lua");
}

int Require(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    if (!ValidModuleName(name))
        return luaL_error(L, "require(\"%s\"): a module name is parts of letters, digits, '_' and '-' "
                             "joined by '.' (\"util\" is scripts/util.lua, \"lib.json\" scripts/lib/json.lua)",
                          name);
    lua_settop(L, 1);
    lua_rawgetp(L, LUA_REGISTRYINDEX, &modulesKey);
    const int mods = lua_gettop(L);
    lua_getfield(L, mods, name);
    if (lua_toboolean(L, -1))
    {
        if (lua_islightuserdata(L, -1))
            return luaL_error(L, "require(\"%s\") while that module is still loading", name);
        return 1;
    }
    lua_pop(L, 1);
    if (LoadModule(L, StateOf(L), name) != LUA_OK)
        return lua_error(L);
    lua_pushlightuserdata(L, &modulesKey);
    lua_setfield(L, mods, name);
    lua_pushstring(L, name);
    lua_call(L, 1, 1);
    if (lua_isnil(L, -1))
    {
        lua_pop(L, 1);
        lua_pushboolean(L, 1);
    }
    lua_pushvalue(L, -1);
    lua_setfield(L, mods, name);
    return 1;
}

void Remove(lua_State* L, const char* lib, const char* const* names)
{
    lua_getglobal(L, lib);
    for (; *names != 0; ++names)
    {
        lua_pushnil(L);
        lua_setfield(L, -2, *names);
    }
    lua_pop(L, 1);
}

struct Setup
{
    OpenApiFn openApi;
};

int SetupState(lua_State* L)
{
    Setup* su = static_cast<Setup*>(lua_touserdata(L, 1));
    State* s = StateOf(L);

    static const luaL_Reg libs[] = {{LUA_GNAME, luaopen_base},       {LUA_COLIBNAME, luaopen_coroutine},
                                          {LUA_TABLIBNAME, luaopen_table}, {LUA_OSLIBNAME, luaopen_os},
                                          {LUA_STRLIBNAME, luaopen_string}, {LUA_MATHLIBNAME, luaopen_math},
                                          {LUA_UTF8LIBNAME, luaopen_utf8},  {0, 0}};
    for (const luaL_Reg* lib = libs; lib->name != 0; ++lib)
    {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);
    }
    static const char* const baseRemoved[] = {"dofile", "loadfile", 0};
    Remove(L, LUA_GNAME, baseRemoved);
    static const char* const osRemoved[] = {"execute", "exit", "getenv", "remove", "rename", "setlocale",
                                             "tmpname", 0};
    Remove(L, LUA_OSLIBNAME, osRemoved);
    static const char* const stringRemoved[] = {"dump", 0};
    Remove(L, LUA_STRLIBNAME, stringRemoved);
    lua_pushcfunction(L, Print);
    lua_setglobal(L, "print");
    lua_pushcfunction(L, Require);
    lua_setglobal(L, "require");

    lua_newtable(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &handlersKey);
    lua_newtable(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &commandsKey);
    lua_newtable(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &modulesKey);

    lua_newtable(L);
    lua_pushcfunction(L, EventsOn);
    lua_setfield(L, -2, "on");
    lua_setglobal(L, "events");
    lua_newtable(L);
    lua_pushcfunction(L, CommandsRegister);
    lua_setfield(L, -2, "register");
    lua_setglobal(L, "commands");

    lua_newtable(L);
    lua_pushstring(L, s->modId.c_str());
    lua_setfield(L, -2, "id");
    lua_pushstring(L, s->modName.c_str());
    lua_setfield(L, -2, "name");
    lua_pushstring(L, s->modVersion.c_str());
    lua_setfield(L, -2, "version");
    lua_pushstring(L, s->modSide.c_str());
    lua_setfield(L, -2, "side");
    lua_setglobal(L, "mod");

    lua_newtable(L);
    lua_pushstring(L, PcContentGameVersion(PcContentLocal()).c_str());
    lua_setfield(L, -2, "version");
    lua_pushinteger(L, kPcModApi);
    lua_setfield(L, -2, "api");
    lua_pushstring(L, sideNames[s->side]);
    lua_setfield(L, -2, "side");
    lua_setglobal(L, "game");

    su->openApi(L);

    if (luaL_loadbufferx(L, PRELUDE_LUA, sizeof(PRELUDE_LUA) - 1, "=prelude", "t") != LUA_OK)
        return lua_error(L);
    lua_pushcfunction(L, Traceback);
    lua_pushcfunction(L, Report);
    lua_call(L, 2, 0);
    return 0;
}

bool IsFile(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

State* OpenState(Side side, const PcMod& mod, OpenApiFn openApi)
{
    State* s = new State();
    s->side = side;
    s->modId = mod.id;
    s->modName = mod.name;
    s->modVersion = mod.version;
    s->modSide = mod.side;
    s->directory = mod.directory;
    s->memory = 0;
    s->deadline = 0;
    s->budgetMilliseconds = 0;
    s->errors = 0;
    s->handlerMask = 0;
    s->stopped = false;
    s->errorShown = false;
    s->L = lua_newstate(Allocate, s);
    if (s->L == 0)
    {
        printf("[pc] scripts: %s: cannot make a Lua state\n", mod.id.c_str());
        delete s;
        return 0;
    }
    lua_State* L = s->L;
    *static_cast<State**>(lua_getextraspace(L)) = s;
    lua_sethook(L, Hook, LUA_MASKCOUNT, HOOK_EVERY);

    Setup setup;
    setup.openApi = openApi;
    lua_pushcfunction(L, Traceback);
    lua_pushcfunction(L, SetupState);
    lua_pushlightuserdata(L, &setup);
    if (lua_pcall(L, 1, 0, 1) != LUA_OK)
    {
        printf("[pc] scripts: %s: cannot set up its Lua state: %s\n", mod.id.c_str(), lua_tostring(L, -1));
        lua_close(L);
        delete s;
        return 0;
    }
    lua_settop(L, 0);

    lua_pushcfunction(L, Traceback);
    int rc = LoadScriptFile(L, s, scriptFiles[side]);
    if (rc == LUA_OK)
    {
        StartBudget(s, LOAD_TIME_MS);
        rc = lua_pcall(L, 0, 0, 1);
        s->deadline = 0;
    }
    if (rc != LUA_OK)
    {
        ReportError(s, scriptFiles[side], lua_tostring(L, -1));
        s->stopped = true;
    }
    lua_settop(L, 0);
    return s;
}

int RunEvent(lua_State* L)
{
    Event* ev = static_cast<Event*>(lua_touserdata(L, 1));
    State* s = StateOf(L);
    lua_settop(L, 0);
    if (ev->mCommand != 0)
    {
        lua_rawgetp(L, LUA_REGISTRYINDEX, &commandsKey);
        lua_newtable(L);
        lua_getfield(L, 1, ev->mCommand);
        lua_rawseti(L, 2, 1);
    }
    else
    {
        lua_rawgetp(L, LUA_REGISTRYINDEX, &handlersKey);
        lua_rawgeti(L, 1, ev->mId + 1);
        if (!lua_istable(L, 2))
            return 0;
    }
    const lua_Integer n = (lua_Integer)lua_rawlen(L, 2);
    const char* where = ev->mCommand != 0 ? ev->mCommand : sideEvents[s->side][ev->mId];
    for (lua_Integer i = 1; i <= n && !s->stopped; ++i)
    {
        lua_settop(L, 2);
        lua_pushcfunction(L, Traceback);
        if (lua_rawgeti(L, 2, i) != LUA_TFUNCTION)
            continue;
        const int nargs = ev->Push(L);
        StartBudget(s, HANDLER_TIME_MS);
        const int rc = lua_pcall(L, nargs, 1, 3);
        s->deadline = 0;
        if (rc != LUA_OK)
        {
            ReportError(s, where, lua_tostring(L, -1));
            continue;
        }
        if (ev->Result(L, lua_gettop(L)))
        {
            ev->mStopped = true;
            break;
        }
    }
    lua_settop(L, 0);
    return 0;
}

bool RunIn(State* s, Event* ev)
{
    lua_State* L = s->L;
    const int top = lua_gettop(L);
    lua_pushcfunction(L, RunEvent);
    lua_pushlightuserdata(L, ev);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        ReportError(s, ev->mCommand != 0 ? ev->mCommand : sideEvents[s->side][ev->mId],
                    lua_tostring(L, -1));
    lua_settop(L, top);
    return ev->mStopped;
}
}

bool CancellableEvent::Result(lua_State* L, int idx)
{
    if (lua_isboolean(L, idx) && !lua_toboolean(L, idx))
    {
        mCancelled = true;
        return true;
    }
    return false;
}

bool ChatEvent::Result(lua_State* L, int idx)
{
    if (lua_type(L, idx) == LUA_TSTRING)
    {
        size_t len;
        const char* text = lua_tolstring(L, idx, &len);
        mText = Wide(text, len);
        return false;
    }
    return CancellableEvent::Result(L, idx);
}

void LoadStates(Side side, const char* const* names, int count, OpenApiFn openApi,
                std::vector<State*>* states)
{
    sideEvents[side] = names;
    sideEventCounts[side] = count;
    const std::vector<PcMod>& mods = PcModsLoaded();
    for (size_t i = 0; i < mods.size(); ++i)
    {
        const PcMod& m = mods[i];
        if (m.scriptFiles == 0)
            continue;
        const bool runs = side == eSide_Client ? m.side != "server" : m.side != "client";
        if (!IsFile(m.directory + "/scripts/" + scriptFiles[side]))
            continue;
        if (!runs)
        {
            printf("[pc] scripts: %s: scripts/%s is not run, as the mod's side is \"%s\"\n", m.id.c_str(),
                   scriptFiles[side], m.side.c_str());
            continue;
        }
        State* s = OpenState(side, m, openApi);
        if (s == 0)
            continue;
        states->push_back(s);
        printf("[pc] scripts: %s: %s %s (%u KB)\n", m.id.c_str(), scriptFiles[side],
               s->stopped ? "failed" : "running", (unsigned int)((s->memory + 1023) / 1024));
    }
    fflush(stdout);
}

bool RunOnce(Side side, const PcMod& mod, OpenApiFn openApi)
{
    sideEvents[side] = 0;
    sideEventCounts[side] = 0;
    State* s = OpenState(side, mod, openApi);
    if (s == 0)
        return false;
    const bool ok = !s->stopped;
    std::vector<State*> states(1, s);
    CloseStates(&states);
    return ok;
}

void CloseStates(std::vector<State*>* states)
{
    for (size_t i = 0; i < states->size(); ++i)
    {
        State* s = (*states)[i];
        std::map<std::string, CommandEntry>& commands = registeredCommands[s->side];
        for (std::map<std::string, CommandEntry>::iterator it = commands.begin(); it != commands.end();)
        {
            if (it->second.state == s)
                commands.erase(it++);
            else
                ++it;
        }
        lua_close(s->L);
        delete s;
    }
    states->clear();
}

bool AnyHandles(const std::vector<State*>& states, int id)
{
    for (size_t i = 0; i < states.size(); ++i)
    {
        if (!states[i]->stopped && (states[i]->handlerMask & (1u << id)) != 0)
            return true;
    }
    return false;
}

bool Dispatch(const std::vector<State*>& states, Event* ev)
{
    for (size_t i = 0; i < states.size(); ++i)
    {
        State* s = states[i];
        if (s->stopped || (s->handlerMask & (1u << ev->mId)) == 0)
            continue;
        if (RunIn(s, ev))
            return true;
    }
    return false;
}

bool RunCommand(Side side, const std::string& name, Event* ev)
{
    std::map<std::string, CommandEntry>::iterator it = registeredCommands[side].find(name);
    if (it == registeredCommands[side].end())
        return false;
    State* s = it->second.state;
    if (s->stopped)
        return false;
    ev->mCommand = it->first.c_str();
    RunIn(s, ev);
    return true;
}

std::map<std::string, std::string> Commands(Side side)
{
    std::map<std::string, std::string> list;
    for (std::map<std::string, CommandEntry>::const_iterator it = registeredCommands[side].begin();
         it != registeredCommands[side].end(); ++it)
    {
        if (!it->second.state->stopped)
            list[it->first] = it->second.help;
    }
    return list;
}

State* StateOf(lua_State* L)
{
    return *static_cast<State**>(lua_getextraspace(L));
}

void ReportError(State* s, const char* where, const char* msg)
{
    printf("[pc] scripts: %s (%s): error in %s: %s\n", s->modId.c_str(), sideNames[s->side], where,
           msg != 0 ? msg : "(no message)");
    if (++s->errors >= MAX_ERRORS && !s->stopped)
    {
        s->stopped = true;
        printf("[pc] scripts: %s (%s): stopped after %u errors\n", s->modId.c_str(), sideNames[s->side],
               s->errors);
    }
    fflush(stdout);
}

std::wstring Wide(const char* s, size_t len)
{
    std::wstring out;
    out.reserve(len);
    const unsigned char* b = reinterpret_cast<const unsigned char*>(s);
    size_t i = 0;
    while (i < len)
    {
        unsigned int cp = b[i];
        size_t extra = cp < 0x80 ? 0 : (cp & 0xE0) == 0xC0 ? 1 : (cp & 0xF0) == 0xE0 ? 2 : (cp & 0xF8) == 0xF0 ? 3 : 4;
        if (extra == 4 || i + extra >= len)
        {
            out.push_back(0xFFFD);
            ++i;
            continue;
        }
        if (extra > 0)
        {
            cp &= 0x3F >> extra;
            bool ok = true;
            for (size_t k = 1; k <= extra; ++k)
            {
                if ((b[i + k] & 0xC0) != 0x80)
                {
                    ok = false;
                    break;
                }
                cp = (cp << 6) | (b[i + k] & 0x3F);
            }
            static const unsigned int minimum[4] = {0, 0x80, 0x800, 0x10000};
            if (!ok || cp < minimum[extra] || cp > 0x10FFFF || (cp >= 0xD800 && cp < 0xE000))
            {
                out.push_back(0xFFFD);
                ++i;
                continue;
            }
        }
        i += extra + 1;
        if (cp >= 0x10000)
        {
            cp -= 0x10000;
            out.push_back((wchar_t)(0xD800 + (cp >> 10)));
            out.push_back((wchar_t)(0xDC00 + (cp & 0x3FF)));
        }
        else
            out.push_back((wchar_t)cp);
    }
    return out;
}

std::string Utf8(const std::wstring& s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        unsigned int cp = (unsigned short)s[i];
        if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < s.size()
            && (unsigned short)s[i + 1] >= 0xDC00 && (unsigned short)s[i + 1] < 0xE000)
        {
            cp = 0x10000 + ((cp - 0xD800) << 10) + ((unsigned short)s[i + 1] - 0xDC00);
            ++i;
        }
        else if (cp >= 0xD800 && cp < 0xE000)
            cp = 0xFFFD;
        if (cp < 0x80)
            out += (char)cp;
        else if (cp < 0x800)
        {
            out += (char)(0xC0 | (cp >> 6));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000)
        {
            out += (char)(0xE0 | (cp >> 12));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else
        {
            out += (char)(0xF0 | (cp >> 18));
            out += (char)(0x80 | ((cp >> 12) & 0x3F));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
    }
    return out;
}

void PushWide(lua_State* L, const std::wstring& text)
{
    const std::string s = Utf8(text);
    lua_pushlstring(L, s.data(), s.size());
}

std::wstring ArgWide(lua_State* L, int idx)
{
    size_t len;
    const char* text = luaL_checklstring(L, idx, &len);
    return Wide(text, len);
}

unsigned int ArgColour(lua_State* L, int idx, unsigned int fallback)
{
    if (lua_isnoneornil(L, idx))
        return fallback;
    const unsigned int c = (unsigned int)(luaL_checkinteger(L, idx) & 0xFFFFFFFF);
    return (c & 0xFF000000u) == 0 ? c | 0xFF000000u : c;
}

void SetFunctions(lua_State* L, const luaL_Reg* f)
{
    for (; f->name != 0; ++f)
    {
        lua_pushcfunction(L, f->func);
        lua_setfield(L, -2, f->name);
    }
}
}
