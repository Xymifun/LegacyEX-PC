#ifndef GAME_UI_FUIFILE_HPP
#define GAME_UI_FUIFILE_HPP

#include <stdint.h>

#include "game/ui/fuiFileHeader.hpp"
#include "game/world/biome/arrayWithLength.hpp"

struct fuiBitmap;
struct fuiEdittext;
struct fuiFontName;
struct fuiImportAsset;
struct fuiReference;
class fuiRenderNode;
class fuiRenderNodeStage;
struct fuiShape;
struct fuiShapeComponent;
struct fuiSymbol;
struct fuiTimeline;
struct fuiTimelineAction;
struct fuiTimelineEvent;
struct fuiTimelineEventName;
struct fuiTimelineFrame;
struct fuiVert;
struct fuiRect;

struct fuiFile
{
    fuiFileHeader header;
    int index;
    int fileIndex;
    fuiTimeline* timelineData;
    fuiTimelineAction* timelineActionData;
    fuiShape* shapeData;
    fuiShapeComponent* shapeComponentData;
    fuiVert* vertexData;
    fuiTimelineFrame* frameData;
    fuiTimelineEvent* eventData;
    fuiTimelineEventName* eventNameData;
    fuiReference* referenceData;
    fuiEdittext* editTextData;
    fuiSymbol* symbolData;
    fuiBitmap* bitmaps;
    unsigned char* imageData;
    fuiFontName* fontNameData;
    fuiImportAsset* importAssetData;
    fuiRenderNodeStage* stage;
    fuiRenderNode* rootNode;
    void (*customDrawCallback)(void*, const char*, fuiRect*);
    void* customDrawContext;
    unsigned char* data;

    fuiFile();
    ~fuiFile();
    intptr_t addDataRegion(unsigned int count, unsigned int stride, unsigned char** cursor, void (*swap)(void*));
    unsigned int load(arrayWithLength<unsigned char> bytes, int fileIndex);
    unsigned int resolveReferences(fuiFile* other);
    void dumpUnresolvedReferences();
    void initialiseRenderTexture(fuiBitmap* bitmap, bool force);
    fuiRenderNode* getRootNode();
    intptr_t createNodeFromSymbol(const char* name, fuiRenderNode* parent, int fileIndex);
    fuiRenderNode* findNode(const char* name);
    void setVisible(const char* name, bool visible);
    void setCustomDrawCallback(void (*callback)(void*, const char*, fuiRect*), void* context);
    float getStageWidth();
    float getStageHeight();
    void setIndex(int value);
    void play(const char* name);
    void stop(const char* name);
    void gotoFrame(const char* name, int frame);
    void gotoFrameAndPlay(const char* name, int frame);
    void gotoFrameAndStop(const char* name, int frame);
};

#endif
