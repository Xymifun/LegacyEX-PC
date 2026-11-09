#ifndef GAME_UI_UITTFFONT_HPP
#define GAME_UI_UITTFFONT_HPP

#include <string>
#include <vector>

class UITTFFont
{
private:
    std::string mFontName;

public:
    UITTFFont(const std::string& fontName, const std::string& fontFile, int missingGlyph);
    UITTFFont(const std::string& fontName, const std::string& regularFontFile, const std::string& boldFontFile,
              const std::string& italicFontFile, int missingGlyph);
    UITTFFont(const std::string& fontName, const std::string& regularFontFile, const std::string& fontFile,
              int missingGlyph);
    ~UITTFFont();
    void init(const std::string& fontName, std::vector<std::string> fontFiles, int missingGlyph);
    std::string getFontName();
};

#endif
