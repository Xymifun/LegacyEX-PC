#ifndef GAME_CLIENT_AUDIO_DLCAUDIOFILE_HPP
#define GAME_CLIENT_AUDIO_DLCAUDIOFILE_HPP

#include <string>
#include <vector>

#include "game/core/DLCFile.hpp"

class DLCAudioFile : public DLCFile
{
public:
    enum EAudioType
    {
        eAudioType_Music,
        eAudioType_Menu,
        eAudioType_Boss,
        eAudioType_MiniGame,
        eAudioType_Credits,
        eAudioType_MiniGame1,
        eAudioType_MiniGame2,
        eAudioType_MiniGame3,
        eAudioType_MiniGame4,
        eAudioType_NumTypes
    };

    enum EAudioParameterType
    {
        eAudioParameterType_Invalid = -1,
        eAudioParameterType_CueName = 0,
        eAudioParameterType_Credit,
        eAudioParameterType_NumTypes
    };

    DLCAudioFile(const std::wstring& path);

    virtual void addData(unsigned char* pData, unsigned int size);
    virtual unsigned char* getData(unsigned int& size);

    EAudioParameterType getParameterType(const std::wstring& name);
    void addParameter(EAudioType type, EAudioParameterType parameter, const std::wstring& value);
    bool processDLCDataFile(unsigned char* pData, unsigned int size);

    int GetCountofType(EAudioType type);
    std::wstring& GetSoundName(int index);

protected:
    unsigned char* m_pData;
    unsigned int m_dataSize;
    std::vector<std::wstring> m_soundNames[eAudioType_NumTypes];
};

#endif
