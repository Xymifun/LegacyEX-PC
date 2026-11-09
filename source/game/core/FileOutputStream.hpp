#ifndef GAME_CORE_FILEOUTPUTSTREAM_HPP
#define GAME_CORE_FILEOUTPUTSTREAM_HPP

#include "game/core/OutputStream.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class File;

class FileOutputStream : public OutputStream
{
public:
    FileOutputStream(const File& file);
    virtual ~FileOutputStream();

    virtual void write(unsigned int b);
    virtual void write(arrayWithLength<unsigned char> data);
    virtual void write(arrayWithLength<unsigned char> data, unsigned int offset, unsigned int length);
    virtual void close();
    virtual void flush();

private:
    int mFile;
};

#endif
