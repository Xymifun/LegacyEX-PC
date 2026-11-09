#include "platform/pc/PcAccount.hpp"

#include "platform/pc/PcEd25519.hpp"

#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include "game/core/C_AOC.hpp"
#include "game/util/ACTUuid.hpp"

#include <coreinit/userconfig.h>
#include <nn/act/client_cpp.h>
#include <nn/fp/fp_cpp.h>
#include <sysapp/args.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <random>
#include <string>

namespace
{
const nn::act::SlotNo kAccountSlot = 1;
const uint32_t kCafeLanguageEnglish = 1;
const uint32_t kCafeCountryUnitedStates = 49;
const unsigned char kProductAreaUsa = 2;

std::string s_playerName = "Player";
const char kDefaultPlayerName[] = "Player";
bool s_identityLoaded = false;
std::string s_identityDirectory;
unsigned int s_principalId = 1;
unsigned char s_accountUuid[16];
unsigned char s_signingSeed[32];
bool s_hasSigningSeed = false;

unsigned int Fnv1a(const std::string& text, unsigned int hash)
{
    for (size_t i = 0; i < text.size(); ++i)
    {
        hash ^= static_cast<unsigned char>(text[i]);
        hash *= 16777619u;
    }
    return hash;
}

void WriteConfigValue(UCSysConfig* config, uint32_t value)
{
    if (config->data == 0)
        return;
    memset(config->data, 0, config->dataSize);
    if (config->dataSize >= 4)
        memcpy(config->data, &value, 4);
    else if (config->dataSize >= 1)
        *static_cast<unsigned char*>(config->data) = static_cast<unsigned char>(value);
}
}

void PcSetPlayerName(const std::string& name)
{
    s_playerName = name.empty() ? std::string("Player") : name.substr(0, 16);
}

const std::string& PcPlayerName()
{
    return s_playerName;
}

namespace
{
void NameIdentity(unsigned int* principalId, unsigned char uuid[16])
{
    unsigned int id = Fnv1a(s_playerName, 2166136261u);
    *principalId = id != 0 ? id : 1;
    unsigned int hash = 2166136261u;
    for (int part = 0; part < 4; ++part)
    {
        hash = Fnv1a(s_playerName, hash ^ static_cast<unsigned int>(part));
        memcpy(uuid + part * 4, &hash, 4);
    }
}

void RandomIdentity(unsigned int* principalId, unsigned char uuid[16])
{
    std::random_device device;
    std::mt19937_64 generator((static_cast<unsigned long long>(device()) << 32) ^ device()
                              ^ PcOsMonotonicNanoseconds() ^ PcOsRealTimeMilliseconds());
    do
        *principalId = static_cast<unsigned int>(generator());
    while (*principalId == 0);
    for (int index = 0; index < 16; ++index)
        uuid[index] = static_cast<unsigned char>(generator());
}

bool ReadIdentity(const std::string& path, unsigned int* principalId, unsigned char uuid[16])
{
    FILE* file = fopen(path.c_str(), "r");
    if (file == 0)
        return false;
    unsigned int values[17];
    bool valid = fscanf(file, "principal=%8x\nuuid=", &values[0]) == 1;
    for (int index = 0; valid && index < 16; ++index)
        valid = fscanf(file, "%2x", &values[index + 1]) == 1;
    unsigned int seed[32];
    int matched = -1;
    if (valid && fscanf(file, " key=%n", &matched) == EOF)
        matched = -1;
    bool hasSeed = matched > 0;
    for (int index = 0; hasSeed && index < 32; ++index)
        hasSeed = fscanf(file, "%2x", &seed[index]) == 1;
    fclose(file);
    if (!valid || values[0] == 0)
        return false;
    *principalId = values[0];
    for (int index = 0; index < 16; ++index)
        uuid[index] = static_cast<unsigned char>(values[index + 1]);
    s_hasSigningSeed = hasSeed;
    for (int index = 0; hasSeed && index < 32; ++index)
        s_signingSeed[index] = static_cast<unsigned char>(seed[index]);
    return true;
}

void WriteIdentity(const std::string& path, unsigned int principalId, const unsigned char uuid[16])
{
    PcOsMakeDirectory(path.substr(0, path.find_last_of("/\\")).c_str());
    FILE* file = fopen(path.c_str(), "w");
    if (file == 0)
        return;
    fprintf(file, "principal=%08x\nuuid=", principalId);
    for (int index = 0; index < 16; ++index)
        fprintf(file, "%02x", uuid[index]);
    fprintf(file, "\n");
    if (s_hasSigningSeed)
    {
        fprintf(file, "key=");
        for (int index = 0; index < 32; ++index)
            fprintf(file, "%02x", s_signingSeed[index]);
        fprintf(file, "\n");
    }
    fclose(file);
}

bool EnsureSigningSeed()
{
    if (s_hasSigningSeed)
        return false;
    s_hasSigningSeed = PcOsRandomBytes(s_signingSeed, sizeof(s_signingSeed));
    if (!s_hasSigningSeed)
    {
        printf("[pc] identity: no secure random source; lobbies that check identities will refuse this game\n");
        fflush(stdout);
    }
    return s_hasSigningSeed;
}

bool ExistingInstall()
{
    struct stat info;
    return stat((PcSaveRoot() + "/80000001").c_str(), &info) == 0;
}

void LoadIdentity()
{
    if (s_identityLoaded)
        return;
    s_identityLoaded = true;
    const std::string userDirectory = s_identityDirectory.empty() ? PcOsUserDataDirectory() : s_identityDirectory;
    const std::string userPath = userDirectory.empty() ? std::string() : userDirectory + "/identity.dat";
    const std::string savePath = PcSaveRoot() + "/pcidentity.dat";
    if (!userPath.empty())
    {
        struct stat info;
        const char* const extras[] = {".dat", ".txt"};
        for (unsigned int index = 0; index < 2 && stat(userPath.c_str(), &info) != 0; ++index)
        {
            const std::string misnamed = userPath + extras[index];
            if (stat(misnamed.c_str(), &info) == 0 && PcOsReplaceFile(misnamed.c_str(), userPath.c_str()))
            {
                printf("[pc] identity: using %s as identity.dat\n", misnamed.c_str());
                fflush(stdout);
            }
        }
    }
    if (!userPath.empty() && ReadIdentity(userPath, &s_principalId, s_accountUuid))
    {
        if (EnsureSigningSeed())
            WriteIdentity(userPath, s_principalId, s_accountUuid);
        return;
    }
    if (!ReadIdentity(savePath, &s_principalId, s_accountUuid))
    {
        if (ExistingInstall() && s_playerName != kDefaultPlayerName)
            NameIdentity(&s_principalId, s_accountUuid);
        else
            RandomIdentity(&s_principalId, s_accountUuid);
    }
    EnsureSigningSeed();
    WriteIdentity(userPath.empty() ? savePath : userPath, s_principalId, s_accountUuid);
}
}

void PcSetIdentityDirectory(const std::string& directory)
{
    s_identityDirectory = directory;
}

namespace
{
struct GuestIdentity
{
    bool loaded;
    unsigned int principalId;
    unsigned char uuid[16];
    unsigned char seed[32];
    bool hasSeed;
    std::string name;
};

const int kGuestIdentities = 3;
GuestIdentity s_guests[kGuestIdentities];

std::string GuestIdentityPath(int guest)
{
    char file[40];
    const std::string userDirectory = s_identityDirectory.empty() ? PcOsUserDataDirectory() : s_identityDirectory;
    if (userDirectory.empty())
    {
        snprintf(file, sizeof(file), "/pcidentity_player%d.dat", guest + 1);
        return PcSaveRoot() + file;
    }
    snprintf(file, sizeof(file), "/identity_player%d.dat", guest + 1);
    return userDirectory + file;
}

bool ReadHex(const char* text, unsigned char* out, int count)
{
    for (int index = 0; index < count; ++index)
    {
        unsigned int value;
        if (sscanf(text + index * 2, "%2x", &value) != 1)
            return false;
        out[index] = static_cast<unsigned char>(value);
    }
    return true;
}

void WriteGuestIdentity(const std::string& path, const GuestIdentity& identity)
{
    PcOsMakeDirectory(path.substr(0, path.find_last_of("/\\")).c_str());
    FILE* file = fopen(path.c_str(), "w");
    if (file == 0)
        return;
    fprintf(file, "principal=%08x\nuuid=", identity.principalId);
    for (int index = 0; index < 16; ++index)
        fprintf(file, "%02x", identity.uuid[index]);
    fprintf(file, "\n");
    if (identity.hasSeed)
    {
        fprintf(file, "key=");
        for (int index = 0; index < 32; ++index)
            fprintf(file, "%02x", identity.seed[index]);
        fprintf(file, "\n");
    }
    fprintf(file, "name=%s\n", identity.name.c_str());
    fclose(file);
}

GuestIdentity& LoadGuestIdentity(int guest)
{
    GuestIdentity& identity = s_guests[guest - 1];
    if (identity.loaded)
        return identity;
    identity.loaded = true;
    identity.principalId = 0;
    identity.hasSeed = false;
    char defaultName[16];
    snprintf(defaultName, sizeof(defaultName), "Player%d", guest + 1);
    identity.name = defaultName;
    const std::string path = GuestIdentityPath(guest);
    bool complete = false;
    bool hasUuid = false;
    FILE* file = fopen(path.c_str(), "r");
    if (file != 0)
    {
        char line[160];
        while (fgets(line, sizeof(line), file) != 0)
        {
            line[strcspn(line, "\r\n")] = 0;
            unsigned int value;
            if (sscanf(line, "principal=%8x", &value) == 1)
                identity.principalId = value;
            else if (strncmp(line, "uuid=", 5) == 0)
                hasUuid = ReadHex(line + 5, identity.uuid, 16);
            else if (strncmp(line, "key=", 4) == 0)
                identity.hasSeed = ReadHex(line + 4, identity.seed, 32);
            else if (strncmp(line, "name=", 5) == 0 && line[5] != 0)
                identity.name = std::string(line + 5).substr(0, 16);
        }
        fclose(file);
        complete = hasUuid && identity.principalId != 0 && identity.hasSeed;
    }
    if (!hasUuid || identity.principalId == 0)
        RandomIdentity(&identity.principalId, identity.uuid);
    if (!identity.hasSeed)
        identity.hasSeed = PcOsRandomBytes(identity.seed, sizeof(identity.seed));
    if (!complete)
    {
        WriteGuestIdentity(path, identity);
        printf("[pc] identity: split-screen player %d is %s (%s)\n", guest + 1, identity.name.c_str(), path.c_str());
        fflush(stdout);
    }
    return identity;
}
}

void PcGuestAccountUuid(int guest, unsigned char uuid[16])
{
    memcpy(uuid, LoadGuestIdentity(guest).uuid, 16);
}

std::string PcGuestName(int guest)
{
    return LoadGuestIdentity(guest).name;
}

bool PcGuestAccountSign(int guest, const unsigned char* message, size_t size, unsigned char publicKey[32],
                        unsigned char signature[64])
{
    GuestIdentity& identity = LoadGuestIdentity(guest);
    if (!identity.hasSeed)
        return false;
    PcEd25519PublicKey(identity.seed, publicKey);
    PcEd25519Sign(identity.seed, message, size, signature);
    return true;
}

unsigned int PcPrincipalId()
{
    LoadIdentity();
    return s_principalId;
}

void PcAccountUuid(unsigned char uuid[16])
{
    LoadIdentity();
    memcpy(uuid, s_accountUuid, sizeof(s_accountUuid));
}

bool PcAccountSign(const unsigned char* message, size_t size, unsigned char publicKey[32],
                   unsigned char signature[64])
{
    LoadIdentity();
    if (!s_hasSigningSeed)
        return false;
    PcEd25519PublicKey(s_signingSeed, publicKey);
    PcEd25519Sign(s_signingSeed, message, size, signature);
    return true;
}

namespace nn
{
namespace act
{
unsigned char GetSlotNoEx(const ACTUuid&) asm("GetSlotNoEx__Q2_2nn3actFRC7ACTUuid");

nn::Result Initialize()
{
    return nn::Result();
}

uint8_t GetNumOfAccounts()
{
    return 1;
}

SlotNo GetSlotNo()
{
    return kAccountSlot;
}

unsigned char GetSlotNoEx(const ACTUuid& uuid)
{
    ACTUuid account;
    PcAccountUuid(account.data);
    return uuid == account ? kAccountSlot : 0;
}

BOOL IsSlotOccupied(SlotNo slot)
{
    return slot == kAccountSlot ? TRUE : FALSE;
}

nn::Result GetUuid(char outUuid[UuidSize])
{
    PcAccountUuid(reinterpret_cast<unsigned char*>(outUuid));
    return nn::Result();
}

BOOL IsNetworkAccount()
{
    return TRUE;
}

nn::Result GetAccountId(char outAccountId[AccountIdSize])
{
    memset(outAccountId, 0, AccountIdSize);
    strncpy(outAccountId, s_playerName.c_str(), AccountIdSize - 1);
    return nn::Result();
}

PrincipalId GetPrincipalId()
{
    return PcPrincipalId();
}
}

namespace fp
{
nn::Result Initialize()
{
    return nn::Result();
}

nn::Result UpdateGameMode(nn::fp::GameMode*, const char16_t*, unsigned int)
{
    return nn::Result();
}

nn::Result UpdateGameModeDescription(const char16_t*)
{
    return nn::Result();
}

uint32_t ResultToErrorCode(nn::Result)
{
    return 0;
}

nn::Result GetFriendList(nn::act::PrincipalId*, unsigned int* outPrincipalBufferReadCount, unsigned int, unsigned int)
{
    *outPrincipalBufferReadCount = 0;
    return nn::Result();
}

nn::Result GetFriendAccountId(char*, nn::act::PrincipalId*, unsigned int)
{
    return nn::Result();
}
}
}

UCHandle UCOpen()
{
    return 1;
}

UCError UCClose(UCHandle)
{
    return UC_ERROR_OK;
}

UCError UCReadSysConfig(UCHandle, uint32_t count, UCSysConfig* settings)
{
    for (uint32_t i = 0; i < count; ++i)
    {
        UCSysConfig& config = settings[i];
        if (strcmp(config.name, "cafe.language") == 0)
            WriteConfigValue(&config, kCafeLanguageEnglish);
        else if (strcmp(config.name, "cafe.cntry_reg") == 0)
            WriteConfigValue(&config, kCafeCountryUnitedStates);
        else
            WriteConfigValue(&config, 0);
        config.error = UC_ERROR_OK;
    }
    return UC_ERROR_OK;
}

extern "C" int MCP_Open()
{
    return 1;
}

extern "C" void MCP_Close(int)
{
}

extern "C" int MCP_GetSysProdSettings(int, void* settings)
{
    memset(settings, 0, 0x46);
    static_cast<unsigned char*>(settings)[3] = kProductAreaUsa;
    return 0;
}

extern "C" int AOC_Initialize()
{
    return 0;
}

extern "C" int AOC_Finalize()
{
    return 0;
}

extern "C" unsigned int AOC_CalculateWorkBufferSize(unsigned int)
{
    return 64;
}

extern "C" int AOC_ListTitle(unsigned int* titleCount, AOCTitle*, unsigned int, void*, unsigned int)
{
    *titleCount = 0;
    return 0;
}

extern "C" int AOC_OpenTitle(char*, AOCTitle*, void*, unsigned int)
{
    return -1;
}

extern "C" int AOC_CloseTitle(AOCTitle*)
{
    return 0;
}

extern "C" int AOC_GetPurchaseInfo(int*, unsigned long long, unsigned short*, unsigned int, void*, unsigned int)
{
    return -1;
}

int32_t SYSGetArguments(SYSArgDataBlock*, SYSCallerInfo*)
{
    return -1;
}
