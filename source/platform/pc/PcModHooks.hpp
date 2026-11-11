#ifndef PLATFORM_PC_PCMODHOOKS_HPP
#define PLATFORM_PC_PCMODHOOKS_HPP

#include <string>

struct PcNativeRegister;
struct PcNativeHost;
class EntityRenderDispatcher;
class UILayer;
class UIScene;
class Level;
class ServerPlayer;

void PcModHooksFillRegister(PcNativeRegister* reg);
void PcModHooksFillHost(PcNativeHost* host);

void PcModHooksAddEntities();
void PcModHooksAddBlockEntities();
void PcModHooksAddRenderers(EntityRenderDispatcher* dispatcher);
int PcModHooksAddEffects();
void PcModHooksAddEnchantments();
bool PcModHooksIsModEffect(int id);

bool PcModHooksIsModType(unsigned long long type);
const int PC_MOD_ENTITY_PACKET = 120;

bool PcModHooksIsScreen(int scene, bool* container);
UIScene* PcModHooksMakeScreen(int scene, int pad, void* data, UILayer* layer);
bool PcModHooksIsContainer(int type);
bool PcModHooksOpenContainer(int type, void* player, int containerId, int size, const std::wstring& title,
                             int entityId);

void PcModHooksChunkDecorated(Level* level, int chunkX, int chunkZ);

#define PC_MOD_MESSAGE_CHANNEL L"PC|Mod"
void PcModHooksMessage(ServerPlayer* player, const unsigned char* data, int len);

#endif
