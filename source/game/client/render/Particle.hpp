#ifndef GAME_CLIENT_RENDER_PARTICLE_HPP
#define GAME_CLIENT_RENDER_PARTICLE_HPP

#include <stdint.h>
#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/client/render/TextureUVCoordinateSet.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"

class BufferBuilder;
class Entity;
class Level;
class TextureAtlasSprite;
class Vec3;

class Particle : public boost::enable_shared_from_this<Particle>
{
public:
    Particle(Level* level, double x, double y, double z, double xa, double ya, double za);
    Particle(Level* level, double x, double y, double z);
    virtual uint64_t GetType() = 0;
    virtual ~Particle();
    virtual Particle* setPower(float power);
    virtual Particle* scale(float factor);
    virtual void tick();
    virtual void render(BufferBuilder* buffer, const boost::shared_ptr<Entity>& camera, float partialTicks, float xa,
                        float ya, float za, float xa2, float za2);
    virtual int getParticleTexture();
    virtual void setSprite(TextureAtlasSprite* sprite);
    virtual void setMiscTex(int texture);
    virtual std::wstring toString();
    virtual bool isSemiTransparent();
    virtual bool isTransparent();
    virtual void setPos(double x, double y, double z);
    virtual void move(double xa, double ya, double za);
    virtual int getLightColor(float partialTicks);
    virtual float getBrightness(float partialTicks);
    virtual void setSize(float width, float height);
    virtual void setLocationFromBoundingbox();

    Level* getLevel();
    float getAlpha();
    uint8_t isAlive();
    void remove();
    void setBoundingBox(const AABB* box);
    AABB* getBoundingBox();
    void setColor(float red, float green, float blue);
    void setAlpha(float alpha);
    void setLifetime(int lifetime);
    float getRedCol();
    float getGreenCol();
    float getBlueCol();

    static void setXOffset(double offset);
    static void setYOffset(double offset);
    static void setZOffset(double offset);
    static void setCamerViewDirection(Vec3* direction);

    static double xOff;
    static double yOff;
    static double zOff;

    Level* level;
    double xo;
    double yo;
    double zo;
    double x;
    double y;
    double z;
    double xd;
    double yd;
    double zd;
    bool onGround;
    bool hasPhysics;
    bool removed;
    float bbWidth;
    float bbHeight;
    Random random;
    int textureIndex;
    TextureUVCoordinateSet uvs;
    float uo;
    float vo;
    float rCol;
    float gCol;
    float bCol;
    float alpha;
    int age;
    int lifetime;
    int cachedLightColor;
    int cachedLightPos;
    float size;
    float gravity;
    TextureAtlasSprite* sprite;
    float roll;
    float oRoll;
    AABB* bb;

private:
    void _init(Level* level, double x, double y, double z);
};

#endif
