#include "game/core/HtmlString.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/StringUtils.hpp"

#include <cwchar>
#include <iomanip>
#include <sstream>

HtmlString::HtmlString(const HtmlString& other)
    : mText(other.mText),
      mColour(other.mColour),
      mItalic(other.mItalic),
      mIndent(other.mIndent),
      mFontSize(other.mFontSize)
{
}

HtmlString::HtmlString(std::wstring text, eMinecraftColour colour, bool italic, bool indent, int fontSize)
{
    mText = escapeXML(text);
    mColour = colour;
    mItalic = italic;
    mIndent = indent;
    mFontSize = fontSize;
}

std::wstring HtmlString::ToString()
{
    std::wstringstream stream;

    if (mIndent)
    {
        stream << L"&nbsp;&nbsp;";
    }

    if (mItalic)
    {
        stream << "<i>";
    }

    eMinecraftColour colour = eMinecraftColour_HTMLColor_7;

    if (mColour != eMinecraftColour_NOTSET)
    {
        colour = mColour;
    }

    if (mFontSize > 0)
    {
        wchar_t fontSize[8];
        swprintf(fontSize, 8, L"%d", mFontSize);

        std::wostream& colourStream = stream << L"<font color=\"#" << std::setfill(L'0') << std::setw(6);
        colourStream.setf(std::ios_base::hex, std::ios_base::basefield);
        colourStream << static_cast<int>(gApp.GetHTMLColor(colour)) << L"\"size=\"" << fontSize << L"\">" << mText
                     << "</font>";
    }
    else
    {
        std::wostream& colourStream = stream << L"<font color=\"#" << std::setfill(L'0') << std::setw(6);
        colourStream.setf(std::ios_base::hex, std::ios_base::basefield);
        colourStream << static_cast<int>(gApp.GetHTMLColor(colour)) << L"\">" << mText << "</font>";
    }

    if (mItalic)
    {
        stream << "</i>";
    }

    return stream.str();
}

std::wstring HtmlString::Compose(std::vector<HtmlString>* lines)
{
    if (lines == NULL)
    {
        return L"";
    }

    std::wstringstream stream;

    for (unsigned int i = 0; i < lines->size(); ++i)
    {
        stream << lines->at(i).ToString();

        if (i + 1 < lines->size())
        {
            if (lines->at(i).mFontSize < 1)
            {
                stream << L"<br>";
            }
            else
            {
                wchar_t fontSize[8];
                swprintf(fontSize, 8, L"%d", lines->at(i).mFontSize);

                stream << L"<font size=\"" << fontSize << L"\">" << L"<br>" << "</font>";
            }
        }
    }

    return stream.str();
}
