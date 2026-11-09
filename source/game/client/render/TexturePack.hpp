#ifndef GAME_CLIENT_RENDER_TEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_TEXTUREPACK_HPP

#include <string>

class ArchiveFile;
class BufferedImage;
class ColourTable;
class DLCPack;
class InputStream;
class MaterialTable;
class ModelDefinition;
class StructureManager;
class Textures;

class TexturePack
{
public:
    TexturePack() : m_bHasAudio(false) {}

    virtual ~TexturePack() {}

    virtual bool hasData() = 0;
    virtual bool hasValidData() = 0;
    virtual bool hasAudio();
    virtual void setHasAudio(bool bHasAudio);
    virtual bool isLoadingData() = 0;
    virtual void loadData();
    virtual void unload(Textures* pTextures) = 0;
    virtual void load(Textures* pTextures) = 0;
    virtual InputStream* getResource(const std::wstring& fileName, bool bCheckParentPack) = 0;
    virtual std::wstring getResource(const std::wstring& fileName);
    virtual unsigned int getId() = 0;
    virtual std::wstring getName() = 0;
    virtual std::wstring getDesc1() = 0;
    virtual std::wstring getDesc2() = 0;
    virtual bool hasFile(const std::wstring& fileName, bool bCheckParentPack) = 0;
    virtual bool isTerrainUpdateCompatible() = 0;
    virtual DLCPack* getDLCPack();
    virtual std::wstring getPath(bool bTitleUpdate, const char* pFileName);
    virtual std::wstring getAnimationString(const std::wstring& fileName, const std::wstring& animationName,
                                            const std::wstring& defaultString, bool bCheckParentPack) = 0;
    virtual BufferedImage* getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate,
                                            const std::wstring& path) = 0;
    virtual void loadColourTable() = 0;
    virtual void loadMaterialTable() = 0;
    virtual void loadStructureManager() = 0;
    virtual void loadUI() = 0;
    virtual void unloadUI() = 0;
    virtual void unloadAudio() = 0;
    virtual const void* getPackIcon(unsigned int& size) = 0;
    virtual const void* getPackComparison(unsigned int& size) = 0;
    virtual unsigned int getDLCParentPackId() = 0;
    virtual unsigned int getDLCSubPackId() = 0;
    virtual ColourTable* getColourTable() = 0;
    virtual MaterialTable* getMaterialTable() = 0;
    virtual StructureManager* getStructureManager() = 0;
    virtual void loadCustomModels() = 0;
    virtual ArchiveFile* getArchiveFile() = 0;
    virtual bool HasCustomModel(const std::wstring& modelName) = 0;
    virtual ModelDefinition* GetCustomModel(const std::wstring& modelName) = 0;

protected:
    bool m_bHasAudio;
};

#endif
