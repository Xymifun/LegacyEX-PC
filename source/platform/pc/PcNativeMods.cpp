#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcModHooks.hpp"

#include "platform/pc/PcDedicated.hpp"
#include "platform/pc/PcMods.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcScripts.hpp"

#include "game/core/Minecraft.hpp"
#include "game/client/render/Font.hpp"
#include "platform/pc/PcNativeMod.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include <vector>

#ifdef _WIN32
#include <vpad/input.h>
#include <windows.h>

#include <boost/unordered_map.hpp>
#endif

extern const char kPcContentVersion[];

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
thread_local size_t currentMod = (size_t)(-1);

const char* CurrentId()
{
    return currentMod < nativeMods.size() ? nativeMods[currentMod]->id.c_str() : "?";
}

void Log(const char* msg)
{
    printf("[pc] native %s: %s\n", CurrentId(), msg != 0 ? msg : "");
    fflush(stdout);
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
        || rec.pointerBits != sizeof(void*) * 8 || strcmp(rec.gameBuild, kPcContentVersion) != 0)
    {
        printf("[pc] native %s: %s was built for game version %s (%u-bit), and this game is %s (%u-bit); "
               "not loaded: it must be built again for this game\n",
               m.id.c_str(), DLL_NAME, rec.gameBuild, rec.pointerBits, kPcContentVersion,
               (unsigned int)(sizeof(void*) * 8));
        return;
    }

    NativeMod* nm = new NativeMod();
    nm->id = m.id;
    nm->directory = m.directory;
    nm->dll = 0;
    nm->started = false;
    nm->failed = false;
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
        host.sendKey = SendKey;
        host.sendChar = SendChar;
        PcModHooksFillHost(&host);
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
