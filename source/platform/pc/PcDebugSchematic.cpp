#include "platform/pc/PcDebugSchematic.hpp"

#include "platform/pc/PcOs.hpp"

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"

#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

namespace
{
const long long MAX_BLOCKS = 32768 * 8;
const char kFolder[] = "schematics";

ConsoleSchematicFile* clipboard = NULL;
std::vector<unsigned char> clipboardBytes;
Level* pasteLevel = NULL;
int s_area[6];

struct StatusLock
{
    StatusLock() { InitializeCriticalSection(&lock); }
    OSFastMutex lock;
};
StatusLock& Lock()
{
    static StatusLock instance;
    return instance;
}
std::wstring s_status;
unsigned int s_revision = 0;

void SetStatus(const std::wstring& st)
{
    EnterCriticalSection(&Lock().lock);
    s_status = st;
    ++s_revision;
    LeaveCriticalSection(&Lock().lock);
    printf("[pc] schematic: %ls\n", st.c_str());
    fflush(stdout);
}

std::string Describe(Level* lvl, const int* area)
{
    std::map<int, int> counts;
    for (int x = area[0]; x < area[3]; ++x)
        for (int y = area[1]; y < area[4]; ++y)
            for (int z = area[2]; z < area[5]; ++z)
                ++counts[Block::getIdWithData(lvl->getBlockState(BlockPos(x, y, z)))];
    std::string s;
    int shown = 0;
    for (std::map<int, int>::iterator it = counts.begin(); it != counts.end() && shown < 8; ++it, ++shown)
    {
        char e[32];
        snprintf(e, sizeof(e), "%s%d:%d x%d", s.empty() ? "" : ", ", it->first & 0xFFF, it->first >> 12,
                 it->second);
        s += e;
    }
    return s;
}

void SetArea(int x0, int y0, int z0, int x1, int y1, int z1)
{
    s_area[0] = x0;
    s_area[1] = y0;
    s_area[2] = z0;
    s_area[3] = x1;
    s_area[4] = y1;
    s_area[5] = z1;
}

std::wstring Format(const wchar_t* format, int a, int b, int c)
{
    wchar_t s[128];
    swprintf(s, 128, format, a, b, c);
    return s;
}

ConsoleSchematicFile* LoadBytes(const std::vector<unsigned char>& bytes)
{
    if (bytes.size() < 20)
        return NULL;
    arrayWithLength<unsigned char> data((unsigned int)bytes.size(), true);
    memcpy(data.data, &bytes[0], bytes.size());
    ByteArrayInputStream byteStream(data);
    DataInputStream input(&byteStream);
    ConsoleSchematicFile* schem = new ConsoleSchematicFile();
    schem->load(&input);
    if (schem->mWidth <= 0 || schem->mHeight <= 0 || schem->mDepth <= 0)
    {
        delete schem;
        return NULL;
    }
    return schem;
}

bool Pasting()
{
    if (clipboard == NULL || !clipboard->mHasCopy)
        return false;
    SetStatus(L"Still pasting");
    return true;
}

void SetClipboard(ConsoleSchematicFile* schem, const std::vector<unsigned char>& bytes)
{
    delete clipboard;
    clipboard = schem;
    clipboardBytes = bytes;
}

std::string FileName(const std::wstring& name)
{
    std::string path = std::string(kFolder) + "/";
    for (size_t i = 0; i < name.size(); ++i)
    {
        wchar_t c = name[i];
        bool safe = (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z') || (c >= L'0' && c <= L'9') || c == L'_'
                    || c == L'-' || c == L' ';
        path.push_back(safe ? (char)c : '_');
    }
    return path + ".sch";
}
}

void PcDebugSchematicCopy(Level* lvl, const PcSchematicCopy& copy)
{
    if (Pasting())
        return;
    const int x0 = copy.x0 < copy.x1 ? copy.x0 : copy.x1, x1 = (copy.x0 < copy.x1 ? copy.x1 : copy.x0) + 1;
    const int y0 = copy.y0 < copy.y1 ? copy.y0 : copy.y1, y1 = (copy.y0 < copy.y1 ? copy.y1 : copy.y0) + 1;
    const int z0 = copy.z0 < copy.z1 ? copy.z0 : copy.z1, z1 = (copy.z0 < copy.z1 ? copy.z1 : copy.z0) + 1;
    const long long vol = (long long)(x1 - x0) * (y1 - y0) * (z1 - z0);
    if (y0 < 0 || y1 > 256 || vol > MAX_BLOCKS)
    {
        SetStatus(L"Too big to copy");
        return;
    }
    ByteArrayOutputStream byteStream;
    DataOutputStream output(&byteStream);
    if (!ConsoleSchematicFile().generateSchematicFile(&output, lvl, x0, y0, z0, x1, y1, z1,
                                                       Compression::eCompressionTypes_RLE))
    {
        SetStatus(L"Nothing to copy");
        return;
    }
    std::vector<unsigned char> bytes(byteStream.mBuffer.data, byteStream.mBuffer.data + byteStream.mCount);
    ConsoleSchematicFile* schem = LoadBytes(bytes);
    if (schem == NULL)
    {
        SetStatus(L"The copy could not be read back");
        return;
    }
    SetClipboard(schem, bytes);
    SetArea(x0, y0, z0, x1, y1, z1);
    printf("[pc] schematic: copied blocks %s\n", Describe(lvl, s_area).c_str());
    SetStatus(Format(L"Copied %d x %d x %d", x1 - x0, y1 - y0, z1 - z0));
}

void PcDebugSchematicPaste(Level* lvl, const PcSchematicPaste& paste)
{
    if (clipboard == NULL)
    {
        SetStatus(L"Copy or load something first");
        return;
    }
    if (Pasting())
        return;
    const ConsoleSchematicFile::ESchematicRotation rotation =
        static_cast<ConsoleSchematicFile::ESchematicRotation>(paste.rotation & 3);
    const bool turned =
        rotation == ConsoleSchematicFile::eSchematicRotation_90 || rotation == ConsoleSchematicFile::eSchematicRotation_270;
    const int w = turned ? clipboard->mDepth : clipboard->mWidth;
    const int depth = turned ? clipboard->mWidth : clipboard->mDepth;
    int h = clipboard->mHeight;
    if (paste.y < 0 || paste.y >= 256)
    {
        SetStatus(L"Cannot paste there");
        return;
    }
    if (paste.y + h > 256)
        h = 256 - paste.y;
    clipboard->storeUndoSchematic(lvl, paste.x, paste.y, paste.z, paste.x + w, paste.y + h,
                                    paste.z + depth);
    AABB* area = AABB::newTemp(paste.x, paste.y, paste.z, paste.x + w, paste.y + h, paste.z + depth);
    clipboard->mMirror = static_cast<ConsoleSchematicFile::ESchematicMirror>(paste.mirror & 3);
    clipboard->_applySchematicData(lvl, area, rotation, clipboard->mElements, clipboard->mExtraElements,
                                     clipboard->mData, clipboard->mExtraData, clipboard->mBlockEntities,
                                     clipboard->mEntities, clipboard->mNamedPositions, clipboard->mWidth,
                                     clipboard->mHeight, clipboard->mDepth, true, true, true);
    pasteLevel = lvl;
    SetArea(paste.x, paste.y, paste.z, paste.x + w, paste.y + h, paste.z + depth);
    SetStatus(Format(L"Pasting at %d %d %d", paste.x, paste.y, paste.z));
}

void PcDebugSchematicUndo(Level* lvl)
{
    if (clipboard == NULL || clipboard->mHasCopy || !clipboard->undoPrevious(lvl))
    {
        SetStatus(L"Nothing to undo");
        return;
    }
    pasteLevel = lvl;
    AABB* area = clipboard->mUndoArea;
    SetArea((int)area->mMinX, (int)area->mMinY, (int)area->mMinZ,
            (int)area->mMaxX, (int)area->mMaxY, (int)area->mMaxZ);
    SetStatus(L"Undoing the paste");
}

void PcDebugSchematicTick()
{
    if (clipboard == NULL || !clipboard->mHasCopy)
        return;
    clipboard->tick();
    if (!clipboard->mHasCopy)
    {
        printf("[pc] schematic: the area now holds %s\n", Describe(pasteLevel, s_area).c_str());
        SetStatus(L"Done");
    }
}

void PcDebugSchematicSave(const std::wstring& name)
{
    if (clipboardBytes.empty())
    {
        SetStatus(L"Copy something first");
        return;
    }
    if (name.empty())
    {
        SetStatus(L"Type a name first");
        return;
    }
    PcOsMakeDirectory(kFolder);
    const std::string path = FileName(name);
    FILE* fp = fopen(path.c_str(), "wb");
    if (fp == NULL || fwrite(&clipboardBytes[0], 1, clipboardBytes.size(), fp) != clipboardBytes.size())
    {
        if (fp != NULL)
            fclose(fp);
        SetStatus(L"Could not write " + std::wstring(path.begin(), path.end()));
        return;
    }
    fclose(fp);
    SetStatus(L"Saved " + std::wstring(path.begin(), path.end()));
}

void PcDebugSchematicLoad(const std::wstring& name)
{
    if (Pasting())
        return;
    const std::string path = FileName(name);
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == NULL)
    {
        SetStatus(L"No file " + std::wstring(path.begin(), path.end()));
        return;
    }
    std::vector<unsigned char> bytes;
    unsigned char buf[65536];
    size_t read;
    while ((read = fread(buf, 1, sizeof(buf), fp)) > 0)
        bytes.insert(bytes.end(), buf, buf + read);
    fclose(fp);
    ConsoleSchematicFile* schem = LoadBytes(bytes);
    if (schem == NULL)
    {
        SetStatus(L"Not a schematic: " + std::wstring(path.begin(), path.end()));
        return;
    }
    SetClipboard(schem, bytes);
    SetStatus(Format(L"Loaded %d x %d x %d", schem->mWidth, schem->mHeight, schem->mDepth));
}

std::wstring PcDebugSchematicStatus(unsigned int* revision)
{
    EnterCriticalSection(&Lock().lock);
    std::wstring st = s_status;
    if (revision != NULL)
        *revision = s_revision;
    LeaveCriticalSection(&Lock().lock);
    return st;
}
