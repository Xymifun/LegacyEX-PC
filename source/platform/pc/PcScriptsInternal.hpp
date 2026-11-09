#ifndef PLATFORM_PC_PCSCRIPTSINTERNAL_HPP
#define PLATFORM_PC_PCSCRIPTSINTERNAL_HPP

#include <lua.hpp>

#include <map>
#include <string>
#include <vector>

struct PcMod;

namespace PcScripts
{
enum Side
{
    eSide_Client,
    eSide_Host,
    eSide_Content,
    eSide_Count
};

struct State
{
    lua_State* L;
    Side side;
    std::string modId;
    std::string modName;
    std::string modVersion;
    std::string modSide;
    std::string directory;
    size_t memory;
    unsigned long long deadline;
    unsigned int budgetMilliseconds;
    unsigned int errors;
    unsigned int handlerMask;
    bool stopped;
    bool errorShown;
};

class Event
{
public:
    Event(int id) : mId(id), mCommand(0), mStopped(false) {}
    virtual ~Event() {}
    virtual int Push(lua_State* L) = 0;
    virtual bool Result(lua_State* L, int index) { return false; }

    int mId;
    const char* mCommand;
    bool mStopped;
};

class CancellableEvent : public Event
{
public:
    CancellableEvent(int id) : Event(id), mCancelled(false) {}
    virtual bool Result(lua_State* L, int index);

    bool mCancelled;
};

class ChatEvent : public CancellableEvent
{
public:
    ChatEvent(int id, const std::wstring& text) : CancellableEvent(id), mText(text) {}
    virtual bool Result(lua_State* L, int index);

    std::wstring mText;
};

typedef void (*OpenApiFn)(lua_State* L);

void LoadStates(Side side, const char* const* eventNames, int eventCount, OpenApiFn openApi,
                std::vector<State*>* states);
void CloseStates(std::vector<State*>* states);
bool RunOnce(Side side, const PcMod& mod, OpenApiFn openApi);

bool AnyHandles(const std::vector<State*>& states, int eventId);
bool Dispatch(const std::vector<State*>& states, Event* event);
bool RunCommand(Side side, const std::string& name, Event* event);
std::map<std::string, std::string> Commands(Side side);

State* StateOf(lua_State* L);
void ReportError(State* state, const char* where, const char* message);

std::wstring Wide(const char* text, size_t length);
std::string Utf8(const std::wstring& text);
void PushWide(lua_State* L, const std::wstring& text);
std::wstring ArgWide(lua_State* L, int index);
unsigned int ArgColour(lua_State* L, int index, unsigned int fallback);
void SetFunctions(lua_State* L, const luaL_Reg* functions);
}

#endif
