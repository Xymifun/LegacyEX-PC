#ifndef GAME_CLIENT_RENDER_ABSTRACTTEXTURE_HPP
#define GAME_CLIENT_RENDER_ABSTRACTTEXTURE_HPP

class ResourceManager;

class TextureObject
{
public:
    virtual void setFilter(bool blur, bool mipmap) = 0;
    virtual void pushFilter(bool blur, bool mipmap) = 0;
    virtual void popFilter() = 0;
    virtual void load(ResourceManager* resourceManager) = 0;
    virtual int getId() = 0;
    virtual void releaseId() = 0;
};

class AbstractTexture : public TextureObject
{
public:
    AbstractTexture();

    virtual void setFilter(bool blur, bool mipmap);
    virtual void pushFilter(bool blur, bool mipmap);
    virtual void popFilter();
    virtual void load(ResourceManager* resourceManager) = 0;
    virtual int getId();
    virtual void releaseId();

protected:
    int id;
    bool blur;
    bool mipmap;
    bool oldBlur;
    bool oldMipmap;
};

#endif
