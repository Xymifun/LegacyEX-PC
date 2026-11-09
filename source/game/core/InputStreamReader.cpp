#include "game/core/InputStreamReader.hpp"

#include "game/core/DataInputStream.hpp"

InputStreamReader::InputStreamReader(InputStream* stream)
{
    mStream = new DataInputStream(stream);
}

void InputStreamReader::close()
{
    mStream->close();
}

int InputStreamReader::read()
{
    return mStream->readUTFChar();
}

unsigned int InputStreamReader::read(wchar_t* buffer, unsigned int offset, unsigned int length)
{
    int character;
    unsigned int index;
    unsigned int count = 0;

    for (index = offset; index < offset + length; index++)
    {
        character = mStream->readUTFChar();

        if (character == -1)
        {
            return count;
        }

        buffer[index] = (wchar_t)character;
        count++;
    }

    return count;
}
