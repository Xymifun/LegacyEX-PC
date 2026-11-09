#include "game/client/render/AbstractTexturePack.hpp"


#include <string>

#include <boost/unordered_map.hpp>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/InputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/MaterialTable.hpp"
#include "game/world/generation/StructureManager.hpp"

static ModelDefinition s_emptyModelDefinition;

AbstractTexturePack::AbstractTexturePack(unsigned int id,
                                         File* pFile,
                                         const std::wstring& packName,
                                         TexturePack* pParentPack)
    : m_id(id)
    , m_packName(packName)
{
    m_pColourTable = NULL;
    m_iconTextureId = -1;
    m_pMaterialTable = NULL;
    m_pStructureManager = NULL;
    m_pFile = pFile;
    m_pParentPack = pParentPack;
    m_pIconData = NULL;
    m_iconSize = 0;
    m_pComparisonData = NULL;
    m_comparisonSize = 0;
}

void AbstractTexturePack::unload(Textures* pTextures)
{
    if (m_pIconImage == NULL)
        return;

    if (m_iconTextureId == -1)
        return;

    pTextures->releaseTexture(m_iconTextureId);
}

void AbstractTexturePack::load(Textures* pTextures)
{
    if (m_pIconImage == NULL)
        return;

    if (m_iconTextureId == -1)
        m_iconTextureId = pTextures->getTexture(m_pIconImage, C4JRender::eTextureFormat_RGBA8, true);

    GlStateManager::bindTexture(m_iconTextureId);
    pTextures->clearLastBoundId();
}

InputStream* AbstractTexturePack::getResource(const std::wstring& fileName, bool bCheckParentPack)
{
    InputStream* pStream = getResourceImplementation(fileName);

    if (pStream == NULL && m_pParentPack != NULL && bCheckParentPack)
        pStream = m_pParentPack->getResource(fileName, true);

    return pStream;
}

unsigned int AbstractTexturePack::getId()
{
    return m_id;
}

std::wstring AbstractTexturePack::getName()
{
    return m_name;
}

std::wstring AbstractTexturePack::getDesc1()
{
    return m_desc1;
}

std::wstring AbstractTexturePack::getDesc2()
{
    return m_desc2;
}

std::wstring AbstractTexturePack::getWorldName()
{
    return m_worldName;
}

bool AbstractTexturePack::hasFile(const std::wstring& fileName, bool bCheckParentPack)
{
    bool bFound = hasFile(fileName);

    if (!bFound && bCheckParentPack && m_pParentPack != NULL)
        bFound = m_pParentPack->hasFile(fileName, bCheckParentPack);

    return bFound;
}

void AbstractTexturePack::loadIcon()
{
}

void AbstractTexturePack::loadComparison()
{
}

void AbstractTexturePack::loadDescription()
{
}

void AbstractTexturePack::loadName()
{
}

std::wstring AbstractTexturePack::getAnimationString(const std::wstring& fileName,
                                                     const std::wstring& animationName,
                                                     const std::wstring& defaultString,
                                                     bool bCheckParentPack)
{
    return getAnimationString(fileName, animationName, defaultString);
}

std::wstring AbstractTexturePack::getAnimationString(const std::wstring& fileName,
                                                     const std::wstring& animationName,
                                                     const std::wstring& defaultString)
{
    static boost::unordered_map<std::wstring, std::wstring> animationStrings;
    static bool bAnimationStringsBuilt = false;

    if (!bAnimationStringsBuilt)
    {
        animationStrings[L"fire_0"] = L"16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15";
        animationStrings[L"fire_1"] = L" ";
        animationStrings[L"lava"] = L"0*2,1*2,2*2,3*2,4*2,5*2,6*2,7*2,8*2,9*2,10*2,11*2,12*2,13*2,14*2,15*2,16*2,17*2,18*2,19*2,18*2,17*2,16*2,15*2,14*2,13*2,12*2,11*2,10*2,9*2,8*2,7*2,6*2,5*2,4*2,3*2,2*2,1*2,";
        animationStrings[L"lava_flow"] = L"0*3,1*3,2*3,3*3,4*3,5*3,6*3,7*3,8*3,9*3,10*3,11*3,12*3,13*3,14*3,15*3";
        animationStrings[L"portal"] = L" ";
        animationStrings[L"water"] = L"0*2,1*2,2*2,3*2,4*2,5*2,6*2,7*2,8*2,9*2,10*2,11*2,12*2,13*2,14*2,15*2,16*2,17*2,18*2,19*2,20*2,21*2,22*2,23*2,24*2,25*2,26*2,27*2,28*2,29*2,30*2,31*2,";
        animationStrings[L"water_flow"] = L" ";
        animationStrings[L"prismarine_rough"] = L"#0*300,1*300,0*300,2*300,0*300,3*300,0*300,1*300,2*300,1*300,3*300,1*300,0*300,2*300,1*300,2*300,3*300,2*300,0*300,3*300,1*300,3*300";
        animationStrings[L"sea_lantern"] = L"0*5,1*5,2*5,3*5,4*5";
        animationStrings[L"magma"] = L"#0*8,1*8,2*8";
        animationStrings[L"kelp_a"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4";
        animationStrings[L"kelp_b"] = L"#5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4,0*4,1*4,2*4,3*4,4*4";
        animationStrings[L"kelp_c"] = L"#11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4,0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4";
        animationStrings[L"kelp_d"] = L"#5*4,16*4,17*4,18*4,19*4,0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4";
        animationStrings[L"kelp_top_a"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4";
        animationStrings[L"kelp_top_b"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4";
        animationStrings[L"kelp_top_c"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4";
        animationStrings[L"kelp_top_d"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4,18*4,19*4";
        animationStrings[L"seagrass"] = L"#0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,8*4,9*4,10*4,11*4,12*4,13*4,14*4,15*4,16*4,17*4";
        animationStrings[L"seagrass_doubletall_bottom"] = L"#0*3,1*3,2*3,3*3,4*3,5*3,6*3,7*3,8*3,9*3,10*3,11*3,12*3,13*3,14*3,15*3,16*3,17*3,18*3";
        animationStrings[L"seagrass_doubletall_top"] = L"#0*3,1*3,2*3,3*3,4*3,5*3,6*3,7*3,8*3,9*3,10*3,11*3,12*3,13*3,14*3,15*3,16*3,17*3,18*3";
        animationStrings[L"clock"] = L" ";
        animationStrings[L"compass"] = L" ";
        animationStrings[L"cauldron_water"] = L"0*2,1*2,2*2,3*2,4*2,5*2,6*2,7*2,8*2,9*2,10*2,11*2,12*2,13*2,14*2,15*2,16*2,17*2,18*2,19*2,20*2,21*2,22*2,23*2,24*2,25*2,26*2,27*2,28*2,29*2,30*2,31*2,";

        bAnimationStringsBuilt = true;
    }

    std::wstring animation = L"";

    boost::unordered_map<std::wstring, std::wstring>::iterator it = animationStrings.find(animationName);

    if (it != animationStrings.end())
        animation = it->second;

    return animation;
}

void AbstractTexturePack::loadDefaultUI()
{
    gConsoleUIController.ReloadSkin();
}

void AbstractTexturePack::loadDefaultColourTable()
{
    File file(TexturePack::getPath(true, NULL).append(L"res/colours.col"));

    if (file.exists())
    {
        unsigned int length = (unsigned int)file.length();
        arrayWithLength<unsigned char> colourData(length, true);

        FileInputStream stream(file);
        stream.read(colourData, 0, length);
        stream.close();

        if (m_pColourTable != NULL)
            delete m_pColourTable;
        m_pColourTable = new ColourTable(colourData.data, length);

        delete[] colourData.data;
    }
    else
    {
        gApp.FatalLoadError(L"<AbstractTexturePack@%d> Failed to load default colours table", __LINE__);
    }
}

void AbstractTexturePack::loadDefaultHTMLColourTable()
{
    if (gApp.hasArchiveFile(L"HTMLColours.col", false))
    {
        arrayWithLength<unsigned char> colourData = gApp.getArchiveFile(L"HTMLColours.col", false);

        m_pColourTable->loadColoursFromData(colourData.data, colourData.length);

        delete[] colourData.data;
    }
}

void AbstractTexturePack::loadDefaultStructureManager()
{
    delete m_pStructureManager;
    m_pStructureManager = new StructureManager(Minecraft::GetInstance()->getFixerUpper());
}

BufferedImage* AbstractTexturePack::getImageResource(const std::wstring& fileName,
                                                     bool bSingleImage,
                                                     bool bTitleUpdate,
                                                     const std::wstring& path)
{
    std::wstring imageName(fileName);
    imageName.append(L".png");
    std::wstring imagePath = L"/" + imageName;

    return new BufferedImage(std::wstring(imagePath), bSingleImage, bTitleUpdate, path);
}

void AbstractTexturePack::loadColourTable()
{
    loadDefaultColourTable();
    loadDefaultHTMLColourTable();
}

void AbstractTexturePack::loadMaterialTable()
{
    m_pMaterialTable = new MaterialTable();
}

void AbstractTexturePack::loadStructureManager()
{
    loadDefaultStructureManager();
}

void AbstractTexturePack::loadUI()
{
    loadColourTable();
}

void AbstractTexturePack::unloadUI()
{
}

void AbstractTexturePack::unloadAudio()
{
}

const void* AbstractTexturePack::getPackIcon(unsigned int& size)
{
    if (m_iconSize == 0 || m_pIconData == NULL)
        loadIcon();

    const void* pData = m_pIconData;
    unsigned int dataSize = m_iconSize;
    size = dataSize;

    return pData;
}

const void* AbstractTexturePack::getPackComparison(unsigned int& size)
{
    if (m_comparisonSize == 0 || m_pComparisonData == NULL)
        loadComparison();

    const void* pData = m_pComparisonData;
    unsigned int dataSize = m_comparisonSize;
    size = dataSize;

    return pData;
}

unsigned int AbstractTexturePack::getDLCParentPackId()
{
    return 0;
}

unsigned int AbstractTexturePack::getDLCSubPackId()
{
    return 0;
}

bool AbstractTexturePack::HasCustomModel(const std::wstring& modelName)
{
    return m_customModels.find(modelName) != m_customModels.end();
}

ModelDefinition* AbstractTexturePack::GetCustomModel(const std::wstring& modelName)
{
    if (HasCustomModel(modelName))
        return &m_customModels.find(modelName)->second;

    return &s_emptyModelDefinition;
}

ColourTable* AbstractTexturePack::getColourTable()
{
    return m_pColourTable;
}

MaterialTable* AbstractTexturePack::getMaterialTable()
{
    return m_pMaterialTable;
}

StructureManager* AbstractTexturePack::getStructureManager()
{
    return m_pStructureManager;
}

ArchiveFile* AbstractTexturePack::getArchiveFile()
{
    return NULL;
}
