#ifndef GAME_UI_FUIRENDERNODEEDITTEXT_HPP
#define GAME_UI_FUIRENDERNODEEDITTEXT_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiFontSet.hpp"
#include "game/ui/fuiRenderNode.hpp"

enum TextFieldAutoSize
{
    TextFieldAutoSizeNone = 0,
    TextFieldAutoSizeLeft = 1,
    TextFieldAutoSizeCenter = 2,
    TextFieldAutoSizeRight = 3
};

class FJ_Base;
class FJ_Slider;
class FJ_TextInput;

class fuiRenderNodeEditText : public fuiRenderNode
{
public:
    friend class FJ_Base;
    friend class FJ_Button;
    friend class FJ_CheckBox;
    friend class FJ_Slider;
    friend class FJ_TextInput;
    friend class FJ_HtmlTextPanel;
    friend class FJ_HtmlTextPanelLarge;
    friend class FJ_Label;
    friend class FJ_LabelOutline;
    friend class FJ_LeaderboardEntry;
    friend class FJ_Label_Blink;
    friend class FJ_List;

    struct HtmlImageDrawData
    {
        fuiFontSet::FontVert vertices[6];
        std::string symbolName;
    };

    class RenderStringStateData
    {
    public:
        RenderStringStateData();
        void init(fuiFontSet* font, float fontSize, float glyphScale);

        fuiFontSet* font;
        float fontSize;
        float lineStartX;
        float lineStartY;
        float x;
        float y;
        float maxX;
        float maxY;
        int lineNumber;
        int currentCharacter;
        float lineHeight;
        float firstLineHeight;
        fuiRGBA rgba;
        unsigned int colour;
        bool shadow;
        bool measure;
        int boundaryCharacter;
        float boundaryXMin;
        float boundaryXMax;
        float boundaryYMin;
        float boundaryYMax;
        int unused;
        std::vector<fuiFontSet::FontVert> bitmapGlyphVerts[4];
        std::vector<fuiFontSet::FontVert> shadowBitmapGlyphVerts[4];
        std::vector<fuiFontSet::FontVert> fontGlyphVerts[4];
        std::vector<fuiFontSet::FontVert> shadowFontGlyphVerts[4];
        std::vector<HtmlImageDrawData> imageDrawData;
        std::vector<HtmlImageDrawData> shadowImageDrawData;
        std::vector<fuiFontSet::FontVert> (*activeBitmapVerts)[4];
        std::vector<fuiFontSet::FontVert> (*activeFontVerts)[4];
        std::vector<HtmlImageDrawData>* activeImageData;
    };

    fuiRenderNodeEditText(fuiRenderNode*, fuiRenderNode*, fuiObject*, unsigned int, unsigned char, fuiRGBA*,
                          unsigned char, bool, fuiFile*);
    virtual ~fuiRenderNodeEditText();
    static const wchar_t* getHtmlTag(const wchar_t*, std::wstring*);
    static const wchar_t* parseHTMLString(const wchar_t*, std::wstring*);
    const wchar_t* prepStringPtr(const std::wstring&) const;
    const wchar_t* incStringPtr(const wchar_t*) const;
    float getGlyphScale();
    float getRenderedGlyphHeight(wchar_t);
    bool useBitmapFont(unsigned short, fuiFontSet*);
    float getRenderedGlyphWidth(wchar_t, RenderStringStateData&);
    float getWordWidth(const std::wstring&, RenderStringStateData&);
    void setHtmlAlignPosition(RenderStringStateData&);
    void newLine(RenderStringStateData&);
    bool isHtmlTagType(const std::wstring&, const std::wstring&);
    void htmlFontColour(const std::wstring&, RenderStringStateData&);
    std::wstring getHtmlTagAttribute(const std::wstring&, const std::wstring&);
    bool htmlImage(const std::wstring&, RenderStringStateData&);
    void checkForUnsupportedAttribute(const std::wstring&, const std::wstring&);
    bool htmlParagraph(const std::wstring&, RenderStringStateData&);
    bool renderHtmlTag(const std::wstring&, RenderStringStateData&);
    bool isCJKChar(wchar_t);
    bool isCJKPunctuationChar(wchar_t);
    bool endOfWord(wchar_t, wchar_t);
    const wchar_t* getNextWord(const wchar_t*, std::wstring*, RenderStringStateData&);
    void increaseCurrentCharacter(int, const wchar_t*, RenderStringStateData&);
    float renderGlyph(unsigned short, float, float, float, unsigned int, RenderStringStateData&);
    bool renderWord(std::wstring&, RenderStringStateData&);
    void renderString(const std::wstring&, RenderStringStateData&);
    void getRenderedStringWidthHeight(const std::wstring&, fuiFontSet*, float, float&, float&);
    bool clipBottom(const fuiFontSet::FontVert&);
    bool clipTop(const fuiFontSet::FontVert&);
    void DrawFontVerts(RenderStringStateData&, std::vector<fuiFontSet::FontVert>*);
    void DrawRenderData(RenderStringStateData&);
    void renderString(const std::wstring&, float, float, fuiFontSet*, float, unsigned int);
    void renderStringShadow(const std::wstring&, float, float, fuiFontSet*, float);
    void autoSizeBounds();
    virtual float getWidth();
    virtual float getHeight();
    virtual void gather(bool);
    virtual void preRender();
    virtual void render(fuiRect*, fuiMatrix*);
    const std::wstring& getText();
    void setText(const std::wstring&);
    void appendText(const std::wstring&);
    void setHTMLText(const std::wstring&);
    void setAutoSize(TextFieldAutoSize);
    fuiRect getCharBoundaries(int);
    float getTextHeight() const { return m_textHeight; }
    std::wstring createWordWrappedString(std::wstring, fuiFontSet*, float, float);
    virtual void setWidth(float);
    virtual void setHeight(float);
    virtual void setX(float);
    virtual void setY(float);
    virtual void getParentSpaceBounds(fuiRect*);
    virtual void setVisibility(bool);
    float getScrollH();
    void setScrollH(float);
    int getNumLines();

private:
    OSFastMutex m_lock;
    bool m_html;
    std::wstring m_text;
    float m_scrollH;
    unsigned int m_alignment;

public:
    unsigned char m_wordWrap;
    unsigned char m_multiline;

private:
    TextFieldAutoSize m_autoSize;
    int m_htmlAlignment;
    float m_textWidth;
    float m_textHeight;
    unsigned int m_numLines;
    std::vector<float> m_lineWidths;
    std::vector<float> m_lineHeights;
    fuiFontSet* m_font;
    fuiFontSet* m_boldFont;
    float m_glyphScale;
    unsigned int m_colour;
    bool m_textSet;
    float m_fieldWidth;
    RenderStringStateData m_renderState;
    std::wstring m_wrapBuffer;
    std::wstring m_wrappedText;
    std::wstring m_tag;
    std::wstring m_attribute;
    std::wstring m_word;
    bool m_dirty;
    int m_lastFrame;
};

#endif
