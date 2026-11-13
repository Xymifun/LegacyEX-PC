#ifndef PLATFORM_PC_PCMODVIEWS_HPP
#define PLATFORM_PC_PCMODVIEWS_HPP

class Entity;
bool PcModViewsBeginFrame(Entity* camera, float partialTick);
int PcModViewsCount();
void PcModViewsBeginView(int idx);
void PcModViewsEndView();
bool PcModViewsKeepWindowView();
void PcModViewsFinishFrame();
void PcModViewsBeforePresent();

class HitResult;
void PcModViewsAimBegin(Entity* camera, float partialTick);
void PcModViewsAimEnd(const HitResult* hit);
bool PcModViewsAimFor(const Entity* entity, double* pos, double* dir);
bool PcModViewsAimRotation(const Entity* entity, float* yRot, float* xRot);
bool PcModViewsLookRotation(const Entity* entity, float* yRot, float* xRot);

void PcModViewsNoteHotbarSlot(int slot, float left, float top, float right, float bottom);
int PcModViewsHudRect(int part, float* rect);
bool PcModViewsDrawsInWorld();
void PcModViewsSetGameView(const float* modelView);
const float* PcModViewsFromCamera();
void PcModViewsDraw();

bool PcModViewsInView();
bool PcModViewsArm(int idx, float* viewFromArm);
bool PcModViewsShowHand();
int PcModViewsItemCount();
int PcModViewsBoxCount();
int PcModViewsWorldBox(int idx, float* viewFromBox, float* colour);
int PcModViewsTextCount();
const char* PcModViewsText(int idx, float* viewFromText, unsigned int* colour, unsigned int* background);
const void* PcModViewsItem(int idx, float* viewFromItem);
void PcModViewsProjection(float nearDist, float farDist, float* matrix);
const float* PcModViewsMatrix();

#endif
