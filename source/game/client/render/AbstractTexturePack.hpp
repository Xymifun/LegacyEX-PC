#ifndef GAME_CLIENT_RENDER_ABSTRACTTEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_ABSTRACTTEXTUREPACK_HPP

#include <string>

#include <boost/unordered_map.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"

class File;
class InputStream;

class AbstractTexturePack : public TexturePack
{
public:
    typedef boost::unordered_map<std::wstring, ModelDefinition> CustomModelMap;

    AbstractTexturePack(unsigned int id, File* pFile, const std::wstring& packName, TexturePack* pParentPack);

    virtual void loadIcon();
    virtual void loadComparison();
    virtual void loadDescription();
    virtual void loadName();
    virtual InputStream* getResourceImplementation(const std::wstring& fileName) = 0;
    virtual bool hasFile(const std::wstring& fileName) = 0;
    virtual std::wstring getWorldName();
    virtual std::wstring getAnimationString(const std::wstring& fileName, const std::wstring& animationName,
                                            const std::wstring& defaultString);

    virtual void unload(Textures* pTextures);
    virtual void load(Textures* pTextures);
    virtual InputStream* getResource(const std::wstring& fileName, bool bCheckParentPack);
    virtual unsigned int getId();
    virtual std::wstring getName();
    virtual std::wstring getDesc1();
    virtual std::wstring getDesc2();
    virtual bool hasFile(const std::wstring& fileName, bool bCheckParentPack);
    virtual DLCPack* getDLCPack() = 0;
    virtual std::wstring getAnimationString(const std::wstring& fileName, const std::wstring& animationName,
                                            const std::wstring& defaultString, bool bCheckParentPack);
    virtual BufferedImage* getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate,
                                            const std::wstring& path);
    virtual void loadColourTable();
    virtual void loadMaterialTable();
    virtual void loadStructureManager();
    virtual void loadUI();
    virtual void unloadUI();
    virtual void unloadAudio();
    virtual const void* getPackIcon(unsigned int& size);
    virtual const void* getPackComparison(unsigned int& size);
    virtual unsigned int getDLCParentPackId();
    virtual unsigned int getDLCSubPackId();
    virtual ColourTable* getColourTable();
    virtual MaterialTable* getMaterialTable();
    virtual StructureManager* getStructureManager();
    virtual ArchiveFile* getArchiveFile();
    virtual bool HasCustomModel(const std::wstring& modelName);
    virtual ModelDefinition* GetCustomModel(const std::wstring& modelName);

    void loadDefaultUI();
    void loadDefaultColourTable();
    void loadDefaultHTMLColourTable();
    void loadDefaultStructureManager();

    static std::wstring trim(std::wstring text);

protected:
    unsigned int m_id;
    std::wstring m_packName;
    File* m_pFile;
    std::wstring m_name;
    std::wstring m_worldName;
    std::wstring m_desc1;
    std::wstring m_desc2;
    const void* m_pIconData;
    unsigned int m_iconSize;
    const void* m_pComparisonData;
    unsigned int m_comparisonSize;
    TexturePack* m_pParentPack;
    MaterialTable* m_pMaterialTable;
    ColourTable* m_pColourTable;
    StructureManager* m_pStructureManager;
    BufferedImage* m_pIconImage;
    CustomModelMap m_customModels;
    int m_iconTextureId;
};

#endif
