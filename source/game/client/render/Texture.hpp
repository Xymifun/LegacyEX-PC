#ifndef GAME_CLIENT_RENDER_TEXTURE_HPP
#define GAME_CLIENT_RENDER_TEXTURE_HPP

#include <string>
#include "game/core/BufferedImage.hpp"
#include "game/core/Rect2i.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class ByteBuffer;

class Texture
{
public:
    Texture(const std::wstring& name, int type, int width, int height, int wrap, int format, int minFilter,
            int magFilter, BufferedImage* image, bool mipmap);
    Texture(const std::wstring& name, int type, int width, int height, int depth, int wrap, int format, int minFilter,
            int magFilter, BufferedImage* image, bool mipmap);
    ~Texture();

    void _init(const std::wstring& name, int type, int width, int height, int depth, int wrap, int format,
               int minFilter, int magFilter, bool mipmap);
    void updateOnGPU();
    static unsigned int crispBlend(int first, int second);
    void transferFromImage(BufferedImage* image);
    void _init(const std::wstring& name, int type, int width, int height, int depth, int wrap, int format,
               int minFilter, int magFilter, BufferedImage* image, bool mipmap);
    ByteBuffer* getData(unsigned int mipLevel);
    int getHeight();
    int getWidth();
    void blit(int x, int y, Texture* texture, bool flip);
    unsigned int getManagerId();
    unsigned int getGlId();
    void bind(int textureUnit);
    Texture(const std::wstring& name, int type, int width, int height, int depth, int wrap, int format, int minFilter,
            int magFilter, bool mipmap);
    Rect2i* getRect();
    void fill(const Rect2i* rect, int color);
    void writeAsBMP(const std::wstring& fileName) {}
    void writeAsPNG(const std::wstring& fileName) {}
    void blit(int x, int y, Texture* texture);
    void transferFromBuffer(arrayWithLength<int> pixels);
    std::wstring getName();
    void setImmediateUpdate(bool immediate);

private:
    friend class TextureAtlasSprite;
    unsigned int m_glId;
    unsigned int m_managerId;
    int m_type;
    int m_width;
    int m_height;
    int m_depth;
    int m_format;
    int m_target;
    int m_minFilter;
    int m_magFilter;
    int m_wrap;
    bool m_mipmap;
    std::wstring m_name;
    Rect2i* m_rect;
    bool m_valid;
    bool m_immediateUpdate;
    bool m_updated;
    int m_mipLevels;
    ByteBuffer* m_data[10];
    bool m_gpuAllocated;
};

#endif
