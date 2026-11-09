#include "game/ui/fuiRenderNodeEditText.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <cwctype>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/TextUtil.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiEdittext.hpp"
#include "game/ui/fuiFontName.hpp"

#ifdef MINECRAFT_PC
int PcButtonGlyphTexture(const char* name, int textureId);

static const char* PcImageCarrier(const char* name)
{
    return std::strncmp(name, "PcKey_", 6) == 0 ? "ActionBottom" : name;
}
#endif

extern Renderer* gRenderer;

static wchar_t cjkPunctuation[] = {0x3001, 0x3002, 0x3003, 0x3006, 0x3015, 0x3017, 0x301e};

unsigned int setRGBAOrder(unsigned int colour)
{
    fuiRGBA rgba;
    rgba.r = (colour >> 24) & 0xFF;
    rgba.g = (colour >> 16) & 0xFF;
    rgba.b = (colour >> 8) & 0xFF;
    rgba.a = colour & 0xFF;
    return rgba.rgba;
}

fuiRenderNodeEditText::RenderStringStateData::RenderStringStateData()
{
}

void fuiRenderNodeEditText::RenderStringStateData::init(fuiFontSet* fontSet, float size, float glyphScale)
{
    font = fontSet;
    fontSize = size;
    lineStartX = glyphScale;
    lineStartY = -glyphScale;
    x = glyphScale;
    y = -glyphScale;
    maxX = 0.0f;
    maxY = 0.0f;
    lineNumber = 0;
    currentCharacter = 0;
    lineHeight = 0.0f;
    firstLineHeight = 0.0f;
    rgba.r = 0;
    rgba.g = 0;
    rgba.b = 0;
    rgba.a = 0;
    colour = 0;
    shadow = false;
    measure = false;
    boundaryCharacter = -1;
    boundaryXMin = 0.0f;
    boundaryXMax = 0.0f;
    boundaryYMin = 0.0f;
    boundaryYMax = 0.0f;
}

fuiRenderNodeEditText::fuiRenderNodeEditText(fuiRenderNode* parent, fuiRenderNode* node, fuiObject* object,
                                             unsigned int symbolIndex, unsigned char type, fuiRGBA* colour,
                                             unsigned char renderFlags, bool visible, fuiFile* file)
    : fuiRenderNode(parent, node, object, symbolIndex, eFuiObjectType_EditText, type, colour, renderFlags, visible,
                    file)
{
    InitializeCriticalSection(&m_lock);

    fuiEdittext* editText = static_cast<fuiEdittext*>(object);
    m_bounds.xMin = editText->bounds.xMin;
    m_bounds.xMax = editText->bounds.xMax;
    m_bounds.yMin = editText->bounds.yMin;
    m_fieldWidth = editText->bounds.xMax - editText->bounds.xMin;
    m_bounds.yMin = 0.0f;
    m_bounds.yMax = editText->bounds.yMax - editText->bounds.yMin;

    m_glyphScale = editText->fontScale != 0.0f ? editText->fontScale : 1.0f;

    if (editText->align < 4)
    {
        static const unsigned char alignments[4] = {0, 1, 2, 3};
        m_alignment = alignments[editText->align];
    }

    if (editText->text[0] != '\0')
    {
        wchar_t wide[1024];
        std::mbstowcs(wide, editText->text, 1024);
        const wchar_t* cursor = parseHTMLString(wide, NULL);
        while (*cursor != L'\0')
        {
            m_text.insert(m_text.end(), *cursor);
            cursor = parseHTMLString(cursor + 1, NULL);
        }
    }

    m_font = NULL;
    unsigned int fontNameCount = file->header.fontNameCount;
    if (fontNameCount != 0)
    {
        unsigned int index = 0;
        do
        {
            fuiFontName& fontName = file->fontNameData[index];
            if (editText->fontId == fontName.fontId)
            {
                m_font = reinterpret_cast<fuiFontSet*>(gFui->getFont(fontName.name, false));
                m_boldFont = reinterpret_cast<fuiFontSet*>(gFui->getFont(fontName.name, true));
                if (m_font == NULL)
                {
                    m_font = reinterpret_cast<fuiFontSet*>(gFui->getDefaultFont(false));
                    m_boldFont = reinterpret_cast<fuiFontSet*>(gFui->getDefaultFont(true));
                }
                fontNameCount = file->header.fontNameCount;
            }
            ++index;
        } while (index < fontNameCount);
    }
    if (m_font == NULL)
        m_font = reinterpret_cast<fuiFontSet*>(gFui->getDefaultFont(false));

    m_html = false;
    m_scrollH = 0.0f;
    m_colour = editText->textColor.rgba;
    m_wordWrap = editText->wordWrap;
    m_dirty = true;
    m_textSet = false;
    m_scrollRect.xMin = 0.0f;
    m_scrollRect.xMax = 0.0f;
    m_autoSize = TextFieldAutoSizeCenter;
    m_scrollRect.yMin = 0.0f;
    m_multiline = editText->multiline;
    m_htmlAlignment = 0;
    m_scrollRect.yMax = 0.0f;
    m_textWidth = getWidth();
    m_textHeight = getHeight();
}

fuiRenderNodeEditText::~fuiRenderNodeEditText()
{
    DeleteCriticalSection(&m_lock);
}

const wchar_t* fuiRenderNodeEditText::getHtmlTag(const wchar_t* text, std::wstring* tag)
{
    if (*text == L'<')
    {
        std::wstring content;
        const wchar_t* cursor = text;
        for (;;)
        {
            ++cursor;
            wchar_t c = *cursor;
            if (c == L'\0')
                break;
            if (c == L'>')
            {
                ++cursor;
                break;
            }
            content += c;
        }
        if (tag != NULL)
            *tag = content;
        while (*cursor == L' ')
            ++cursor;
        return cursor;
    }
    if (tag != NULL)
        *tag = L"";
    return text;
}

const wchar_t* fuiRenderNodeEditText::parseHTMLString(const wchar_t* text, std::wstring* result)
{
    if (result == NULL)
    {
        while (*text == L'<')
            text = getHtmlTag(text, NULL);
    }
    else if (*text == L'<')
    {
        text = getHtmlTag(text, result);
    }
    else
    {
        *result = L"";
    }
    return text;
}

const wchar_t* fuiRenderNodeEditText::prepStringPtr(const std::wstring& text) const
{
    if (m_html)
        return parseHTMLString(text.c_str(), NULL);
    return text.c_str();
}

const wchar_t* fuiRenderNodeEditText::incStringPtr(const wchar_t* text) const
{
    const wchar_t* cursor = text + 1;
    if (m_html)
        return parseHTMLString(cursor, NULL);
    return cursor;
}

float fuiRenderNodeEditText::getGlyphScale()
{
    return m_boldFont->getGlyphScale(m_glyphScale);
}

float fuiRenderNodeEditText::getRenderedGlyphHeight(wchar_t character)
{
    return m_boldFont->getRenderedGlyphHeight(static_cast<unsigned short>(character), m_glyphScale);
}

bool fuiRenderNodeEditText::useBitmapFont(unsigned short character, fuiFontSet* font)
{
    bool result = true;
    if (!m_boldFont->hasGlyph(character) && font->hasGlyph(character))
        result = false;
    return result;
}

float fuiRenderNodeEditText::getRenderedGlyphWidth(wchar_t character, RenderStringStateData& state)
{
    if (useBitmapFont(static_cast<unsigned short>(character), state.font))
        return m_boldFont->getRenderedGlyphWidth(static_cast<unsigned short>(character), state.fontSize);
    return state.font->getRenderedGlyphWidth(static_cast<unsigned short>(character), state.fontSize);
}

float fuiRenderNodeEditText::getWordWidth(const std::wstring& word, RenderStringStateData& state)
{
    float width = 0.0f;
    std::wstring::const_iterator it;
    for (it = word.begin(); it != word.end(); ++it)
        width += getRenderedGlyphWidth(*it, state);
    return width;
}

void fuiRenderNodeEditText::setHtmlAlignPosition(RenderStringStateData& state)
{
    float lineWidth = m_lineWidths[static_cast<std::vector<float>::size_type>(state.lineNumber)];
    unsigned int alignment = m_alignment;
    if (m_html && m_htmlAlignment != 0 && m_htmlAlignment < 4)
    {
        static const unsigned char htmlAlignments[4] = {0, 0, 2, 1};
        alignment = htmlAlignments[m_htmlAlignment];
    }
    if (alignment == 0)
    {
        state.x = state.lineStartX;
        return;
    }
    if (alignment == 1)
    {
        state.x = state.lineStartX + (m_fieldWidth - lineWidth);
        return;
    }
    if (alignment != 2)
        return;
    state.x = (m_fieldWidth - lineWidth) * 0.5f + state.lineStartX;
}

void fuiRenderNodeEditText::newLine(RenderStringStateData& state)
{
    float lineEndX = state.x;
    if (state.maxX < lineEndX)
        state.maxX = lineEndX;
    ++state.lineNumber;
    if (!state.measure)
    {
        setHtmlAlignPosition(state);
    }
    else
    {
        float lineWidth = lineEndX - state.lineStartX;
        state.x = state.lineStartX;
        m_lineWidths.push_back(lineWidth);
        m_lineHeights.push_back(state.lineHeight);
    }
    state.y += state.lineHeight;
    if (state.y > state.maxY)
        state.maxY = state.y;
    state.lineHeight = getRenderedGlyphHeight(m_text.c_str()[0]);
}

bool fuiRenderNodeEditText::isHtmlTagType(const std::wstring& tag, const std::wstring& type)
{
    for (std::wstring::size_type i = 0; i < type.size(); ++i)
    {
        wchar_t c = tag[i];
        if (c + ((c >= L'A' && c <= L'Z') ? (L'a' - L'A') : 0) != type[i])
            return false;
    }
    return true;
}

void fuiRenderNodeEditText::htmlFontColour(const std::wstring& tag, RenderStringStateData& state)
{
    static const std::wstring prefix = L"font color=\"#";
    std::wstring hex = tag.substr(prefix.size(), 8);
    unsigned long rgb = std::wcstoul(hex.c_str(), NULL, 16);
    state.colour = (static_cast<unsigned int>(rgb) << 8) | (state.rgba.rgba & 0xffu);
}

std::wstring fuiRenderNodeEditText::getHtmlTagAttribute(const std::wstring& tag, const std::wstring& attribute)
{
    std::wstring lowerTag = tag;
    std::transform(lowerTag.begin(), lowerTag.end(), lowerTag.begin(), tolower);
    std::wstring key = attribute + L"=";
    std::wstring::size_type position = lowerTag.find(key);
    std::wstring value;
    if (position != std::wstring::npos)
    {
        position += key.size();
        if (tag[position] != L'"')
        {
            while (tag[position] != L'\0' && tag[position] != L'>')
            {
                value += tag[position];
                ++position;
            }
        }
        else
        {
            ++position;
            while (tag[position] != L'\0' && tag[position] != L'"')
            {
                value += tag[position];
                ++position;
            }
        }
    }
    return value;
}

bool fuiRenderNodeEditText::htmlImage(const std::wstring& tag, RenderStringStateData& state)
{
    std::wstring widthAttribute = getHtmlTagAttribute(tag, L"width");
    std::wstring heightAttribute = getHtmlTagAttribute(tag, L"height");
    std::wstring srcAttribute = getHtmlTagAttribute(tag, L"src");
    float width = widthAttribute.empty() ? 0.0f : static_cast<float>(std::wcstol(widthAttribute.c_str(), NULL, 10));
    float height = heightAttribute.empty() ? 0.0f : static_cast<float>(std::wcstol(heightAttribute.c_str(), NULL, 10));
    char symbolName[128];
    std::wcstombs(symbolName, srcAttribute.c_str(), sizeof(symbolName) - 1);
    symbolName[sizeof(symbolName) - 1] = '\0';
    fuiFile* file = NULL;
#ifdef MINECRAFT_PC
    intptr_t bitmap = gFui->findBitmapFromSymbol(PcImageCarrier(symbolName), &file);
#else
    intptr_t bitmap = gFui->findBitmapFromSymbol(symbolName, &file);
#endif
    if (bitmap != 0)
    {
        fuiBitmap* bitmapObject = reinterpret_cast<fuiBitmap*>(bitmap);
        if (width == 0.0f)
        {
            width = static_cast<float>(bitmapObject->width) * height / static_cast<float>(bitmapObject->height);
        }
        if (height == 0.0f)
        {
            height = static_cast<float>(bitmapObject->height) * width / static_cast<float>(bitmapObject->width);
        }
        state.x += 8.0f;
        float left = state.x;
        if (m_wordWrap && m_fieldWidth < (left - state.lineStartX) + width)
        {
            newLine(state);
            left = state.x;
        }
        if (!state.measure && !state.shadow)
        {
            float top = (m_lineHeights[static_cast<std::vector<float>::size_type>(state.lineNumber)] - height) * 0.5f
                        + state.y;
            float right = left + width;
            float bottom = top + height;
            file->initialiseRenderTexture(bitmapObject, false);
            HtmlImageDrawData image;
            image.symbolName.assign(symbolName);
            image.vertices[0].x = left;
            image.vertices[0].y = top;
            image.vertices[0].u = 0.0f;
            image.vertices[0].v = 0.0f;
            image.vertices[0].colour = 0xffffffff;
            image.vertices[1].x = right;
            image.vertices[1].y = bottom;
            image.vertices[1].u = 1.0f;
            image.vertices[1].v = 1.0f;
            image.vertices[1].colour = 0xffffffff;
            image.vertices[2].x = left;
            image.vertices[2].y = bottom;
            image.vertices[2].u = 0.0f;
            image.vertices[2].v = 1.0f;
            image.vertices[2].colour = 0xffffffff;
            image.vertices[3].x = right;
            image.vertices[3].y = top;
            image.vertices[3].u = 1.0f;
            image.vertices[3].v = 0.0f;
            image.vertices[3].colour = 0xffffffff;
            image.vertices[4].x = left;
            image.vertices[4].y = top;
            image.vertices[4].u = 0.0f;
            image.vertices[4].v = 0.0f;
            image.vertices[4].colour = 0xffffffff;
            image.vertices[5].x = right;
            image.vertices[5].y = bottom;
            image.vertices[5].u = 1.0f;
            image.vertices[5].v = 1.0f;
            image.vertices[5].colour = 0xffffffff;
            state.activeImageData->push_back(image);
        }
        state.x = left + width + 8.0f;
        if (state.lineHeight < height + 16.0f)
            state.lineHeight = height + 16.0f;
    }
    return true;
}

void fuiRenderNodeEditText::checkForUnsupportedAttribute(const std::wstring& tag, const std::wstring& attribute)
{
    getHtmlTagAttribute(tag, attribute);
}

bool fuiRenderNodeEditText::htmlParagraph(const std::wstring& tag, RenderStringStateData&)
{
    std::wstring align = getHtmlTagAttribute(tag, L"align");
    checkForUnsupportedAttribute(tag, L"class");
    checkForUnsupportedAttribute(tag, L"id");
    checkForUnsupportedAttribute(tag, L"style");
    checkForUnsupportedAttribute(tag, L"title");

    if (!align.empty())
    {
        for (std::wstring::size_type i = 0; i < align.size(); ++i)
        {
            wchar_t c = align[i];
            if (c >= L'A' && c <= L'Z')
                align[i] = static_cast<wchar_t>(c + (L'a' - L'A'));
        }
        if (align == L"left")
            m_htmlAlignment = 1;
        else if (align == L"right")
            m_htmlAlignment = 3;
        else if (align == L"center")
            m_htmlAlignment = 2;
    }
    return true;
}

bool fuiRenderNodeEditText::renderHtmlTag(const std::wstring& tag, RenderStringStateData& state)
{
    if (tag == L"br" || tag == L"BR" || tag == L"br /" || tag == L"BR /")
    {
        newLine(state);
        return true;
    }
    if (isHtmlTagType(tag, L"font color"))
    {
        htmlFontColour(tag, state);
        return true;
    }
    if (isHtmlTagType(tag, L"img"))
    {
        htmlImage(tag, state);
        return true;
    }
#ifdef MINECRAFT_PC
    if (isHtmlTagType(tag, L"/font"))
    {
        state.colour = state.rgba.rgba;
        return true;
    }
#endif
    if (!isHtmlTagType(tag, L"/font"))
    {
        if (isHtmlTagType(tag, L"p"))
        {
            htmlParagraph(tag, state);
            return true;
        }
        if (!isHtmlTagType(tag, L"/p"))
        {
            if (!isHtmlTagType(tag, L"i"))
                isHtmlTagType(tag, L"/i");
        }
    }
    return true;
}

bool fuiRenderNodeEditText::isCJKChar(wchar_t character)
{
    return character - 0x2e80U < 0x7180;
}

bool fuiRenderNodeEditText::isCJKPunctuationChar(wchar_t character)
{
    if (character == cjkPunctuation[0] || character == cjkPunctuation[1] || character == cjkPunctuation[2]
        || character == cjkPunctuation[3] || character == cjkPunctuation[4] || character == cjkPunctuation[5]
        || character == cjkPunctuation[6])
        return true;
    return false;
}

bool fuiRenderNodeEditText::endOfWord(wchar_t previous, wchar_t current)
{
    bool result = false;
    if (previous == 0 || previous == L' ' || previous == L'\t' || isNewLineChar(previous)
        || (m_html && previous == L'<'))
        result = true;
    if (current == 0 || current == L' ' || current == L'\t' || isNewLineChar(current) || (m_html && current == L'<')
        || (isCJKChar(current) && !isCJKPunctuationChar(current)))
        result = true;
    return result;
}

const wchar_t* fuiRenderNodeEditText::getNextWord(const wchar_t* text, std::wstring* word, RenderStringStateData& state)
{
    wchar_t next = *text != L'\0' ? text[1] : L'\0';
    float width = 0.0f;
    m_word.clear();

    const wchar_t* cursor = text;
    wchar_t current = *cursor;
    bool done;
    for (;;)
    {
        if (current == 0xa0)
        {
            m_word.push_back(L' ');
            cursor += 1;
            current = *cursor;
            done = endOfWord(0x41, current);
        }
        else if (std::wcsncmp(cursor, L"&nbsp;", 6) == 0)
        {
            m_word.push_back(L' ');
            cursor += 6;
            current = *cursor;
            done = endOfWord(0x41, current);
        }
        else if (std::wcsncmp(cursor, L"&gt;", 4) == 0)
        {
            m_word.push_back(L'>');
            current = cursor[4];
            done = endOfWord(L'>', current);
            cursor += 4;
        }
        else if (std::wcsncmp(cursor, L"&lt;", 4) != 0)
        {
            if (std::wcsncmp(cursor, L"&amp;", 5) == 0)
            {
                m_word.push_back(L'&');
                cursor += 5;
                current = *cursor;
                done = endOfWord(L'&', current);
            }
            else
            {
                current = *cursor;
                done = endOfWord(current, next);
                m_word.push_back(current);
                width += getRenderedGlyphWidth(*cursor, state);
                float lookaheadWidth = 0.0f;
                if (next != L'\0')
                    lookaheadWidth = getRenderedGlyphWidth(next, state);
                cursor += 1;
                current = *cursor;
                if (m_fieldWidth < width + lookaheadWidth)
                    done = true;
            }
        }
        else
        {
            m_word.push_back(L'<');
            current = cursor[4];
            done = endOfWord(L'<', current);
            cursor += 4;
        }

        next = current != L'\0' ? cursor[1] : L'\0';
        if (done)
        {
            *word = m_word;
            return cursor;
        }
    }
}

void fuiRenderNodeEditText::increaseCurrentCharacter(int count, const wchar_t* text, RenderStringStateData& state)
{
    if (count == 0)
        return;
    do
    {
        if (state.currentCharacter == state.boundaryCharacter)
        {
            float glyphWidth = getRenderedGlyphWidth(*text, state);
            float glyphHeight = getRenderedGlyphHeight(*text);
            state.boundaryYMin = state.y;
            state.boundaryXMin = state.x;
            state.boundaryXMax = state.x + glyphWidth;
            state.boundaryYMax = state.y + glyphHeight;
        }
        --count;
        ++state.currentCharacter;
        ++text;
    } while (count != 0);
}

float fuiRenderNodeEditText::renderGlyph(unsigned short character, float, float y, float, unsigned int colour,
                                         RenderStringStateData& state)
{
    if (useBitmapFont(character, state.font))
        return m_boldFont->renderGlyph(character, state.x, y, state.fontSize, colour, *state.activeBitmapVerts);
    fuiFontSet* font = state.font;
    return font->renderGlyph(character, state.x, getGlyphScale() * 3.0f + y, state.fontSize, colour,
                             *state.activeFontVerts);
}

bool fuiRenderNodeEditText::renderWord(std::wstring& word, RenderStringStateData& state)
{
    const wchar_t* cursor = word.c_str();
    wchar_t character = *cursor;
    while (character != L'\0')
    {
        if (character == L'\t')
            character = L' ';
        if (!state.measure)
        {
            unsigned int colour = state.colour;
            if (state.shadow)
                colour = setRGBAOrder(m_color);
            float lineHeight = m_lineHeights[static_cast<std::vector<float>::size_type>(state.lineNumber)];
            float glyphHeight = getRenderedGlyphHeight(character);
            float advance = renderGlyph(static_cast<unsigned short>(character), state.x,
                                        (lineHeight - glyphHeight) * 0.5f + state.y, state.fontSize, colour, state);
            increaseCurrentCharacter(1, cursor, state);
            state.x += advance;
        }
        else
        {
            float advance = getRenderedGlyphWidth(character, state);
            increaseCurrentCharacter(1, cursor, state);
            state.x += advance;
        }
        ++cursor;
        character = *cursor;
    }
    return true;
}

void fuiRenderNodeEditText::renderString(const std::wstring& text, RenderStringStateData& state)
{
    m_tag = L"";
    m_attribute = L"";
    m_word = L"";

    const wchar_t* cursor = text.c_str();
    if (!state.measure)
        setHtmlAlignPosition(state);

    while (*cursor != L'\0')
    {
        if (m_html)
            cursor = getHtmlTag(cursor, &m_tag);
        if (m_tag.size() != 0)
        {
            renderHtmlTag(m_tag, state);
            continue;
        }
        if (!state.measure && state.x == state.lineStartX)
            setHtmlAlignPosition(state);

        cursor = getNextWord(cursor, &m_word, state);
        if (isNewLineChar(m_word.c_str()[0]))
        {
            if (m_multiline || m_wordWrap || m_html)
            {
                increaseCurrentCharacter(static_cast<int>(m_word.size()), m_word.c_str(), state);
                newLine(state);
            }
            continue;
        }

        float wordWidth = getWordWidth(m_word, state);
        if (!m_wordWrap || (state.x - state.lineStartX) + wordWidth <= m_fieldWidth)
        {
            renderWord(m_word, state);
            continue;
        }

        newLine(state);
        if (m_word.c_str()[0] != L' ')
            renderWord(m_word, state);
        else
            increaseCurrentCharacter(static_cast<int>(m_word.size()), m_word.c_str(), state);
    }
}

void fuiRenderNodeEditText::getRenderedStringWidthHeight(const std::wstring& text, fuiFontSet* font, float size,
                                                         float& width, float& height)
{
    EnterCriticalSection(&m_lock);
    m_renderState.init(font, size, getGlyphScale());
    m_renderState.font = font;
    m_renderState.fontSize = size;

    float glyphHeight = getRenderedGlyphHeight(text.c_str()[0]);
    m_renderState.firstLineHeight = glyphHeight;
    m_renderState.measure = true;
    m_renderState.lineStartX = 0.0f;
    m_renderState.lineStartY = 0.0f;
    m_renderState.y = 0.0f;
    m_renderState.lineHeight = glyphHeight;
    m_lineWidths.clear();
    m_lineHeights.clear();

    renderString(text, m_renderState);
    newLine(m_renderState);

    width = m_renderState.maxX;
    height = m_renderState.maxY;
    m_numLines = static_cast<unsigned int>(m_lineWidths.size());
    LeaveCriticalSection(&m_lock);
}

bool fuiRenderNodeEditText::clipBottom(const fuiFontSet::FontVert& vertex)
{
    const fuiMatrix& matrix = gFui->m_matrixStack[gFui->m_matrixDepth];
    const float y = vertex.x * matrix.rotateSkew0 + vertex.y * matrix.scaleY + matrix.translateY;
    return y > gFui->m_screenHeight;
}

bool fuiRenderNodeEditText::clipTop(const fuiFontSet::FontVert& vertex)
{
    const fuiMatrix& matrix = gFui->m_matrixStack[gFui->m_matrixDepth];
    const float y = vertex.x * matrix.rotateSkew0 + vertex.y * matrix.scaleY + matrix.translateY;
    return y < 0.0f;
}

void fuiRenderNodeEditText::DrawFontVerts(RenderStringStateData&, std::vector<fuiFontSet::FontVert>* vertices)
{
    unsigned int count = 0;
    if (!vertices->empty())
        count = static_cast<unsigned int>(vertices->size());

    int drawn = 0;
    unsigned int remaining = count >> 2;
    while (remaining != 0)
    {
        unsigned int batch = remaining;
        if (0x200 < static_cast<int>(remaining))
            batch = 0x200;

        int start = drawn * 4;
        bool clipped = clipBottom(vertices->at(static_cast<std::vector<fuiFontSet::FontVert>::size_type>(start)));
        drawn += static_cast<int>(batch);
        if (!clipped
            && !clipTop(vertices->at(static_cast<std::vector<fuiFontSet::FontVert>::size_type>(drawn * 4 - 1))))
        {
            gRenderer->DrawVertices(C4JRender::ePrimitiveType_Quads, static_cast<int>(batch * 4),
                                    &vertices->at(static_cast<std::vector<fuiFontSet::FontVert>::size_type>(start)),
                                    C4JRender::eVertexType_ColouredUI, C4JRender::ePixelShaderType_UI, NULL);
        }
        remaining -= batch;
    }
}

void fuiRenderNodeEditText::DrawRenderData(RenderStringStateData& state)
{
    if (!state.shadow)
    {
        gRenderer->StateSetAlphaFunc(4, 0.003921569f);
        gRenderer->StateSetAlphaTestEnable(true);
        if ((gFui->m_filterStack[gFui->m_filterDepth] & 2) != 0)
        {
            gFui->incrementStencilMask();
            gRenderer->StateSetStencil(7, gFui->getCurrentStencilMask(), 0, 0xff);
        }
    }
    else
    {
        gRenderer->StateSetStencil(5, gFui->getCurrentStencilMask(), 0xff, 0);
    }

    if (!(*state.activeBitmapVerts)[0].empty())
    {
        gRenderer->TextureBind(m_boldFont->getTextureIndex(m_glyphScale, 0));
        DrawFontVerts(state, &(*state.activeBitmapVerts)[0]);
    }
    if (!(*state.activeFontVerts)[0].empty())
    {
        gRenderer->TextureBind(m_font->getTextureIndex(m_glyphScale, 0));
        DrawFontVerts(state, &(*state.activeFontVerts)[0]);
    }
    if (!(*state.activeFontVerts)[1].empty())
    {
        gRenderer->TextureBind(m_font->getTextureIndex(m_glyphScale, 1));
        DrawFontVerts(state, &(*state.activeFontVerts)[1]);
    }
    if (!(*state.activeFontVerts)[2].empty())
    {
        gRenderer->TextureBind(m_font->getTextureIndex(m_glyphScale, 2));
        DrawFontVerts(state, &(*state.activeFontVerts)[2]);
    }
    if (!(*state.activeFontVerts)[3].empty())
    {
        gRenderer->TextureBind(m_font->getTextureIndex(m_glyphScale, 3));
        DrawFontVerts(state, &(*state.activeFontVerts)[3]);
    }

    gRenderer->StateSetStencil(7, 0, 0, 0);
    gRenderer->StateSetDepthTestEnable(false);
    gRenderer->StateSetDepthMask(false);
    gRenderer->StateSetDepthFunc(3);
    gRenderer->StateSetAlphaTestEnable(false);

    for (unsigned int index = 0; index < state.activeImageData->size(); ++index)
    {
        HtmlImageDrawData& image = state.activeImageData->at(index);
        fuiFile* file = NULL;
#ifdef MINECRAFT_PC
        intptr_t bitmap = gFui->findBitmapFromSymbol(PcImageCarrier(image.symbolName.c_str()), &file);
#else
        intptr_t bitmap = gFui->findBitmapFromSymbol(image.symbolName.c_str(), &file);
#endif
        if (bitmap != 0)
        {
            fuiBitmap* bitmapObject = reinterpret_cast<fuiBitmap*>(bitmap);
            file->initialiseRenderTexture(bitmapObject, false);
#ifdef MINECRAFT_PC
            gRenderer->TextureBind(PcButtonGlyphTexture(image.symbolName.c_str(), bitmapObject->textureId));
#else
            gRenderer->TextureBind(bitmapObject->textureId);
#endif
            gRenderer->TextureSetParam(1, 1);
            gRenderer->TextureSetParam(2, 1);
            gRenderer->DrawVertices(C4JRender::ePrimitiveType_Triangles, 6, &image, C4JRender::eVertexType_ColouredUI,
                                    C4JRender::ePixelShaderType_UIColourAdd, NULL);
        }
    }
}

void fuiRenderNodeEditText::renderString(const std::wstring& text, float x, float y, fuiFontSet* font, float size,
                                         unsigned int colour)
{
    EnterCriticalSection(&m_lock);
    if (!m_dirty)
    {
        m_renderState.shadow = false;
        m_renderState.activeFontVerts = &m_renderState.fontGlyphVerts;
        m_renderState.activeImageData = &m_renderState.imageDrawData;
        m_renderState.activeBitmapVerts = &m_renderState.bitmapGlyphVerts;
        DrawRenderData(m_renderState);
        LeaveCriticalSection(&m_lock);
        return;
    }

    m_renderState.init(font, size, getGlyphScale());
    m_renderState.font = font;
    m_renderState.fontSize = size;

    float glyphHeight = getRenderedGlyphHeight(text.c_str()[0]);
    m_renderState.lineHeight = glyphHeight;
    m_renderState.firstLineHeight = glyphHeight;
    m_renderState.lineStartX = 1.0f * getGlyphScale() + x;
    float top = -1.0f * getGlyphScale() + y;
    m_renderState.maxY = y;
    m_renderState.rgba.r = (colour >> 24) & 0xFF;
    m_renderState.rgba.g = (colour >> 16) & 0xFF;
    m_renderState.rgba.b = (colour >> 8) & 0xFF;
    m_renderState.rgba.a = colour & 0xFF;
    m_renderState.measure = false;
    m_renderState.maxX = x;
    m_renderState.lineStartY = top;
    m_renderState.y = top;
    m_renderState.colour = m_renderState.rgba.rgba;
    m_renderState.shadow = false;
    m_renderState.x = m_renderState.lineStartX;

    m_renderState.bitmapGlyphVerts[0].clear();
    m_renderState.fontGlyphVerts[0].clear();
    m_renderState.bitmapGlyphVerts[1].clear();
    m_renderState.fontGlyphVerts[1].clear();
    m_renderState.bitmapGlyphVerts[2].clear();
    m_renderState.fontGlyphVerts[2].clear();
    m_renderState.bitmapGlyphVerts[3].clear();
    m_renderState.fontGlyphVerts[3].clear();
    m_renderState.imageDrawData.clear();

    m_renderState.activeImageData = &m_renderState.imageDrawData;
    m_renderState.activeFontVerts = &m_renderState.fontGlyphVerts;
    m_renderState.boundaryCharacter = -1;
    m_renderState.activeBitmapVerts = &m_renderState.bitmapGlyphVerts;

    renderString(text, m_renderState);
    DrawRenderData(m_renderState);
    LeaveCriticalSection(&m_lock);
}

void fuiRenderNodeEditText::renderStringShadow(const std::wstring& text, float x, float y, fuiFontSet* font, float size)
{
    EnterCriticalSection(&m_lock);
    if (!m_dirty)
    {
        m_renderState.activeFontVerts = &m_renderState.shadowFontGlyphVerts;
        m_renderState.activeBitmapVerts = &m_renderState.shadowBitmapGlyphVerts;
        m_renderState.activeImageData = &m_renderState.shadowImageDrawData;
        m_renderState.shadow = true;
        DrawRenderData(m_renderState);
        LeaveCriticalSection(&m_lock);
        return;
    }

    m_renderState.init(font, size, getGlyphScale());
    m_renderState.font = font;
    m_renderState.fontSize = size;

    float glyphHeight = getRenderedGlyphHeight(text.c_str()[0]);
    m_renderState.firstLineHeight = glyphHeight;
    m_renderState.lineHeight = glyphHeight;
    m_renderState.lineStartX = 1.0f * getGlyphScale() + x;
    float top = -1.0f * getGlyphScale() + y;
    m_renderState.maxY = y;
    m_renderState.measure = false;
    m_renderState.shadow = true;
    m_renderState.lineStartY = top;
    m_renderState.y = top;
    m_renderState.x = m_renderState.lineStartX;
    m_renderState.maxX = x;

    m_renderState.shadowBitmapGlyphVerts[0].clear();
    m_renderState.shadowFontGlyphVerts[0].clear();
    m_renderState.shadowBitmapGlyphVerts[1].clear();
    m_renderState.shadowFontGlyphVerts[1].clear();
    m_renderState.shadowBitmapGlyphVerts[2].clear();
    m_renderState.shadowFontGlyphVerts[2].clear();
    m_renderState.shadowBitmapGlyphVerts[3].clear();
    m_renderState.shadowFontGlyphVerts[3].clear();
    m_renderState.shadowImageDrawData.clear();

    m_renderState.activeBitmapVerts = &m_renderState.shadowBitmapGlyphVerts;
    m_renderState.activeImageData = &m_renderState.shadowImageDrawData;
    m_renderState.activeFontVerts = &m_renderState.shadowFontGlyphVerts;

    renderString(text, m_renderState);
    DrawRenderData(m_renderState);
    LeaveCriticalSection(&m_lock);
}

void fuiRenderNodeEditText::autoSizeBounds()
{
    float width = 0.0f;
    float height = 0.0f;
    getRenderedStringWidthHeight(m_text, m_font, static_cast<fuiEdittext*>(m_object)->fontScale, width, height);

    float left = -(1.0f * getGlyphScale() - width);
    if (m_textWidth != left || m_textHeight != height)
    {
        m_textWidth = left;
        m_textHeight = height;
        m_dirty = true;
    }

    if (m_autoSize == TextFieldAutoSizeLeft)
    {
        m_bounds.xMax = m_bounds.xMin + (1.0f * getGlyphScale() + width);
        m_bounds.yMax = height * 1.0436591f + m_bounds.yMin;
        progogateBounds();
    }
    else if (m_autoSize == TextFieldAutoSizeRight)
    {
        m_bounds.yMax = height * 1.0436591f + m_bounds.yMin;
        m_alignment = 1;
        progogateBounds();
    }
}

float fuiRenderNodeEditText::getWidth()
{
    autoSizeBounds();
    return fuiRenderNode::getWidth();
}

float fuiRenderNodeEditText::getHeight()
{
    autoSizeBounds();
    return fuiRenderNode::getHeight();
}

void fuiRenderNodeEditText::gather(bool)
{
}

void fuiRenderNodeEditText::preRender()
{
    unsigned int frame = gFui->GetCurrentFrameCount();
    if (frame != static_cast<unsigned int>(m_lastFrame + 1))
        m_dirty = true;
    m_lastFrame = static_cast<int>(frame);

    if (isVisible() && m_text.size() != 0)
    {
        fuiEdittext* editText = static_cast<fuiEdittext*>(m_object);
        const wchar_t* cursor = prepStringPtr(m_text);
        while (*cursor != L'\0')
        {
            if (!useBitmapFont(static_cast<unsigned short>(*cursor), m_font))
                m_font->loadGlyph(*cursor, editText->fontScale);
            cursor = incStringPtr(cursor);
        }
    }
}

void fuiRenderNodeEditText::render(fuiRect*, fuiMatrix*)
{
    if (!isVisible())
        return;
    if (m_text.size() == 0)
        return;

    pushMatrix();
    pushColor();
    pushFilterFlags();
    setScrollRectViewport();

    const fui::ColorState& colourState = gFui->m_colorStack[gFui->m_colorDepth];
    gRenderer->StateSetBlendFunc(1, 5);
    gRenderer->StateSetColour(colourState.redMultiply, colourState.greenMultiply, colourState.blueMultiply,
                              colourState.alphaMultiply);
    gRenderer->StateSetColourAdd(0.0f, 0.0f, 0.0f, 0.0f);

    float left = m_bounds.xMin - m_scrollH;
    float top = m_bounds.yMin;
    renderString(m_text, left, top, m_font, m_glyphScale, m_colour);
    if ((gFui->m_filterStack[gFui->m_filterDepth] & 2) != 0)
    {
        float offset = static_cast<float>(m_visible);
        renderStringShadow(m_text, left + offset, top + offset, m_font, m_glyphScale);
    }
    m_dirty = false;

    clearScrollRectViewport();
    popFilterFlags();
    popColor();
    popMatrix();
}

const std::wstring& fuiRenderNodeEditText::getText()
{
    return m_text;
}

void fuiRenderNodeEditText::setText(const std::wstring& text)
{
    m_html = false;
    if (m_text.compare(text) != 0)
    {
        m_text = text;
        m_dirty = true;
    }
    m_textSet = true;
    autoSizeBounds();
}

void fuiRenderNodeEditText::appendText(const std::wstring& text)
{
    m_text += text;
    m_dirty = true;
    autoSizeBounds();
}

void fuiRenderNodeEditText::setHTMLText(const std::wstring& text)
{
    m_html = true;
    m_textSet = true;
    if (m_text.compare(text) != 0)
    {
        m_text = text;
        m_dirty = true;
        autoSizeBounds();
    }
}

void fuiRenderNodeEditText::setAutoSize(TextFieldAutoSize autoSize)
{
    if (m_autoSize != autoSize)
    {
        if (m_textSet)
        {
            autoSizeBounds();
            m_dirty = true;
        }
        m_autoSize = autoSize;
    }
}

fuiRect fuiRenderNodeEditText::getCharBoundaries(int index)
{
    EnterCriticalSection(&m_lock);
    if (m_text.size() == 0)
    {
        LeaveCriticalSection(&m_lock);
        fuiRect emptyRect = {0.0f, 0.0f, 0.0f, 0.0f};
        return emptyRect;
    }

    fuiEdittext* editText = static_cast<fuiEdittext*>(m_object);
    float size = editText->fontScale != 0.0f ? editText->fontScale : 1.0f;
    m_renderState.init(m_font, size, getGlyphScale());
    m_renderState.font = m_font;
    m_renderState.fontSize = size;

    float glyphHeight = getRenderedGlyphHeight(m_text[static_cast<std::wstring::size_type>(index)]);
    m_renderState.firstLineHeight = glyphHeight;
    m_renderState.lineHeight = glyphHeight;
    m_renderState.measure = true;
    m_renderState.boundaryCharacter = index;
    m_lineWidths.clear();
    m_lineHeights.clear();

    renderString(m_text, m_renderState);
    newLine(m_renderState);

    fuiRect result;
    result.xMin = m_renderState.boundaryXMin;
    result.xMax = m_renderState.boundaryXMax;
    result.yMin = m_renderState.boundaryYMin;
    result.yMax = m_renderState.boundaryYMax;
    m_numLines = static_cast<unsigned int>(m_lineWidths.size());
    LeaveCriticalSection(&m_lock);
    return result;
}

std::wstring fuiRenderNodeEditText::createWordWrappedString(std::wstring text, fuiFontSet* font, float size,
                                                            float width)
{
    std::wstring wrapped;
    float glyphWidth = 0.0f;
    float lineWidth = 0.0f;
    const wchar_t* cursor = parseHTMLString(text.c_str(), NULL);
    while (*cursor == L'\0')
    {
        lineWidth = lineWidth + glyphWidth;
        if (width < lineWidth)
        {
            lineWidth = 0.0f + glyphWidth;
            m_wrapBuffer[0] = L'\n';
        }
        wrapped += m_wrapBuffer;
        m_wrapBuffer = L"";
        if (isNewLineChar(*cursor))
        {
            glyphWidth = 0.0f;
            lineWidth = 0.0f;
        }
        else
        {
            glyphWidth = 0.0f + font->getRenderedGlyphWidth(*cursor, size);
        }
        m_wrapBuffer += *cursor;
        cursor = parseHTMLString(cursor + 1, NULL);
    }
    return wrapped;
}

void fuiRenderNodeEditText::setWidth(float width)
{
    float right = m_bounds.xMin + width;
    if (right == m_bounds.xMax && m_fieldWidth == width)
        return;
    m_fieldWidth = width;
    m_dirty = true;
    m_bounds.xMax = right;
    autoSizeBounds();
}

void fuiRenderNodeEditText::setHeight(float height)
{
    float bottom = m_bounds.yMin + height;
    if (bottom == m_bounds.yMax)
        return;
    m_dirty = true;
    m_bounds.yMax = bottom;
    autoSizeBounds();
}

void fuiRenderNodeEditText::setX(float x)
{
    if (x != fuiRenderNode::getX())
    {
        fuiRenderNode::setX(x);
        m_dirty = true;
    }
}

void fuiRenderNodeEditText::setY(float y)
{
    if (y != fuiRenderNode::getY())
    {
        fuiRenderNode::setY(y);
        m_dirty = true;
    }
}

void fuiRenderNodeEditText::getParentSpaceBounds(fuiRect* rect)
{
    rect->xMin = m_bounds.xMin * m_localMatrix.scaleX + m_localMatrix.translateX;
    rect->xMax = m_bounds.xMax * m_localMatrix.scaleX + m_localMatrix.translateX;
    rect->yMin = m_bounds.yMin * m_localMatrix.scaleY;
    rect->yMax = m_bounds.yMax * m_localMatrix.scaleY;
}

void fuiRenderNodeEditText::setVisibility(bool visible)
{
    if (isVisible() != visible)
        m_dirty = true;
    fuiRenderNode::setVisibility(visible);
}

float fuiRenderNodeEditText::getScrollH()
{
    return m_scrollH;
}

void fuiRenderNodeEditText::setScrollH(float scroll)
{
    m_scrollH = scroll;
    m_dirty = true;
}

int fuiRenderNodeEditText::getNumLines()
{
    return m_numLines;
}
