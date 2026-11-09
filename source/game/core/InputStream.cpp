#include "game/core/InputStream.hpp"

#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"

InputStream* InputStream::getResourceAsStream(const std::wstring& fileName)
{
    File file(fileName);
    InputStream* stream = NULL;

    if (file.exists())
    {
        stream = new FileInputStream(file);
    }

    return stream;
}
