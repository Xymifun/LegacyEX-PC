#ifndef GAME_CLIENT_RENDER_DLCTEXTUREPACK_HPP
#define GAME_CLIENT_RENDER_DLCTEXTUREPACK_HPP

#include <string>

#include "game/client/render/AbstractTexturePack.hpp"

class ArchiveFile;
class DLCPack;
class StringTable;

class DLCTexturePack : public AbstractTexturePack
{
public:
    DLCTexturePack(unsigned int id, DLCPack* pack, TexturePack* parentPack);
    virtual ~DLCTexturePack();

    virtual void loadIcon();
    virtual void loadName();
    virtual void loadDescription();
    virtual std::wstring getResource(const std::wstring& fileName);
    virtual DLCPack* getDLCPack();
    virtual std::wstring getDesc1();
    virtual std::wstring getName();
    virtual std::wstring getWorldName();
    virtual void loadComparison();
    virtual InputStream* getResourceImplementation(const std::wstring& fileName);
    virtual bool hasFile(const std::wstring& fileName);
    virtual bool isTerrainUpdateCompatible();
    virtual std::wstring getPath(bool bTitleUpdate, const char* pFileName);
    virtual std::wstring getAnimationString(const std::wstring& fileName, const std::wstring& animationName,
                                            const std::wstring& defaultString);
    virtual BufferedImage* getImageResource(const std::wstring& fileName, bool bSingleImage, bool bTitleUpdate,
                                            const std::wstring& path);
    virtual void loadMaterialTable();
    virtual void loadColourTable();
    virtual void loadStructureManager();
    virtual bool hasData();
    virtual bool hasValidData();
    virtual bool isLoadingData();
    virtual void loadCustomModels();
    void loadBehaviours();
    static int packMounted(void* data, int userIndex, unsigned int result, unsigned int licenseMask);
    virtual void loadData();
    virtual void loadUI();
    virtual void unloadUI();
    virtual void unloadAudio();
    virtual ArchiveFile* getArchiveFile();
    virtual unsigned int getDLCParentPackId();
    virtual DLCPack* getDLCInfoParentPack();
    virtual unsigned int getDLCSubPackId();
    int GetDLCDeviceID();

private:
    DLCPack* m_dlcPack;
    DLCPack* m_loadedPack;
    StringTable* m_stringTable;
    bool m_uiLoaded;
    bool m_loadingData;
    bool m_hasData;
    bool m_hasValidData;
    bool m_colourTableLoaded;
    bool m_materialTableLoaded;
    ArchiveFile* m_archiveFile;
};

#endif
