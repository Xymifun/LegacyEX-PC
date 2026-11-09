#include "game/core/BufferedOutputStream.hpp"

BufferedOutputStream::BufferedOutputStream(OutputStream* out, int size)
{
    this->out = out;
    buf = arrayWithLength<unsigned char>(size, true);
    count = 0;
}

BufferedOutputStream::~BufferedOutputStream()
{
    delete[] buf.data;
    buf.data = NULL;
}

void BufferedOutputStream::flush()
{
    out->write(buf, 0, count);
    count = 0;
}

void BufferedOutputStream::close()
{
    flush();
    out->close();
}

void BufferedOutputStream::write(arrayWithLength<unsigned char> b, unsigned int off, unsigned int len)
{
    if (len >= buf.length)
    {
        flush();
        out->write(b, off, len);
        return;
    }

    for (unsigned int i = 0; i < len; i++)
    {
        write(b[off + i]);
    }
}

void BufferedOutputStream::write(arrayWithLength<unsigned char> b)
{
    write(b, 0, b.length);
}

void BufferedOutputStream::write(unsigned int b)
{
    buf[count++] = (unsigned char)b;

    if (count == buf.length)
    {
        flush();
    }
}
