#include "game/client/render/DLCTexturePack.hpp"

#include <string>

#include "game/client/render/DLCModelsFile.hpp"
#include "game/client/audio/DLCAudioFile.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/core/ArchiveFile.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCBehavioursFile.hpp"
#include "game/core/DLCColourTableFile.hpp"
#include "game/core/DLCLocalisationFile.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/data/StringTable.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/DLCMaterialFile.hpp"
#include "game/world/block/MaterialTable.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"

extern C4JStorage* gStorage;

void DLCTexturePack::loadIcon()
{
    if (m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"icon", true, true))
    {
        DLCFile* file = m_dlcPack->getFile(DLCManager::eDLCType_Texture, L"icon", true, true);
        m_pIconData = file->getData(m_iconSize);
    }
    else
        AbstractTexturePack::loadIcon();
}

void DLCTexturePack::loadName()
{
    std::wstring name = L"";
    if (m_stringTable != NULL)
    {
        if ((m_dlcPack->GetPackId() & 0x400) != 0)
            m_worldName = m_stringTable->getString(L"IDS_WORLD_NAME");
        name = m_stringTable->getString(L"IDS_DISPLAY_NAME");
        if (name == L"")
            name = m_stringTable->getString(L"title");
    }
    m_name = name;
}

void DLCTexturePack::loadDescription()
{
    std::wstring description = L"";
    if (m_stringTable != NULL)
    {
        description = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
        if (description == L"")
            description = m_stringTable->getString(L"description");
    }
    m_desc1 = description;
}

DLCTexturePack::DLCTexturePack(unsigned int id, DLCPack* pack, TexturePack* parentPack)
    : AbstractTexturePack(id, NULL, pack->getName(), parentPack),
      m_dlcPack(pack),
      m_loadedPack(NULL),
      m_uiLoaded(false),
      m_loadingData(false),
      m_hasData(false),
      m_hasValidData(false),
      m_archiveFile(NULL)
{
    if (gApp.getLevelGenerationOptions() != NULL)
        gApp.getLevelGenerationOptions()->setLoadedData();

    m_colourTableLoaded = true;
    m_materialTableLoaded = true;
    m_stringTable = NULL;
    if (m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true))
    {
        DLCFile* file = m_dlcPack->getFile(DLCManager::eDLCType_Localisation, L"languages.loc", true, true);
        m_stringTable = static_cast<DLCLocalisationFile*>(file)->getStringTable();
    }

    loadIcon();
    loadName();
    loadDescription();
}

DLCTexturePack::~DLCTexturePack()
{
    delete m_archiveFile;
}

std::wstring DLCTexturePack::getResource(const std::wstring& fileName)
{
    return L"";
}

DLCPack* DLCTexturePack::getDLCPack()
{
    return m_loadedPack;
}

std::wstring DLCTexturePack::getDesc1()
{
    std::wstring description = m_stringTable->getString(L"IDS_TP_DESCRIPTION");
    if (description == L"")
        return m_stringTable->getString(L"description");
    return description;
}

std::wstring DLCTexturePack::getName()
{
    std::wstring name = m_stringTable->getString(L"IDS_DISPLAY_NAME");
    if (name == L"")
        return m_stringTable->getString(L"title");
    return name;
}

std::wstring DLCTexturePack::getWorldName()
{
    return m_stringTable->getString(L"IDS_WORLD_NAME");
}

void DLCTexturePack::loadComparison()
{
    if (m_dlcPack->doesPackContainFile(DLCManager::eDLCType_Texture, L"comparison", true, true))
    {
        DLCFile* file = m_dlcPack->getFile(DLCManager::eDLCType_Texture, L"comparison", true, true);
        m_pComparisonData = file->getData(m_comparisonSize);
    }
}

InputStream* DLCTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    return NULL;
}

bool DLCTexturePack::hasFile(const std::wstring& fileName)
{
    bool found = false;
    if (m_loadedPack != NULL)
        found = m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Texture, fileName, true, true);
    return found;
}

bool DLCTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring DLCTexturePack::getPath(bool bTitleUpdate, const char* pFileName)
{
    return L"";
}

std::wstring DLCTexturePack::getAnimationString(const std::wstring& fileName, const std::wstring& animationName,
                                                const std::wstring& defaultString)
{
    std::wstring result = L"";
    std::wstring path = L"res/" + defaultString + fileName;
    if (hasFile(path))
    {
        DLCFile* file = m_loadedPack->getFile(DLCManager::eDLCType_Texture, path, true, true);
        result = file->getParameterAsString(DLCManager::eDLCParameterType_Anim, animationName);
    }
    return result;
}

BufferedImage* DLCTexturePack::getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate,
                                                const std::wstring& path)
{
    if (m_loadedPack != NULL)
        return new BufferedImage(m_loadedPack, L"/" + fileName, bSingleImage);
    return m_pParentPack->getImageResource(fileName, bSingleImage, bTitleUpdate, path);
}

void DLCTexturePack::loadMaterialTable()
{
    if (m_loadedPack != NULL
        && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Material, L"entityMaterials.bin", true, true))
    {
        DLCMaterialFile* file = static_cast<DLCMaterialFile*>(
            m_loadedPack->getFile(DLCManager::eDLCType_Material, L"entityMaterials.bin", true, true));
        m_pMaterialTable = file->m_pMaterialTable;
        m_materialTableLoaded = false;
    }
    else
    {
        if (!m_materialTableLoaded)
            m_pMaterialTable = NULL;
        AbstractTexturePack::loadMaterialTable();
        m_materialTableLoaded = true;
    }
}

void DLCTexturePack::loadColourTable()
{
    if (m_loadedPack != NULL
        && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true))
    {
        DLCColourTableFile* file = static_cast<DLCColourTableFile*>(
            m_loadedPack->getFile(DLCManager::eDLCType_ColourTable, L"colours.col", true, true));
        m_pColourTable = file->m_pColourTable;
        m_colourTableLoaded = false;
    }
    else
    {
        if (!m_colourTableLoaded)
            m_pColourTable = NULL;
        AbstractTexturePack::loadDefaultColourTable();
        m_colourTableLoaded = true;
    }
    if (gApp.hasArchiveFile(L"HTMLColours.col", false))
    {
        arrayWithLength<unsigned char> colourData = gApp.getArchiveFile(L"HTMLColours.col", false);
        m_pColourTable->loadColoursFromData(colourData.data, colourData.length);
        delete[] colourData.data;
    }
}

void DLCTexturePack::loadStructureManager()
{
    delete m_pStructureManager;
    m_pStructureManager = new StructureManager(NULL);
}

bool DLCTexturePack::hasData()
{
    return m_hasData;
}

bool DLCTexturePack::hasValidData()
{
    return m_hasValidData;
}

bool DLCTexturePack::isLoadingData()
{
    return m_loadingData;
}

void DLCTexturePack::loadCustomModels()
{
    if (m_loadedPack != NULL
        && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Models, L"models.bin", true, true))
    {
        m_customModels.clear();
        DLCModelsFile* file = static_cast<DLCModelsFile*>(
            m_loadedPack->getFile(DLCManager::eDLCType_Models, L"models.bin", true, true));
        ByteArrayInputStream byteStream(file->getData());
        DataInputStream stream(&byteStream);
        int version = stream.readInt();
        int count = stream.readInt();
        for (int i = 0; i < count; ++i)
        {
            std::wstring name = stream.readUTF();
            m_customModels[name].readModel(&stream, version);
        }
        byteStream.reset();
    }
}

void DLCTexturePack::loadBehaviours()
{
    if (m_loadedPack != NULL
        && m_loadedPack->doesPackContainFile(DLCManager::eDLCType_Behaviours, L"behaviours.bin", true, true))
    {
        DLCBehavioursFile* file = static_cast<DLCBehavioursFile*>(
            m_loadedPack->getFile(DLCManager::eDLCType_Behaviours, L"behaviours.bin", true, true));
        ByteArrayInputStream byteStream(file->getData());
        DataInputStream stream(&byteStream);
        stream.readInt();
        int count = stream.readInt();
        for (int i = 0; i < count; ++i)
            RiderPositionOverrides::ReadOverrides(&stream);
        byteStream.reset();
    }
}

int DLCTexturePack::packMounted(void* data, int userIndex, unsigned int result, unsigned int licenseMask)
{
    DLCTexturePack* pack = static_cast<DLCTexturePack*>(data);
    pack->m_loadingData = false;
    if (result != 0)
    {
        pack->loadColourTable();
        pack->loadMaterialTable();
        for (int attempts = 200; gMinecraftServer == NULL && attempts > 0; --attempts)
            Sleep(10);

        if (gApp.getLevelGenerationOptions() != NULL)
            gApp.getLevelGenerationOptions()->setLoadedData();
    }
    else
    {
        pack->m_loadedPack = new DLCPack(pack->m_dlcPack->getName(), licenseMask);
        pack->setHasAudio(false);
        pack->m_hasValidData = true;

        unsigned int numItemsAdded = 0;
        std::wstring dataPath = pack->m_dlcPack->getFullDataPath();
        if (dataPath.empty())
        {
            pack->hasAudio();
        }
        else
        {
            if (!gApp.mDLCManager.readDLCDataFile(
                    numItemsAdded, gApp.getFilePath(pack->m_dlcPack->GetPackId(), dataPath, true, L"TPACK:"),
                    pack->m_loadedPack, pack->m_dlcPack->GetPackId(), false))
            {
                delete pack->m_loadedPack;
                pack->m_loadedPack = NULL;
            }
            else
            {
                if (pack->m_loadedPack != NULL)
                {
                    File mediaArchive(gApp.getFilePath(pack->m_dlcPack->GetPackId(), L"media.arc", true, L"TPACK:"));
                    if (mediaArchive.exists())
                    {
                        delete pack->m_archiveFile;
                        pack->m_archiveFile = new ArchiveFile(mediaArchive);
                    }

                    DLCPack* parentPack = pack->m_dlcPack->GetParentPack();
                    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
                    if (options != NULL && options->getParentPackId() == pack->m_dlcPack->GetPackId()
                        && !options->hasLoadedData())
                    {
                        LevelGenerationOptions::loadGameRulesHeaders(parentPack, pack->m_stringTable, options);
                        if (options->requiresBaseSave() && !options->getBaseSavePath().empty())
                        {
                            File baseSave(gApp.getFilePath(pack->m_dlcPack->GetPackId(), options->getBaseSavePath(),
                                                           true, L"TPACK:"));
                            options->setBaseSaveData(baseSave);
                        }
                    }

                    if (parentPack->getDLCItemsCount(DLCManager::eDLCType_Audio) > 0)
                    {
                        DLCAudioFile* audioFile = static_cast<DLCAudioFile*>(
                            parentPack->getFile(DLCManager::eDLCType_Audio, 0, true, true));
                        pack->setHasAudio(true);

                        int musicStart = 0;
                        int musicCount = audioFile->GetCountofType(DLCAudioFile::eAudioType_Music);
                        int menuStart = musicCount;
                        int menuCount = audioFile->GetCountofType(DLCAudioFile::eAudioType_Menu);
                        int bossStart = menuStart + menuCount;
                        int bossCount = audioFile->GetCountofType(DLCAudioFile::eAudioType_Boss);
                        int nextStart = bossStart + bossCount;

                        int miniGameStart;
                        int miniGameCount = audioFile->GetCountofType(DLCAudioFile::eAudioType_MiniGame);
                        if (miniGameCount == 0)
                        {
                            miniGameStart = musicStart;
                            miniGameCount = musicCount;
                        }
                        else
                        {
                            miniGameStart = nextStart;
                            nextStart += miniGameCount;
                        }

                        int creditsStart;
                        int creditsCount = audioFile->GetCountofType(DLCAudioFile::eAudioType_Credits);
                        if (creditsCount == 0)
                        {
                            creditsStart = musicStart;
                            creditsCount = musicCount;
                        }
                        else
                        {
                            creditsStart = nextStart;
                            nextStart += creditsCount;
                        }

                        int miniGame1Start;
                        int miniGame1Count = audioFile->GetCountofType(DLCAudioFile::eAudioType_MiniGame1);
                        if (miniGame1Count == 0)
                        {
                            miniGame1Start = musicStart;
                            miniGame1Count = musicCount;
                        }
                        else
                        {
                            miniGame1Start = nextStart;
                            nextStart += miniGame1Count;
                        }

                        int miniGame2Start;
                        int miniGame2Count = audioFile->GetCountofType(DLCAudioFile::eAudioType_MiniGame2);
                        if (miniGame2Count == 0)
                        {
                            miniGame2Start = musicStart;
                            miniGame2Count = musicCount;
                        }
                        else
                        {
                            miniGame2Start = nextStart;
                            nextStart += miniGame2Count;
                        }

                        int miniGame3Start;
                        int miniGame3Count = audioFile->GetCountofType(DLCAudioFile::eAudioType_MiniGame3);
                        if (miniGame3Count == 0)
                        {
                            miniGame3Start = musicStart;
                            miniGame3Count = musicCount;
                        }
                        else
                        {
                            miniGame3Start = nextStart;
                            nextStart += miniGame3Count;
                        }

                        int miniGame4Start;
                        int miniGame4Count = audioFile->GetCountofType(DLCAudioFile::eAudioType_MiniGame4);
                        if (miniGame4Count == 0)
                        {
                            miniGame4Start = miniGameStart;
                            miniGame4Count = miniGameCount;
                        }
                        else
                        {
                            miniGame4Start = nextStart;
                            nextStart += miniGame4Count;
                        }

                        Minecraft::GetInstance()->mSoundEngine->SetStreamingSounds(
                            musicStart, musicStart + musicCount - 1, menuStart, menuStart + menuCount - 1, bossStart,
                            bossStart + bossCount - 1, miniGameStart, miniGameStart + miniGameCount - 1, creditsStart,
                            creditsStart + creditsCount - 1, miniGame1Start, miniGame1Start + miniGame1Count - 1,
                            miniGame2Start, miniGame2Start + miniGame2Count - 1, miniGame3Start,
                            miniGame3Start + miniGame3Count - 1, miniGame4Start, miniGame4Start + miniGame4Count - 1,
                            nextStart);
                    }
                }
            }

            pack->loadColourTable();
            pack->loadMaterialTable();
            pack->loadCustomModels();
            pack->loadBehaviours();
            pack->hasAudio();
        }
    }

    pack->m_hasData = true;
    if (gApp.getLevelGenerationOptions() != NULL
        && gApp.getLevelGenerationOptions()->getParentPackId() == pack->m_dlcPack->GetPackId())
        gApp.getLevelGenerationOptions()->setLoadedData();
    gApp.StartReloadingTexturePack();
    return 0;
}

void DLCTexturePack::loadData()
{
    int mountIndex = m_dlcPack->GetDLCMountIndex();
    if (mountIndex >= 0)
    {
        int result = gStorage->MountInstalledDLC(gProfile->GetPrimaryPad(), (unsigned int)mountIndex, packMounted, this,
                                                 "TPACK");
        if (result != 0x3e5)
        {
            loadColourTable();
            loadMaterialTable();
            m_hasData = true;
            if (gApp.getLevelGenerationOptions() != NULL)
                gApp.getLevelGenerationOptions()->setLoadedData();
            return;
        }
        m_loadingData = true;
    }
    else
    {
        loadColourTable();
        loadMaterialTable();
        m_hasData = true;
        if (gApp.getLevelGenerationOptions() != NULL)
            gApp.getLevelGenerationOptions()->setLoadedData();
        gApp.StartReloadingTexturePack();
    }
}

void DLCTexturePack::loadUI()
{
    if (m_archiveFile != NULL && m_archiveFile->hasFile(L"skin.swf"))
        gConsoleUIController.ReloadSkin();
    else
        AbstractTexturePack::loadDefaultUI();
    m_uiLoaded = true;
    AbstractTexturePack::loadUI();
    if (hasAudio() == false)
        gConsoleUIController.IsReloadingSkin();
}

void DLCTexturePack::unloadUI()
{
    if (m_uiLoaded)
        setHasAudio(false);
    AbstractTexturePack::unloadUI();
    gApp.mDLCManager.removePack(m_loadedPack);
    m_loadedPack = NULL;
    delete m_archiveFile;
    m_archiveFile = NULL;
    m_hasData = false;
    m_uiLoaded = false;
}

void DLCTexturePack::unloadAudio()
{
}

ArchiveFile* DLCTexturePack::getArchiveFile()
{
    return m_archiveFile;
}

unsigned int DLCTexturePack::getDLCParentPackId()
{
    return m_dlcPack->GetParentPackId();
}

DLCPack* DLCTexturePack::getDLCInfoParentPack()
{
    return m_dlcPack->GetParentPack();
}

unsigned int DLCTexturePack::getDLCSubPackId()
{
    return m_dlcPack->GetPackId() >> 24;
}
