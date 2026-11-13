#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcFunctionHooks.hpp"
#include "platform/pc/PcModHooks.hpp"
#include "platform/pc/PcModViews.hpp"
#include "platform/pc/PcModStorage.hpp"
#include "platform/pc/PcModStructures.hpp"
#include "platform/pc/PcModSounds.hpp"

#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcScripts.hpp"

#include "game/core/Minecraft.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Textures.hpp"
#include "platform/pc/PcNativeMod.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <vpad/input.h>
#include <windows.h>

#include <boost/unordered_map.hpp>
#include <cxxabi.h>
#include <stdlib.h>
#endif

extern const char kPcContentVersion[];
extern const char kPcAbiFlags[];
extern const char kPcHeaderHashes[];

#if defined(_WIN64)
static_assert(offsetof(PcNativeViews, parts) == 38908, "PcNativeViews: a field before parts moved");
static_assert(sizeof(PcNativeViewArm) == 68, "PcNativeViewArm changed size (it is in an array)");
static_assert(sizeof(PcNativeView) == 92, "PcNativeView changed size (it is in an array)");
static_assert(sizeof(PcNativeViewPart) == 84, "PcNativeViewPart changed size (it is in an array)");
static_assert(sizeof(PcNativeViewBox) == 84, "PcNativeViewBox changed size (it is in an array)");
static_assert(sizeof(PcNativeViewItem) == 72, "PcNativeViewItem changed size (it is in an array)");
static_assert(sizeof(PcNativeViewText) == 120, "PcNativeViewText changed size (it is in an array)");
static_assert(sizeof(PcNativePad) == 36, "PcNativePad changed size");
#endif

namespace
{
bool nativeEnabled = true;
bool nativeStarted = false;
bool nativeLoaded = false;

struct Callback
{
    PcNativeCallback function;
    void* user;
    size_t mod;
};

struct NativeMod
{
    std::string id;
    std::string directory;
    PcNativeHost host;
    PcNativeRegister reg;
    void* dll;
    bool started;
    bool failed;
    int hudHidden;
};

std::vector<NativeMod*> nativeMods;
std::vector<Callback> frameCallbacks;
std::vector<Callback> hostStartCallbacks;
std::vector<Callback> hostTickCallbacks;
std::vector<Callback> hostStopCallbacks;

struct ViewCallback
{
    PcNativeViewCallback function;
    void* user;
    size_t mod;
};

std::vector<ViewCallback> viewFrameCallbacks;
std::vector<Callback> hudCallbacks;
std::vector<PcScriptHudItem>* hudItems = 0;
std::vector<ViewCallback> viewDoneCallbacks;
std::vector<ViewCallback> viewDrawCallbacks;
std::map<std::string, int> modTextures;
thread_local size_t currentMod = (size_t)(-1);

const char* CurrentId()
{
    return currentMod < nativeMods.size() ? nativeMods[currentMod]->id.c_str() : "?";
}

size_t ModFor(void* from)
{
    if (currentMod < nativeMods.size())
        return currentMod;
#ifdef _WIN32
    HMODULE dll = 0;
    if (from != 0
        && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                              (LPCWSTR)from, &dll))
    {
        for (size_t i = 0; i < nativeMods.size(); i++)
            if (nativeMods[i]->dll == (void*)dll)
                return i;
    }
#endif
    return (size_t)(-1);
}

void Log(const char* msg)
{
    size_t m = ModFor(__builtin_return_address(0));
    printf("[pc] native %s: %s\n", m < nativeMods.size() ? nativeMods[m]->id.c_str() : "?", msg != 0 ? msg : "");
    fflush(stdout);
}

void NotBoth(const char* what, const char* name)
{
    printf("[pc] native %s: %s \"%s\" not added: only a mod with \"side\": \"both\" in its mod.json can add "
           "one (every player's game needs it)\n", PcNativeModsCurrentId(), what, name != 0 ? name : "");
    fflush(stdout);
}

void NoBlock(const char* name, const char*, int, PcNativeMakeBlock)
{
    NotBoth("block", name);
}

void NoItem(const char* name, const char*, int, PcNativeMakeItem)
{
    NotBoth("item", name);
}

unsigned long long NoBlockEntity(const char* name, unsigned long long, PcNativeMakeBlockEntity)
{
    NotBoth("block entity", name);
    return 0;
}

unsigned long long NoEntity(const char* name, unsigned long long, PcNativeMakeEntity)
{
    NotBoth("entity", name);
    return 0;
}

int NoEffect(const char* name, const char*, PcNativeMakeEffect)
{
    NotBoth("effect", name);
    return 0;
}

int NoEnchantment(const char* name, const char*, PcNativeMakeEnchantment)
{
    NotBoth("enchantment", name);
    return 0;
}

int NoStructure(const char* name, int, PcNativeMakeStructure, PcNativeStructureMobs)
{
    NotBoth("structure", name);
    return 0;
}

unsigned int NoStructureStart(const char* name, PcNativeMakeStructureStart)
{
    NotBoth("structure start", name);
    return 0;
}

unsigned int NoStructurePiece(const char* name, PcNativeMakeStructurePiece)
{
    NotBoth("structure piece", name);
    return 0;
}

void* NoSound(const char* name, const char*, float)
{
    NotBoth("sound", name);
    return 0;
}

void HideHud(int parts)
{
    size_t m = ModFor(__builtin_return_address(0));
    if (m < nativeMods.size())
        nativeMods[m]->hudHidden = parts & (PC_NATIVE_HUD_CROSSHAIR | PC_NATIVE_HUD_HINTS);
}

void Add(std::vector<Callback>* list, PcNativeCallback fn, void* user)
{
    if (fn == 0)
        return;
    Callback cb;
    cb.function = fn;
    cb.user = user;
    cb.mod = currentMod;
    list->push_back(cb);
}

void OnFrame(PcNativeCallback fn, void* user)
{
    Add(&frameCallbacks, fn, user);
}

void OnHostStart(PcNativeCallback fn, void* user)
{
    Add(&hostStartCallbacks, fn, user);
}

void OnHostTick(PcNativeCallback fn, void* user)
{
    Add(&hostTickCallbacks, fn, user);
}

void OnHostStop(PcNativeCallback fn, void* user)
{
    Add(&hostStopCallbacks, fn, user);
}

void AddView(std::vector<ViewCallback>* list, PcNativeViewCallback fn, void* user)
{
    if (fn == 0)
        return;
    ViewCallback cb;
    cb.function = fn;
    cb.user = user;
    cb.mod = currentMod;
    list->push_back(cb);
}

bool hideMenus = false;

void SetHideMenus(int on)
{
    hideMenus = on != 0;
}

void SendKey(int key, int down)
{
    PcOsWindowEvent event;
    memset(&event, 0, sizeof(event));
    event.type = ePcOsWindowEvent_Key;
    event.key = key;
    event.down = down != 0;
    PcInputHandleEvent(event);
}

void SendChar(unsigned int character)
{
    PcOsWindowEvent event;
    memset(&event, 0, sizeof(event));
    event.type = ePcOsWindowEvent_Text;
    event.character = character;
    PcInputHandleEvent(event);
}

void SetPad(const PcNativePad* pad)
{
    static const unsigned int buttonMap[16] = {VPAD_BUTTON_A,      VPAD_BUTTON_B,       VPAD_BUTTON_X,    VPAD_BUTTON_Y,
                                              VPAD_BUTTON_L,      VPAD_BUTTON_R,       VPAD_BUTTON_ZL,   VPAD_BUTTON_ZR,
                                              VPAD_BUTTON_STICK_L, VPAD_BUTTON_STICK_R, VPAD_BUTTON_PLUS, VPAD_BUTTON_MINUS,
                                              VPAD_BUTTON_UP,     VPAD_BUTTON_DOWN,    VPAD_BUTTON_LEFT, VPAD_BUTTON_RIGHT};
    PcInputModPad st;
    memset(&st, 0, sizeof(st));
    if (pad != 0)
    {
        for (int b = 0; b < 16; ++b)
        {
            if (pad->buttons & (1u << b))
                st.hold |= buttonMap[b];
        }
        st.leftX = pad->leftX;
        st.leftY = pad->leftY;
        st.rightX = pad->rightX;
        st.rightY = pad->rightY;
        st.pointer = pad->pointer != 0;
        st.pointerX = pad->pointerX;
        st.pointerY = pad->pointerY;
        st.press = pad->pointerPress != 0;
        st.chat = (pad->buttons & PC_NATIVE_PAD_CHAT) != 0;
        st.sneak = (pad->buttons & PC_NATIVE_PAD_SNEAK) != 0;
    }
    PcInputSetModPad(st);
}

std::wstring WidenUtf8(const char* s)
{
    std::wstring out;
    if (s == 0)
        return out;
    int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, 0, 0);
    if (len > 1)
    {
        out.resize(len);
        MultiByteToWideChar(CP_UTF8, 0, s, -1, &out[0], len);
        out.resize(len - 1);
    }
    return out;
}

void OnHud(PcNativeCallback fn, void* user)
{
    Add(&hudCallbacks, fn, user);
}

void DrawRect(int x1, int y1, int x2, int y2, unsigned int argb)
{
    if (hudItems == 0)
        return;
    PcScriptHudItem item;
    item.text = false;
    item.shadow = false;
    item.x1 = x1;
    item.y1 = y1;
    item.x2 = x2;
    item.y2 = y2;
    item.colour = argb;
    hudItems->push_back(item);
}

void DrawText(const char* text, int x, int y, unsigned int argb)
{
    if (hudItems == 0 || text == 0)
        return;
    PcScriptHudItem item;
    item.text = true;
    item.shadow = true;
    item.x1 = x;
    item.y1 = y;
    item.x2 = x;
    item.y2 = y;
    item.colour = argb;
    item.string = WidenUtf8(text);
    hudItems->push_back(item);
}

int TextWidth(const char* text)
{
    Minecraft* mc = Minecraft::GetInstance();
    return mc != 0 && mc->mFont != 0 ? mc->mFont->width(WidenUtf8(text)) : 0;
}

void OnViewFrame(PcNativeViewCallback fn, void* user)
{
    AddView(&viewFrameCallbacks, fn, user);
}

void OnViewDone(PcNativeViewCallback fn, void* user)
{
    AddView(&viewDoneCallbacks, fn, user);
}

void OnViewDraw(PcNativeViewCallback fn, void* user)
{
    AddView(&viewDrawCallbacks, fn, user);
}

int LoadTexture(const char* file)
{
    size_t m = ModFor(__builtin_return_address(0));
    if (m >= nativeMods.size() || file == 0 || file[0] == 0 || strstr(file, "..") != 0 || file[0] == '/'
        || file[0] == '\\' || strchr(file, ':') != 0)
        return 0;
    std::string key = nativeMods[m]->id + "/" + file;
    std::map<std::string, int>::iterator it = modTextures.find(key);
    if (it != modTextures.end())
        return it->second;
    int tex = 0;
    std::string path = nativeMods[m]->directory + "/textures/" + file;
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp != 0)
    {
        std::vector<unsigned char> data;
        unsigned char buf[65536];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0 && data.size() < 16 * 1024 * 1024)
            data.insert(data.end(), buf, buf + n);
        fclose(fp);
        BufferedImage* img = data.empty() ? 0 : new BufferedImage(&data[0], (unsigned int)data.size());
        Minecraft* mc = Minecraft::GetInstance();
        if (img != 0 && img->getWidth() > 0 && mc != 0 && mc->mTextures != 0)
            tex = mc->mTextures->getTexture(img, C4JRender::eTextureFormat_RGBA8, false);
        delete img;
    }
    if (tex == 0)
    {
        printf("[pc] native %s: texture %s could not be read\n", nativeMods[m]->id.c_str(), path.c_str());
        fflush(stdout);
    }
    modTextures[key] = tex;
    return tex;
}

void RunViews(const std::vector<ViewCallback>& list, PcNativeViews* views)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
        const size_t prev = currentMod;
        currentMod = list[i].mod;
        list[i].function(views, list[i].user);
        currentMod = prev;
    }
}

void Run(const std::vector<Callback>& list)
{
    for (size_t i = 0; i < list.size(); ++i)
    {
        const size_t prev = currentMod;
        currentMod = list[i].mod;
        list[i].function(list[i].user);
        currentMod = prev;
    }
}

void Drop(std::vector<Callback>* list, size_t mod)
{
    for (size_t i = list->size(); i > 0; --i)
    {
        if ((*list)[i - 1].mod == mod)
            list->erase(list->begin() + (long)(i - 1));
    }
}

bool IsFile(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

#ifdef _WIN32
const char* const DLL_NAME = sizeof(void*) == 8 ? "native/win64.dll" : "native/win32.dll";

bool ReadAt(FILE* fp, long off, void* buf, size_t len)
{
    return fseek(fp, off, SEEK_SET) == 0 && fread(buf, 1, len, fp) == len;
}

bool ReadSections(FILE* fp, IMAGE_FILE_HEADER* header, std::vector<IMAGE_SECTION_HEADER>* sections)
{
    IMAGE_DOS_HEADER dos;
    DWORD signature = 0;
    if (!ReadAt(fp, 0, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE
        || !ReadAt(fp, dos.e_lfanew, &signature, sizeof(signature)) || signature != IMAGE_NT_SIGNATURE
        || !ReadAt(fp, dos.e_lfanew + 4, header, sizeof(*header)) || header->NumberOfSections > 256)
        return false;
    sections->resize(header->NumberOfSections);
    const long start = dos.e_lfanew + 4 + (long)sizeof(*header) + header->SizeOfOptionalHeader;
    return header->NumberOfSections == 0
           || ReadAt(fp, start, &(*sections)[0], sizeof(IMAGE_SECTION_HEADER) * header->NumberOfSections);
}

bool ReadRecord(const std::string& path, PcNativeModRecord* rec, std::string* err)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
    {
        *err = "cannot be read";
        return false;
    }
    IMAGE_FILE_HEADER header;
    std::vector<IMAGE_SECTION_HEADER> sections;
    bool found = false;
    bool ok = ReadSections(fp, &header, &sections);
    for (size_t i = 0; ok && i < sections.size() && !found; ++i)
    {
        if (strncmp(reinterpret_cast<const char*>(sections[i].Name), ".pcmod", 8) != 0)
            continue;
        found = sections[i].SizeOfRawData >= sizeof(*rec)
                && ReadAt(fp, (long)sections[i].PointerToRawData, rec, sizeof(*rec));
    }
    fclose(fp);
    if (!ok)
        *err = "is not a Windows DLL";
    else if (!found)
        *err = "has no native mod record (PC_NATIVE_MOD_RECORD)";
    return ok && found;
}

std::string ReadAbiList(const std::string& path)
{
    std::string list;
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return list;
    IMAGE_FILE_HEADER header;
    std::vector<IMAGE_SECTION_HEADER> sections;
    if (ReadSections(fp, &header, &sections))
    {
        for (size_t i = 0; i < sections.size(); ++i)
        {
            if (strncmp((const char*)sections[i].Name, ".pcabi", 8) != 0)
                continue;
            DWORD size = sections[i].SizeOfRawData;
            if (size > 0 && size < 4 * 1024 * 1024)
            {
                list.resize(size);
                if (!ReadAt(fp, (long)sections[i].PointerToRawData, &list[0], size))
                    list.clear();
            }
            break;
        }
    }
    fclose(fp);
    size_t end = list.find('\0');
    if (end != std::string::npos)
        list.resize(end);
    return list;
}

std::map<std::string, std::string>* headerHashes = 0;

bool AbiMatches(const std::string& list, std::string* bad, int* used)
{
    if (headerHashes == 0)
    {
        headerHashes = new std::map<std::string, std::string>();
        const char* p = kPcHeaderHashes;
        while (*p)
        {
            const char* sp = strchr(p, ' ');
            const char* nl = strchr(p, '\n');
            if (sp == 0 || nl == 0 || sp > nl)
                break;
            (*headerHashes)[std::string(p, sp - p)] = std::string(sp + 1, nl - sp - 1);
            p = nl + 1;
        }
    }
    bool flags = false;
    int n = 0;
    int changed = 0;
    size_t pos = 0;
    while (pos < list.size())
    {
        size_t nl = list.find('\n', pos);
        if (nl == std::string::npos)
            nl = list.size();
        std::string line = list.substr(pos, nl - pos);
        pos = nl + 1;
        size_t sp = line.find(' ');
        if (sp == std::string::npos)
            continue;
        std::string name = line.substr(0, sp);
        std::string hash = line.substr(sp + 1);
        if (name == "flags")
        {
            flags = hash == kPcAbiFlags;
            continue;
        }
        if (name == "platform/pc/PcNativeMod.h")
            continue;
        ++n;
        std::map<std::string, std::string>::iterator it = headerHashes->find(name);
        if (it != headerHashes->end() && it->second == hash)
            continue;
        if (++changed <= 5)
        {
            if (!bad->empty())
                *bad += ", ";
            *bad += name;
        }
    }
    if (changed > 5)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), " and %d more", changed - 5);
        *bad += buf;
    }
    if (!flags)
        *bad = bad->empty() ? "the build settings" : "the build settings, " + *bad;
    *used = n;
    return flags && changed == 0;
}

boost::unordered_map<std::string, void*>* gameSymbols = 0;

void LoadSymbols()
{
    gameSymbols = new boost::unordered_map<std::string, void*>();
    wchar_t exe[MAX_PATH];
    const DWORD len = GetModuleFileNameW(0, exe, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return;
    FILE* fp = _wfopen(exe, L"rb");
    if (fp == 0)
        return;
    IMAGE_FILE_HEADER header;
    std::vector<IMAGE_SECTION_HEADER> sections;
    if (!ReadSections(fp, &header, &sections) || header.PointerToSymbolTable == 0 || header.NumberOfSymbols == 0)
    {
        fclose(fp);
        printf("[pc] native: this executable has no symbol table; native mods cannot find the game's code\n");
        return;
    }
    std::vector<unsigned char> table((size_t)header.NumberOfSymbols * IMAGE_SIZEOF_SYMBOL);
    DWORD strSize = 0;
    const long strAt = (long)(header.PointerToSymbolTable + table.size());
    std::vector<char> strings;
    bool ok = ReadAt(fp, (long)header.PointerToSymbolTable, &table[0], table.size())
                && ReadAt(fp, strAt, &strSize, sizeof(strSize)) && strSize >= 4;
    if (ok)
    {
        strings.resize(strSize + 1, 0);
        ok = ReadAt(fp, strAt, &strings[0], strSize);
    }
    fclose(fp);
    if (!ok)
        return;
    char* base = reinterpret_cast<char*>(GetModuleHandleW(0));
    for (DWORD i = 0; i < header.NumberOfSymbols; ++i)
    {
        const IMAGE_SYMBOL* sym = reinterpret_cast<const IMAGE_SYMBOL*>(&table[i * IMAGE_SIZEOF_SYMBOL]);
        i += sym->NumberOfAuxSymbols;
        if (sym->StorageClass != IMAGE_SYM_CLASS_EXTERNAL || sym->SectionNumber <= 0
            || sym->SectionNumber > static_cast<SHORT>(sections.size()))
            continue;
        std::string name;
        if (sym->N.Name.Short != 0)
            name.assign(reinterpret_cast<const char*>(sym->N.ShortName),
                        strnlen(reinterpret_cast<const char*>(sym->N.ShortName), 8));
        else if (sym->N.Name.Long < strSize)
            name = &strings[sym->N.Name.Long];
        if (!name.empty())
            (*gameSymbols)[name] = base + sections[sym->SectionNumber - 1].VirtualAddress + sym->Value;
    }
}

void* FindSymbol(const char* name)
{
    if (name == 0)
        return 0;
    if (gameSymbols == 0)
        LoadSymbols();
    boost::unordered_map<std::string, void*>::const_iterator it = gameSymbols->find(name);
    if (it == gameSymbols->end() && sizeof(void*) == 4)
        it = gameSymbols->find(std::string("_") + name);
    return it != gameSymbols->end() ? it->second : 0;
}

struct PlainName
{
    std::string name;
    std::string key;
    void* addr;
};
std::vector<PlainName>* plainNames = 0;

std::string NoSpaces(const std::string& s)
{
    std::string out;
    for (size_t i = 0; i < s.size(); i++)
        if (s[i] != ' ')
            out += s[i];
    return out;
}

void LoadPlainNames()
{
    plainNames = new std::vector<PlainName>();
    if (gameSymbols == 0)
        LoadSymbols();
    for (boost::unordered_map<std::string, void*>::const_iterator it = gameSymbols->begin(); it != gameSymbols->end(); ++it)
    {
        const char* sym = it->first.c_str();
        if (sizeof(void*) == 4 && sym[0] == '_')
            sym++;
        if (strncmp(sym, "_Z", 2) != 0)
            continue;
        int status = 0;
        char* d = abi::__cxa_demangle(sym, 0, 0, &status);
        if (d == 0)
            continue;
        if (status == 0 && strchr(d, '(') != 0 && strstr(d, "thunk to ") == 0)
        {
            PlainName pn;
            pn.name = d;
            pn.key = NoSpaces(d);
            pn.addr = it->second;
            plainNames->push_back(pn);
        }
        free(d);
    }
}

template <class T> T* At(HMODULE base, DWORD rva)
{
    return reinterpret_cast<T*>(reinterpret_cast<char*>(base) + rva);
}

void Patch(ULONG_PTR* slot, ULONG_PTR value)
{
    DWORD prot;
    VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &prot);
    *slot = value;
    VirtualProtect(slot, sizeof(*slot), prot, &prot);
}

bool ResolveImports(HMODULE dll, std::string* err)
{
    IMAGE_NT_HEADERS* nt = At<IMAGE_NT_HEADERS>(dll, At<IMAGE_DOS_HEADER>(dll, 0)->e_lfanew);
    const IMAGE_DATA_DIRECTORY& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.Size == 0)
        return true;
    unsigned int missing = 0;
    for (IMAGE_IMPORT_DESCRIPTOR* imp = At<IMAGE_IMPORT_DESCRIPTOR>(dll, dir.VirtualAddress);
         imp->Name != 0; ++imp)
    {
        const char* libName = At<const char>(dll, imp->Name);
        const bool game = _stricmp(libName, "pcgame.dll") == 0;
        HMODULE libDll = game ? 0 : LoadLibraryA(libName);
        if (!game && libDll == 0)
        {
            *err = std::string("needs ") + libName + ", which is not there";
            return false;
        }
        IMAGE_THUNK_DATA* lookup =
            At<IMAGE_THUNK_DATA>(dll, imp->OriginalFirstThunk != 0 ? imp->OriginalFirstThunk : imp->FirstThunk);
        IMAGE_THUNK_DATA* iat = At<IMAGE_THUNK_DATA>(dll, imp->FirstThunk);
        for (; lookup->u1.AddressOfData != 0; ++lookup, ++iat)
        {
            void* target = 0;
            const char* name = "(by ordinal)";
            if (IMAGE_SNAP_BY_ORDINAL(lookup->u1.Ordinal))
                target = game ? 0
                              : reinterpret_cast<void*>(GetProcAddress(
                                    libDll, reinterpret_cast<LPCSTR>(IMAGE_ORDINAL(lookup->u1.Ordinal))));
            else
            {
                name = reinterpret_cast<const char*>(
                    At<IMAGE_IMPORT_BY_NAME>(dll, (DWORD)lookup->u1.AddressOfData)->Name);
                target = game ? FindSymbol(name) : reinterpret_cast<void*>(GetProcAddress(libDll, name));
            }
            if (target == 0)
            {
                if (++missing <= 10)
                    printf("[pc] native %s: %s has no %s\n", CurrentId(), game ? "the game" : libName, name);
                continue;
            }
            Patch(reinterpret_cast<ULONG_PTR*>(&iat->u1.Function), reinterpret_cast<ULONG_PTR>(target));
        }
    }
    if (missing > 0)
    {
        char buf[96];
        snprintf(buf, sizeof(buf), "uses %u function(s) or variable(s) this game does not have", missing);
        *err = buf;
        return false;
    }
    return true;
}

typedef BOOL(WINAPI* EntryFn)(HINSTANCE, DWORD, LPVOID);

bool StartModule(HMODULE dll)
{
    IMAGE_NT_HEADERS* nt = At<IMAGE_NT_HEADERS>(dll, At<IMAGE_DOS_HEADER>(dll, 0)->e_lfanew);
    const IMAGE_DATA_DIRECTORY& tls = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (tls.Size != 0)
    {
        IMAGE_TLS_DIRECTORY* dir = At<IMAGE_TLS_DIRECTORY>(dll, tls.VirtualAddress);
        PIMAGE_TLS_CALLBACK* cb = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(dir->AddressOfCallBacks);
        for (; cb != 0 && *cb != 0; ++cb)
            (*cb)(dll, DLL_PROCESS_ATTACH, 0);
    }
    if (nt->OptionalHeader.AddressOfEntryPoint == 0)
        return true;
    EntryFn entry =
        reinterpret_cast<EntryFn>(reinterpret_cast<char*>(dll) + nt->OptionalHeader.AddressOfEntryPoint);
    return entry(reinterpret_cast<HINSTANCE>(dll), DLL_PROCESS_ATTACH, 0) != FALSE;
}

std::wstring WidePath(const std::string& path)
{
    std::wstring out;
    const int len = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, 0, 0);
    if (len > 0)
    {
        out.resize((size_t)len);
        MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, &out[0], len);
        out.resize((size_t)(len - 1));
    }
    for (size_t i = 0; i < out.size(); ++i)
    {
        if (out[i] == L'/')
            out[i] = L'\\';
    }
    return out;
}

void StartMod(const PcMod& m)
{
    const std::string path = m.directory + "/" + DLL_NAME;
    PcNativeModRecord rec;
    std::string err;
    if (!ReadRecord(path, &rec, &err))
    {
        printf("[pc] native %s: %s %s; not loaded\n", m.id.c_str(), DLL_NAME, err.c_str());
        return;
    }
    rec.gameBuild[sizeof(rec.gameBuild) - 1] = 0;
    if (memcmp(rec.magic, "PCNATIVE", 8) != 0 || rec.abi != PC_NATIVE_ABI
        || rec.pointerBits != sizeof(void*) * 8)
    {
        printf("[pc] native %s: %s was built for game version %s (%u-bit), and this game is %s (%u-bit); "
               "not loaded: it must be built again for this game\n",
               m.id.c_str(), DLL_NAME, rec.gameBuild, rec.pointerBits, kPcContentVersion,
               (unsigned int)(sizeof(void*) * 8));
        return;
    }
    if (strcmp(rec.gameBuild, kPcContentVersion) != 0)
    {
        std::string list = ReadAbiList(path);
        std::string bad;
        int used = 0;
        if (list.empty() || !AbiMatches(list, &bad, &used))
        {
            printf("[pc] native %s: %s was built for game version %s, and this game is %s", m.id.c_str(),
                   DLL_NAME, rec.gameBuild, kPcContentVersion);
            if (!bad.empty())
                printf(" (changed since: %s)", bad.c_str());
            printf("; not loaded: it must be built again for this game\n");
            return;
        }
        printf("[pc] native %s: %s was built for game version %s; the %d game headers it uses are the same "
               "in this game (%s), so it is loaded\n", m.id.c_str(), DLL_NAME, rec.gameBuild, used,
               kPcContentVersion);
    }

    NativeMod* nm = new NativeMod();
    nm->id = m.id;
    nm->directory = m.directory;
    nm->dll = 0;
    nm->started = false;
    nm->failed = false;
    nm->hudHidden = 0;
    nativeMods.push_back(nm);
    const size_t idx = nativeMods.size() - 1;
    const size_t prev = currentMod;
    currentMod = idx;

    HMODULE dll = LoadLibraryExW(WidePath(path).c_str(), 0, DONT_RESOLVE_DLL_REFERENCES);
    if (dll == 0)
        err = "could not be loaded";
    else if (!ResolveImports(dll, &err))
        ;
    else if (!StartModule(dll))
        err = "refused to start (its DllMain returned FALSE)";
    else
    {
        nm->dll = dll;
        nm->started = true;
        PcNativeModRegisterFn reg = (PcNativeModRegisterFn)GetProcAddress(dll, PC_NATIVE_REGISTER);
        if (reg != 0)
        {
            memset(&nm->reg, 0, sizeof(nm->reg));
            nm->reg.abi = PC_NATIVE_ABI;
            nm->reg.size = sizeof(nm->reg);
            nm->reg.gameBuild = kPcContentVersion;
            nm->reg.modId = nm->id.c_str();
            nm->reg.modDirectory = nm->directory.c_str();
            nm->reg.log = Log;
            PcModHooksFillRegister(&nm->reg);
            nm->reg.hookFunction = PcHookFunction;
            PcModStorageFillRegister(&nm->reg);
            PcModStructuresFillRegister(&nm->reg);
            PcModSoundsFillRegister(&nm->reg);
            if (m.side != "both")
            {
                nm->reg.addBlock = NoBlock;
                nm->reg.addItem = NoItem;
                nm->reg.addBlockEntity = NoBlockEntity;
                nm->reg.addEntity = NoEntity;
                nm->reg.addEffect = NoEffect;
                nm->reg.addEnchantment = NoEnchantment;
                nm->reg.addStructure = NoStructure;
                nm->reg.addStructureStart = NoStructureStart;
                nm->reg.addStructurePiece = NoStructurePiece;
                nm->reg.addSound = NoSound;
            }
            const int rc = reg(&nm->reg);
            if (rc != 0)
            {
                char buf[64];
                snprintf(buf, sizeof(buf), "returned %d from " PC_NATIVE_REGISTER, rc);
                err = buf;
                nm->failed = true;
            }
        }
    }
    currentMod = prev;
    if (nm->started && !nm->failed)
        return;
    nm->failed = true;
    printf("[pc] native %s: %s %s; not loaded\n", m.id.c_str(), DLL_NAME, err.c_str());
    fflush(stdout);
    if (dll != 0 && !nm->started)
        FreeLibrary(dll);
}

void RunMain(size_t idx)
{
    NativeMod* nm = nativeMods[idx];
    if (nm->failed || !nm->started)
        return;
    const size_t prev = currentMod;
    currentMod = idx;
    std::string err;
    bool ok = false;
    PcNativeModMainFn modMain = (PcNativeModMainFn)GetProcAddress((HMODULE)nm->dll, PC_NATIVE_ENTRY);
    if (modMain == 0)
        err = "has no " PC_NATIVE_ENTRY;
    else
    {
        PcNativeHost& host = nm->host;
        memset(&host, 0, sizeof(host));
        host.abi = PC_NATIVE_ABI;
        host.size = sizeof(host);
        host.gameBuild = kPcContentVersion;
        host.modId = nm->id.c_str();
        host.modDirectory = nm->directory.c_str();
        host.dedicatedServer = PcIsDedicatedServer() ? 1 : 0;
        host.log = Log;
        host.findSymbol = FindSymbol;
        host.onFrame = OnFrame;
        host.onHostStart = OnHostStart;
        host.onHostTick = OnHostTick;
        host.onHostStop = OnHostStop;
        host.onViewFrame = OnViewFrame;
        host.onViewDone = OnViewDone;
        host.setPad = SetPad;
        host.onHud = OnHud;
        host.drawRect = DrawRect;
        host.drawText = DrawText;
        host.textWidth = TextWidth;
        host.setHideMenus = SetHideMenus;
        host.hideHud = HideHud;
        host.hudRect = PcModViewsHudRect;
        host.onViewDraw = OnViewDraw;
        host.loadTexture = LoadTexture;
        host.sendKey = SendKey;
        host.sendChar = SendChar;
        PcModHooksFillHost(&host);
        PcModStorageFillHost(&host);
        const int rc = modMain(&host);
        ok = rc == 0;
        if (!ok)
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "returned %d from " PC_NATIVE_ENTRY, rc);
            err = buf;
        }
    }
    currentMod = prev;
    if (ok)
    {
        printf("[pc] native %s: %s started\n", nm->id.c_str(), DLL_NAME);
        fflush(stdout);
        return;
    }
    nm->failed = true;
    printf("[pc] native %s: %s %s; not loaded\n", nm->id.c_str(), DLL_NAME, err.c_str());
    fflush(stdout);
    Drop(&frameCallbacks, idx);
    Drop(&hostStartCallbacks, idx);
    Drop(&hostTickCallbacks, idx);
    Drop(&hostStopCallbacks, idx);
}
#endif
}

void* PcNativeModsFindFunction(const char* name, std::string* found, std::string* err)
{
#ifdef _WIN32
    if (name == 0 || name[0] == 0)
    {
        *err = "no function name given";
        return 0;
    }
    void* p = FindSymbol(name);
    if (p != 0)
    {
        *found = name;
        return p;
    }
    if (plainNames == 0)
        LoadPlainNames();
    std::string want = NoSpaces(name);
    bool full = want.find('(') != std::string::npos;
    std::vector<size_t> hits;
    for (size_t i = 0; i < plainNames->size(); i++)
    {
        const std::string& key = (*plainNames)[i].key;
        bool match;
        if (full)
            match = key == want;
        else
            match = key.size() > want.size() && key.compare(0, want.size(), want) == 0 && key[want.size()] == '(';
        if (!match)
            continue;
        bool same = false;
        for (size_t j = 0; j < hits.size(); j++)
            if ((*plainNames)[hits[j]].addr == (*plainNames)[i].addr)
                same = true;
        if (!same)
            hits.push_back(i);
    }
    if (hits.empty())
    {
        *err = "the game has no function called that (or the compiler left it out, as nothing used it)";
        return 0;
    }
    if (hits.size() > 1)
    {
        *err = "more than one function is called that, give the full signature:";
        for (size_t i = 0; i < hits.size() && i < 6; i++)
            *err += " \"" + (*plainNames)[hits[i]].name + "\"";
        if (hits.size() > 6)
            *err += " ...";
        return 0;
    }
    *found = (*plainNames)[hits[0]].name;
    return (*plainNames)[hits[0]].addr;
#else
    *err = "native mods need the Windows build";
    return 0;
#endif
}

void PcNativeModsSetEnabled(bool on)
{
    nativeEnabled = on;
}

void PcNativeModsRegister()
{
    if (nativeStarted)
        return;
    nativeStarted = true;
    const std::vector<PcMod>& list = PcModsLoaded();
    for (size_t i = 0; i < list.size(); ++i)
    {
        const PcMod& m = list[i];
        const bool hasNative = IsFile(m.directory + "/native/win64.dll") || IsFile(m.directory + "/native/win32.dll");
        if (!hasNative)
            continue;
        if (!nativeEnabled)
        {
            printf("[pc] native %s: not loaded (--no-native-mods)\n", m.id.c_str());
            continue;
        }
#ifdef _WIN32
        if (!IsFile(m.directory + "/" + DLL_NAME))
        {
            printf("[pc] native %s: has no %s for this %u-bit game; not loaded\n", m.id.c_str(), DLL_NAME,
                   (unsigned int)(sizeof(void*) * 8));
            continue;
        }
        if (m.side == "client" && PcIsDedicatedServer())
        {
            printf("[pc] native %s: a client mod; not loaded on a dedicated server\n", m.id.c_str());
            continue;
        }
        StartMod(m);
#else
        printf("[pc] native %s: native mods need the Windows build; not loaded\n", m.id.c_str());
#endif
    }
    fflush(stdout);
}

void PcNativeModsLoad()
{
    if (nativeLoaded)
        return;
    nativeLoaded = true;
    PcNativeModsRegister();
#ifdef _WIN32
    for (size_t i = 0; i < nativeMods.size(); ++i)
        RunMain(i);
#endif
    fflush(stdout);
}

const char* PcNativeModsIdFor(void* from)
{
    size_t m = ModFor(from);
    return m < nativeMods.size() ? nativeMods[m]->id.c_str() : 0;
}

std::string PcNativeModsDirectoryOf(const char* id)
{
    for (size_t i = 0; id != 0 && i < nativeMods.size(); i++)
        if (nativeMods[i]->id == id)
            return nativeMods[i]->directory;
    return std::string();
}

const char* PcNativeModsCurrentId()
{
    return CurrentId();
}

size_t PcNativeModsCurrent()
{
    return currentMod;
}

void PcNativeModsSetCurrent(size_t mod)
{
    currentMod = mod;
}

void PcNativeModsFrame()
{
    Run(frameCallbacks);
}

void PcNativeModsHostStart()
{
    Run(hostStartCallbacks);
}

void PcNativeModsHostTick()
{
    Run(hostTickCallbacks);
}

void PcNativeModsHostStop()
{
    Run(hostStopCallbacks);
}

bool PcNativeModsHideMenus()
{
    return hideMenus;
}

bool PcNativeModsWantHud()
{
    return !hudCallbacks.empty();
}

void PcNativeModsHud(std::vector<PcScriptHudItem>* out)
{
    hudItems = out;
    Run(hudCallbacks);
    hudItems = 0;
}

bool PcNativeModsWantViews()
{
    return !viewFrameCallbacks.empty();
}

void PcNativeModsViewFrame(PcNativeViews* views)
{
    RunViews(viewFrameCallbacks, views);
}

void PcNativeModsViewDone(PcNativeViews* views)
{
    RunViews(viewDoneCallbacks, views);
}

bool PcNativeModsWantViewDraw()
{
    return !viewDrawCallbacks.empty();
}

void PcNativeModsViewDraw(PcNativeViews* views)
{
    RunViews(viewDrawCallbacks, views);
}

bool PcNativeModsHudHidden(int part)
{
    for (size_t i = 0; i < nativeMods.size(); i++)
        if (!nativeMods[i]->failed && (nativeMods[i]->hudHidden & part) != 0)
            return true;
    return false;
}
