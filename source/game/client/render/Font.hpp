#ifndef GAME_CLIENT_RENDER_FONT_HPP
#define GAME_CLIENT_RENDER_FONT_HPP

#include <map>
#include <string>

class BufferBuilder;
class Options;
class Random;
class ResourceLocation;
class Textures;

class Font
{
    friend class UIScene_EndPoem;

private:
    struct Character
    {
        float width;
        float height;
        float unused[2];
        float u0;
        float u1;
        float v0;
        float v1;
        float advance;
    };

    float* mCharWidths;
    Character* mCharacters;
    int mUnused08;
    Random* mRandom;
    int mColors[32];
    Textures* mTextures;
    float mX;
    float mY;
    bool mEnforceUnicodeSheet;
    bool mBidirectional;
    int mColumns;
    int mRows;
    int mCharacterWidth;
    int mCharacterHeight;
    ResourceLocation* mTextureLocation;
    std::map<int, int> mCharacterMap;

public:
    Font(Options* options, const std::wstring& name, Textures* textures, bool anaglyph,
         ResourceLocation* textureLocation, int columns, int rows, int characterWidth, int characterHeight,
         unsigned short* characterMap);

    bool CharacterExists(wchar_t character);
    int MapCharacter(wchar_t character);
    std::wstring sanitize(const std::wstring& text, bool trim);
    int width(const std::wstring& text);
    void renderCharacter(wchar_t character, BufferBuilder* builder);
    unsigned int draw(const std::wstring& text, bool darken, bool useBuilder);
    unsigned int draw(const std::wstring& text, int x, int y, int color, bool darken, bool useBuilder);
    unsigned int draw(const std::wstring& text, int x, int y, int color);
    void cacheCharacter(int character);
    void drawShadow(const std::wstring& text, int x, int y, int color);
    int wordWrapHeight(const std::wstring& text, int width);
    void drawWordWrapInternal(const std::wstring& text, int x, int y, int width, int color, int height);
    void drawWordWrapInternal(const std::wstring& text, int x, int y, int width, int color, bool unicode, int height);
    unsigned int drawNoBeginEndBuilder(const std::wstring& text, int x, int y, int color);
    void bindTexture();
    void drawWordWrap(const std::wstring& text, int x, int y, int width, int color, int height);

    ~Font();
    void drawShadowWordWrap(const std::wstring& text, int x, int y, int width, int color, int height);
    std::wstring reorderBidi(const std::wstring& text);
    void drawWordWrap(const std::wstring& text, int x, int y, int width, int color, bool unicode, int height);
    void setEnforceUnicodeSheet(bool enforce);
    void setBidirectional(bool bidirectional);
    static bool AllCharactersValid(const std::wstring& text);
};

#endif
