#ifndef GAME_CLIENT_RENDER_MEMTEXTUREPROCESSOR_HPP
#define GAME_CLIENT_RENDER_MEMTEXTUREPROCESSOR_HPP

class BufferedImage;

class MemTextureProcessor
{
public:
    virtual ~MemTextureProcessor() {}

    virtual BufferedImage* process(BufferedImage* image) = 0;
};

#endif
