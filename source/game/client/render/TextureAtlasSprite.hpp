#ifndef GAME_CLIENT_RENDER_TEXTUREATLASSprite_HPP
#define GAME_CLIENT_RENDER_TEXTUREATLASSprite_HPP
#include <string>
#include <vector>
struct Texture;
class BufferedReader;
class TextureAtlasSprite
{
public:
    TextureAtlasSprite(const std::wstring&, const std::wstring&, float, float, float, float, float, float);
    virtual ~TextureAtlasSprite();
    static TextureAtlasSprite* create(const std::wstring&);
    void initUVs(float, float, float, float);
    void init(Texture*, std::vector<Texture*>*, int, int, int, int, bool);
    void replaceWith(TextureAtlasSprite*);
    virtual unsigned int getX() const;
    virtual unsigned int getY() const;
    virtual unsigned int getWidth() const;
    virtual unsigned int getHeight() const;
    virtual float getU0(bool) const;
    virtual float getU1(bool) const;
    virtual float getU(double) const;
    virtual float getV0(bool) const;
    virtual float getV1(bool) const;
    virtual float getV(double) const;
    void adjustUVs(float&, float&, float&, float&);
    void adjustU(float&, float&);
    void adjustV(float&, float&);
    virtual std::wstring getName() const;
    virtual unsigned int getSourceWidth() const;
    virtual unsigned int getSourceHeight() const;
    void uploadInterpolatedFrame();
    virtual void cycleFrames();
    int getFrameCount() const;
    Texture* getSource();
    Texture* getFrame(int);
    virtual int getFrames();
    void loadAnimationFrames(BufferedReader*);
    void loadAnimationFrames(const std::wstring&);
    virtual unsigned int getFlags() const;
    virtual void setFlags(int);
    virtual void freeFrameTextures();
    virtual unsigned int hasOwnData();

protected:
    friend class TextureAtlas;
    std::wstring mName;
    std::wstring mParent;
    Texture* mSource;
    std::vector<Texture*>* mFrames;
    Texture* mOwnTexture;
    std::vector<std::pair<int, int> >* mAnimationFrames;
    int mFlags;
    bool mInterpolated;
    bool mRotated;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    float mU0;
    float mU1;
    float mV0;
    float mV1;
    float mUOffset;
    float mVOffset;
    float mUScale;
    float mVScale;
    int mFrame;
    int mFrameTime;
};
#endif
