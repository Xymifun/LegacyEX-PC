#ifndef GAME_CLIENT_RENDER_TEXTUREPACKREPOSITORY_HPP
#define GAME_CLIENT_RENDER_TEXTUREPACKREPOSITORY_HPP

#include <string>
#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/client/render/DefaultTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/File.hpp"

class Minecraft;
class DLCPack;

class TexturePackRepository
{
public:
    typedef std::pair<unsigned int, std::wstring> TexturePackIdName;
    typedef std::vector<TexturePackIdName> TexturePackIdNames;

    static TexturePack* defaultTexturePack;

    TexturePackRepository(File file, Minecraft* minecraft);

    TexturePack* getSelected();
    void clearInvalidTexturePacks();
    void addDebugPacks();
    bool selectSkin(TexturePack* texturePack);
    unsigned int getSelectedId();
    TexturePackIdNames* getTexturePackIdNames();
    bool selectTexturePackById(unsigned int packId, bool forceReload);
    TexturePack* getTexturePackById(unsigned int packId);
    TexturePack* addTexturePackFromDLC(DLCPack* dlcPack, unsigned int packId);
    void updateUI();
    bool needsUIUpdate();
    int getTexturePackCount();
    TexturePack* getTexturePackByIndex(unsigned int index);
    unsigned int getTexturePackIndexByPackId(unsigned int packId);

    void updateList();
    void createWorkingDirecoryUnlessExists();
    void selectWebSkin(const std::wstring& path);
    void downloadWebSkin(const std::wstring& path, File file);
    bool isUsingWebSkin();
    void resetWebSkin();
    unsigned int getIdOrNull(File file);
    std::vector<File*>* getWorkDirContents();
    std::vector<TexturePack*>* getAll();
    bool shouldPromptForWebSkin();
    bool canUseWebSkin();
    void removeTexturePackById(unsigned int packId);

private:
    unsigned int m_unknown0;
    void* m_unknown1;
    Minecraft* m_minecraft;
    File m_workDir;
    File m_texturePackDir;
    std::vector<TexturePack*>* m_allTexturePacks;
    std::vector<TexturePack*> m_texturePacks;
    boost::unordered_map<unsigned int, TexturePack*> m_texturePacksById;
    TexturePack* m_selected;
    TexturePack* m_previousSelected;
    bool m_skinReloaded;
};

#endif
