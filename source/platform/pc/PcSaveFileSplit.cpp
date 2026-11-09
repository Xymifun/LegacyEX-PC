#include "platform/pc/PcSaveFileSplit.hpp"

#include "game/core/File.hpp"
#include "game/core/System.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcSaveRegions.hpp"
#include "platform/wiiu/C4JStorage.hpp"

#include <string.h>
#include <sys/stat.h>

#include <algorithm>

extern C4JStorage* gStorage;

namespace
{
const char WORK_SUFFIX[] = ".work";
const unsigned int MAX_OPEN_FILES = 48;
const unsigned int COPY_BUFFER_SIZE = 1 << 16;
const unsigned int SEEK_FROM_END = 2;

bool EndsWith(const std::string& s, const char* suffix)
{
    const size_t len = strlen(suffix);
    return s.size() > len && s.compare(s.size() - len, len, suffix) == 0;
}

unsigned int FileSize(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 ? (unsigned int)st.st_size : 0;
}

bool CopyWholeFile(const std::string& from, const std::string& to)
{
    FILE* in = fopen(from.c_str(), "rb");
    if (in == NULL)
        return false;
    FILE* out = fopen(to.c_str(), "wb");
    if (out == NULL)
    {
        fclose(in);
        return false;
    }
    std::vector<char> buf(COPY_BUFFER_SIZE);
    bool ok = true;
    for (;;)
    {
        const size_t n = fread(&buf[0], 1, buf.size(), in);
        if (n == 0)
            break;
        if (fwrite(&buf[0], 1, n, out) != n)
        {
            ok = false;
            break;
        }
    }
    ok = ok && !ferror(in);
    fclose(in);
    return fclose(out) == 0 && ok;
}
}

PcSaveFileSplit::PcSaveFileSplit(const std::wstring& filename, void* data, unsigned int dataSize,
    ESavePlatform platform)
    : ConsoleSaveFileOriginal(filename, data, dataSize, false, platform), mUseCounter(0), mBytesRead(0), mBytesWritten(0)
{
    UseDirectoryFor(CurrentSaveName());
    ScanDirectory();
    MigrateContainerRegions();
}

PcSaveFileSplit::~PcSaveFileSplit()
{
    LockSaveAccess();
    unsigned int discarded = 0;
    bool anySaved = false;
    for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
    {
        Region* r = it->second;
        CloseHandle(r);
        if (r->working)
        {
            PcOsRemoveFile(WorkPath(r).c_str());
            ++discarded;
        }
        anySaved = anySaved || r->committed;
        delete r;
    }
    mRegions.clear();
    mRegionsByEntry.clear();
    if (!anySaved && !mDirectory.empty())
        PcOsRemoveDirectory(mDirectory.c_str());
    ReleaseSaveAccess();
    printf("[pc] save: closed regions (read %llu KB, wrote %llu KB this session)\n", mBytesRead / 1024,
           mBytesWritten / 1024);
    if (discarded != 0)
        printf("[pc] save: discarded unsaved changes to %u region files\n", discarded);
    fflush(stdout);
}

bool PcSaveFileSplit::IsRegionName(const std::wstring& name)
{
    return name.size() > 4 && name.compare(name.size() - 4, 4, L".mcr") == 0;
}

std::string PcSaveFileSplit::DiskName(const std::wstring& name)
{
    std::string out;
    out.reserve(name.size());
    for (size_t i = 0; i < name.size(); ++i)
    {
        const wchar_t c = name[i];
        if (c == L'/' || c == L'\\')
            out += '#';
        else
            out += c < 0x80 ? (char)c : '_';
    }
    return out;
}

std::wstring PcSaveFileSplit::SaveName(const std::string& disk)
{
    std::wstring out;
    out.reserve(disk.size());
    for (size_t i = 0; i < disk.size(); ++i)
        out += disk[i] == '#' ? L'/' : (wchar_t)((unsigned char)disk[i]);
    return out;
}

std::string PcSaveFileSplit::CurrentSaveName()
{
    char buf[128];
    memset(buf, 0, sizeof(buf));
    if (gStorage != NULL)
        gStorage->GetSaveUniqueFileDir(buf);
    buf[sizeof(buf) - 1] = 0;
    return std::string(buf);
}

void PcSaveFileSplit::UseDirectoryFor(const std::string& saveName)
{
    std::string name = saveName;
    if (name.empty())
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "unsaved-%llu", PcOsRealTimeMilliseconds());
        name = buf;
    }
    mSaveName = saveName;
    mDirectory = PcSaveRegionDirectory(name);
    PcOsMakeDirectory(mDirectory.c_str());
}

void PcSaveFileSplit::ScanDirectory()
{
    std::vector<std::string> names;
    PcListDirectory(mDirectory, &names);
    unsigned int n = 0;
    for (size_t i = 0; i < names.size(); ++i)
    {
        const std::string& name = names[i];
        if (EndsWith(name, WORK_SUFFIX))
        {
            PcOsRemoveFile((mDirectory + "/" + name).c_str());
            continue;
        }
        if (!EndsWith(name, ".mcr"))
            continue;
        Region* r = FindOrCreate(SaveName(name));
        r->committed = true;
        r->entry.m_data.m_fileSize = FileSize(mDirectory + "/" + name);
        ++n;
    }
    if (n != 0)
    {
        printf("[pc] save: %u region files in %s\n", n, mDirectory.c_str());
        fflush(stdout);
    }
}

void PcSaveFileSplit::MigrateContainerRegions()
{
    std::vector<FileEntry*>* files = ConsoleSaveFileOriginal::getFilesWithPrefix(L"");
    if (files == NULL)
        return;
    std::vector<FileEntry*> regions;
    for (size_t i = 0; i < files->size(); ++i)
    {
        if (IsRegionName((*files)[i]->m_fileName))
            regions.push_back((*files)[i]);
    }
    delete files;
    unsigned int moved = 0;
    for (size_t i = 0; i < regions.size(); ++i)
    {
        FileEntry* e = regions[i];
        Region* r = FindOrCreate(e->m_fileName);
        if (!r->committed && EnsureWorking(r))
        {
            FILE* fp = Open(r, true);
            const unsigned char* src = static_cast<const unsigned char*>(mData) + e->m_data.m_startOffset;
            if (fp != NULL && fseek(fp, 0, SEEK_SET) == 0
                && fwrite(src, 1, e->m_data.m_fileSize, fp) == e->m_data.m_fileSize)
            {
                r->entry.m_data.m_fileSize = e->m_data.m_fileSize;
                ++moved;
            }
        }
        ConsoleSaveFileOriginal::deleteFile(e);
    }
    if (!regions.empty())
    {
        printf("[pc] save: moved %u of %u region files out of the save file into %s\n", moved,
               (unsigned int)regions.size(), mDirectory.c_str());
        fflush(stdout);
    }
}

PcSaveFileSplit::Region* PcSaveFileSplit::Find(const std::wstring& name)
{
    std::map<std::wstring, Region*>::iterator it = mRegions.find(name);
    return it == mRegions.end() ? NULL : it->second;
}

PcSaveFileSplit::Region* PcSaveFileSplit::FindOrCreate(const std::wstring& name)
{
    Region* r = Find(name);
    if (r != NULL)
        return r;
    r = new Region;
    r->diskName = DiskName(name);
    r->committed = false;
    r->working = false;
    r->deleted = false;
    r->handle = NULL;
    r->handleWritable = false;
    r->lastUse = 0;
    r->entry.m_fileName = name;
    const size_t len = std::min(name.size(), sizeof(r->entry.m_data.m_name) / sizeof(wchar_t) - 1);
    for (size_t i = 0; i < len; ++i)
        r->entry.m_data.m_name[i] = name[i];
    r->entry.m_data.m_fileSize = 0;
    r->entry.m_data.m_startOffset = 0;
    r->entry.m_data.m_timestamp = 0;
    r->entry.m_filePointer = 0;
    r->entry.m_isNewFile = true;
    mRegions[name] = r;
    mRegionsByEntry[&r->entry] = r;
    return r;
}

PcSaveFileSplit::Region* PcSaveFileSplit::RegionFor(FileEntry* entry)
{
    std::map<FileEntry*, Region*>::iterator it = mRegionsByEntry.find(entry);
    return it == mRegionsByEntry.end() ? NULL : it->second;
}

bool PcSaveFileSplit::Exists(const Region* r) const
{
    return r != NULL && (r->working || (r->committed && !r->deleted));
}

std::string PcSaveFileSplit::CommittedPath(const Region* r) const
{
    return mDirectory + "/" + r->diskName;
}

std::string PcSaveFileSplit::WorkPath(const Region* r) const
{
    return mDirectory + "/" + r->diskName + WORK_SUFFIX;
}

void PcSaveFileSplit::CloseHandle(Region* r)
{
    if (r->handle != NULL)
    {
        fclose(r->handle);
        r->handle = NULL;
    }
}

void PcSaveFileSplit::LimitOpenHandles()
{
    unsigned int openCount = 0;
    Region* oldest = NULL;
    for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
    {
        Region* r = it->second;
        if (r->handle == NULL)
            continue;
        ++openCount;
        if (oldest == NULL || r->lastUse < oldest->lastUse)
            oldest = r;
    }
    if (openCount >= MAX_OPEN_FILES && oldest != NULL)
        CloseHandle(oldest);
}

FILE* PcSaveFileSplit::Open(Region* r, bool writable)
{
    r->lastUse = ++mUseCounter;
    if (r->handle != NULL && (r->handleWritable || !writable))
        return r->handle;
    CloseHandle(r);
    std::string path;
    if (r->working)
        path = WorkPath(r);
    else if (r->committed && !r->deleted && !writable)
        path = CommittedPath(r);
    else
        return NULL;
    LimitOpenHandles();
    r->handle = fopen(path.c_str(), r->working ? "r+b" : "rb");
    r->handleWritable = r->working;
    if (r->handle == NULL)
    {
        printf("[pc] save: cannot open %s\n", path.c_str());
        fflush(stdout);
    }
    return r->handle;
}

bool PcSaveFileSplit::EnsureWorking(Region* r)
{
    if (r->working)
        return true;
    CloseHandle(r);
    const std::string workPath = WorkPath(r);
    bool ok;
    if (r->committed && !r->deleted)
        ok = CopyWholeFile(CommittedPath(r), workPath);
    else
    {
        FILE* fp = fopen(workPath.c_str(), "wb");
        ok = fp != NULL && fclose(fp) == 0;
        r->entry.m_data.m_fileSize = 0;
    }
    if (!ok)
    {
        printf("[pc] save: cannot create %s\n", workPath.c_str());
        fflush(stdout);
        return false;
    }
    r->working = true;
    return true;
}

bool PcSaveFileSplit::WriteAt(Region* r, const void* buf, unsigned int len, bool zero)
{
    if (!EnsureWorking(r))
        return false;
    FILE* fp = Open(r, true);
    if (fp == NULL)
        return false;
    unsigned int& fileSize = r->entry.m_data.m_fileSize;
    unsigned int pos = r->entry.m_filePointer;
    static const unsigned char zeroes[4096] = {0};
    if (pos > fileSize)
    {
        if (fseek(fp, fileSize, SEEK_SET) != 0)
            return false;
        for (unsigned int gap = pos - fileSize; gap != 0;)
        {
            const unsigned int n = std::min(gap, (unsigned int)sizeof(zeroes));
            if (fwrite(zeroes, 1, n, fp) != n)
                return false;
            gap -= n;
        }
    }
    else if (fseek(fp, pos, SEEK_SET) != 0)
        return false;
    if (zero)
    {
        for (unsigned int left = len; left != 0;)
        {
            const unsigned int n = std::min(left, (unsigned int)sizeof(zeroes));
            if (fwrite(zeroes, 1, n, fp) != n)
                return false;
            left -= n;
        }
    }
    else if (len != 0 && fwrite(buf, 1, len, fp) != len)
        return false;
    mBytesWritten += len;
    pos += len;
    if (pos > fileSize)
        fileSize = pos;
    r->entry.m_filePointer = pos;
    r->entry.m_data.m_timestamp = System::currentRealTimeMillis();
    return true;
}

FileEntry* PcSaveFileSplit::createFile(const ConsoleSavePath& path)
{
    const std::wstring name = path.getPath();
    if (!IsRegionName(name))
        return ConsoleSaveFileOriginal::createFile(path);
    LockSaveAccess();
    Region* r = FindOrCreate(name);
    if (!Exists(r))
        r->entry.m_data.m_fileSize = 0;
    ReleaseSaveAccess();
    return &r->entry;
}

void PcSaveFileSplit::deleteFile(FileEntry* entry)
{
    LockSaveAccess();
    Region* r = RegionFor(entry);
    if (r == NULL)
    {
        ReleaseSaveAccess();
        ConsoleSaveFileOriginal::deleteFile(entry);
        return;
    }
    CloseHandle(r);
    if (r->working)
        PcOsRemoveFile(WorkPath(r).c_str());
    r->working = false;
    r->deleted = true;
    r->entry.m_data.m_fileSize = 0;
    r->entry.m_filePointer = 0;
    ReleaseSaveAccess();
}

void PcSaveFileSplit::setFilePointer(FileEntry* entry, long distanceToMove, int* distanceToMoveHigh,
    unsigned int moveMethod)
{
    LockSaveAccess();
    Region* r = RegionFor(entry);
    if (r == NULL)
    {
        ReleaseSaveAccess();
        ConsoleSaveFileOriginal::setFilePointer(entry, distanceToMove, distanceToMoveHigh, moveMethod);
        return;
    }
    long pos = distanceToMove;
    if (moveMethod == SEEK_FROM_END)
        pos += (long)r->entry.m_data.m_fileSize;
    r->entry.m_filePointer = pos < 0 ? 0 : (unsigned int)pos;
    ReleaseSaveAccess();
}

bool PcSaveFileSplit::writeFile(FileEntry* entry, const void* buffer, unsigned int bytesToWrite,
    unsigned int* bytesWritten)
{
    LockSaveAccess();
    Region* r = RegionFor(entry);
    if (r == NULL)
    {
        ReleaseSaveAccess();
        return ConsoleSaveFileOriginal::writeFile(entry, buffer, bytesToWrite, bytesWritten);
    }
    const bool ok = WriteAt(r, buffer, bytesToWrite, false);
    *bytesWritten = ok ? bytesToWrite : 0;
    ReleaseSaveAccess();
    return ok;
}

bool PcSaveFileSplit::zeroFile(FileEntry* entry, unsigned int bytesToZero, unsigned int* bytesWritten)
{
    LockSaveAccess();
    Region* r = RegionFor(entry);
    if (r == NULL)
    {
        ReleaseSaveAccess();
        return ConsoleSaveFileOriginal::zeroFile(entry, bytesToZero, bytesWritten);
    }
    const bool ok = WriteAt(r, NULL, bytesToZero, true);
    *bytesWritten = ok ? bytesToZero : 0;
    ReleaseSaveAccess();
    return ok;
}

bool PcSaveFileSplit::readFile(FileEntry* entry, void* buffer, unsigned int bytesToRead, unsigned int* bytesRead)
{
    LockSaveAccess();
    Region* r = RegionFor(entry);
    if (r == NULL)
    {
        ReleaseSaveAccess();
        return ConsoleSaveFileOriginal::readFile(entry, buffer, bytesToRead, bytesRead);
    }
    *bytesRead = 0;
    const unsigned int len = r->entry.m_data.m_fileSize;
    const unsigned int pos = r->entry.m_filePointer;
    unsigned int n = pos >= len ? 0 : std::min(bytesToRead, len - pos);
    if (n != 0)
    {
        FILE* fp = Open(r, false);
        if (fp == NULL || fseek(fp, pos, SEEK_SET) != 0)
            n = 0;
        else
            n = (unsigned int)fread(buffer, 1, n, fp);
    }
    mBytesRead += n;
    *bytesRead = n;
    r->entry.m_filePointer = pos + n;
    ReleaseSaveAccess();
    return true;
}

bool PcSaveFileSplit::doesFileExist(ConsoleSavePath path)
{
    const std::wstring name = path.getPath();
    if (!IsRegionName(name))
        return ConsoleSaveFileOriginal::doesFileExist(path);
    LockSaveAccess();
    const bool exists = Exists(Find(name));
    ReleaseSaveAccess();
    return exists;
}

bool PcSaveFileSplit::fjHasRegionFile(const File& file)
{
    return doesFileExist(ConsoleSavePath(file.getPath()));
}

void PcSaveFileSplit::Flush(bool bForce, bool bBlocking)
{
    LockSaveAccess();
    const std::string name = CurrentSaveName();
    if (!name.empty() && name != mSaveName)
    {
        const std::string oldDir = mDirectory;
        UseDirectoryFor(name);
        for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
        {
            Region* r = it->second;
            CloseHandle(r);
            const std::string oldPath = oldDir + "/" + r->diskName;
            if (r->working)
                PcOsReplaceFile((oldPath + WORK_SUFFIX).c_str(), WorkPath(r).c_str());
            if (r->committed)
            {
                if (r->deleted)
                    r->committed = false;
                else
                    r->committed = PcOsReplaceFile(oldPath.c_str(), CommittedPath(r).c_str());
            }
        }
        PcOsRemoveDirectory(oldDir.c_str());
    }
    unsigned int written = 0;
    unsigned int failed = 0;
    for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
    {
        Region* r = it->second;
        if (r->working)
        {
            CloseHandle(r);
            if (PcOsReplaceFile(WorkPath(r).c_str(), CommittedPath(r).c_str()))
            {
                r->working = false;
                r->committed = true;
                r->deleted = false;
                ++written;
            }
            else
                ++failed;
        }
        else if (r->deleted && r->committed)
        {
            CloseHandle(r);
            PcOsRemoveFile(CommittedPath(r).c_str());
            r->committed = false;
            r->deleted = false;
        }
    }
    if (written != 0 || failed != 0)
    {
        printf("[pc] save: wrote %u region files%s\n", written, failed != 0 ? " (some failed)" : "");
        fflush(stdout);
    }
    ReleaseSaveAccess();
    ConsoleSaveFileOriginal::Flush(bForce, bBlocking);
}

unsigned int PcSaveFileSplit::getSizeOnDisk()
{
    LockSaveAccess();
    unsigned long long total = ConsoleSaveFileOriginal::getSizeOnDisk();
    for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
    {
        if (Exists(it->second))
            total += it->second->entry.m_data.m_fileSize;
    }
    ReleaseSaveAccess();
    return total > 0xFFFFFFFFull ? 0xFFFFFFFFu : (unsigned int)total;
}

std::vector<FileEntry*>* PcSaveFileSplit::getFilesWithPrefix(const std::wstring& prefix)
{
    LockSaveAccess();
    std::vector<FileEntry*>* files = ConsoleSaveFileOriginal::getFilesWithPrefix(prefix);
    for (std::map<std::wstring, Region*>::iterator it = mRegions.begin(); it != mRegions.end(); ++it)
    {
        if (!Exists(it->second) || it->first.compare(0, prefix.size(), prefix) != 0)
            continue;
        if (files == NULL)
            files = new std::vector<FileEntry*>();
        files->push_back(&it->second->entry);
    }
    ReleaseSaveAccess();
    return files;
}

unsigned char* PcSaveFileSplit::getWritePointer(FileEntry* entry)
{
    if (RegionFor(entry) != NULL)
        return NULL;
    return ConsoleSaveFileOriginal::getWritePointer(entry);
}
