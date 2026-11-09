#include "game/client/audio/DLCAudioFile.hpp"

#include <wchar.h>

#include <boost/unordered_map.hpp>

#include "game/core/DLCPack.hpp"
#include "game/core/StringConversion.hpp"

static const wchar_t* s_audioParameterNames[] = {L"CUENAME", L"CREDIT"};

DLCAudioFile::DLCAudioFile(const std::wstring& path) : DLCFile(DLCManager::eDLCType_Audio, path)
{
    m_pData = NULL;
    m_dataSize = 0;
}

void DLCAudioFile::addData(unsigned char* pData, unsigned int size)
{
    m_dataSize = size;
    m_pData = pData;

    processDLCDataFile(pData, size);
}

unsigned char* DLCAudioFile::getData(unsigned int& size)
{
    size = m_dataSize;
    return m_pData;
}

DLCAudioFile::EAudioParameterType DLCAudioFile::getParameterType(const std::wstring& name)
{
    EAudioParameterType result = eAudioParameterType_Invalid;

    if (name == s_audioParameterNames[eAudioParameterType_CueName])
    {
        result = eAudioParameterType_CueName;
    }
    else if (name == s_audioParameterNames[eAudioParameterType_Credit])
    {
        result = eAudioParameterType_Credit;
    }

    return result;
}

void DLCAudioFile::addParameter(EAudioType type, EAudioParameterType parameter, const std::wstring& value)
{
    if (parameter == eAudioParameterType_CueName)
    {
        m_soundNames[type].push_back(value);
    }
    else if (parameter == eAudioParameterType_Credit)
    {
        m_pPack->AddCreditString(value);
    }
}

bool DLCAudioFile::processDLCDataFile(unsigned char* pData, unsigned int size)
{
    boost::unordered_map<int, EAudioParameterType> parameterTypes;

    if (*(unsigned int*)pData == 0)
    {
        delete[] pData;
        return false;
    }

    unsigned int parameterCount = ((unsigned int*)pData)[1];
    unsigned int offset = 8;
    unsigned int* pParameter = (unsigned int*)(pData + offset);

    for (unsigned int i = 0; i < parameterCount; i++)
    {
        std::wstring parameterName = char16towstring((wchar_t*)(pParameter + 2));
        EAudioParameterType parameterType = getParameterType(parameterName);

        if (parameterType != eAudioParameterType_Invalid)
        {
            parameterTypes[(int)pParameter[0]] = parameterType;
        }

        offset += pParameter[1] * 2 + 12;
        pParameter = (unsigned int*)(pData + offset);
    }

    unsigned int itemCount = *(unsigned int*)(pData + offset);
    offset += 4;

    unsigned int dataOffset = offset;
    unsigned int* pItemData = (unsigned int*)(pData + dataOffset);

    for (unsigned int i = 0; i < itemCount; i++)
    {
        dataOffset += pItemData[2] * 2 + 16;
        pItemData = (unsigned int*)(pData + dataOffset);
    }

    while (itemCount != 0)
    {
        unsigned int* pItemHeader = (unsigned int*)(pData + offset);
        EAudioType audioType = (EAudioType)pItemHeader[1];

        unsigned int itemParameterCount = pItemData[0];
        pItemData++;

        for (unsigned int i = 0; i < itemParameterCount; i++)
        {
            if (parameterTypes.find((int)pItemData[0]) != parameterTypes.end())
            {
                addParameter(audioType, (EAudioParameterType)pItemData[0], char16towstring((wchar_t*)(pItemData + 2)));
            }

            pItemData = (unsigned int*)((unsigned char*)pItemData + pItemData[1] * 2 + 12);
        }

        itemCount--;
        offset += pItemHeader[2] * 2 + 16;
        pItemData = (unsigned int*)((unsigned char*)pItemData + pItemHeader[0]);
    }

    return true;
}

int DLCAudioFile::GetCountofType(EAudioType type)
{
    return (int)m_soundNames[type].size();
}

std::wstring& DLCAudioFile::GetSoundName(int index)
{
    int type = 0;

    while (m_soundNames[type].size() <= (unsigned int)index)
    {
        index -= (int)m_soundNames[type].size();
        type++;
    }

    return m_soundNames[type].at(index);
}
