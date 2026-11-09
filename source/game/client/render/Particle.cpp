#include "game/client/render/Particle.hpp"

#include <vector>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

double Particle::xOff = 0.0;
double Particle::yOff = 0.0;
double Particle::zOff = 0.0;

static Vec3* cameraViewDirection = Vec3::newPermanent(0.0, 0.0, 0.0);
static AABB* INITIAL_AABB = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

Particle::Particle(Level* level, double x, double y, double z, double xa, double ya, double za)
{
    _init(level, x, y, z);

    xd = xa + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;
    yd = ya + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;
    zd = za + (Math::random() * 2.0 - 1.0) * 0.4000000059604645;

    float power = (float)(Math::random() + Math::random() + 1.0) * 0.15f;
    double length = Mth::sqrt(xd * xd + yd * yd + zd * zd);

    xd = xd / length * power * 0.4000000059604645;
    yd = yd / length * power * 0.4000000059604645 + 0.10000000149011612;
    zd = zd / length * power * 0.4000000059604645;
}

Particle::Particle(Level* level, double x, double y, double z)
{
    _init(level, x, y, z);
}

Particle::~Particle()
{
    delete bb;
}

void Particle::_init(Level* level, double x, double y, double z)
{
    this->level = level;
    bb = AABB::newPermanent(INITIAL_AABB);

    textureIndex = 0;
    onGround = false;
    hasPhysics = true;
    removed = false;

    alpha = 1.0f;
    bbWidth = 0.6f;
    bbHeight = 1.8f;

    gravity = 0.0f;
    sprite = NULL;
    roll = 0.0f;
    oRoll = 0.0f;

    rCol = 1.0f;
    gCol = 1.0f;
    bCol = 1.0f;

    uo = random.nextFloat() * 3.0f;
    vo = random.nextFloat() * 3.0f;
    size = (random.nextFloat() * 0.5f + 0.5f) * 2.0f;
    lifetime = (int)(4.0f / (random.nextFloat() * 0.9f + 0.1f));
    age = 0;

    uvs = TextureUVCoordinateSet(0.0f, 0.0f, 1.0f, 1.0f, 16, 16);

    setSize(0.2f, 0.2f);
    setPos(x, y, z);

    xo = x;
    yo = y;
    zo = z;

    cachedLightPos = -1;

    xd = 0.0;
    yd = 0.0;
    zd = 0.0;
}

Particle* Particle::setPower(float power)
{
    yd = (yd - 0.10000000149011612) * power + 0.10000000149011612;
    xd *= power;
    zd *= power;
    return this;
}

Particle* Particle::scale(float factor)
{
    setSize(0.2f * factor, 0.2f * factor);
    size *= factor;

    return this;
}

void Particle::tick()
{
    xo = x;
    yo = y;
    zo = z;

    if (age++ >= lifetime)
    {
        remove();
    }

    yd -= 0.04 * (double)gravity;

    move(xd, yd, zd);

    xd *= 0.98f;
    yd *= 0.98f;
    zd *= 0.98f;

    if (onGround)
    {
        xd *= 0.7f;
        zd *= 0.7f;
    }
}

void Particle::render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                      float ya, float za, float xa2, float za2)
{
    float halfSize = 0.1f * size;

    if (sprite != NULL)
    {
        uvs.u0 = sprite->getU0(false);
        uvs.u1 = sprite->getU1(false);
        uvs.v0 = sprite->getV0(false);
        uvs.v1 = sprite->getV1(false);
    }

    float px = (float)(xo + (x - xo) * partialTicks - xOff);
    float py = (float)(yo + (y - yo) * partialTicks - yOff);
    float pz = (float)(zo + (z - zo) * partialTicks - zOff);

    int light = getLightColor(partialTicks);

    Vec3* corner0 = Vec3::newTemp(-xa * halfSize - xa2 * halfSize, -ya * halfSize, -za * halfSize - za2 * halfSize);
    Vec3* corner1 = Vec3::newTemp(-xa * halfSize + xa2 * halfSize, ya * halfSize, -za * halfSize + za2 * halfSize);
    Vec3* corner2 = Vec3::newTemp(xa * halfSize + xa2 * halfSize, ya * halfSize, za * halfSize + za2 * halfSize);
    Vec3* corner3 = Vec3::newTemp(xa * halfSize - xa2 * halfSize, -ya * halfSize, za * halfSize - za2 * halfSize);

    if (roll != 0.0f)
    {
        float angle = (roll + (roll - oRoll) * partialTicks) * 0.5f;
        float cosAngle = Mth::cos(angle);
        Vec3* axis = Vec3::newTemp(Mth::sin(angle) * (float)cameraViewDirection->x,
                                   Mth::sin(angle) * (float)cameraViewDirection->y,
                                   Mth::sin(angle) * (float)cameraViewDirection->z);

        Vec3* along0 = axis->scale(2.0 * corner0->dot(axis));
        Vec3* kept0 = corner0->scale((double)(cosAngle * cosAngle) - axis->dot(axis));
        Vec3* sum0 = along0->add(kept0);
        corner0 = sum0->add(axis->cross(corner0)->scale(2.0f * cosAngle));
        Vec3* along1 = axis->scale(2.0 * corner1->dot(axis));
        Vec3* kept1 = corner1->scale((double)(cosAngle * cosAngle) - axis->dot(axis));
        Vec3* sum1 = along1->add(kept1);
        corner1 = sum1->add(axis->cross(corner1)->scale(2.0f * cosAngle));
        Vec3* along2 = axis->scale(2.0 * corner2->dot(axis));
        Vec3* kept2 = corner2->scale((double)(cosAngle * cosAngle) - axis->dot(axis));
        Vec3* sum2 = along2->add(kept2);
        corner2 = sum2->add(axis->cross(corner2)->scale(2.0f * cosAngle));
        Vec3* along3 = axis->scale(2.0 * corner3->dot(axis));
        Vec3* kept3 = corner3->scale((double)(cosAngle * cosAngle) - axis->dot(axis));
        Vec3* sum3 = along3->add(kept3);
        corner3 = sum3->add(axis->cross(corner3)->scale(2.0f * cosAngle));
    }

    buffer->color(rCol, gCol, bCol, alpha);
    buffer->tex2(light);

    buffer->vertexUV(px + (float)corner0->x, py + (float)corner0->y, pz + (float)corner0->z, uvs.u1, uvs.v1);
    buffer->vertexUV(px + (float)corner1->x, py + (float)corner1->y, pz + (float)corner1->z, uvs.u1, uvs.v0);
    buffer->vertexUV(px + (float)corner2->x, py + (float)corner2->y, pz + (float)corner2->z, uvs.u0, uvs.v0);
    buffer->vertexUV(px + (float)corner3->x, py + (float)corner3->y, pz + (float)corner3->z, uvs.u0, uvs.v1);
}

int Particle::getParticleTexture()
{
    return 0;
}

void Particle::setSprite(TextureAtlasSprite* sprite)
{
    int texture = getParticleTexture();

    if (texture == 1 || texture == 2)
    {
        this->sprite = sprite;
    }
}

void Particle::setMiscTex(int texture)
{
    if (getParticleTexture() == 0 || getParticleTexture() == 4)
    {
        uvs = TextureUVCoordinateSet::fromOldSystem(texture);
    }
}

std::wstring Particle::toString()
{
    return std::wstring(L"A particle");
}

bool Particle::isSemiTransparent()
{
    return false;
}

bool Particle::isTransparent()
{
    return false;
}

void Particle::setBoundingBox(const AABB* box)
{
    bb->set(box);
}

AABB* Particle::getBoundingBox()
{
    return bb;
}

void Particle::setPos(double x, double y, double z)
{
    this->x = x;
    this->y = y;
    this->z = z;

    float halfWidth = bbWidth / 2.0f;

    setBoundingBox(AABB::newTemp(x - halfWidth, y, z - halfWidth, x + halfWidth, y + bbHeight, z + halfWidth));
}

void Particle::move(double xa, double ya, double za)
{
    double xaOrg = xa;
    double yaOrg = ya;
    double zaOrg = za;

    if (hasPhysics)
    {
        boost::shared_ptr<Entity> entity;
        std::vector<AABB*>& boxes = *level->getCollisionAABBs(entity, getBoundingBox()->expand(xa, ya, za), false,
                                                              false, true);

        std::vector<AABB*>::iterator box;

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            ya = (*box)->clipYCollide(getBoundingBox(), ya);
        }

        setBoundingBox(getBoundingBox()->move(0.0, ya, 0.0));

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            xa = (*box)->clipXCollide(getBoundingBox(), xa);
        }

        setBoundingBox(getBoundingBox()->move(xa, 0.0, 0.0));

        for (box = boxes.begin(); box != boxes.end(); ++box)
        {
            za = (*box)->clipZCollide(getBoundingBox(), za);
        }

        setBoundingBox(getBoundingBox()->move(0.0, 0.0, za));
        setLocationFromBoundingbox();
    }
    else
    {
        setBoundingBox(getBoundingBox()->move(xa, ya, za));
        setLocationFromBoundingbox();
    }

    onGround = yaOrg != ya && yaOrg < 0.0;

    if (xaOrg != xa)
    {
        xd = 0.0;
    }

    if (zaOrg != za)
    {
        zd = 0.0;
    }
}

int Particle::getLightColor(float partialTicks)
{
    BlockPos pos(x, y, z);
    uint32_t key = ((pos.x & 0xFF) << 16) | ((pos.y & 0xFF) << 8) | (pos.z & 0xFF);

    if (key == cachedLightPos)
    {
        return cachedLightColor;
    }
    if (level->hasChunkAt(pos))
    {
        int color = level->getLightColor(pos, 0, -1);
        cachedLightPos = key;
        cachedLightColor = color;
        return color;
    }
    return 0;
}

float Particle::getBrightness(float partialTicks)
{
    BlockPos pos(x, y, z);
    if (level->hasChunkAt(pos))
        return level->getBrightness(pos);
    return 0.0;
}

void Particle::setColor(float red, float green, float blue)
{
    rCol = red;
    gCol = green;
    bCol = blue;
}

void Particle::setAlpha(float alpha)
{
    this->alpha = alpha;
}

void Particle::setLifetime(int lifetime)
{
    this->lifetime = lifetime;
}

float Particle::getRedCol()
{
    return rCol;
}

float Particle::getGreenCol()
{
    return gCol;
}

float Particle::getBlueCol()
{
    return bCol;
}

void Particle::setSize(float width, float height)
{
    if (width != bbWidth || height != bbHeight)
    {
        bbWidth = width;
        bbHeight = height;

        AABB* box = getBoundingBox();

        setBoundingBox(AABB::newTemp(box->mMinX, box->mMinY, box->mMinZ, box->mMinX + width, box->mMinY + height,
                                     box->mMinZ + width));
    }
}

void Particle::setLocationFromBoundingbox()
{
    AABB* box = getBoundingBox();

    x = (box->mMinX + box->mMaxX) * 0.5;
    y = box->mMinY;
    z = (box->mMinZ + box->mMaxZ) * 0.5;
}

Level* Particle::getLevel()
{
    return level;
}

float Particle::getAlpha()
{
    return alpha;
}

uint8_t Particle::isAlive()
{
    return !removed;
}

void Particle::remove()
{
    removed = true;
}

void Particle::setXOffset(double offset)
{
    xOff = offset;
}

void Particle::setYOffset(double offset)
{
    yOff = offset;
}

void Particle::setZOffset(double offset)
{
    zOff = offset;
}

void Particle::setCamerViewDirection(Vec3* direction)
{
    cameraViewDirection->x = direction->x;
    cameraViewDirection->y = direction->y;
    cameraViewDirection->z = direction->z;
}
