#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcScriptsInternal.hpp"

#include "platform/pc/PcDedicated.hpp"

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/PlayerList.hpp"

using namespace PcScripts;

namespace
{
enum HostEvent
{
    eHost_ServerStart,
    eHost_ServerStop,
    eHost_Tick,
    eHost_PlayerJoin,
    eHost_PlayerLeave,
    eHost_Chat,
    eHost_BlockBreak,
    eHost_BlockPlace,
    eHost_Count
};
const char* const hostEvents[eHost_Count] = {
    "server_start",
    "server_stop",
    "tick",
    "player_join",
    "player_leave",
    "chat",
    "block_break",
    "block_place"};

const char PLAYER_TYPE[] = "pc.player";
const size_t CHAT_CHUNK = 120;
const int MAX_GIVE = 64 * 36;

std::vector<State*> hostStates;
bool hostRunning = false;
pthread_t serverThread;
unsigned long long tickCount = 0;
char playerHandlesKey;

bool Active()
{
    return hostRunning && !hostStates.empty() && pthread_equal(pthread_self(), serverThread);
}

INetworkPlayer* NetworkPlayer(ServerPlayer* p)
{
    return p->mConnection ? p->mConnection->getNetworkPlayer() : 0;
}

bool Hidden(ServerPlayer* p)
{
    return PcIsHiddenHost(NetworkPlayer(p));
}

PlayerList* Players()
{
    return gMinecraftServer != 0 ? gMinecraftServer->getPlayers() : 0;
}

ServerPlayer* FindPlayer(const std::wstring& uid, const std::wstring& name)
{
    PlayerList* all = Players();
    if (all == 0)
        return 0;
    const std::vector<boost::shared_ptr<ServerPlayer> >& list = *all->getPlayers();
    for (size_t i = 0; i < list.size(); ++i)
    {
        ServerPlayer* p = list[i].get();
        if (p->getName() == name && p->mXuid.toString() == uid)
            return p;
    }
    return 0;
}

void SendText(ServerPlayer* player, const std::wstring& text, bool everyone)
{
    for (size_t pos = 0; pos < text.size();)
    {
        size_t len = std::min(CHAT_CHUNK, text.size() - pos);
        if (pos + len < text.size())
        {
            const size_t sp = text.rfind(L' ', pos + len);
            if (sp != std::wstring::npos && sp > pos + len / 2)
                len = sp - pos + 1;
            else if (text[pos + len - 1] >= 0xD800 && text[pos + len - 1] < 0xDC00)
                --len;
        }
        boost::shared_ptr<Packet> pkt(new ClientboundChatPacket(
            text.substr(pos, len), ClientboundChatPacket::eChatPacketMessage_Text, -1));
        if (everyone)
        {
            if (Players() != 0)
                Players()->broadcastAll(pkt);
        }
        else if (player->mConnection)
            player->mConnection->send(pkt);
        pos += len;
    }
}

std::wstring Clean(const std::wstring& s)
{
    std::wstring out;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 0x20 && s[i] != 0x7F)
            out.push_back(s[i]);
    }
    return out;
}

void PushPlayer(lua_State* L, ServerPlayer* p)
{
    const std::string uid = Utf8(p->mXuid.toString());
    const std::string name = Utf8(p->getName());
    const std::string key = uid + "\n" + name;
    lua_rawgetp(L, LUA_REGISTRYINDEX, &playerHandlesKey);
    if (lua_getfield(L, -1, key.c_str()) == LUA_TUSERDATA)
    {
        lua_remove(L, -2);
        return;
    }
    lua_pop(L, 1);
    lua_newuserdatauv(L, 0, 2);
    lua_pushlstring(L, uid.data(), uid.size());
    lua_setiuservalue(L, -2, 1);
    lua_pushlstring(L, name.data(), name.size());
    lua_setiuservalue(L, -2, 2);
    luaL_setmetatable(L, PLAYER_TYPE);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, key.c_str());
    lua_remove(L, -2);
}

ServerPlayer* ArgPlayer(lua_State* L, int idx)
{
    luaL_checkudata(L, idx, PLAYER_TYPE);
    lua_getiuservalue(L, idx, 1);
    lua_getiuservalue(L, idx, 2);
    size_t uidLen;
    size_t nameLen;
    const char* uid = lua_tolstring(L, -2, &uidLen);
    const char* name = lua_tolstring(L, -1, &nameLen);
    ServerPlayer* p = FindPlayer(Wide(uid, uidLen), Wide(name, nameLen));
    lua_pop(L, 2);
    return p;
}

ServerLevel* ArgLevel(lua_State* L, int idx)
{
    const lua_Integer dim = luaL_optinteger(L, idx, 0);
    if (dim != 0 && dim != -1 && dim != 1)
        luaL_error(L, "dimension %d is not 0 (overworld), -1 (nether) or 1 (end)", (int)dim);
    return gMinecraftServer != 0 ? gMinecraftServer->getLevel((int)dim) : 0;
}

int DimensionOf(Level* lvl)
{
    return lvl->mDimension->getType()->getId();
}

void PushBlock(lua_State* L, const BlockState* bs)
{
    Block* b = bs->getBlock();
    PushWide(L, Block::blockRegistry->getKey(b).toString());
    lua_pushinteger(L, Block::getIdWithData(bs) >> 12);
}

bool InBuildHeight(Level* lvl, int y)
{
    return y >= 0 && y < lvl->getMaxBuildHeight();
}

int PlayerName(lua_State* L)
{
    luaL_checkudata(L, 1, PLAYER_TYPE);
    lua_getiuservalue(L, 1, 2);
    return 1;
}

int PlayerUid(lua_State* L)
{
    luaL_checkudata(L, 1, PLAYER_TYPE);
    lua_getiuservalue(L, 1, 1);
    return 1;
}

int PlayerOnline(lua_State* L)
{
    lua_pushboolean(L, ArgPlayer(L, 1) != 0);
    return 1;
}

int PlayerPosition(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    if (p == 0)
        return 0;
    lua_pushnumber(L, p->mX);
    lua_pushnumber(L, p->mY);
    lua_pushnumber(L, p->mZ);
    return 3;
}

int PlayerDimension(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    if (p == 0)
        return 0;
    lua_pushinteger(L, p->mDimension);
    return 1;
}

int PlayerMessage(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    luaL_checkany(L, 2);
    lua_settop(L, 2);
    luaL_tolstring(L, 2, 0);
    if (p != 0)
        SendText(p, Clean(ArgWide(L, -1)), false);
    lua_pushboolean(L, p != 0);
    return 1;
}

int PlayerTeleport(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    const double x = luaL_checknumber(L, 2);
    const double y = luaL_checknumber(L, 3);
    const double z = luaL_checknumber(L, 4);
    if (p == 0 || !p->mConnection || x != x || y != y || z != z || x < -3.0e7 || x > 3.0e7 || z < -3.0e7
        || z > 3.0e7 || y < -512.0 || y > 1024.0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    p->teleportTo(x, y, z);
    lua_pushboolean(L, 1);
    return 1;
}

int PlayerGive(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    luaL_checkstring(L, 2);
    const lua_Integer n = luaL_optinteger(L, 3, 1);
    const lua_Integer data = luaL_optinteger(L, 4, 0);
    if (n < 1 || n > MAX_GIVE || data < 0 || data > 0x7FFF)
        return luaL_error(L, "give: a count from 1 to %d and a data value from 0 to 32767", MAX_GIVE);
    if (p == 0)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const ResourceLocation loc(ArgWide(L, 2));
    if (!Item::itemRegistry.containsKey(loc))
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    Item* item = Item::itemRegistry.get(loc);
    int left = (int)n;
    const int stack = std::max(1, item->getMaxStackSize());
    while (left > 0)
    {
        const int amount = std::min(left, stack);
        not_null_ptr<ItemInstance> inst(new ItemInstance(item, amount, (int)data));
        p->mInventory->add(inst);
        if (!inst->isEmpty() && inst->getCount() > 0)
            p->drop(inst, false);
        left -= amount;
    }
    lua_pushboolean(L, 1);
    return 1;
}

int PlayerHealth(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    if (p == 0)
        return 0;
    lua_pushnumber(L, p->getHealth());
    lua_pushnumber(L, p->getMaxHealth());
    return 2;
}

int PlayerSetHealth(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    const lua_Number hp = luaL_checknumber(L, 2);
    if (p == 0 || !p->isAlive() || hp != hp)
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const float maxHp = p->getMaxHealth();
    p->setHealth(hp < 1.0 ? 1.0f : hp > maxHp ? maxHp : (float)hp);
    lua_pushboolean(L, 1);
    return 1;
}

int PlayerKill(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    if (p != 0 && p->isAlive())
        p->kill();
    lua_pushboolean(L, p != 0);
    return 1;
}

int PlayerIsHost(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    INetworkPlayer* net = p != 0 ? NetworkPlayer(p) : 0;
    lua_pushboolean(L, net != 0 && net->IsHost());
    return 1;
}

int PlayerIsOperator(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    INetworkPlayer* net = p != 0 ? NetworkPlayer(p) : 0;
    lua_pushboolean(L, net != 0 && ((net->IsHost() && !PcIsHiddenHost(net)) || PcIsServerOperator(net)));
    return 1;
}

int PlayerKick(lua_State* L)
{
    ServerPlayer* p = ArgPlayer(L, 1);
    INetworkPlayer* net = p != 0 ? NetworkPlayer(p) : 0;
    if (net == 0 || net->IsHost())
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    p->mConnection->disconnect(DisconnectPacket::eDisconnectReason_Kicked);
    lua_pushboolean(L, 1);
    return 1;
}

int PlayerToString(lua_State* L)
{
    luaL_checkudata(L, 1, PLAYER_TYPE);
    lua_getiuservalue(L, 1, 2);
    lua_pushfstring(L, "player(%s)", lua_tostring(L, -1));
    return 1;
}

int ServerPlayers(lua_State* L)
{
    lua_newtable(L);
    PlayerList* all = Players();
    if (all == 0)
        return 1;
    lua_Integer n = 0;
    const std::vector<boost::shared_ptr<ServerPlayer> >& list = *all->getPlayers();
    for (size_t i = 0; i < list.size(); ++i)
    {
        ServerPlayer* p = list[i].get();
        if (Hidden(p))
            continue;
        PushPlayer(L, p);
        lua_rawseti(L, -2, ++n);
    }
    return 1;
}

int ServerPlayer_(lua_State* L)
{
    luaL_checkstring(L, 1);
    PlayerList* all = Players();
    if (all == 0)
        return 0;
    const std::wstring name = ArgWide(L, 1);
    ServerPlayer* match = 0;
    const std::vector<boost::shared_ptr<ServerPlayer> >& list = *all->getPlayers();
    for (size_t i = 0; i < list.size() && match == 0; ++i)
    {
        ServerPlayer* p = list[i].get();
        const std::wstring pname = p->getName();
        if (!Hidden(p) && pname.size() == name.size()
            && std::equal(pname.begin(), pname.end(), name.begin(), [](wchar_t a, wchar_t b) {
                   return (a >= L'A' && a <= L'Z' ? a + 32 : a) == (b >= L'A' && b <= L'Z' ? b + 32 : b);
               }))
            match = p;
    }
    if (match == 0)
        return 0;
    PushPlayer(L, match);
    return 1;
}

int ServerBroadcast(lua_State* L)
{
    luaL_checkany(L, 1);
    lua_settop(L, 1);
    luaL_tolstring(L, 1, 0);
    SendText(0, Clean(ArgWide(L, -1)), true);
    return 0;
}

int ServerTicks(lua_State* L)
{
    lua_pushinteger(L, (lua_Integer)tickCount);
    return 1;
}

int ServerTime(lua_State* L)
{
    if (gMinecraftServer == 0 || gMinecraftServer->mLevels.data[0] == 0)
        return 0;
    lua_pushinteger(L, gMinecraftServer->mLevels.data[0]->getDayTime());
    return 1;
}

int ServerSetTime(lua_State* L)
{
    const lua_Integer t = luaL_checkinteger(L, 1);
    if (t < 0)
        return luaL_error(L, "set_time: a time from 0 (24000 is a day)");
    MinecraftServer::s_setDayTime = t;
    MinecraftServer::s_setDayTimeAtEndOfTick = true;
    return 0;
}

int ServerBlock(lua_State* L)
{
    const int x = (int)luaL_checkinteger(L, 1);
    const int y = (int)luaL_checkinteger(L, 2);
    const int z = (int)luaL_checkinteger(L, 3);
    ServerLevel* lvl = ArgLevel(L, 4);
    const BlockPos pos(x, y, z);
    if (lvl == 0 || !InBuildHeight(lvl, y) || !lvl->hasChunkAt(pos))
        return 0;
    PushBlock(L, lvl->getBlockState(pos));
    return 2;
}

int ServerSetBlock(lua_State* L)
{
    const int x = (int)luaL_checkinteger(L, 1);
    const int y = (int)luaL_checkinteger(L, 2);
    const int z = (int)luaL_checkinteger(L, 3);
    luaL_checkstring(L, 4);
    const lua_Integer data = luaL_optinteger(L, 5, 0);
    ServerLevel* lvl = ArgLevel(L, 6);
    if (data < 0 || data > 15)
        return luaL_error(L, "set_block: a data value from 0 to 15");
    const BlockPos pos(x, y, z);
    if (lvl == 0 || !InBuildHeight(lvl, y) || !lvl->hasChunkAt(pos))
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    const ResourceLocation loc(ArgWide(L, 4));
    Block* b = Block::blockRegistry->containsKey(loc) ? Block::blockRegistry->get(loc) : 0;
    const BlockState* bs = b != 0 ? b->getBlockState((int)data) : 0;
    lua_pushboolean(L, bs != 0 && lvl->setBlockAndUpdate(pos, bs));
    return 1;
}

int ServerDedicated(lua_State* L)
{
    lua_pushboolean(L, PcIsDedicatedServer());
    return 1;
}

void OpenHostApi(lua_State* L)
{
    static const luaL_Reg playerMethods[] = {
        {"name", PlayerName},
        {"uid", PlayerUid},
        {"online", PlayerOnline},
        {"position", PlayerPosition},
        {"dimension", PlayerDimension},
        {"message", PlayerMessage},
        {"teleport", PlayerTeleport},
        {"give", PlayerGive},
        {"health", PlayerHealth},
        {"set_health", PlayerSetHealth},
        {"kill", PlayerKill},
        {"is_host", PlayerIsHost},
        {"is_operator", PlayerIsOperator},
        {"kick", PlayerKick},
        {0, 0}};
    luaL_newmetatable(L, PLAYER_TYPE);
    lua_newtable(L);
    SetFunctions(L, playerMethods);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, PlayerToString);
    lua_setfield(L, -2, "__tostring");
    lua_pushliteral(L, "player");
    lua_setfield(L, -2, "__name");
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);

    lua_newtable(L);
    lua_newtable(L);
    lua_pushliteral(L, "v");
    lua_setfield(L, -2, "__mode");
    lua_setmetatable(L, -2);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &playerHandlesKey);

    static const luaL_Reg serverFuncs[] = {
        {"players", ServerPlayers},
        {"player", ServerPlayer_},
        {"broadcast", ServerBroadcast},
        {"ticks", ServerTicks},
        {"time", ServerTime},
        {"set_time", ServerSetTime},
        {"block", ServerBlock},
        {"set_block", ServerSetBlock},
        {"dedicated", ServerDedicated},
        {0, 0}};
    lua_newtable(L);
    SetFunctions(L, serverFuncs);
    lua_setglobal(L, "server");
}

class SimpleEvent : public Event
{
public:
    SimpleEvent(int id) : Event(id) {}
    virtual int Push(lua_State*) { return 0; }
};

class TickEvent : public Event
{
public:
    TickEvent() : Event(eHost_Tick) {}
    virtual int Push(lua_State* L)
    {
        lua_pushinteger(L, (lua_Integer)tickCount);
        return 1;
    }
};

class PlayerEvent : public Event
{
public:
    PlayerEvent(int id, ServerPlayer* player) : Event(id), mPlayer(player) {}
    virtual int Push(lua_State* L)
    {
        PushPlayer(L, mPlayer);
        return 1;
    }
    ServerPlayer* mPlayer;
};

class HostChatEvent : public ChatEvent
{
public:
    HostChatEvent(ServerPlayer* player, const std::wstring& text) : ChatEvent(eHost_Chat, text), mPlayer(player) {}
    virtual int Push(lua_State* L)
    {
        PushPlayer(L, mPlayer);
        PushWide(L, mText);
        return 2;
    }
    ServerPlayer* mPlayer;
};

class BlockEvent : public CancellableEvent
{
public:
    BlockEvent(int id, ServerPlayer* player, const BlockPos& pos, const BlockState* state, Level* level)
        : CancellableEvent(id), mPlayer(player), mPos(pos), mState(state), mDimension(DimensionOf(level))
    {
    }
    virtual int Push(lua_State* L)
    {
        PushPlayer(L, mPlayer);
        lua_pushinteger(L, mPos.x);
        lua_pushinteger(L, mPos.y);
        lua_pushinteger(L, mPos.z);
        PushBlock(L, mState);
        lua_pushinteger(L, mDimension);
        return 7;
    }
    ServerPlayer* mPlayer;
    BlockPos mPos;
    const BlockState* mState;
    int mDimension;
};

class CommandEvent : public Event
{
public:
    CommandEvent(ServerPlayer* player, const std::wstring& arguments)
        : Event(0), mPlayer(player), mArguments(arguments)
    {
    }
    virtual int Push(lua_State* L)
    {
        PushPlayer(L, mPlayer);
        PushWide(L, mArguments);
        return 2;
    }
    virtual bool Result(lua_State* L, int idx)
    {
        if (lua_type(L, idx) == LUA_TSTRING)
        {
            size_t len;
            const char* text = lua_tolstring(L, idx, &len);
            mReply = Wide(text, len);
        }
        return true;
    }
    ServerPlayer* mPlayer;
    std::wstring mArguments;
    std::wstring mReply;
};
}

void PcScriptsHostStart()
{
    PcScriptsHostStop();
    serverThread = pthread_self();
    tickCount = 0;
    LoadStates(eSide_Host, hostEvents, eHost_Count, OpenHostApi, &hostStates);
    hostRunning = true;
    SimpleEvent ev(eHost_ServerStart);
    Dispatch(hostStates, &ev);
}

void PcScriptsHostStop()
{
    if (!hostRunning)
        return;
    if (pthread_equal(pthread_self(), serverThread))
    {
        if (PlayerList* all = Players())
        {
            const std::vector<boost::shared_ptr<ServerPlayer> > list = *all->getPlayers();
            for (size_t i = 0; i < list.size(); ++i)
                PcScriptsHostPlayerLeaving(list[i].get());
        }
        SimpleEvent ev(eHost_ServerStop);
        Dispatch(hostStates, &ev);
    }
    hostRunning = false;
    CloseStates(&hostStates);
}

void PcScriptsHostTick()
{
    if (!Active())
        return;
    ++tickCount;
    if (!AnyHandles(hostStates, eHost_Tick))
        return;
    TickEvent ev;
    Dispatch(hostStates, &ev);
}

void PcScriptsHostPlayerJoined(ServerPlayer* p)
{
    if (!Active() || Hidden(p))
        return;
    PlayerEvent ev(eHost_PlayerJoin, p);
    Dispatch(hostStates, &ev);
}

void PcScriptsHostPlayerLeaving(ServerPlayer* p)
{
    if (!Active() || Hidden(p))
        return;
    PlayerEvent ev(eHost_PlayerLeave, p);
    Dispatch(hostStates, &ev);
}

bool PcScriptsHostChat(ServerPlayer* p, std::wstring* msg)
{
    if (!Active() || !AnyHandles(hostStates, eHost_Chat))
        return true;
    HostChatEvent ev(p, *msg);
    Dispatch(hostStates, &ev);
    if (ev.mCancelled)
        return false;
    *msg = Clean(ev.mText).substr(0, 100);
    return !msg->empty();
}

bool PcScriptsHostCommand(ServerPlayer* player, const std::wstring& line)
{
    std::wstring::size_type space = line.find(L' ');
    std::wstring name = line.substr(1, space == std::wstring::npos ? std::wstring::npos : space - 1);
    for (size_t i = 0; i < name.size(); ++i)
    {
        if (name[i] >= L'A' && name[i] <= L'Z')
            name[i] = (wchar_t)(name[i] + 32);
    }
    std::wstring args = space == std::wstring::npos ? std::wstring() : line.substr(space + 1);
    const std::wstring::size_type start = args.find_first_not_of(L' ');
    args = start == std::wstring::npos ? std::wstring() : args.substr(start);

    if (name == L"help")
    {
        std::map<std::string, std::string> list;
        if (Active())
            list = Commands(eSide_Host);
        if (list.empty())
        {
            SendText(player, L"There are no commands on this game.", false);
            return true;
        }
        SendText(player, L"Commands:", false);
        for (std::map<std::string, std::string>::const_iterator it = list.begin(); it != list.end(); ++it)
        {
            const std::string s = "/" + it->first + (it->second.empty() ? "" : " - " + it->second);
            SendText(player, Clean(Wide(s.data(), s.size())), false);
        }
        return true;
    }
    if (!Active())
        return false;
    CommandEvent ev(player, args);
    if (!RunCommand(eSide_Host, Utf8(name), &ev))
        return false;
    if (!ev.mReply.empty())
        SendText(player, Clean(ev.mReply), false);
    return true;
}

bool PcScriptsHostBlockBreak(ServerPlayer* player, ServerLevel* level, const BlockPos& pos)
{
    if (!Active() || !AnyHandles(hostStates, eHost_BlockBreak) || Hidden(player))
        return true;
    BlockEvent ev(eHost_BlockBreak, player, pos, level->getBlockState(pos), level);
    Dispatch(hostStates, &ev);
    return !ev.mCancelled;
}

bool PcScriptsHostBlockPlace(ServerPlayer* player, ServerLevel* level, const BlockPos& pos, Block* block, int data)
{
    if (!Active() || !AnyHandles(hostStates, eHost_BlockPlace) || Hidden(player) || block == 0)
        return true;
    const BlockState* bs = block->getBlockState(data & 15);
    if (bs == 0)
        bs = block->getBlockState(0);
    if (bs == 0)
        return true;
    BlockEvent ev(eHost_BlockPlace, player, pos, bs, level);
    Dispatch(hostStates, &ev);
    return !ev.mCancelled;
}
