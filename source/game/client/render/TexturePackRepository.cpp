#include "game/client/render/TexturePackRepository.hpp"

#include <boost/unordered_map.hpp>
#include <new>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/MapRenderer.hpp"

TexturePack* TexturePackRepository::defaultTexturePack = NULL;

TexturePackRepository::TexturePackRepository(File file, Minecraft* minecraft)
{
    if (defaultTexturePack == NULL)
    {
        defaultTexturePack = new DefaultTexturePack();
    }

    m_skinReloaded = false;
    m_selected = NULL;
    m_allTexturePacks = new std::vector<TexturePack*>;
    m_minecraft = minecraft;
    m_allTexturePacks->push_back(defaultTexturePack);
    m_texturePacksById[defaultTexturePack->getId()] = defaultTexturePack;
    m_selected = defaultTexturePack;
    defaultTexturePack->loadColourTable();
    defaultTexturePack->loadStructureManager();
    m_unknown0 = 0;
    m_unknown1 = NULL;
    m_previousSelected = NULL;
}

TexturePack* TexturePackRepository::getSelected()
{
    if (m_selected != NULL && m_selected->hasData() && m_selected->hasValidData())
    {
        return m_selected;
    }
    return defaultTexturePack;
}

void TexturePackRepository::clearInvalidTexturePacks()
{
    std::vector<TexturePack*>::iterator iterator = m_texturePacks.begin();
    while (iterator != m_texturePacks.end())
    {
        if (*iterator != NULL)
        {
            delete *iterator;
        }
        ++iterator;
    }
}

void TexturePackRepository::addDebugPacks()
{
}

bool TexturePackRepository::selectSkin(TexturePack* texturePack)
{
    if (texturePack == m_selected)
    {
        return false;
    }
    m_previousSelected = m_selected;
    m_selected = texturePack;
    m_skinReloaded = false;
    return true;
}

unsigned int TexturePackRepository::getSelectedId()
{
    if (m_selected != NULL)
    {
        return m_selected->getId();
    }
    return 0;
}

TexturePackRepository::TexturePackIdNames* TexturePackRepository::getTexturePackIdNames()
{
    TexturePackIdNames* names = new TexturePackIdNames;
    std::vector<TexturePack*>::iterator iterator;
    for (iterator = m_allTexturePacks->begin(); iterator != m_allTexturePacks->end(); ++iterator)
    {
        TexturePack* texturePack = *iterator;
        names->push_back(TexturePackIdName(texturePack->getId(), texturePack->getName()));
    }
    return names;
}

bool TexturePackRepository::selectTexturePackById(unsigned int packId, bool forceReload)
{
    bool result = false;
    gApp.SetRequiredTexturePackID(packId);
    boost::unordered_map<unsigned int, TexturePack*>::iterator iterator = m_texturePacksById.find(packId);
    if (iterator != m_texturePacksById.end())
    {
        TexturePack* texturePack = iterator->second;
        if (texturePack != m_selected)
        {
            selectSkin(texturePack);
            if (texturePack->hasData())
            {
                gApp.StartReloadingTexturePack();
            }
            else
            {
                texturePack->loadData();
            }
        }
        result = true;
    }
    else
    {
        if (selectSkin(defaultTexturePack))
        {
            gApp.StartReloadingTexturePack();
        }
    }
    return result;
}

TexturePack* TexturePackRepository::getTexturePackById(unsigned int packId)
{
    boost::unordered_map<unsigned int, TexturePack*>::iterator iterator = m_texturePacksById.find(packId);
    if (iterator != m_texturePacksById.end())
    {
        return iterator->second;
    }
    return NULL;
}

TexturePack* TexturePackRepository::addTexturePackFromDLC(DLCPack* dlcPack, unsigned int packId)
{
    TexturePack* texturePack = NULL;
    unsigned int id = packId & 0x00ffffff;
    if (dlcPack != NULL)
    {
        texturePack = new DLCTexturePack(id, dlcPack, defaultTexturePack);
        m_allTexturePacks->push_back(texturePack);
        m_texturePacksById[id] = texturePack;
    }
    return texturePack;
}

void TexturePackRepository::updateUI()
{
    if (m_previousSelected != NULL && m_previousSelected != m_selected)
    {
        m_previousSelected->unloadUI();
        m_selected->loadUI();
        MapRenderer::reloadColours();
        gConsoleUIController.StartReloadSkinThread();
        m_previousSelected = NULL;
    }
}

bool TexturePackRepository::needsUIUpdate()
{
    return m_previousSelected != NULL && m_previousSelected != m_selected;
}

int TexturePackRepository::getTexturePackCount()
{
    return static_cast<int>(m_allTexturePacks->size());
}

TexturePack* TexturePackRepository::getTexturePackByIndex(unsigned int index)
{
    TexturePack* texturePack = NULL;
    if (index < m_allTexturePacks->size())
    {
        texturePack = m_allTexturePacks->at(index);
    }
    return texturePack;
}

unsigned int TexturePackRepository::getTexturePackIndexByPackId(unsigned int packId)
{
    unsigned int index = 0;
    for (std::vector<TexturePack*>::iterator iterator = m_allTexturePacks->begin();
         iterator != m_allTexturePacks->end(); ++index, ++iterator)
    {
        if ((*iterator)->getId() == packId)
        {
            break;
        }
    }
    if (index >= m_allTexturePacks->size())
    {
        index = 0;
    }
    return index;
}
