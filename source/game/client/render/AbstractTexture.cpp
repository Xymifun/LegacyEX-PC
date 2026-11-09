#include "game/client/render/AbstractTexture.hpp"

#include "game/client/render/TextureUtil.hpp"

AbstractTexture::AbstractTexture() : id(-1), blur(false), mipmap(false), oldBlur(false), oldMipmap(false)
{
}

void AbstractTexture::setFilter(bool blur, bool mipmap)
{
    this->blur = blur;
    this->mipmap = mipmap;
}

void AbstractTexture::pushFilter(bool blur, bool mipmap)
{
    this->oldBlur = this->blur;
    this->oldMipmap = this->mipmap;
    setFilter(blur, mipmap);
}

void AbstractTexture::popFilter()
{
    setFilter(this->oldBlur, this->oldMipmap);
}

int AbstractTexture::getId()
{
    if (this->id == -1)
    {
        this->id = TextureUtil::generateTextureId();
    }
    return this->id;
}

void AbstractTexture::releaseId()
{
    if (this->id != -1)
    {
        TextureUtil::releaseTextureId(this->id);
        this->id = -1;
    }
}
