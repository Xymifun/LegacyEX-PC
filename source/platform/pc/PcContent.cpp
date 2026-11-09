#include "platform/pc/PcContent.hpp"

#include "platform/pc/PcEd25519.hpp"
#include "platform/pc/PcMods.hpp"

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>

extern const char kPcContentVersion[];
extern const unsigned int kPcContentBuildTime;

namespace
{
const unsigned char SUMMARY_FORMAT = 1;
const unsigned int SUMMARY_SIZE = 21;
const size_t MAX_TEXT = 64;

PcContentIdentity localContent;
std::wstring mismatchText;

unsigned int HexValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

std::string Hex(const unsigned char* b, size_t n)
{
    static const char digits[] = "0123456789abcdef";
    std::string out;
    for (size_t i = 0; i < n; ++i)
    {
        out += digits[b[i] >> 4];
        out += digits[b[i] & 15];
    }
    return out;
}

bool ById(const PcContentMod& a, const PcContentMod& b)
{
    return a.id < b.id;
}

bool ZeroDigest(const unsigned char digest[8])
{
    for (int i = 0; i < 8; ++i)
    {
        if (digest[i] != 0)
            return false;
    }
    return true;
}

const PcContentMod* FindMod(const std::vector<PcContentMod>& mods, const std::string& id)
{
    for (size_t i = 0; i < mods.size(); ++i)
    {
        if (mods[i].id == id)
            return &mods[i];
    }
    return 0;
}

std::string Named(const PcContentMod& m)
{
    return m.version.empty() ? m.id : m.id + " " + m.version;
}

std::string JoinNames(const std::vector<std::string>& names)
{
    std::string out;
    for (size_t i = 0; i < names.size(); ++i)
        out += (i == 0 ? "" : ", ") + names[i];
    return out;
}

std::wstring Widen(const std::string& s)
{
    std::wstring out;
    for (size_t i = 0; i < s.size(); ++i)
        out += (unsigned char)s[i] < 0x80 ? (wchar_t)s[i] : L'?';
    return out;
}
}

PcContentIdentity::PcContentIdentity() : present(false), detailed(false), buildTime(0)
{
    memset(game, 0, sizeof(game));
    memset(mods, 0, sizeof(mods));
}

void PcContentInitialise()
{
    localContent = PcContentIdentity();
    localContent.present = true;
    localContent.detailed = true;
    for (int i = 0; i < 8 && kPcContentVersion[i * 2] != 0 && kPcContentVersion[i * 2 + 1] != 0; ++i)
        localContent.game[i] = (unsigned char)((HexValue(kPcContentVersion[i * 2]) << 4)
                                                     | HexValue(kPcContentVersion[i * 2 + 1]));
    localContent.buildTime = kPcContentBuildTime;
    const std::vector<PcMod>& loaded = PcModsLoaded();
    for (size_t i = 0; i < loaded.size(); ++i)
    {
        if (loaded[i].gameplayFiles == 0)
            continue;
        PcContentMod m;
        m.id = loaded[i].id.substr(0, MAX_TEXT);
        m.version = loaded[i].version.substr(0, MAX_TEXT);
        memcpy(m.hash, loaded[i].gameplayHash, sizeof(m.hash));
        localContent.modList.push_back(m);
    }
    std::sort(localContent.modList.begin(), localContent.modList.end(), ById);
    if (!localContent.modList.empty())
    {
        std::string list;
        for (size_t i = 0; i < localContent.modList.size(); ++i)
        {
            list += localContent.modList[i].id + '\0' + localContent.modList[i].version + '\0';
            list.append(reinterpret_cast<const char*>(localContent.modList[i].hash), 8);
        }
        unsigned char digest[64];
        PcSha512(reinterpret_cast<const unsigned char*>(list.data()), list.size(), digest);
        memcpy(localContent.mods, digest, sizeof(localContent.mods));
        if (ZeroDigest(localContent.mods))
            localContent.mods[0] = 1;
    }
    printf("[pc] content: game version %s, %u mod(s) that change gameplay\n", Hex(localContent.game, 8).c_str(),
           (unsigned int)localContent.modList.size());
}

const PcContentIdentity& PcContentLocal()
{
    return localContent;
}

std::string PcContentGameVersion(const PcContentIdentity& ci)
{
    return Hex(ci.game, sizeof(ci.game));
}

void PcContentAppendSummary(std::vector<unsigned char>* out)
{
    out->push_back('C');
    out->push_back((unsigned char)SUMMARY_SIZE);
    out->push_back(SUMMARY_FORMAT);
    out->insert(out->end(), localContent.game, localContent.game + 8);
    for (int b = 0; b < 4; ++b)
        out->push_back((unsigned char)(localContent.buildTime >> (b * 8)));
    out->insert(out->end(), localContent.mods, localContent.mods + 8);
}

void PcContentAppendDetail(std::vector<unsigned char>* out)
{
    const size_t n = std::min<size_t>(localContent.modList.size(), 255);
    out->push_back('D');
    out->push_back((unsigned char)n);
    for (size_t i = 0; i < n; ++i)
    {
        const PcContentMod& m = localContent.modList[i];
        out->push_back((unsigned char)m.id.size());
        out->insert(out->end(), m.id.begin(), m.id.end());
        out->push_back((unsigned char)m.version.size());
        out->insert(out->end(), m.version.begin(), m.version.end());
        out->insert(out->end(), m.hash, m.hash + 8);
    }
}

void PcContentParse(const std::vector<unsigned char>& data, size_t start, PcContentIdentity* out)
{
    *out = PcContentIdentity();
    size_t pos = start;
    while (pos + 2 <= data.size())
    {
        const unsigned char tag = data[pos];
        const size_t n = data[pos + 1];
        if (tag == 'D')
        {
            std::vector<PcContentMod> list;
            size_t p = pos + 2;
            bool ok = true;
            for (size_t i = 0; i < n && ok; ++i)
            {
                PcContentMod m;
                ok = p < data.size() && p + 1 + data[p] < data.size();
                if (!ok)
                    break;
                m.id.assign(data.begin() + p + 1, data.begin() + p + 1 + data[p]);
                p += 1 + data[p];
                ok = p + 1 + data[p] + 8 <= data.size();
                if (!ok)
                    break;
                m.version.assign(data.begin() + p + 1, data.begin() + p + 1 + data[p]);
                p += 1 + data[p];
                memcpy(m.hash, &data[p], 8);
                p += 8;
                list.push_back(m);
            }
            if (ok)
            {
                out->modList.swap(list);
                out->detailed = true;
            }
            break;
        }
        const size_t unitLen = tag == 'M' ? 4 : tag == 'T' ? 2 : tag == 'C' ? 1 : 0;
        if (unitLen == 0 || pos + 2 + n * unitLen > data.size())
            break;
        const unsigned char* v = &data[pos + 2];
        if (tag == 'C' && n >= SUMMARY_SIZE && v[0] == SUMMARY_FORMAT)
        {
            out->present = true;
            memcpy(out->game, v + 1, 8);
            out->buildTime = v[9] | (v[10] << 8) | (v[11] << 16)
                                  | ((unsigned int)v[12] << 24);
            memcpy(out->mods, v + 13, 8);
        }
        pos += 2 + n * unitLen;
    }
    if (!out->present)
    {
        out->detailed = false;
        out->modList.clear();
    }
}

bool PcContentMatches(const PcContentIdentity& other)
{
    return memcmp(other.mods, localContent.mods, 8) == 0;
}

bool PcContentModded(const PcContentIdentity& ci)
{
    return ci.present && !ZeroDigest(ci.mods);
}

std::vector<std::string> PcContentDifferences(const PcContentIdentity& other, bool isHost)
{
    std::vector<std::string> lines;
    const std::string who = isHost ? "The host" : "That player";
    if (memcmp(other.mods, localContent.mods, 8) == 0)
        return lines;
    if (!other.present)
    {
        lines.push_back(who + " is playing a version of the game without mods.");
        return lines;
    }
    if (!other.detailed)
    {
        lines.push_back(who + "'s mods that change gameplay are not the same as yours.");
        return lines;
    }
    std::vector<std::string> theirs;
    std::vector<std::string> ours;
    std::vector<std::string> changed;
    for (size_t i = 0; i < other.modList.size(); ++i)
    {
        const PcContentMod* mine = FindMod(localContent.modList, other.modList[i].id);
        if (mine == 0)
            theirs.push_back(Named(other.modList[i]));
        else if (memcmp(mine->hash, other.modList[i].hash, 8) != 0 || mine->version != other.modList[i].version)
        {
            if (mine->version != other.modList[i].version)
                changed.push_back(other.modList[i].id + " (" + (isHost ? "host " : "theirs ")
                                    + (other.modList[i].version.empty() ? "-" : other.modList[i].version)
                                    + ", yours " + (mine->version.empty() ? "-" : mine->version) + ")");
            else
                changed.push_back(Named(*mine) + " (the files differ)");
        }
    }
    for (size_t i = 0; i < localContent.modList.size(); ++i)
    {
        if (FindMod(other.modList, localContent.modList[i].id) == 0)
            ours.push_back(Named(localContent.modList[i]));
    }
    if (!theirs.empty())
        lines.push_back((isHost ? "Mods only the host has: " : "Mods only that player has: ") + JoinNames(theirs));
    if (!ours.empty())
        lines.push_back("Mods only you have: " + JoinNames(ours));
    if (!changed.empty())
        lines.push_back("Mods that differ: " + JoinNames(changed));
    if (theirs.empty() && ours.empty() && changed.empty())
        lines.push_back(who + "'s mods that change gameplay are not the same as yours.");
    return lines;
}

std::wstring PcContentListLabel(const PcContentIdentity& other)
{
    if (memcmp(other.mods, localContent.mods, 8) != 0)
        return L"[Different Mods]";
    if (PcContentModded(other))
        return L"[Modded]";
    return std::wstring();
}

void PcContentSetMismatch(const PcContentIdentity& host)
{
    std::vector<std::string> msgs = PcContentDifferences(host, true);
    std::wstring text = L"Mismatched content (Updated or Modded Version).";
    for (size_t i = 0; i < msgs.size(); ++i)
        text += (i == 0 ? L"\n\n" : L"\n") + Widen(msgs[i]);
    mismatchText = text;
}

const wchar_t* PcContentMismatchText()
{
    return mismatchText.empty() ? L"Mismatched content (Updated or Modded Version)." : mismatchText.c_str();
}
