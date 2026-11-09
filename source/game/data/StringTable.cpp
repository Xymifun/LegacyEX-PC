#include "game/data/StringTable.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/ByteArrayOutputStream.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/StringConversion.hpp"
#include "game/util/Random.hpp"

#include <algorithm>
#include <cstring>

StringTable::StringTable()
{
    m_isLanguageTable = false;
    m_localized = false;
    m_strings = NULL;
    m_stringsById = NULL;
}

StringTable::StringTable(arrayWithLength<unsigned char> data, bool localized, bool borrowed)
{
    m_strings = NULL;
    m_isLanguageTable = false;
    m_stringsById = NULL;
    m_localized = localized;
    if (borrowed)
    {
        m_ownsData = false;
        m_data = data;
        ProcessStringTableData();
    }
    else
    {
        unsigned int length = data.length;
        unsigned char* source = data.data;
        m_ownsData = true;
        arrayWithLength<unsigned char> copy(length, true);
        unsigned char* dest = copy.data;
        for (unsigned int i = 0; i < length; ++i)
            dest[i] = source[i];
        m_data = copy;
        ProcessStringTableData();
    }
    if (m_ownsData)
    {
        delete[] m_data.data;
        m_data.length = 0;
        m_data.data = NULL;
    }
}

StringTable::~StringTable()
{
    if (m_ownsData)
        delete[] m_data.data;
}

std::wstring StringTable::FindLanguage(bool& found, long long& count)
{
    std::vector<std::wstring> localeNames;
    gApp.getLocale(localeNames);

    std::wstring selected = L"";
    for (std::vector<std::wstring>::iterator localeName = localeNames.begin();
         localeName != localeNames.end() && !found; ++localeName)
    {
        count = 0;
        for (std::vector<std::pair<std::wstring, int> >::iterator entry = m_entries.begin(); entry != m_entries.end();
             ++entry)
        {
            if (entry->first == *localeName)
            {
                found = true;
                selected = *localeName;
                break;
            }
            count += entry->second;
        }
    }
    if (!found)
        m_isLanguageTable = false;
    return selected;
}

std::wstring StringTable::ReadLanguageFromStream(DataInputStream* stream)
{
    int format = stream->readInt();
    m_isLanguageTable = false;
    if (format > 0)
        m_isLanguageTable = stream->readBoolean();
    std::wstring language = stream->readUTF();
    int count = stream->readInt();
    _LanguageData& data = m_languages[language];
    if (!m_isLanguageTable)
    {
        for (int i = 0; i < count; ++i)
        {
            if (format > 1)
            {
                data.strings.insert(std::make_pair(m_keys[i], stream->readUTF()));
            }
            else
            {
                std::wstring key = stream->readUTF();
                std::wstring value = stream->readUTF();
                data.strings.insert(std::make_pair(key, value));
            }
        }
    }
    else if (format >= 1)
    {
        if (!m_ids.empty())
        {
            for (int i = 0; i < count; ++i)
            {
                data.stringsById.insert(std::make_pair(m_ids[i], stream->readUTF()));
            }
        }
    }
    else
    {
        for (int i = 0; i < count; ++i)
            data.orderedStrings.push_back(stream->readUTF());
    }
    return language;
}

void StringTable::SelectLanguage(const std::wstring& language)
{
    _LanguageData& data = m_languages[language];
    m_stringsById = &data.stringsById;
    m_strings = &data.strings;
}

void StringTable::ReadAllFromStream(DataInputStream* stream)
{
    int version = stream->readInt();
    int languageCount = stream->readInt();
    if (version == 2)
    {
        bool isLanguageTable = stream->readBoolean();
        int count = stream->readInt();
        if (!isLanguageTable)
        {
            for (int i = 0; i < count; ++i)
                m_keys.push_back(stream->readUTF());
        }
        else
        {
            for (int i = 0; i < count; ++i)
                m_ids.push_back(stream->readInt());
        }
    }
    m_entries.clear();
    for (int i = 0; i < languageCount; ++i)
    {
        std::wstring language = stream->readUTF();
        int size = stream->readInt();
        m_entries.push_back(std::make_pair(language, size));
    }
    bool found = false;
    long long offset = 0;
    std::wstring language = FindLanguage(found, offset);
    if (!m_localized)
    {
        if (found)
        {
            stream->skip(offset);
            ReadLanguageFromStream(stream);
        }
    }
    else if (m_languages.empty())
    {
        for (std::vector<std::pair<std::wstring, int> >::iterator entry = m_entries.begin(); entry != m_entries.end();
             ++entry)
            ReadLanguageFromStream(stream);
    }
    if (found)
        SelectLanguage(language);
    m_ids.clear();
    m_keys.clear();
}

void StringTable::ProcessStringTableData()
{
    if (m_data.data != NULL)
    {
        ByteArrayInputStream byteStream(m_data);
        DataInputStream stream(&byteStream);
        ReadAllFromStream(&stream);
        stream.close();
        byteStream.reset();
    }
    else
    {
        bool found = false;
        long long count = 0;
        std::wstring language = FindLanguage(found, count);
        if (found)
            SelectLanguage(language);
    }
}

void StringTable::ReloadStringTable()
{
    if (!m_localized)
    {
        m_strings->clear();
        m_stringsById->clear();
    }
    ProcessStringTableData();
}

void StringTable::getData(unsigned char** data, unsigned int* length)
{
    *data = m_data.data;
    *length = m_data.length;
}

wchar_t* StringTable::getString(const std::wstring& key)
{
    if (m_strings != NULL)
    {
        boost::unordered_map<std::wstring, std::wstring>::iterator it = m_strings->find(key);
        if (it != m_strings->end())
            return const_cast<wchar_t*>(it->second.c_str());
    }
    return const_cast<wchar_t*>(L"");
}

#ifdef MINECRAFT_PC
void StringTable::RewriteStrings(void (*rewrite)(std::wstring&))
{
    if (m_stringsById != NULL)
    {
        for (boost::unordered_map<int, std::wstring>::iterator it = m_stringsById->begin();
             it != m_stringsById->end(); ++it)
            rewrite(it->second);
    }
    if (m_strings != NULL)
    {
        for (boost::unordered_map<std::wstring, std::wstring>::iterator it = m_strings->begin();
             it != m_strings->end(); ++it)
            rewrite(it->second);
    }
}
#endif

const wchar_t* StringTable::getString(int id)
{
    if (id != -1 && id != -2)
    {
        boost::unordered_map<int, std::wstring>::iterator it = m_stringsById->find(id);
        if (it != m_stringsById->end())
            return it->second.c_str();
    }
    return L"";
}

unsigned int StringTable::getRandomStringId(Random* random)
{
    int selected = random->nextInt(m_stringsById->size());
    boost::unordered_map<int, std::wstring>::const_iterator it = m_stringsById->begin();
    for (int i = 0; i < selected; ++i)
        ++it;
    return (unsigned int)it->first;
}

std::vector<int> StringTable::getRandomStringIds(int count, Random* random)
{
    std::vector<int> result;
    unsigned int collected = 0;
    while ((int)collected < count && collected < m_stringsById->size())
    {
        int id = getRandomStringId(random);
        if (std::find(result.begin(), result.end(), id) == result.end())
        {
            result.push_back(id);
            ++collected;
        }
    }
    return result;
}

void StringTable::copyStringToTable(const std::wstring& key, StringTable* table)
{
    std::wstring value = L"";
    for (boost::unordered_map<std::wstring, _LanguageData>::iterator language = m_languages.begin();
         language != m_languages.end(); ++language)
    {
        if (m_isLanguageTable)
            continue;
        boost::unordered_map<std::wstring, std::wstring>::iterator found = language->second.strings.find(key);
        if (found != language->second.strings.end())
            value = found->second.c_str();
        else
            value = L"";
        table->m_languages[language->first].strings[key] = value;
    }
}

void StringTable::write(DataOutputStream* stream)
{
    stream->writeInt(2);
    stream->writeInt(static_cast<int>(m_languages.size()));
    if (m_strings == NULL || m_stringsById == NULL)
    {
        _LanguageData& first = m_languages.begin()->second;
        m_strings = &first.strings;
        m_stringsById = &first.stringsById;
    }

    stream->writeBoolean(m_isLanguageTable);
    if (m_isLanguageTable)
    {
        stream->writeInt(static_cast<int>(m_stringsById->size()));
        for (boost::unordered_map<int, std::wstring>::const_iterator item = m_stringsById->begin();
             item != m_stringsById->end(); ++item)
        {
            stream->writeInt(item->first);
        }
    }
    else
    {
        stream->writeInt(static_cast<int>(m_strings->size()));
        for (boost::unordered_map<std::wstring, std::wstring>::const_iterator item = m_strings->begin();
             item != m_strings->end(); ++item)
        {
            stream->writeUTF(item->first);
        }
    }

    boost::unordered_map<std::wstring, ByteArrayOutputStream*> languageStreams;
    for (boost::unordered_map<std::wstring, _LanguageData>::iterator lang = m_languages.begin();
         lang != m_languages.end(); ++lang)
    {
        ByteArrayOutputStream* bytes = new ByteArrayOutputStream();
        DataOutputStream languageStream(bytes);
        languageStream.writeInt(2);
        languageStream.writeBoolean(m_isLanguageTable);
        languageStream.writeUTF(lang->first);
        if (m_isLanguageTable)
        {
            languageStream.writeInt(static_cast<int>(lang->second.stringsById.size()));
            for (boost::unordered_map<int, std::wstring>::const_iterator item = lang->second.stringsById.begin();
                 item != lang->second.stringsById.end(); ++item)
            {
                languageStream.writeUTF(item->second);
            }
        }
        else
        {
            languageStream.writeInt(static_cast<int>(lang->second.strings.size()));
            for (boost::unordered_map<std::wstring, std::wstring>::const_iterator item = lang->second.strings.begin();
                 item != lang->second.strings.end(); ++item)
            {
                languageStream.writeUTF(item->second);
            }
        }
        languageStreams.insert(std::make_pair(lang->first, bytes));
    }

    std::vector<std::wstring> languageNames;
    for (boost::unordered_map<std::wstring, _LanguageData>::iterator lang = m_languages.begin();
         lang != m_languages.end(); ++lang)
    {
        languageNames.push_back(lang->first);
    }
    std::sort(languageNames.begin(), languageNames.end(), compareStrings);

    for (std::vector<std::wstring>::iterator name = languageNames.begin(); name != languageNames.end(); ++name)
    {
        std::wstring language = *name;
        stream->writeUTF(language);
        stream->writeInt(languageStreams[language]->mCount);
    }
    for (std::vector<std::wstring>::iterator name = languageNames.begin(); name != languageNames.end(); ++name)
    {
        std::wstring language = *name;
        ByteArrayOutputStream* bytes = languageStreams[language];
        stream->write(arrayWithLength<unsigned char>(bytes->mBuffer.data, bytes->mCount));
        delete bytes;
    }
}

void StringTable::Reset()
{
    m_strings = NULL;
    m_stringsById = NULL;
    m_languages.clear();
    m_ids.clear();
    m_keys.clear();
    if (m_strings != NULL)
        m_strings->clear();
    if (m_stringsById != NULL)
        m_stringsById->clear();
}
