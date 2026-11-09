#include "game/ui/UITTFFont.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/fui.hpp"
#include "platform/wiiu/NintendoFileSystem_WiiU.hpp"
#include "platform/wiiu/WiiU_FileCache.hpp"

UITTFFont::UITTFFont(const std::string& fontName, const std::string& fontFile, int missingGlyph)
{
    mFontName = fontName;
    std::vector<std::string> fontFiles;
    fontFiles.push_back(fontFile);
    init(fontName, fontFiles, missingGlyph);
}

UITTFFont::UITTFFont(const std::string& fontName, const std::string& regularFontFile, const std::string& boldFontFile,
                     const std::string& italicFontFile, int missingGlyph)
{
    mFontName = fontName;
    std::vector<std::string> fontFiles;
    fontFiles.push_back(regularFontFile);
    fontFiles.push_back(boldFontFile);
    fontFiles.push_back(italicFontFile);
    init(fontName, fontFiles, missingGlyph);
}

UITTFFont::~UITTFFont()
{
    gFui->uninstallTrueTypeFont(mFontName.c_str());
}

void UITTFFont::init(const std::string& fontName, std::vector<std::string> fontFiles, int missingGlyph)
{
    const void** fontData = new const void*[fontFiles.size()];
    unsigned int fontIndex;
    unsigned int fontCount = fontFiles.size();
    for (fontIndex = 0; fontIndex < fontCount; ++fontIndex)
    {
        int file = CreateFileA(fontFiles[fontIndex].c_str(), 0x80000000, 0, NULL, 3, 0x80, 0);
        unsigned int fileSizeHigh = 0;
        unsigned int fileSize = GetFileSize(file, &fileSizeHigh);
        if (fileSize != 0)
        {
            unsigned char* data = new unsigned char[fileSize];
            fontData[fontIndex] = data;
            unsigned int bytesRead;
            if (ReadFile(file, data, fileSize, &bytesRead, NULL) == 0)
            {
                gApp.FatalLoadError(L"<UITTFFont@%d> Failed to load font %s", 72, fontFiles[fontIndex].c_str());
            }
            CloseHandle(file);
        }
    }

    gFui->installTrueTypeFont((int)fontCount, fontData, mFontName.c_str(), (unsigned short)missingGlyph);
    delete[] fontData;
}

std::string UITTFFont::getFontName()
{
    return mFontName;
}
