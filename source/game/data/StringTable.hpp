#ifndef GAME_DATA_STRINGTABLE_HPP
#define GAME_DATA_STRINGTABLE_HPP

#include <string>
#include <vector>
#include <utility>
#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"

class DataInputStream;
class DataOutputStream;
class Random;

class StringTable
{
public:
    struct _LanguageData
    {
        boost::unordered_map<int, std::wstring> stringsById;
        boost::unordered_map<std::wstring, std::wstring> strings;
        std::vector<std::wstring> orderedStrings;
    };

    StringTable();
    StringTable(arrayWithLength<unsigned char> data, bool localized, bool borrowed);
    ~StringTable();

    std::wstring FindLanguage(bool& found, long long& count);
    std::wstring ReadLanguageFromStream(DataInputStream* stream);
    void SelectLanguage(const std::wstring& language);
    void ReadAllFromStream(DataInputStream* stream);
    void ProcessStringTableData();
    void ReloadStringTable();
    void getData(unsigned char** data, unsigned int* length);
    wchar_t* getString(const std::wstring& key);
    const wchar_t* getString(int id);
    unsigned int getRandomStringId(Random* random);
    std::vector<int> getRandomStringIds(int count, Random* random);
    void copyStringToTable(const std::wstring& key, StringTable* table);
    void write(DataOutputStream* stream);
    void Reset();
#ifdef MINECRAFT_PC
    void RewriteStrings(void (*rewrite)(std::wstring&));
#endif

private:
    bool m_isLanguageTable;
    bool m_localized;
    bool m_ownsData;
    boost::unordered_map<std::wstring, _LanguageData> m_languages;
    std::vector<int> m_ids;
    std::vector<std::wstring> m_keys;
    boost::unordered_map<std::wstring, std::wstring>* m_strings;
    boost::unordered_map<int, std::wstring>* m_stringsById;
    std::vector<std::pair<std::wstring, int> > m_entries;
    arrayWithLength<unsigned char> m_data;
};

#endif
