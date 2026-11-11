// An example native mod (tools/pc/README.md, Native mods). Build it with the game:
//   ninja -C build/pc64 pc_native_example
// then copy build/pc64/native-example/win64.dll to mods/native-example/native/win64.dll, next to a
// mod.json ({"id": "native-example", "side": "both", "api": 2}), a 16x16 textures/example_block.png and
// sounds/chime.ogg (tools/pc/native-example/sounds/chime.ogg).
// A mod that adds content must be "both": every player in a world needs it.
//
// It calls the game directly: Minecraft::GetInstance, the server's player list and a level's name
// (a std::wstring the game makes and this mod frees, from the one heap they share). It also shows the
// content hooks (tools/pc/README.md, Native content hooks): a block and a mob type registered while the
// game starts, the mob spawned next to the first player, a chunk callback, and a message sent from the
// player's game to the host and answered. It changes a vanilla behaviour with a function hook
// (tools/pc/README.md, Function hooks): zombies that land a hit set their target on fire. Finally it adds
// a status effect and an enchantment (tools/pc/README.md, Effects and enchantments): the player gets the
// effect and an enchanted book once the world starts. It also keeps data (tools/pc/README.md, Mod data,
// settings and keys): a setting in config/native-example.cfg, a key players can rebind under Controls,
// a counter saved with the world and a flag saved with each player, so the gifts come only once. Last, a
// structure spread over chunks (tools/pc/README.md, Structures): small stone brick towers, found by name.
// And a sound of its own (tools/pc/README.md, Sounds): the example key makes the host play a chime at the
// player, for everyone near them.

#include "PcGameBuild.h"
#include "platform/pc/PcNativeMod.h"

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ZombieRenderer.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/Rarity.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructureStart.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

namespace
{
const char kMessageChannel[] = "native-example";

const PcNativeHost* s_host = 0;
unsigned long long s_frames = 0;
unsigned long long s_framesInWorld = 0;
unsigned long long s_ticks = 0;
const wchar_t* s_blockSprite = L"";
unsigned long long s_mobType = 0;
bool s_mobSpawned = false;
bool s_messageSent = false;
int s_decoratedChunks = 0;
bool s_rewardsGiven = false;
int s_statusInterval = 200;
const SoundEvent* s_chime = 0;
int s_waveKey = 0;
const char kGiftsReceived = 1;

// A plain cube. Every block class builds its state definition in its constructor (DerivedInit).
class ExampleBlock : public Block
{
public:
    ExampleBlock() : Block(Material::STONE)
    {
        DerivedInit();
        setIconName(s_blockSprite);
        setDestroyTime(1.5f);
    }
};

// A zombie with a type of its own: the game's "is it a monster" checks still see it as one, because the
// type was registered under EntityType::MONSTER. GetType must return the registered type.
class ExampleMob : public Zombie
{
public:
    explicit ExampleMob(Level* level) : Zombie(level) {}
    virtual unsigned long long GetType() { return s_mobType; }
};

void* MakeBlock(int)
{
    return new ExampleBlock();
}

void* MakeMob(void* level)
{
    return new ExampleMob(static_cast<Level*>(level));
}

void* MakeMobRenderer(void* dispatcher)
{
    return new ZombieRenderer(static_cast<EntityRenderDispatcher*>(dispatcher));
}

// A status effect: heals one point every two seconds (faster at higher amplifiers). The game asks
// isDurationEffectTick every tick and calls applyEffectTick when it says yes. It borrows the game's
// regeneration icon; the name in the inventory comes from addEffect.
class ExampleEffect : public MobEffect
{
public:
    ExampleEffect() : MobEffect(false, eMinecraftColour_Effect_Regeneration)
    {
        setIcon(EMobEffectIcon_Regeneration);
        setBeneficial();
    }

    virtual bool isDurationEffectTick(int duration, int amplifier)
    {
        const int interval = 40 >> amplifier;
        return interval <= 1 || duration % interval == 0;
    }

    virtual void applyEffectTick(const boost::shared_ptr<LivingEntity>& entity, int)
    {
        if (entity->getHealth() < entity->getMaxHealth())
            entity->heal(1.0f);
    }
};

// An enchantment for weapons: each hit heals the attacker half a point per level. The category decides
// which items can carry it; rarity, levels and costs decide how often the enchanting table offers it.
class ExampleEnchantment : public Enchantment
{
public:
    ExampleEnchantment() : Enchantment(Rarity::RARE, EnchantmentCategory::WEAPON, MainHandOnly()) {}

    virtual int getMaxLevel() { return 2; }
    virtual int getMinCost(int level) { return 10 + (level - 1) * 15; }
    virtual int getMaxCost(int level) { return getMinCost(level) + 30; }

    virtual void doPostAttack(boost::shared_ptr<LivingEntity> attacker, boost::shared_ptr<Entity>, int level)
    {
        if (attacker != 0)
            attacker->heal(0.5f * static_cast<float>(level));
    }

private:
    static arrayWithLength<const EquipmentSlot*> MainHandOnly()
    {
        arrayWithLength<const EquipmentSlot*> slots(1, true);
        slots.data[0] = EquipmentSlot::MAINHAND;
        return slots;
    }
};

ExampleEffect* s_effect = 0;
ExampleEnchantment* s_enchantment = 0;

void* MakeEffect()
{
    s_effect = new ExampleEffect();
    return s_effect;
}

void* MakeEnchantment()
{
    s_enchantment = new ExampleEnchantment();
    return s_enchantment;
}

// --- A structure: a small stone brick tower, at most one in every 8 x 8 chunks ---

const int kTowerSpacing = 8;     // chunks per region; each region may hold one tower
const int kTowerSeparation = 2;  // towers keep this many chunks from a region's far edge
const int kTowerSalt = 0x5eed7;  // keeps this structure's random choices apart from other structures'
unsigned int s_towerStartType = 0;
unsigned int s_towerPieceType = 0;

int FloorDivide(int value, int divisor)
{
    return value >= 0 ? value / divisor : -((-value + divisor - 1) / divisor);
}

// The building itself. A piece saves its box and orientation by itself; this one also saves the ground
// height it found, so it is built the same after a reload.
class TowerPiece : public StructurePiece
{
public:
    TowerPiece() : StructurePiece(0), m_groundY(-1) {}

    TowerPiece(int x, int z) : StructurePiece(0), m_groundY(-1)
    {
        setOrientation(Direction::NORTH);
        mBoundingBox = new BoundingBox(x, 64, z, x + 4, 72, z + 4);
    }

    virtual EStructurePiece GetType() { return static_cast<EStructurePiece>(s_towerPieceType); }

    virtual void addAdditionalSaveData(DataOutputStream* output) { output->writeInt(m_groundY); }

    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager*) { m_groundY = input->readInt(); }

    // Called for every chunk the piece reaches as it is decorated; box is the part that may be built now.
    virtual bool postProcess(Level* level, Random&, BoundingBox* box)
    {
        if (m_groundY < 0)
        {
            const BlockPos centre(mBoundingBox->x0 + 2, 64, mBoundingBox->z0 + 2);
            m_groundY = level->getTopSolidBlockPos(centre).y;
            move(0, m_groundY - mBoundingBox->y0, 0);
        }
        const BlockState* bricks = Blocks::stonebrick->defaultBlockState();
        const BlockState* air = Blocks::air->defaultBlockState();
        generateBox(level, box, 0, 0, 0, 4, 8, 4, bricks, air, false);  // walls, hollow inside
        generateBox(level, box, 0, 8, 0, 4, 8, 4, bricks, bricks, false);  // roof
        generateBox(level, box, 2, 1, 0, 2, 2, 0, air, air, false);  // a door
        return true;
    }

private:
    int m_groundY;
};

class TowerStart : public StructureStart
{
public:
    TowerStart() {}

    TowerStart(int chunkX, int chunkZ) : StructureStart(chunkX, chunkZ)
    {
        mPieces.push_back(new TowerPiece(chunkX * 16 + 5, chunkZ * 16 + 5));
        calculateBoundingBox();
    }

    virtual EStructureStart GetType() { return static_cast<EStructureStart>(s_towerStartType); }
};

// The structure's rules. The world generator asks isFeatureChunk for chunks near the ones it makes and
// keeps the starts in the world's saved data (under getFeatureName).
class TowerFeature : public StructureFeature
{
public:
    virtual std::wstring getFeatureName() { return L"native_example_tower"; }

    virtual bool isFeatureChunk(int chunkX, int chunkZ, bool)
    {
        const int regionX = FloorDivide(chunkX, kTowerSpacing);
        const int regionZ = FloorDivide(chunkZ, kTowerSpacing);
        Random* random = level->getRandomFor(regionX, regionZ, kTowerSalt);
        const int towerX = regionX * kTowerSpacing + random->nextInt(kTowerSpacing - kTowerSeparation);
        const int towerZ = regionZ * kTowerSpacing + random->nextInt(kTowerSpacing - kTowerSeparation);
        return chunkX == towerX && chunkZ == towerZ;
    }

    virtual StructureStart* createStructureStart(int chunkX, int chunkZ) { return new TowerStart(chunkX, chunkZ); }

    virtual BlockPos* getNearestGeneratedFeature(Level* searchLevel, const BlockPos& pos, bool skipExistingChunks)
    {
        level = searchLevel;
        return findNearestFeaturePositionBySpacing(searchLevel, this, pos, kTowerSpacing, kTowerSeparation, kTowerSalt,
                                                   true, 100, skipExistingChunks);
    }
};

void* MakeTowerFeature(void*, void*)
{
    return new TowerFeature();
}

void* MakeTowerStart()
{
    return new TowerStart();
}

void* MakeTowerPiece()
{
    return new TowerPiece();
}

// Skeletons spawn inside the towers (at night or in the dark, as monsters do).
void* TowerMobs(void* feature, void* category, const void* pos)
{
    static std::vector<Biome::MobSpawnerData*> s_towerMobs(1, new Biome::MobSpawnerData(1, EntityType::SKELETON, 1, 2));
    if (static_cast<MobCategory*>(category) != MobCategory::MONSTER
        || !static_cast<StructureFeature*>(feature)->isInsideFeature(*static_cast<const BlockPos*>(pos)))
        return 0;
    return &s_towerMobs;
}

// The game's own Zombie::doHurtTarget, as hookFunction hands it back. A member function is called as a plain
// function that takes the object first, with the same return type and arguments.
typedef bool (*ZombieHurtTargetFunction)(Zombie* zombie, boost::shared_ptr<Entity> target);
ZombieHurtTargetFunction s_originalZombieHurtTarget = 0;
bool s_zombieHitLogged = false;

// Replaces Zombie::doHurtTarget for every zombie that uses it (this mod's mob included). It keeps the
// game's behaviour by calling the original first, then adds to it.
bool ZombieHurtTarget(Zombie* zombie, boost::shared_ptr<Entity> target)
{
    const bool hit = s_originalZombieHurtTarget(zombie, target);
    if (hit && target != 0)
    {
        target->setSecondsOnFire(2);
        if (!s_zombieHitLogged && s_host != 0)
        {
            s_zombieHitLogged = true;
            s_host->log("a zombie landed a hit; the hooked doHurtTarget set the target on fire");
        }
    }
    return hit;
}

// a static constructor: runs when the game starts the mod's C runtime
std::string s_greeting = std::string("hello from native code, built for game ") + PC_GAME_BUILD;

std::string Narrow(const std::wstring& text)
{
    std::string narrow;
    for (size_t i = 0; i < text.size(); ++i)
        narrow += text[i] < 0x80 ? static_cast<char>(text[i]) : '?';
    return narrow;
}

void Log(const std::string& text)
{
    s_host->log(text.c_str());
}

void Frame(void*)
{
    ++s_frames;
    // keyPressed counts presses made while playing; menus, chat and typing do not count.
    if (s_waveKey != 0 && s_host->keyPressed(s_waveKey) > 0)
    {
        // Ask the host to chime; it plays the sound for every player near this one (Message).
        Log("the example key was pressed");
        s_host->sendToHost(kMessageChannel, "wave", 4);
    }
    Minecraft* game = Minecraft::GetInstance();
    if (game != 0 && game->mLocalPlayers[0] != 0)
        ++s_framesInWorld;
    else
        s_framesInWorld = 0;
    if (!s_messageSent && s_framesInWorld > 120)
    {
        // From the player's game to the host, once the player has been in a world for a moment (sendToHost
        // drops messages while a world loads); the host answers in Message.
        s_messageSent = true;
        s_host->sendToHost(kMessageChannel, "hello", 5);
    }
    if (s_frames % 120 != 0)
        return;
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiplayerLocalPlayer* player = minecraft != 0 ? minecraft->mLocalPlayers[0].get() : 0;
    char text[128];
    if (player != 0)
        snprintf(text, sizeof(text), "frame %llu: the player is at %.1f %.1f %.1f", s_frames, player->mX, player->mY,
                 player->mZ);
    else
        snprintf(text, sizeof(text), "frame %llu: not in a world", s_frames);
    Log(text);
}

void HostStart(void*)
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    Log("a world started: \"" + Narrow(level->getLevelData()->getLevelName()) + "\"");

    // World data is a block of bytes of the mod's own, saved in the world's level.dat. Here: a counter of
    // how many times this world was started with the mod.
    int size = 0;
    const void* data = s_host->getWorldData(&size);
    int starts = 0;
    if (data != 0 && size == static_cast<int>(sizeof(starts)))
        memcpy(&starts, data, sizeof(starts));
    ++starts;
    s_host->setWorldData(&starts, static_cast<int>(sizeof(starts)));
    char text[96];
    snprintf(text, sizeof(text), "this world was started %d time(s) with the example mod", starts);
    Log(text);

    // Structures are found by their registered name, as the game's own are (explorer maps use the same call).
    BlockPos* tower = level->findNearestMapFeature(L"native-example:tower", level->getSharedSpawnPos(), false);
    if (tower != 0)
    {
        snprintf(text, sizeof(text), "the nearest example tower is at %d %d", tower->x, tower->z);
        Log(text);
        delete tower;
    }
}

void HostTick(void*)
{
    ++s_ticks;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    if (!s_mobSpawned && s_ticks >= 100 && !players->empty())
    {
        s_mobSpawned = true;
        boost::shared_ptr<ServerPlayer> player = players->back();
        boost::shared_ptr<Entity> mob(new ExampleMob(player->mLevel));
        mob->moveTo(player->mX + 3.0, player->mY, player->mZ, 0.0f, 0.0f);
        player->mLevel->addEntity(mob);
        Log("spawned an example mob next to the player");
    }
    if (!s_rewardsGiven && s_ticks >= 40 && !players->empty() && s_effect != 0 && s_enchantment != 0)
    {
        // The effect for 30 seconds, and a book with the enchantment at its highest level, once per player:
        // player data is saved with the player, so the flag survives restarts.
        s_rewardsGiven = true;
        boost::shared_ptr<ServerPlayer> player = players->back();
        int size = 0;
        const void* data = s_host->getPlayerData(player.get(), &size);
        if (data != 0 && size == 1 && *static_cast<const char*>(data) == kGiftsReceived)
        {
            Log("the player already received the example gifts");
            return;
        }
        s_host->setPlayerData(player.get(), &kGiftsReceived, 1);
        player->addEffect(new MobEffectInstance(s_effect, 30 * 20, 0), boost::shared_ptr<Entity>());
        player->mInventory->add(
            EnchantedBookItem::createForEnchantment(new EnchantmentInstance(s_enchantment, s_enchantment->getMaxLevel())));
        char text[128];
        snprintf(text, sizeof(text), "gave the player the example effect (id %d) and an example book (enchantment id %d)",
                 MobEffect::getId(s_effect), Enchantment::getId(s_enchantment));
        Log(text);
    }
    if (s_ticks % static_cast<unsigned long long>(s_statusInterval) != 0)
        return;
    char text[96];
    snprintf(text, sizeof(text), "server tick %llu: %d player(s), day time %lld", s_ticks,
             static_cast<int>(gMinecraftServer->getPlayers()->getPlayers()->size()), gMinecraftServer->getLevel(0)->getDayTime());
    Log(text);
}

void HostStop(void*)
{
    char text[96];
    snprintf(text, sizeof(text), "the world stopped; %d new chunks were decorated while it ran", s_decoratedChunks);
    Log(text);
}

// After the game decorated a new chunk, on the thread that made it: the place for structures and
// features of the mod's own (Level::setBlock with positions inside chunk x, z).
void ChunkDecorated(void*, int, int, int, void*)
{
    ++s_decoratedChunks;
}

// On the host, player is the ServerPlayer that sent the message; on a player's game it is 0.
void Message(void* player, const char* channel, const void* data, int size, void*)
{
    const std::string text(static_cast<const char*>(data), static_cast<size_t>(size));
    Log(std::string(player != 0 ? "the host" : "the player's game") + " received \"" + text + "\" on " + channel);
    if (player != 0 && text == "wave" && s_chime != 0)
    {
        // A mod's sound plays like the game's own: Level::playSound sends it to every player in range (an
        // empty player means nobody is left out).
        ServerPlayer* sender = static_cast<ServerPlayer*>(player);
        sender->mLevel->playSound(boost::shared_ptr<Player>(), sender->mX, sender->mY, sender->mZ, s_chime,
                                  SoundSource::eSoundSource_Players, 1.0f, 1.0f, 16.0f);
        return;
    }
    if (player != 0)
        s_host->sendToPlayer(player, kMessageChannel, "hello back", 10);
}
}

// Optional, called while the game starts, before it makes its blocks, items and entity types.
extern "C" __declspec(dllexport) int PcNativeModRegister(const PcNativeRegister* registry)
{
    s_blockSprite = registry->addTexture(PC_NATIVE_ATLAS_BLOCKS, "example_block");
    registry->addBlock("example_block", "Example Block", PC_NATIVE_TAB_BUILDING, MakeBlock);
    s_mobType = registry->addEntity("example_mob", EntityType::MONSTER, MakeMob);
    registry->addEntityRenderer(s_mobType, MakeMobRenderer);

    // Function hooks go in here, while the game starts. "Zombie::doHurtTarget" is enough while only one
    // function has that name; the full signature, "Zombie::doHurtTarget(boost::shared_ptr<Entity>)", always
    // works. If the hook fails, the game logs why and the zombies keep their usual behaviour.
    registry->hookFunction("Zombie::doHurtTarget", reinterpret_cast<void*>(&ZombieHurtTarget),
                           reinterpret_cast<void**>(&s_originalZombieHurtTarget));

    // The game makes these right after its own effects and enchantments, as "native-example:mending_touch"
    // and so on; the make functions keep the objects for later use.
    registry->addEffect("mending_touch", "Mending Touch", MakeEffect);
    registry->addEnchantment("lifesteal", "Lifesteal", MakeEnchantment);

    // Players rebind this under Help & Options > Controls (keyboard), in the Mods section.
    s_waveKey = registry->addKey("wave", "Example: Wave", 'G');

    // A structure in the overworld, with its start and piece classes (saved by these names).
    s_towerStartType = registry->addStructureStart("tower_start", MakeTowerStart);
    s_towerPieceType = registry->addStructurePiece("tower", MakeTowerPiece);
    registry->addStructure("tower", PC_NATIVE_OVERWORLD, MakeTowerFeature, TowerMobs);

    // A sound from the mod's sounds/ folder (.ogg, .wav or .mp3), heard up to the game's usual 16 blocks away.
    s_chime = static_cast<const SoundEvent*>(registry->addSound("chime", "chime.ogg", 0.0f));
    return 0;
}

PC_NATIVE_MOD_RECORD
{
    s_host = host;
    Log(s_greeting);
    // A setting from config/native-example.cfg; the first run writes it there with this comment.
    s_statusInterval = host->getConfigInt("status_interval", 200,
                                          "Server ticks between the example's status lines in the log (20 a second).");
    if (s_statusInterval < 20)
        s_statusInterval = 20;
    char text[160];
    snprintf(text, sizeof(text), "mod %s in %s; Minecraft::GetInstance is at %p, found by name at %p", host->modId,
             host->modDirectory, reinterpret_cast<void*>(&Minecraft::GetInstance),
             host->findSymbol("_ZN9Minecraft11GetInstanceEv"));
    Log(text);
    host->onFrame(Frame, 0);
    host->onHostStart(HostStart, 0);
    host->onHostTick(HostTick, 0);
    host->onHostStop(HostStop, 0);
    host->onChunkDecorated(ChunkDecorated, 0);
    host->onMessage(Message, 0);
    return 0;
}
