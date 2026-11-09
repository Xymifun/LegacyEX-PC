#ifndef GAME_CORE_XMLPARSER_HPP
#define GAME_CORE_XMLPARSER_HPP

#include <string>

class DataOutputStream;
class File;
class XmlChild;
class XmlObject;

#define RAPIDXML_NO_EXCEPTIONS
#define RAPIDXML_STATIC_POOL_SIZE 1024
#include <rapidxml.hpp>

namespace rapidxml
{
inline void parse_error_handler(const char* what, void* where)
{
}
}

class XmlParser
{
public:
    enum eFileFormat
    {
        eFileFormatAuto = 0,
        eFileFormatAnsi = 1,
        eFileFormatUtf8 = 2,
        eFileFormatUtf16BigEndian = 3,
        eFileFormatUtf16LittleEndian = 4
    };

    XmlParser(unsigned char* data, unsigned int length);
    ~XmlParser();
    static unsigned int compare(char* first, char* second, unsigned int length);
    unsigned int detectFileFormat(void* data);
    void _set_file_format(eFileFormat format);
    void _parse();
    std::wstring cnv2Wstr(const char* text);
    XmlChild _getXml(rapidxml::xml_node<char>* node);
    XmlChild _getXml(rapidxml::xml_node<wchar_t>* node);
    XmlObject* getXml();
    static XmlObject* readXml(unsigned char* data, unsigned int length);
    static void writeXml(DataOutputStream* stream, XmlObject* object, bool formatted);
    void _init();
    void _readFile(File* file);
    XmlParser(File* file);
    XmlParser(File* file, eFileFormat format);
    unsigned char* getXmlSrc();
    static XmlObject* readXml(File* file);
    static XmlObject* readXml(std::wstring file);
    static bool is_big_endian();
    static void writeXml(File* file, XmlObject* object, bool formatted);
    static void writeXml(std::wstring file, XmlObject* object, bool formatted);
    static void writeXml(unsigned char*& data, unsigned int& length, XmlObject* object, bool formatted);

private:
    std::wstring m_sourceName;
    unsigned int m_fileFormat;
    unsigned char* m_xmlSrc;
    unsigned int m_xmlSrcLength;
    rapidxml::xml_document<char> m_document;
    rapidxml::xml_document<wchar_t> m_wideDocument;
};

#endif
