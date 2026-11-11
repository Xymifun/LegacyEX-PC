#include "platform/pc/PcModHooks.hpp"

#include "platform/pc/PcModContent.hpp"
#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/wiiu/CProfile.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/item/Enchantment.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundCustomPayloadPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/network/ServerboundCustomPayloadPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/UIScene.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/generation/PlayerList.hpp"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <set>
#include <string>
#include <vector>

namespace
{
const int ENTITY_ID_FIRST = 1000;
const int ENTITY_ID_COUNT = 1000;
const int BLOCK_ENTITY_ID_FIRST = 1000;
const int BLOCK_ENTITY_ID_COUNT = 1000;
const int SCREEN_ID_FIRST = 4000;
const int CONTAINER_FIRST = 100;
const int CONTAINER_COUNT = 100;
const int MAX_CHANNEL = 32;
const int MAX_MESSAGE = 30000;
const int EFFECT_ID_FIRST = 64;
const int EFFECT_ID_COUNT = 192;
const int ENCHANT_ID_FIRST = 1000;
const int ENCHANT_ID_COUNT = 30000;

struct EntityDef
{
    std::string key;
    std::string name;
    unsigned long long type;
    PcNativeMakeEntity make;
};

struct BlockEntityDef
{
    std::string key;
    unsigned long long type;
    PcNativeMakeBlockEntity make;
};

struct RendererDef
{
    unsigned long long type;
    PcNativeMakeRenderer make;
};

struct ScreenDef
{
    PcNativeMakeScreen make;
    int container;
    size_t mod;
};

struct ContainerDef
{
    int type;
    std::string key;
    PcNativeOpenContainer open;
    void* user;
    size_t mod;
};

struct NamedDef
{
    std::string key;
    std::string mod;
    std::string name;
    std::string display;
    void* (*make)();
    size_t modIdx;
};

struct ChunkHook
{
    PcNativeChunkCallback fn;
    void* user;
    size_t mod;
};

struct MessageHook
{
    PcNativeMessageCallback fn;
    void* user;
    size_t mod;
};

std::vector<EntityDef> entities;
std::vector<BlockEntityDef> blockEntities;
std::vector<RendererDef> renderers;
std::vector<ScreenDef> screens;
std::vector<ContainerDef> containers;
std::vector<ChunkHook> chunkHooks;
std::vector<MessageHook> messageHooks;
unsigned int nextType = 1;
bool entitiesDone = false;
bool blockEntitiesDone = false;
std::vector<NamedDef> effects;
std::vector<NamedDef> enchants;
bool effectsDone = false;
bool enchantsDone = false;

std::wstring Widen(const std::string& s)
{
    std::wstring w;
    for (size_t i = 0; i < s.size(); i++)
        w += (wchar_t)(unsigned char)s[i];
    return w;
}

unsigned int Hash(const std::string& s)
{
    unsigned int h = 2166136261u;
    for (size_t i = 0; i < s.size(); i++)
    {
        h ^= (unsigned char)s[i];
        h *= 16777619u;
    }
    return h;
}

bool GoodName(const char* name)
{
    if (name == 0 || name[0] == 0 || strlen(name) > 64)
        return false;
    for (const char* c = name; *c != 0; c++)
    {
        if (!((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') || *c == '_'))
            return false;
    }
    return true;
}

unsigned long long NewType(unsigned long long family)
{
    unsigned long long t = family & 0xFFFFFFC0ULL;
    t |= (unsigned long long)nextType << 32;
    t |= 0x3F;
    nextType++;
    return t;
}

void Say(const char* what, const char* name)
{
    printf("[pc] native %s: %s \"%s\" not added\n", PcNativeModsCurrentId(), what, name != 0 ? name : "");
    fflush(stdout);
}

void AddBlock(const char* name, const char* displayName, int tab, PcNativeMakeBlock make)
{
    if (!PcModContentAddNativeBlock(PcNativeModsCurrentId(), name, displayName, tab, make))
        Say("block (bad or taken name)", name);
}

void AddItem(const char* name, const char* displayName, int tab, PcNativeMakeItem make)
{
    if (!PcModContentAddNativeItem(PcNativeModsCurrentId(), name, displayName, tab, make))
        Say("item (bad or taken name)", name);
}

const wchar_t* AddTexture(int atlas, const char* file)
{
    const wchar_t* sprite = PcModContentAddNativeTexture(atlas, PcNativeModsCurrentId(), file);
    if (sprite == 0)
    {
        Say("texture (bad file name)", file);
        return L"";
    }
    return sprite;
}

unsigned long long AddBlockEntity(const char* name, unsigned long long family, PcNativeMakeBlockEntity make)
{
    if (blockEntitiesDone || !GoodName(name) || make == 0)
    {
        Say(blockEntitiesDone ? "block entity (too late, register it in PcNativeModRegister)" : "block entity",
            name);
        return 0;
    }
    BlockEntityDef d;
    d.key = std::string(PcNativeModsCurrentId()) + ":" + name;
    d.type = NewType(family);
    d.make = make;
    blockEntities.push_back(d);
    return d.type;
}

unsigned long long AddEntity(const char* name, unsigned long long family, PcNativeMakeEntity make)
{
    if (entitiesDone || !GoodName(name) || make == 0)
    {
        Say(entitiesDone ? "entity (too late, register it in PcNativeModRegister)" : "entity", name);
        return 0;
    }
    EntityDef d;
    d.key = std::string(PcNativeModsCurrentId()) + ":" + name;
    d.name = name;
    d.type = NewType(family);
    d.make = make;
    entities.push_back(d);
    return d.type;
}

int AddNamed(std::vector<NamedDef>* list, bool done, const char* what, const char* name, const char* display,
             void* (*make)())
{
    std::string key = std::string(PcNativeModsCurrentId()) + ":" + (name != 0 ? name : "");
    bool taken = false;
    for (size_t i = 0; i < list->size(); i++)
        if ((*list)[i].key == key)
            taken = true;
    if (done || taken || !GoodName(name) || make == 0)
    {
        std::string w = what;
        if (done)
            w += " (too late, register it in PcNativeModRegister)";
        else if (taken)
            w += " (that name is taken)";
        Say(w.c_str(), name);
        return 0;
    }
    NamedDef d;
    d.key = key;
    d.mod = PcNativeModsCurrentId();
    d.name = name;
    d.display = display != 0 ? display : name;
    d.make = make;
    d.modIdx = PcNativeModsCurrent();
    list->push_back(d);
    return 1;
}

int AddEffect(const char* name, const char* displayName, PcNativeMakeEffect make)
{
    return AddNamed(&effects, effectsDone, "effect", name, displayName, make);
}

int AddEnchantment(const char* name, const char* displayName, PcNativeMakeEnchantment make)
{
    return AddNamed(&enchants, enchantsDone, "enchantment", name, displayName, make);
}

bool NamedByKey(const NamedDef& a, const NamedDef& b)
{
    return a.key < b.key;
}

void* MakeNamed(const NamedDef& d)
{
    size_t prev = PcNativeModsCurrent();
    PcNativeModsSetCurrent(d.modIdx);
    void* p = d.make();
    PcNativeModsSetCurrent(prev);
    return p;
}

void AddEntityRenderer(unsigned long long type, PcNativeMakeRenderer make)
{
    if (type == 0 || make == 0)
        return;
    RendererDef d;
    d.type = type;
    d.make = make;
    renderers.push_back(d);
}

int AddScreen(PcNativeMakeScreen make, int container)
{
    if (make == 0)
        return -1;
    ScreenDef d;
    d.make = make;
    d.container = container;
    d.mod = PcNativeModsCurrent();
    screens.push_back(d);
    return SCREEN_ID_FIRST + (int)screens.size() - 1;
}

bool ContainerTaken(int type)
{
    for (size_t i = 0; i < containers.size(); i++)
    {
        if (containers[i].type == type)
            return true;
    }
    return false;
}

int AddContainerType(const char* name, PcNativeOpenContainer open, void* user)
{
    if (!GoodName(name) || open == 0 || (int)containers.size() >= CONTAINER_COUNT)
    {
        Say("container type", name);
        return -1;
    }
    ContainerDef d;
    d.key = std::string(PcNativeModsCurrentId()) + ":" + name;
    int slot = (int)(Hash(d.key) % (unsigned int)CONTAINER_COUNT);
    while (ContainerTaken(CONTAINER_FIRST + slot))
        slot = (slot + 1) % CONTAINER_COUNT;
    d.type = CONTAINER_FIRST + slot;
    d.open = open;
    d.user = user;
    d.mod = PcNativeModsCurrent();
    containers.push_back(d);
    return d.type;
}

void OnChunkDecorated(PcNativeChunkCallback fn, void* user)
{
    if (fn == 0)
        return;
    ChunkHook h;
    h.fn = fn;
    h.user = user;
    h.mod = PcNativeModsCurrent();
    chunkHooks.push_back(h);
}

void OnMessage(PcNativeMessageCallback fn, void* user)
{
    if (fn == 0)
        return;
    MessageHook h;
    h.fn = fn;
    h.user = user;
    h.mod = PcNativeModsCurrent();
    messageHooks.push_back(h);
}

arrayWithLength<unsigned char> Pack(const char* channel, const void* data, int size)
{
    int n = channel != 0 ? (int)strlen(channel) : 0;
    if (n < 1 || n > MAX_CHANNEL || size < 0 || size > MAX_MESSAGE || (size > 0 && data == 0))
    {
        printf("[pc] native %s: message on \"%s\" not sent (channel 1 to %d characters, data up to %d bytes)\n",
               PcNativeModsCurrentId(), channel != 0 ? channel : "", MAX_CHANNEL, MAX_MESSAGE);
        fflush(stdout);
        return arrayWithLength<unsigned char>();
    }
    arrayWithLength<unsigned char> buf(1 + n + size, true);
    buf.data[0] = (unsigned char)n;
    memcpy(buf.data + 1, channel, n);
    if (size > 0)
        memcpy(buf.data + 1 + n, data, size);
    return buf;
}

void SendToHost(const char* channel, const void* data, int size)
{
    arrayWithLength<unsigned char> buf = Pack(channel, data, size);
    if (buf.data == 0)
        return;
    Minecraft* mc = Minecraft::GetInstance();
    int pad = gProfile->GetPrimaryPad();
    ClientPacketListener* conn = 0;
    if (mc != 0 && pad >= 0 && pad < 4 && mc->mLocalPlayers[pad] != 0)
        conn = mc->getConnection(pad);
    if (conn == 0)
    {
        delete[] buf.data;
        return;
    }
    conn->send(boost::shared_ptr<Packet>(new ServerboundCustomPayloadPacket(PC_MOD_MESSAGE_CHANNEL, buf)));
}

void SendToPlayer(void* serverPlayer, const char* channel, const void* data, int size)
{
    arrayWithLength<unsigned char> buf = Pack(channel, data, size);
    if (buf.data == 0)
        return;
    boost::shared_ptr<Packet> packet(new ClientboundCustomPayloadPacket(PC_MOD_MESSAGE_CHANNEL, buf));
    if (serverPlayer != 0)
    {
        ServerPlayer* p = (ServerPlayer*)serverPlayer;
        if (p->mConnection != 0)
            p->mConnection->send(packet);
        return;
    }
    if (gMinecraftServer != 0 && gMinecraftServer->getPlayers() != 0)
        gMinecraftServer->getPlayers()->broadcastAll(packet);
}

bool ByKey(const EntityDef& a, const EntityDef& b)
{
    return a.key < b.key;
}

bool BlockEntityByKey(const BlockEntityDef& a, const BlockEntityDef& b)
{
    return a.key < b.key;
}
}

void PcModHooksFillRegister(PcNativeRegister* reg)
{
    reg->addBlock = AddBlock;
    reg->addItem = AddItem;
    reg->addTexture = AddTexture;
    reg->addBlockEntity = AddBlockEntity;
    reg->addEntity = AddEntity;
    reg->addEntityRenderer = AddEntityRenderer;
    reg->addEffect = AddEffect;
    reg->addEnchantment = AddEnchantment;
}

void PcModHooksFillHost(PcNativeHost* host)
{
    host->addScreen = AddScreen;
    host->addContainerType = AddContainerType;
    host->onChunkDecorated = OnChunkDecorated;
    host->sendToHost = SendToHost;
    host->sendToPlayer = SendToPlayer;
    host->onMessage = OnMessage;
}

void PcModHooksAddEntities()
{
    PcNativeModsRegister();
    entitiesDone = true;
    if (entities.empty())
        return;
    std::sort(entities.begin(), entities.end(), ByKey);
    for (size_t i = 0; i < entities.size(); i++)
    {
        EntityDef& d = entities[i];
        int slot = (int)(Hash(d.key) % (unsigned int)ENTITY_ID_COUNT);
        int tries = 0;
        while (EntityIO::sRegistry.containsId(ENTITY_ID_FIRST + slot) && tries < ENTITY_ID_COUNT)
        {
            slot = (slot + 1) % ENTITY_ID_COUNT;
            tries++;
        }
        if (tries >= ENTITY_ID_COUNT)
        {
            printf("[pc] native: no entity id left for %s\n", d.key.c_str());
            continue;
        }
        EntityIO::Register((EntityIO::Creator)d.make, ENTITY_ID_FIRST + slot, Widen(d.key), d.type, Widen(d.name));
    }
    printf("[pc] native: %u mod entity types\n", (unsigned int)entities.size());
    fflush(stdout);
}

void PcModHooksAddBlockEntities()
{
    PcNativeModsRegister();
    blockEntitiesDone = true;
    if (blockEntities.empty())
        return;
    std::sort(blockEntities.begin(), blockEntities.end(), BlockEntityByKey);
    std::set<int> used;
    for (size_t i = 0; i < blockEntities.size(); i++)
    {
        BlockEntityDef& d = blockEntities[i];
        int slot = (int)(Hash(d.key) % (unsigned int)BLOCK_ENTITY_ID_COUNT);
        while (used.count(slot) != 0)
            slot = (slot + 1) % BLOCK_ENTITY_ID_COUNT;
        used.insert(slot);
        BlockEntity::Register(BLOCK_ENTITY_ID_FIRST + slot, (BlockEntity::Creator)d.make, d.type, Widen(d.key));
    }
    printf("[pc] native: %u mod block entity types\n", (unsigned int)blockEntities.size());
    fflush(stdout);
}

int PcModHooksAddEffects()
{
    PcNativeModsRegister();
    effectsDone = true;
    std::sort(effects.begin(), effects.end(), NamedByKey);
    int top = 0;
    int n = 0;
    for (size_t i = 0; i < effects.size(); i++)
    {
        NamedDef& d = effects[i];
        int slot = (int)(Hash(d.key) % (unsigned int)EFFECT_ID_COUNT);
        int tries = 0;
        while (MobEffect::mobEffectRegistry->containsId(EFFECT_ID_FIRST + slot) && tries < EFFECT_ID_COUNT)
        {
            slot = (slot + 1) % EFFECT_ID_COUNT;
            tries++;
        }
        if (tries >= EFFECT_ID_COUNT)
        {
            printf("[pc] native: no effect id left for %s\n", d.key.c_str());
            continue;
        }
        MobEffect* e = (MobEffect*)MakeNamed(d);
        if (e == 0)
        {
            printf("[pc] native: %s made no effect\n", d.key.c_str());
            continue;
        }
        int id = EFFECT_ID_FIRST + slot;
        e->setDescriptionId(PcModContentAddName(d.display.c_str()));
        MobEffect::mobEffectRegistry->registerMapping(id, ResourceLocation(Widen(d.mod), Widen(d.name)), e);
        if (id > top)
            top = id;
        n++;
    }
    if (n > 0)
    {
        printf("[pc] native: %d mod effects\n", n);
        fflush(stdout);
    }
    return top;
}

bool PcModHooksIsModEffect(int id)
{
    return id >= EFFECT_ID_FIRST;
}

void PcModHooksAddEnchantments()
{
    PcNativeModsRegister();
    enchantsDone = true;
    std::sort(enchants.begin(), enchants.end(), NamedByKey);
    int n = 0;
    for (size_t i = 0; i < enchants.size(); i++)
    {
        NamedDef& d = enchants[i];
        int slot = (int)(Hash(d.key) % (unsigned int)ENCHANT_ID_COUNT);
        int tries = 0;
        while (Enchantment::enchantmentRegistry->containsId(ENCHANT_ID_FIRST + slot) && tries < ENCHANT_ID_COUNT)
        {
            slot = (slot + 1) % ENCHANT_ID_COUNT;
            tries++;
        }
        if (tries >= ENCHANT_ID_COUNT)
        {
            printf("[pc] native: no enchantment id left for %s\n", d.key.c_str());
            continue;
        }
        Enchantment* e = (Enchantment*)MakeNamed(d);
        if (e == 0)
        {
            printf("[pc] native: %s made no enchantment\n", d.key.c_str());
            continue;
        }
        e->setDescriptionId(PcModContentAddName(d.display.c_str()));
        Enchantment::enchantmentRegistry->registerMapping(ENCHANT_ID_FIRST + slot,
                                                          ResourceLocation(Widen(d.mod), Widen(d.name)), e);
        n++;
    }
    if (n > 0)
    {
        printf("[pc] native: %d mod enchantments\n", n);
        fflush(stdout);
    }
}

void PcModHooksAddRenderers(EntityRenderDispatcher* dispatcher)
{
    PcNativeModsRegister();
    for (size_t i = 0; i < renderers.size(); i++)
    {
        EntityRenderer* r = (EntityRenderer*)renderers[i].make(dispatcher);
        if (r != 0)
            (*dispatcher)[renderers[i].type] = r;
    }
}

bool PcModHooksIsModType(unsigned long long type)
{
    return (type >> 32) != 0;
}

bool PcModHooksIsScreen(int scene, bool* container)
{
    int i = scene - SCREEN_ID_FIRST;
    if (i < 0 || i >= (int)screens.size())
        return false;
    if (container != 0)
        *container = screens[i].container != 0;
    return true;
}

UIScene* PcModHooksMakeScreen(int scene, int pad, void* data, UILayer* layer)
{
    int i = scene - SCREEN_ID_FIRST;
    if (i < 0 || i >= (int)screens.size())
        return 0;
    size_t prev = PcNativeModsCurrent();
    PcNativeModsSetCurrent(screens[i].mod);
    UIScene* made = (UIScene*)screens[i].make(pad, data, layer);
    PcNativeModsSetCurrent(prev);
    return made;
}

bool PcModHooksIsContainer(int type)
{
    return ContainerTaken(type);
}

bool PcModHooksOpenContainer(int type, void* player, int containerId, int size, const std::wstring& title,
                             int entityId)
{
    for (size_t i = 0; i < containers.size(); i++)
    {
        if (containers[i].type == type)
        {
            size_t prev = PcNativeModsCurrent();
            PcNativeModsSetCurrent(containers[i].mod);
            int ok = containers[i].open(player, containerId, size, title.c_str(), entityId, containers[i].user);
            PcNativeModsSetCurrent(prev);
            return ok != 0;
        }
    }
    return false;
}

void PcModHooksChunkDecorated(Level* level, int chunkX, int chunkZ)
{
    if (chunkHooks.empty() || level == 0)
        return;
    int dim = level->mDimension->getType()->getId();
    size_t prev = PcNativeModsCurrent();
    for (size_t i = 0; i < chunkHooks.size(); i++)
    {
        PcNativeModsSetCurrent(chunkHooks[i].mod);
        chunkHooks[i].fn(level, chunkX, chunkZ, dim, chunkHooks[i].user);
    }
    PcNativeModsSetCurrent(prev);
}

void PcModHooksMessage(ServerPlayer* player, const unsigned char* data, int len)
{
    if (messageHooks.empty() || data == 0 || len < 1)
        return;
    int n = data[0];
    if (n < 1 || n > MAX_CHANNEL || 1 + n > len)
        return;
    std::string channel((const char*)data + 1, n);
    size_t prev = PcNativeModsCurrent();
    for (size_t i = 0; i < messageHooks.size(); i++)
    {
        PcNativeModsSetCurrent(messageHooks[i].mod);
        messageHooks[i].fn(player, channel.c_str(), data + 1 + n, len - 1 - n, messageHooks[i].user);
    }
    PcNativeModsSetCurrent(prev);
}
