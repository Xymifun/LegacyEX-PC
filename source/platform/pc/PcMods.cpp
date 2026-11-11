#include "platform/pc/PcMods.hpp"

#include "platform/pc/PcEd25519.hpp"
#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace
{
struct IndexEntry
{
    std::string path;
    std::string original;
    unsigned int mod;
};

std::string modsDir = "mods";
bool modsDirSet = false;
bool modsEnabled = true;
std::vector<PcMod> allMods;
std::map<std::string, IndexEntry> fileIndex;

const unsigned int MAX_MANIFEST_SIZE = 64 * 1024;
const unsigned int MAX_CONFLICT_LINES = 20;

char Lower(char c)
{
    return c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c;
}

std::string Key(const std::string& rel)
{
    std::string key;
    key.reserve(rel.size());
    size_t pos = 0;
    while (pos <= rel.size())
    {
        size_t stop = rel.find_first_of("/\\", pos);
        if (stop == std::string::npos)
            stop = rel.size();
        std::string part = rel.substr(pos, stop - pos);
        pos = stop + 1;
        if (part.empty() || part == ".")
            continue;
        if (!key.empty())
            key += '/';
        for (size_t i = 0; i < part.size(); ++i)
            key += Lower(part[i]);
    }
    return key;
}

bool IsDirectory(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

bool IsFile(const std::string& path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

std::vector<std::string> ListDirectory(const std::string& path)
{
    std::vector<std::string> names;
    DIR* d = opendir(path.c_str());
    if (d == 0)
        return names;
    struct dirent* e;
    while ((e = readdir(d)) != 0)
    {
        if (e->d_name[0] != '.')
            names.push_back(e->d_name);
    }
    closedir(d);
    std::sort(names.begin(), names.end());
    return names;
}

bool ReadText(const std::string& path, std::string* text)
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    char buf[4096];
    size_t bytes;
    text->clear();
    while ((bytes = fread(buf, 1, sizeof(buf), fp)) > 0 && text->size() <= MAX_MANIFEST_SIZE)
        text->append(buf, bytes);
    fclose(fp);
    return text->size() <= MAX_MANIFEST_SIZE;
}

class Json
{
public:
    struct Value
    {
        enum Type
        {
            eNone,
            eString,
            eNumber,
            eBool,
            eArray,
            eOther
        };
        Type type;
        std::string text;
        double number;
        std::vector<std::string> strings;
        Value() : type(eNone), number(0) {}
    };

    Json(const std::string& text) : mText(text), mPosition(0) {}

    bool ParseObject(std::map<std::string, Value>* fields, std::string* error)
    {
        SkipBom();
        Skip();
        if (!Take('{'))
            return Fail("expected '{' at the start", error);
        Skip();
        if (Take('}'))
            return Finish(error);
        for (;;)
        {
            std::string name;
            Skip();
            if (!ParseString(&name))
                return Fail("expected a quoted field name", error);
            Skip();
            if (!Take(':'))
                return Fail("expected ':' after a field name", error);
            Value v;
            if (!ParseValue(&v, 0))
                return Fail("malformed value for \"" + name + "\"", error);
            (*fields)[name] = v;
            Skip();
            if (Take(','))
                continue;
            if (Take('}'))
                return Finish(error);
            return Fail("expected ',' or '}'", error);
        }
    }

private:
    const std::string& mText;
    size_t mPosition;

    bool Fail(const std::string& msg, std::string* error)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), " (byte %u)", (unsigned int)mPosition);
        *error = msg + buf;
        return false;
    }

    bool Finish(std::string* error)
    {
        Skip();
        return mPosition == mText.size() || Fail("unexpected text after the closing '}'", error);
    }

    void SkipBom()
    {
        if (mText.compare(0, 3, "\xEF\xBB\xBF") == 0)
            mPosition = 3;
    }

    void Skip()
    {
        while (mPosition < mText.size()
               && (mText[mPosition] == ' ' || mText[mPosition] == '\t' || mText[mPosition] == '\r'
                   || mText[mPosition] == '\n'))
            ++mPosition;
    }

    bool Take(char c)
    {
        if (mPosition < mText.size() && mText[mPosition] == c)
        {
            ++mPosition;
            return true;
        }
        return false;
    }

    bool TakeWord(const char* word)
    {
        size_t len = strlen(word);
        if (mText.compare(mPosition, len, word) != 0)
            return false;
        mPosition += len;
        return true;
    }

    static void AppendUtf8(unsigned int cp, std::string* out)
    {
        if (cp < 0x80)
            *out += (char)cp;
        else if (cp < 0x800)
        {
            *out += (char)(0xC0 | (cp >> 6));
            *out += (char)(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000)
        {
            *out += (char)(0xE0 | (cp >> 12));
            *out += (char)(0x80 | ((cp >> 6) & 0x3F));
            *out += (char)(0x80 | (cp & 0x3F));
        }
        else
        {
            *out += (char)(0xF0 | (cp >> 18));
            *out += (char)(0x80 | ((cp >> 12) & 0x3F));
            *out += (char)(0x80 | ((cp >> 6) & 0x3F));
            *out += (char)(0x80 | (cp & 0x3F));
        }
    }

    bool ParseHex4(unsigned int* out)
    {
        if (mPosition + 4 > mText.size())
            return false;
        *out = 0;
        for (int i = 0; i < 4; ++i)
        {
            char c = mText[mPosition++];
            *out <<= 4;
            if (c >= '0' && c <= '9')
                *out |= c - '0';
            else if (c >= 'a' && c <= 'f')
                *out |= c - 'a' + 10;
            else if (c >= 'A' && c <= 'F')
                *out |= c - 'A' + 10;
            else
                return false;
        }
        return true;
    }

    bool ParseString(std::string* out)
    {
        if (!Take('"'))
            return false;
        out->clear();
        while (mPosition < mText.size())
        {
            char c = mText[mPosition++];
            if (c == '"')
                return true;
            if ((unsigned char)c < 0x20)
                return false;
            if (c != '\\')
            {
                *out += c;
                continue;
            }
            if (mPosition >= mText.size())
                return false;
            char esc = mText[mPosition++];
            switch (esc)
            {
            case '"':
            case '\\':
            case '/':
                *out += esc;
                break;
            case 'b':
                *out += '\b';
                break;
            case 'f':
                *out += '\f';
                break;
            case 'n':
                *out += '\n';
                break;
            case 'r':
                *out += '\r';
                break;
            case 't':
                *out += '\t';
                break;
            case 'u':
            {
                unsigned int cp;
                if (!ParseHex4(&cp))
                    return false;
                if (cp >= 0xD800 && cp < 0xDC00)
                {
                    unsigned int lo;
                    if (!TakeWord("\\u") || !ParseHex4(&lo) || lo < 0xDC00 || lo >= 0xE000)
                        return false;
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                }
                AppendUtf8(cp, out);
                break;
            }
            default:
                return false;
            }
        }
        return false;
    }

    bool ParseNumber(double* out)
    {
        size_t from = mPosition;
        if (mPosition < mText.size() && mText[mPosition] == '-')
            ++mPosition;
        while (mPosition < mText.size() && strchr("0123456789.eE+-", mText[mPosition]) != 0)
            ++mPosition;
        if (mPosition == from)
            return false;
        std::string numText = mText.substr(from, mPosition - from);
        char* stop = 0;
        *out = strtod(numText.c_str(), &stop);
        return stop != 0 && *stop == 0;
    }

    bool ParseValue(Value* v, int depth)
    {
        if (depth > 32)
            return false;
        Skip();
        if (mPosition >= mText.size())
            return false;
        char c = mText[mPosition];
        if (c == '"')
        {
            v->type = Value::eString;
            return ParseString(&v->text);
        }
        if (c == '[')
        {
            ++mPosition;
            v->type = Value::eArray;
            Skip();
            if (Take(']'))
                return true;
            for (;;)
            {
                Value elem;
                if (!ParseValue(&elem, depth + 1))
                    return false;
                if (elem.type == Value::eString)
                    v->strings.push_back(elem.text);
                Skip();
                if (Take(','))
                    continue;
                return Take(']');
            }
        }
        if (c == '{')
        {
            ++mPosition;
            v->type = Value::eOther;
            Skip();
            if (Take('}'))
                return true;
            for (;;)
            {
                std::string name;
                Value elem;
                Skip();
                if (!ParseString(&name))
                    return false;
                Skip();
                if (!Take(':') || !ParseValue(&elem, depth + 1))
                    return false;
                Skip();
                if (Take(','))
                    continue;
                return Take('}');
            }
        }
        if (TakeWord("true") || TakeWord("false"))
        {
            v->type = Value::eBool;
            v->number = mText[mPosition - 2] == 'u' ? 1 : 0;
            return true;
        }
        if (TakeWord("null"))
        {
            v->type = Value::eOther;
            return true;
        }
        v->type = Value::eNumber;
        return ParseNumber(&v->number);
    }
};

bool ValidId(const std::string& id)
{
    if (id.empty() || id.size() > 64)
        return false;
    for (size_t i = 0; i < id.size(); ++i)
    {
        char c = id[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.'))
            return false;
    }
    return true;
}

std::string StringField(const std::map<std::string, Json::Value>& fields, const char* name)
{
    std::map<std::string, Json::Value>::const_iterator it = fields.find(name);
    return it != fields.end() && it->second.type == Json::Value::eString ? it->second.text : std::string();
}

bool ReadManifest(const std::string& folder, const std::string& dir, PcMod* m)
{
    std::string text;
    std::string path = dir + "/mod.json";
    if (!IsFile(path))
    {
        printf("[pc] mods: skipping %s: no mod.json\n", folder.c_str());
        return false;
    }
    if (!ReadText(path, &text))
    {
        printf("[pc] mods: skipping %s: cannot read mod.json (or it is over 64 KB)\n", folder.c_str());
        return false;
    }
    std::map<std::string, Json::Value> fields;
    std::string err;
    if (!Json(text).ParseObject(&fields, &err))
    {
        printf("[pc] mods: skipping %s: mod.json: %s\n", folder.c_str(), err.c_str());
        return false;
    }
    m->id = StringField(fields, "id");
    if (!ValidId(m->id))
    {
        printf("[pc] mods: skipping %s: mod.json needs an \"id\" of lowercase letters, digits, '_', '-' or '.'\n",
               folder.c_str());
        return false;
    }
    if (m->id != folder)
        printf("[pc] mods: %s: folder name differs from its id \"%s\"\n", folder.c_str(), m->id.c_str());
    m->name = StringField(fields, "name");
    if (m->name.empty())
        m->name = m->id;
    m->version = StringField(fields, "version");
    m->description = StringField(fields, "description");
    std::map<std::string, Json::Value>::const_iterator authors = fields.find("authors");
    if (authors != fields.end() && authors->second.type == Json::Value::eArray)
        m->authors = authors->second.strings;
    else if (authors != fields.end() && authors->second.type == Json::Value::eString)
        m->authors.push_back(authors->second.text);
    std::map<std::string, Json::Value>::const_iterator prio = fields.find("priority");
    m->priority = prio != fields.end() && prio->second.type == Json::Value::eNumber
                        ? (int)prio->second.number
                        : 0;
    std::map<std::string, Json::Value>::const_iterator api = fields.find("api");
    m->api = 1;
    if (api != fields.end())
    {
        const double v = api->second.number;
        if (api->second.type != Json::Value::eNumber || v < 1 || v > 1000000
            || v != (double)((int)v))
        {
            printf("[pc] mods: skipping %s: \"api\" must be a whole number from 1\n", folder.c_str());
            return false;
        }
        m->api = (int)v;
    }
    if (m->api > kPcModApi)
    {
        printf("[pc] mods: skipping %s: made for a newer game (mod api %d; this game supports up to %d)\n",
               folder.c_str(), m->api, kPcModApi);
        return false;
    }
    std::map<std::string, Json::Value>::const_iterator side = fields.find("side");
    m->side = side == fields.end() ? "both" : side->second.type == Json::Value::eString ? side->second.text : "";
    if (m->side != "both" && m->side != "client" && m->side != "server")
    {
        printf("[pc] mods: skipping %s: \"side\" must be \"client\", \"server\" or \"both\"\n", folder.c_str());
        return false;
    }
    m->directory = dir;
    m->files = 0;
    m->gameplayFiles = 0;
    m->scriptFiles = 0;
    memset(m->gameplayHash, 0, sizeof(m->gameplayHash));
    return true;
}

void CollectFiles(const std::string& dir, const std::string& rel, int depth,
    std::vector<std::pair<std::string, std::string> >* files)
{
    if (depth > 32)
        return;
    std::vector<std::string> names = ListDirectory(dir);
    for (size_t i = 0; i < names.size(); ++i)
    {
        std::string path = dir + "/" + names[i];
        std::string child = rel.empty() ? names[i] : rel + "/" + names[i];
        if (IsDirectory(path))
            CollectFiles(path, child, depth + 1, files);
        else if (IsFile(path))
            files->push_back(std::make_pair(child, path));
    }
}

const char* const presentationExts[] = {
    "png",
    "jpg",
    "jpeg",
    "tga",
    "binka",
    "wav",
    "ogg",
    "msscmp",
    "ttf",
    "otf",
    "fui",
    "swf",
    "loc",
    "lang",
    "txt",
    "col"};

const char* const worldDataNames[] = {
    "worldpack.pck",
    "lobby.pck",
    "tutorial.pck"};
const char* const worldDataExts[] = {
    "mcs",
    "grf"};
const char WORLD_DATA_FOLDER[] = "common/res/titleupdate/gamerules/";

std::string FileName(const std::string& key)
{
    size_t pos = key.rfind('/');
    return pos == std::string::npos ? key : key.substr(pos + 1);
}

std::string Extension(const std::string& key)
{
    std::string name = FileName(key);
    size_t pos = name.rfind('.');
    return pos == std::string::npos ? std::string() : name.substr(pos + 1);
}

bool HashFile(const std::string& path, unsigned char digest[64])
{
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == 0)
        return false;
    PcSha512Context ctx;
    PcSha512Begin(&ctx);
    std::vector<unsigned char> buf(1024 * 1024);
    size_t n;
    while ((n = fread(&buf[0], 1, buf.size(), fp)) > 0)
        PcSha512Add(&ctx, &buf[0], n);
    const bool ok = ferror(fp) == 0;
    fclose(fp);
    PcSha512End(&ctx, digest);
    return ok;
}

void HashGameplayFiles()
{
    std::vector<std::string> listings(allMods.size());
    for (std::map<std::string, IndexEntry>::const_iterator it = fileIndex.begin(); it != fileIndex.end(); ++it)
    {
        if (!PcModsCountsForGameplay(it->first, allMods[it->second.mod].side))
            continue;
        unsigned char digest[64];
        if (!HashFile(it->second.path, digest))
            memset(digest, 0, sizeof(digest));
        std::string& list = listings[it->second.mod];
        list += it->first;
        list += '\0';
        list.append(reinterpret_cast<const char*>(digest), sizeof(digest));
        ++allMods[it->second.mod].gameplayFiles;
    }
    for (size_t k = 0; k < allMods.size(); ++k)
    {
        PcMod& mod = allMods[k];
        std::vector<std::pair<std::string, std::string> > files;
        CollectFiles(mod.directory + "/scripts", std::string(), 0, &files);
        mod.scriptFiles = (unsigned int)files.size();
        for (size_t i = 0; i < files.size(); ++i)
            files[i].first = "scripts/" + files[i].first;
        std::vector<std::pair<std::string, std::string> > nativeFiles;
        CollectFiles(mod.directory + "/native", std::string(), 0, &nativeFiles);
        for (size_t i = 0; i < nativeFiles.size(); ++i)
            files.push_back(std::make_pair("native/" + nativeFiles[i].first, nativeFiles[i].second));
        if (mod.side != "both")
            continue;
        for (size_t i = 0; i < files.size(); ++i)
        {
            unsigned char digest[64];
            if (!HashFile(files[i].second, digest))
                memset(digest, 0, sizeof(digest));
            listings[k] += Key(files[i].first);
            listings[k] += '\0';
            listings[k].append(reinterpret_cast<const char*>(digest), sizeof(digest));
            ++mod.gameplayFiles;
        }
    }
    for (size_t k = 0; k < allMods.size(); ++k)
    {
        if (allMods[k].gameplayFiles == 0)
            continue;
        unsigned char digest[64];
        PcSha512(reinterpret_cast<const unsigned char*>(listings[k].data()), listings[k].size(), digest);
        memcpy(allMods[k].gameplayHash, digest, sizeof(allMods[k].gameplayHash));
    }
}

bool LoadsBefore(const PcMod& a, const PcMod& b)
{
    if (a.priority != b.priority)
        return a.priority > b.priority;
    return a.id < b.id;
}
}

void PcModsSetDirectory(const std::string& dir)
{
    modsDir = dir;
    modsDirSet = true;
}

void PcModsSetEnabled(bool on)
{
    modsEnabled = on;
}

void PcModsLoad()
{
    allMods.clear();
    fileIndex.clear();
    if (!modsEnabled)
    {
        printf("[pc] mods: disabled (--no-mods)\n");
        return;
    }
    std::vector<std::string> dirs;
    if (modsDirSet)
    {
        if (!IsDirectory(modsDir))
        {
            printf("[pc] mods: %s is not a folder; no mods loaded\n", modsDir.c_str());
            return;
        }
        dirs.push_back(modsDir);
    }
    else
    {
        PcOsMakeDirectory("Windows64");
        PcOsMakeDirectory("Windows64/mods");
        dirs.push_back("Windows64/mods");
        if (IsDirectory("mods"))
            dirs.push_back("mods");
    }

    for (size_t d = 0; d < dirs.size(); ++d)
    {
        std::vector<std::string> folders = ListDirectory(dirs[d]);
        for (size_t i = 0; i < folders.size(); ++i)
        {
            std::string dir = dirs[d] + "/" + folders[i];
            if (!IsDirectory(dir))
                continue;
            PcMod m;
            if (!ReadManifest(folders[i], dir, &m))
                continue;
            bool dup = false;
            for (size_t j = 0; j < allMods.size() && !dup; ++j)
                dup = allMods[j].id == m.id;
            if (dup)
            {
                printf("[pc] mods: skipping %s: another folder already has the id \"%s\"\n", dir.c_str(),
                       m.id.c_str());
                continue;
            }
            allMods.push_back(m);
        }
    }
    std::stable_sort(allMods.begin(), allMods.end(), LoadsBefore);

    unsigned int conflicts = 0;
    for (unsigned int k = 0; k < allMods.size(); ++k)
    {
        PcMod& m = allMods[k];
        std::vector<std::pair<std::string, std::string> > files;
        CollectFiles(m.directory + "/assets", std::string(), 0, &files);
        for (size_t i = 0; i < files.size(); ++i)
        {
            std::string key = Key(files[i].first);
            std::map<std::string, IndexEntry>::iterator it = fileIndex.find(key);
            if (it != fileIndex.end())
            {
                if (++conflicts <= MAX_CONFLICT_LINES)
                    printf("[pc] mods: %s from %s is used; %s's copy is not\n", files[i].first.c_str(),
                           allMods[it->second.mod].id.c_str(), m.id.c_str());
                continue;
            }
            IndexEntry e;
            e.path = files[i].second;
            e.original = files[i].first;
            e.mod = k;
            fileIndex[key] = e;
            ++m.files;
        }
    }
    if (conflicts > MAX_CONFLICT_LINES)
        printf("[pc] mods: %u more files are provided by more than one mod\n", conflicts - MAX_CONFLICT_LINES);
    HashGameplayFiles();

    std::string from = dirs[0];
    for (size_t d = 1; d < dirs.size(); ++d)
        from += " and " + dirs[d];
    printf("[pc] mods: %u loaded from %s\n", (unsigned int)allMods.size(), from.c_str());
    for (size_t i = 0; i < allMods.size(); ++i)
        printf("[pc] mods:   %s %s, %s side (%u files, %u scripts, %u change gameplay)\n", allMods[i].id.c_str(),
               allMods[i].version.empty() ? "-" : allMods[i].version.c_str(), allMods[i].side.c_str(), allMods[i].files,
               allMods[i].scriptFiles, allMods[i].gameplayFiles);
}

const std::vector<PcMod>& PcModsLoaded()
{
    return allMods;
}

bool PcModsIsWorldData(const std::string& path)
{
    std::string key = Key(path);
    if (key.compare(0, sizeof(WORLD_DATA_FOLDER) - 1, WORLD_DATA_FOLDER) == 0)
        return true;
    std::string name = FileName(key);
    for (size_t i = 0; i < sizeof(worldDataNames) / sizeof(worldDataNames[0]); ++i)
    {
        if (name == worldDataNames[i])
            return true;
    }
    std::string ext = Extension(key);
    for (size_t i = 0; i < sizeof(worldDataExts) / sizeof(worldDataExts[0]); ++i)
    {
        if (ext == worldDataExts[i])
            return true;
    }
    return false;
}

bool PcModsCountsForGameplay(const std::string& path, const std::string& side)
{
    if (PcModsIsWorldData(path))
        return true;
    if (side != "both")
        return false;
    std::string key = Key(path);
    if (FileName(key).find('.') == std::string::npos)
        return true;
    std::string ext = Extension(key);
    for (size_t i = 0; i < sizeof(presentationExts) / sizeof(presentationExts[0]); ++i)
    {
        if (ext == presentationExts[i])
            return false;
    }
    return true;
}

std::string PcModsFindContent(const std::string& path)
{
    if (fileIndex.empty())
        return std::string();
    std::map<std::string, IndexEntry>::const_iterator it = fileIndex.find(Key(path));
    return it != fileIndex.end() ? it->second.path : std::string();
}

void PcModsArchiveFiles(const char* arcPath, std::vector<PcModArchiveFile>* files)
{
    files->clear();
    if (fileIndex.empty())
        return;
    std::string rel = PcContentRelativePath(arcPath);
    if (rel.empty())
        return;
    std::string prefix = Key(rel) + "/";
    for (std::map<std::string, IndexEntry>::const_iterator it = fileIndex.lower_bound(prefix);
         it != fileIndex.end() && it->first.compare(0, prefix.size(), prefix) == 0; ++it)
    {
        const std::string& orig = it->second.original;
        std::string inside = orig.substr(orig.size() - (it->first.size() - prefix.size()));
        PcModArchiveFile f;
        for (size_t i = 0; i < inside.size(); ++i)
            f.name += (wchar_t)(inside[i] == '/' ? L'\\' : (unsigned char)inside[i]);
        f.path = it->second.path;
        files->push_back(f);
    }
}
