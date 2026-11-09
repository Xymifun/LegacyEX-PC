#include "game/core/XmlParser.hpp"

#include <cstring>
#include <vector>

#include "game/core/DataOutputStream.hpp"
#include "game/core/XmlChild.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"

unsigned int XmlParser::compare(char* first, char* second, unsigned int length)
{
    unsigned int index;
    for (index = 0; index < length; ++index)
        if (first[index] != second[index])
            return 0;
    return 1;
}

static std::string gByteOrderMarks[] = {"", "", "\xEF\xBB\xBF", "\xFE\xFF", "\xFF\xFE"};

unsigned int XmlParser::detectFileFormat(void* data)
{
    if (compare((char*)data, (char*)gByteOrderMarks[eFileFormatUtf8].c_str(), gByteOrderMarks[eFileFormatUtf8].size())
        == 0)
    {
        if (compare((char*)data, (char*)gByteOrderMarks[eFileFormatUtf16BigEndian].c_str(),
                    gByteOrderMarks[eFileFormatUtf16BigEndian].size())
            != 0)
            return eFileFormatUtf16BigEndian;
        if (compare((char*)data, (char*)gByteOrderMarks[eFileFormatUtf16LittleEndian].c_str(),
                    gByteOrderMarks[eFileFormatUtf16LittleEndian].size())
            != 0)
            return eFileFormatUtf16LittleEndian;
    }
    return eFileFormatUtf8;
}

void XmlParser::_set_file_format(eFileFormat format)
{
    if (format == eFileFormatAuto)
        m_fileFormat = detectFileFormat(m_xmlSrc);
}

void XmlParser::_parse()
{
    switch (m_fileFormat)
    {
    case eFileFormatAnsi:
    case eFileFormatUtf8:
        m_document.parse<0>((char*)m_xmlSrc);
        break;
    case eFileFormatUtf16BigEndian:
    case eFileFormatUtf16LittleEndian:
        m_wideDocument.parse<16>((wchar_t*)(m_xmlSrc + 2));
        break;
    }
}

XmlParser::XmlParser(unsigned char* data, unsigned int length)
{
    m_sourceName = L"[FROM BYTEARRAY]";
    m_xmlSrcLength = length + 2;
    m_xmlSrc = new unsigned char[m_xmlSrcLength];
    std::memset(m_xmlSrc, 0, m_xmlSrcLength);
    std::memcpy(m_xmlSrc, data, length);
    _set_file_format(eFileFormatAuto);
    _parse();
}

XmlParser::~XmlParser()
{
    delete[] m_xmlSrc;
}

std::wstring XmlParser::cnv2Wstr(const char* text)
{
    switch (m_fileFormat)
    {
    case eFileFormatUtf8:
        return convUtf8ToWstring(text);
    case eFileFormatAnsi:
        return convStringToWstring(std::string(text));
    default:
        return std::wstring(L"");
    }
}

XmlChild XmlParser::_getXml(rapidxml::xml_node<char>* node)
{
    if (node->type() == rapidxml::node_element || node->type() == rapidxml::node_document)
    {
        XmlObject* object = new XmlObject(cnv2Wstr(node->name()));
        rapidxml::xml_attribute<char>* attribute;
        for (attribute = node->first_attribute(); attribute != NULL; attribute = attribute->next_attribute())
        {
            object->addAttr(cnv2Wstr(attribute->name()), cnv2Wstr(attribute->value()));
        }
        rapidxml::xml_node<char>* child;
        for (child = node->first_node(); child != NULL; child = child->next_sibling())
            object->addChildRaw(_getXml(child));
        return XmlChild(object);
    }
    if (node->type() == rapidxml::node_cdata || node->type() == rapidxml::node_data)
        return XmlChild(cnv2Wstr(node->value()), node->type() == rapidxml::node_data);
    return XmlChild((XmlObject*)NULL);
}

XmlChild XmlParser::_getXml(rapidxml::xml_node<wchar_t>* node)
{
    if (node->type() == rapidxml::node_element || node->type() == rapidxml::node_document)
    {
        XmlObject* object = new XmlObject(std::wstring(node->name()));
        rapidxml::xml_attribute<wchar_t>* attribute;
        for (attribute = node->first_attribute(); attribute != NULL; attribute = attribute->next_attribute())
        {
            object->addAttr(std::wstring(attribute->name()), std::wstring(attribute->value()));
        }
        rapidxml::xml_node<wchar_t>* child;
        for (child = node->first_node(); child != NULL; child = child->next_sibling())
            object->addChildRaw(_getXml(child));
        return XmlChild(object);
    }
    if (node->type() == rapidxml::node_cdata || node->type() == rapidxml::node_data)
        return XmlChild(std::wstring(node->value()));
    return XmlChild();
}

XmlObject* XmlParser::getXml()
{
    XmlObject* object = new XmlObject(L"__ROOT__");
    switch (m_fileFormat)
    {
    case eFileFormatAnsi:
    case eFileFormatUtf8:
        object->addChildRaw(_getXml(m_document.first_node()));
        break;
    case eFileFormatUtf16BigEndian:
    case eFileFormatUtf16LittleEndian:
        object->addChildRaw(_getXml(m_wideDocument.first_node()));
        break;
    }
    return object;
}

XmlObject* XmlParser::readXml(unsigned char* data, unsigned int length)
{
    XmlParser parser(data, length);
    return parser.getXml();
}

void XmlParser::writeXml(DataOutputStream* stream, XmlObject* object, bool formatted)
{
    arrayWithLength<unsigned char> output;
    output = arrayWithLength<unsigned char>((unsigned char*)gByteOrderMarks[eFileFormatUtf8].c_str(),
                                            gByteOrderMarks[eFileFormatUtf8].size());
    stream->write(output);
    toUtf8(std::wstring(L""), (char**)&output.data, output.length);
    --output.length;
    stream->write(output);
    delete[] output.data;
    std::vector<XmlChild>::const_iterator child;
    for (child = object->getChildrenRaw().begin(); child != object->getChildrenRaw().end(); ++child)
    {
        toUtf8(child->xml()->toString(0, true), (char**)&output.data, output.length);
        --output.length;
        stream->write(output);
        delete[] output.data;
    }
}
