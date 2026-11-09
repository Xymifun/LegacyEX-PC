#ifndef GAME_CLIENT_RENDER_HTTPTEXTUREPROCESSOR_HPP
#define GAME_CLIENT_RENDER_HTTPTEXTUREPROCESSOR_HPP

class BufferedImage;

class HttpTextureProcessor
{
public:
    virtual BufferedImage* process(BufferedImage* image) = 0;
};

#endif
