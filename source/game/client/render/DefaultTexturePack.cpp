#include "game/client/render/DefaultTexturePack.hpp"
#include "game/core/StringIds.hpp"

#include <string>

#include "game/core/ByteArrayInputStream.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/InputStream.hpp"
#include "game/core/L10N.hpp"

extern "C" void OSReport(const char*, ...);

DefaultTexturePack::DefaultTexturePack()
    : AbstractTexturePack(0, NULL, L10N::GetString(IDS_DEFAULT_TEXTUREPACK_NAME), NULL)
{
    loadIcon();
    loadName();
    loadDescription();
    AbstractTexturePack::loadColourTable();
    AbstractTexturePack::loadMaterialTable();
    loadCustomModels();
}

void DefaultTexturePack::loadIcon()
{
    if (gApp.hasArchiveFile(std::wstring(L"Graphics\\TexturePackIcon.png"), false))
    {
        arrayWithLength<unsigned char> iconData = gApp.getArchiveFile(std::wstring(L"Graphics\\TexturePackIcon.png"),
                                                                      false);
        m_pIconData = iconData.data;
        m_iconSize = iconData.length;
    }
}

void DefaultTexturePack::loadName()
{
    m_name = L10N::GetString(IDS_DEFAULT_TEXTUREPACK_NAME);
}

void DefaultTexturePack::loadDescription()
{
    m_desc1 = L"LOCALISE ME: The default look of Minecraft";
}

void DefaultTexturePack::loadCustomModels()
{
    m_customModels.clear();

    File file(TexturePack::getPath(true, NULL).append(L"res/models.bin"));

    if (file.exists())
    {
        unsigned int length = (unsigned int)file.length();
        arrayWithLength<unsigned char> data(length, true);
        FileInputStream fileStream(file);
        fileStream.read(data, 0, length);
        fileStream.close();

        ByteArrayInputStream byteStream(data);
        DataInputStream input(&byteStream);
        int version = input.readInt();
        int count = input.readInt();

        for (int index = 0; index < count; index++)
        {
            std::wstring modelName = input.readUTF();
            m_customModels[modelName].readModel(&input, version);
        }
        byteStream.reset();
    }
    else
    {
#if defined(RECON_NATIVE)
        OSReport("[native] optional default texture-pack models.bin unavailable; continuing without custom models\n");
#else
        gApp.FatalLoadError(L"<AbstractTexturePack@%d> Failed to load default models", 191);
#endif
    }
}

bool DefaultTexturePack::hasFile(const std::wstring& fileName)
{
    return true;
}

bool DefaultTexturePack::isTerrainUpdateCompatible()
{
    return true;
}

std::wstring DefaultTexturePack::getName()
{
    return L10N::GetString(IDS_DEFAULT_TEXTUREPACK_NAME);
}

std::wstring DefaultTexturePack::getDesc1()
{
    return L10N::GetString(IDS_DEFAULT_TEXTUREPACK);
}

InputStream* DefaultTexturePack::getResourceImplementation(const std::wstring& fileName)
{
    std::wstring resourceRoot = L"";
    resourceRoot = L"Common\\res\\TitleUpdate\\res";
    InputStream* resource = InputStream::getResourceAsStream(resourceRoot + fileName);
    return resource;
}

void DefaultTexturePack::loadUI()
{
    AbstractTexturePack::loadDefaultUI();
    AbstractTexturePack::loadUI();
}

void DefaultTexturePack::unloadUI()
{
    AbstractTexturePack::unloadUI();
}

DLCPack* DefaultTexturePack::getDLCPack()
{
    return NULL;
}

bool DefaultTexturePack::hasData()
{
    return true;
}

bool DefaultTexturePack::hasValidData()
{
    return true;
}

bool DefaultTexturePack::hasAudio()
{
    return false;
}

bool DefaultTexturePack::isLoadingData()
{
    return false;
}
