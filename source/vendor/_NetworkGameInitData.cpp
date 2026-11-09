#include "vendor/_NetworkGameInitData.hpp"

_NetworkGameInitData::_NetworkGameInitData()
{
    m_seed = 0;
    m_saveData = 0;
    m_hostOptions = 0;
    m_miniGameHostOptions = 0;
    m_levelGenerationOptions = 0;
    m_texturePackId = 0;
    m_findSeed = false;
    m_xzSize = 54;
    m_hellScale = 3;
    m_savePlatform = eSavePlatform_WIIU;
    m_gameType = 0;
    m_levelTypeOptions = 0;
    m_biomeScale = 0;
    m_biomeCentreXChunk = 0;
    m_biomeCentreZChunk = 0;
    m_levelType = 0;
    m_recordLevelResume = 0;
}
