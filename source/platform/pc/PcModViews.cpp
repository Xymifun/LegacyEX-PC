#include "platform/pc/PcModViews.hpp"

#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcGl.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcNativeMod.h"
#include "platform/pc/PcNativeMods.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/core/HitResult.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"

#include <algorithm>
#include <math.h>
#include <stdio.h>
#include <string.h>

namespace
{
const int MIN_VIEW_SIZE = 16;
const int MAX_VIEW_SIZE = 8192;
const float PANEL_NEAR = 0.05f;
const float PANEL_FAR = 1000.0f;
const float MENU_BACKGROUND[3] = {0.03f, 0.03f, 0.04f};

struct ViewTarget
{
    GLuint framebuffer;
    GLuint colour;
    GLuint depth;
    int width;
    int height;
};

ViewTarget renderTargets[2];
ViewTarget outputTargets[2];
ViewTarget panelTarget;
bool panelReady = false;
bool scaled = false;
PcNativeViews views;
bool active = false;
bool worldFrame = false;
bool panelFrame = false;
int currentView = -1;
bool warned = false;
bool aimSet = false;
float aimOrigin[3];
float aimDirection[3];
const Entity* aimEntity = 0;
double aimStart[3];
double aimDir[3];
float aimHit = 0.0f;
struct HudRect
{
    float gathered[4];
    bool gathering;
    float rect[4];
};
HudRect hotbarRect;
HudRect offhandRect;
bool drawFrame = false;
bool gameViewSet = false;
float gameFromCamera[16];
bool lookSet = false;
float lookForward[3];
double cameraPos[3];
float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float cameraHeight = 0.0f;

void KeepHudRect(HudRect* hud)
{
    if (hud->gathering)
        memcpy(hud->rect, hud->gathered, sizeof(hud->rect));
    else
        memset(hud->rect, 0, sizeof(hud->rect));
    hud->gathering = false;
}

void GatherHudRect(HudRect* hud, float left, float top, float right, float bottom)
{
    if (!hud->gathering)
    {
        hud->gathered[0] = left;
        hud->gathered[1] = top;
        hud->gathered[2] = right;
        hud->gathered[3] = bottom;
        hud->gathering = true;
        return;
    }
    hud->gathered[0] = std::min(hud->gathered[0], left);
    hud->gathered[1] = std::min(hud->gathered[1], top);
    hud->gathered[2] = std::max(hud->gathered[2], right);
    hud->gathered[3] = std::max(hud->gathered[3], bottom);
}

bool ValidView(const PcNativeView& v)
{
    return v.width >= MIN_VIEW_SIZE && v.height >= MIN_VIEW_SIZE && v.width <= MAX_VIEW_SIZE
           && v.height <= MAX_VIEW_SIZE && v.tanLeft < v.tanRight && v.tanDown < v.tanUp;
}

bool PrepareTarget(ViewTarget* t, int width, int height, bool depth, const char* what)
{
    if (t->framebuffer != 0 && t->width == width && t->height == height)
        return true;
    if (t->framebuffer == 0)
    {
        PcGl::GenTextures(1, &t->colour);
        if (depth)
            PcGl::GenRenderbuffers(1, &t->depth);
        PcGl::GenFramebuffers(1, &t->framebuffer);
    }
    t->width = width;
    t->height = height;
    PcGl::BindTexture(GL_TEXTURE_2D, t->colour);
    PcGl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    PcGl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (depth)
    {
        PcGl::BindRenderbuffer(GL_RENDERBUFFER, t->depth);
        PcGl::RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
    PcGl::BindFramebuffer(GL_FRAMEBUFFER, t->framebuffer);
    PcGl::FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, t->colour, 0);
    if (depth)
        PcGl::FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, t->depth);
    const bool ok = PcGl::CheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    printf("[pc] mod views: %s %dx%d%s\n", what, width, height, ok ? "" : " is incomplete");
    fflush(stdout);
    return ok;
}

void Multiply(const float* a, const float* b, float* product)
{
    float out[16];
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k)
                s += a[k * 4 + row] * b[col * 4 + k];
            out[col * 4 + row] = s;
        }
    }
    memcpy(product, out, sizeof(out));
}

const float* FromCamera()
{
    if (currentView >= 0)
        return views.view[currentView].fromCamera;
    if (gameViewSet && drawFrame)
        return gameFromCamera;
    return 0;
}

void PanelMatrix(int idx, float* matrix, const float* model)
{
    const PcNativeView& v = views.view[idx];
    float proj[16];
    PcMakeFrustumMatrix(v.tanLeft * PANEL_NEAR, v.tanRight * PANEL_NEAR, v.tanDown * PANEL_NEAR,
                        v.tanUp * PANEL_NEAR, PANEL_NEAR, PANEL_FAR, proj);
    proj[2] = proj[2] * 2.0f + proj[3];
    proj[6] = proj[6] * 2.0f + proj[7];
    proj[10] = proj[10] * 2.0f + proj[11];
    proj[14] = proj[14] * 2.0f + proj[15];
    if (model == 0)
    {
        Multiply(proj, v.fromCamera, matrix);
        return;
    }
    float viewFromModel[16];
    Multiply(v.fromCamera, model, viewFromModel);
    Multiply(proj, viewFromModel, matrix);
}

void DrawPanel(int idx, bool opaque)
{
    const ViewTarget& t = outputTargets[idx];
    PcClearDepth(t.framebuffer);
    float proj[16];
    PanelMatrix(idx, proj, 0);
    if (panelReady)
    {
        int nParts = views.partCount;
        if (nParts < 0)
            nParts = 0;
        if (nParts > (int)(sizeof(views.parts) / sizeof(views.parts[0])))
            nParts = (int)(sizeof(views.parts) / sizeof(views.parts[0]));
        float cuts[4 * 4];
        int nCuts = 0;
        for (int i = 0; i < nParts; ++i)
        {
            if (views.parts[i].cut != 0)
                memcpy(cuts + 4 * nCuts++, views.parts[i].source, sizeof(float) * 4);
        }
        float matrix[16];
        if (views.panel.show != 0)
        {
            PanelMatrix(idx, matrix, views.panel.model);
            PcDrawPanel(t.framebuffer, t.width, t.height, panelTarget.colour, matrix, opaque, 0, cuts,
                        nCuts);
        }
        for (int i = 0; i < nParts; ++i)
        {
            const float* src = views.parts[i].source;
            if (!(src[0] >= 0.0f && src[2] <= 1.0f && src[0] < src[2] && src[1] >= 0.0f
                  && src[3] <= 1.0f && src[1] < src[3]))
                continue;
            PanelMatrix(idx, matrix, views.parts[i].model);
            PcDrawPanel(t.framebuffer, t.width, t.height, panelTarget.colour, matrix, opaque, src, 0,
                        0);
        }
    }
    int n = views.boxCount;
    if (n > (int)(sizeof(views.boxes) / sizeof(views.boxes[0])))
        n = (int)(sizeof(views.boxes) / sizeof(views.boxes[0]));
    if (n <= 0)
        return;
    static float matrices[192 * 16];
    static float colours[192 * 4];
    int shown = 0;
    for (int i = 0; i < n; ++i)
    {
        if (views.boxes[i].depth != 0)
            continue;
        PanelMatrix(idx, matrices + shown * 16, views.boxes[i].model);
        memcpy(colours + shown * 4, views.boxes[i].colour, sizeof(float) * 4);
        ++shown;
    }
    if (shown > 0)
        PcDrawBoxes(t.framebuffer, t.width, t.height, matrices, colours, shown);
}

bool AskMod(bool inWorld)
{
    memset(&views, 0, sizeof(views));
    views.inWorld = inWorld ? 1 : 0;
    if (inWorld)
    {
        memcpy(views.camera, cameraPos, sizeof(cameraPos));
        views.cameraYaw = cameraYaw;
        views.cameraPitch = cameraPitch;
    }
    views.aimHit = aimHit;
    views.menuOpen = !inWorld || PcInputPointerContext() ? 1 : 0;
    views.textEntry = PcTextEntryActive() ? 1 : 0;
    if (panelReady)
    {
        views.panel.width = panelTarget.width;
        views.panel.height = panelTarget.height;
    }
    PcNeutralGlState();
    PcNativeModsViewFrame(&views);
    PcBindMainTargetAfterCallback();
    aimSet = inWorld && views.aim != 0;
    if (aimSet)
    {
        memcpy(aimOrigin, views.aimOrigin, sizeof(aimOrigin));
        memcpy(aimDirection, views.aimDirection, sizeof(aimDirection));
    }
    lookSet = false;
    if (views.viewCount == 0)
        return false;
    bool good = views.viewCount >= 1 && views.viewCount <= 2;
    for (int i = 0; good && i < views.viewCount; ++i)
        good = ValidView(views.view[i]);
    if (!good)
    {
        if (!warned)
            printf("[pc] mod views: ignoring %d views, or a view of a bad size or field of view\n",
                   views.viewCount);
        warned = true;
        return false;
    }
    if (inWorld)
    {
        const float* fwd = views.view[0].fromCamera;
        lookForward[0] = -fwd[2];
        lookForward[1] = -fwd[6];
        lookForward[2] = -fwd[10];
        lookSet = true;
    }
    return true;
}

void HandOver()
{
    for (int i = 0; i < views.viewCount; ++i)
        views.view[i].texture = outputTargets[i].colour;
    PcNeutralGlState();
    PcNativeModsViewDone(&views);
    PcBindMainTargetAfterCallback();
}

void TakePanel()
{
    int w = 0;
    int h = 0;
    PcGetRenderSize(&w, &h);
    if (w < MIN_VIEW_SIZE || h < MIN_VIEW_SIZE || !PrepareTarget(&panelTarget, w, h, false, "panel"))
    {
        PcResyncRendererState();
        return;
    }
    PcCopyMainTargetTo(panelTarget.framebuffer);
    panelReady = true;
}
}

bool PcModViewsBeginFrame(Entity* camera, float partialTick)
{
    active = false;
    if (!PcNativeModsWantViews() || gApp.GetLocalPlayerCount() != 1)
        return false;
    memset(cameraPos, 0, sizeof(cameraPos));
    cameraYaw = 0.0f;
    cameraPitch = 0.0f;
    cameraHeight = 0.0f;
    if (camera != 0)
    {
        Vec3* pos = camera->getEyePosition(partialTick);
        const float eyeHeight = camera->mEyeHeightO + (camera->mEyeHeight - camera->mEyeHeightO) * partialTick;
        cameraPos[0] = pos->x;
        cameraPos[1] = pos->y - camera->getEyeHeight() + eyeHeight;
        cameraPos[2] = pos->z;
        cameraHeight = (float)(cameraPos[1] - (camera->mYo + (camera->mY - camera->mYo) * partialTick));
        cameraYaw = camera->mYRotO + (camera->mYRot - camera->mYRotO) * partialTick;
        cameraPitch = camera->mXRotO + (camera->mXRot - camera->mXRotO) * partialTick;
    }
    worldFrame = true;
    const bool own = AskMod(true);
    drawFrame = true;
    if (!own)
        return false;
    float scale = views.renderScale > 0.0f ? views.renderScale : 1.0f;
    if (scale < 0.1f)
        scale = 0.1f;
    if (scale > 1.0f)
        scale = 1.0f;
    scaled = scale < 0.999f;
    for (int i = 0; i < views.viewCount; ++i)
    {
        const int w = views.view[i].width;
        const int h = views.view[i].height;
        const int rw = scaled ? (int)(w * scale) : w;
        const int rh = scaled ? (int)(h * scale) : h;
        if (!PrepareTarget(&outputTargets[i], w, h, true, "view image")
            || (scaled && !PrepareTarget(&renderTargets[i], rw, rh, true, "view drawing")))
        {
            PcResyncRendererState();
            return false;
        }
    }
    PcResyncRendererState();
    active = true;
    return true;
}

int PcModViewsCount()
{
    return active ? views.viewCount : 0;
}

void PcModViewsBeginView(int idx)
{
    if (!active || idx < 0 || idx >= views.viewCount)
        return;
    const ViewTarget& t = scaled ? renderTargets[idx] : outputTargets[idx];
    if (PcBeginExternalTarget(t.framebuffer, t.width, t.height))
        currentView = idx;
}

void PcModViewsEndView()
{
    if (currentView < 0)
        return;
    const int idx = currentView;
    PcEndExternalTarget();
    currentView = -1;
    if (scaled)
        PcScaleTarget(renderTargets[idx].framebuffer, renderTargets[idx].width, renderTargets[idx].height, outputTargets[idx].framebuffer,
                      outputTargets[idx].width, outputTargets[idx].height);
    DrawPanel(idx, false);
}

bool PcModViewsKeepWindowView()
{
    return active && views.keepWindowView != 0;
}

void PcModViewsFinishFrame()
{
    if (!active)
        return;
    active = false;
    panelFrame = views.panel.show != 0;
    if (panelFrame)
    {
        PcClearMainTarget(0.0f, 0.0f, 0.0f, 0.0f);
        PcSetWindowUnderlay(outputTargets[0].framebuffer, outputTargets[0].width, outputTargets[0].height,
                            views.windowViewOnly != 0);
    }
    else if (views.keepWindowView == 0)
        PcCopyToMainTarget(outputTargets[0].framebuffer, outputTargets[0].width, outputTargets[0].height);
    HandOver();
}

void PcModViewsBeforePresent()
{
    KeepHudRect(&hotbarRect);
    KeepHudRect(&offhandRect);
    drawFrame = false;
    gameViewSet = false;
    const bool wasWorld = worldFrame;
    const bool wasPanel = panelFrame;
    worldFrame = false;
    panelFrame = false;
    if (wasPanel)
    {
        TakePanel();
        return;
    }
    if (wasWorld || !PcNativeModsWantViews() || gApp.GetLocalPlayerCount() > 1)
        return;
    TakePanel();
    if (!panelReady || !AskMod(false) || views.panel.show == 0)
        return;
    for (int i = 0; i < views.viewCount; ++i)
    {
        if (!PrepareTarget(&outputTargets[i], views.view[i].width, views.view[i].height, true, "view image"))
        {
            PcResyncRendererState();
            return;
        }
        PcGl::BindFramebuffer(GL_FRAMEBUFFER, outputTargets[i].framebuffer);
        PcGl::Disable(GL_SCISSOR_TEST);
        PcGl::ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        PcGl::ClearColor(MENU_BACKGROUND[0], MENU_BACKGROUND[1], MENU_BACKGROUND[2], 1.0f);
        PcGl::Clear(GL_COLOR_BUFFER_BIT);
        Minecraft* mc = Minecraft::GetInstance();
        if (mc != 0 && mc->mGameRenderer != 0 && PcBeginExternalTarget(outputTargets[i].framebuffer, outputTargets[i].width,
                                                                       outputTargets[i].height))
        {
            currentView = i;
            mc->mGameRenderer->renderModThingsInView();
            currentView = -1;
            PcEndExternalTarget();
        }
        DrawPanel(i, true);
    }
    PcBindMainTargetAfterCallback();
    HandOver();
    if (views.windowViewOnly != 0)
        PcSetWindowUnderlay(outputTargets[0].framebuffer, outputTargets[0].width, outputTargets[0].height, true);
}

void PcModViewsAimBegin(Entity* camera, float partialTick)
{
    aimEntity = 0;
    if (!aimSet || camera == 0)
        return;
    Vec3* pos = camera->getEyePosition(partialTick);
    const float yaw = (camera->mYRotO + (camera->mYRot - camera->mYRotO) * partialTick) * 0.017453292f;
    const double right[3] = {-cos(yaw), 0.0, -sin(yaw)};
    const double forward[3] = {-sin(yaw), 0.0, cos(yaw)};
    for (int a = 0; a < 3; ++a)
    {
        const double up = a == 1 ? 1.0 : 0.0;
        aimDir[a] = aimDirection[0] * right[a] + aimDirection[1] * up - aimDirection[2] * forward[a];
        aimStart[a] = aimOrigin[0] * right[a] + aimOrigin[1] * up - aimOrigin[2] * forward[a];
    }
    aimStart[0] += pos->x;
    aimStart[1] += pos->y;
    aimStart[2] += pos->z;
    aimEntity = camera;
}

void PcModViewsAimEnd(const HitResult* hit)
{
    if (aimEntity != 0)
    {
        aimHit = 0.0f;
        if (hit != 0 && hit->mType != HitResult::MISS && hit->mLocation != 0)
        {
            const double dx = hit->mLocation->x - aimStart[0];
            const double dy = hit->mLocation->y - aimStart[1];
            const double dz = hit->mLocation->z - aimStart[2];
            aimHit = (float)sqrt(dx * dx + dy * dy + dz * dz);
        }
    }
    aimEntity = 0;
}

bool PcModViewsAimFor(const Entity* entity, double* pos, double* dir)
{
    if (entity == 0 || entity != aimEntity)
        return false;
    memcpy(pos, aimStart, sizeof(aimStart));
    memcpy(dir, aimDir, sizeof(aimDir));
    return true;
}

bool PcModViewsArm(int idx, float* viewFromArm)
{
    const float* from = FromCamera();
    if (from == 0 || idx < 0 || idx > 1 || views.arms[idx].show == 0)
        return false;
    Multiply(from, views.arms[idx].model, viewFromArm);
    return true;
}

bool PcModViewsInView()
{
    return currentView >= 0;
}

bool PcModViewsShowHand()
{
    return currentView < 0 || views.showHand != 0;
}

void PcModViewsProjection(float nearDist, float farDist, float* matrix)
{
    const PcNativeView& v = views.view[currentView < 0 ? 0 : currentView];
    PcMakeFrustumMatrix(v.tanLeft * nearDist, v.tanRight * nearDist, v.tanDown * nearDist,
                        v.tanUp * nearDist, nearDist, farDist, matrix);
}

const float* PcModViewsMatrix()
{
    return views.view[currentView < 0 ? 0 : currentView].fromCamera;
}

void PcModViewsNoteHotbarSlot(int slot, float left, float top, float right, float bottom)
{
    if (left < right && top < bottom)
        GatherHudRect(slot == 9 ? &offhandRect : &hotbarRect, left, top, right, bottom);
}

int PcModViewsHudRect(int part, float* rect)
{
    const HudRect* hud = part == PC_NATIVE_HUD_HOTBAR ? &hotbarRect : part == PC_NATIVE_HUD_OFFHAND ? &offhandRect : 0;
    if (hud == 0 || !(hud->rect[2] > hud->rect[0]))
    {
        memset(rect, 0, sizeof(float) * 4);
        return 0;
    }
    memcpy(rect, hud->rect, sizeof(float) * 4);
    return 1;
}

bool PcModViewsDrawsInWorld()
{
    return currentView >= 0 || (drawFrame && views.inWorld != 0);
}

void PcModViewsSetGameView(const float* modelView)
{
    gameViewSet = false;
    if (currentView >= 0 || !drawFrame || views.inWorld == 0 || modelView == 0)
        return;
    const float yaw = cameraYaw * 0.017453292f;
    float worldFromCamera[16] = {-cosf(yaw), 0.0f, -sinf(yaw), 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                 sinf(yaw), 0.0f, -cosf(yaw), 0.0f, 0.0f, cameraHeight, 0.0f, 1.0f};
    Multiply(modelView, worldFromCamera, gameFromCamera);
    gameViewSet = true;
}

namespace
{
void RotationAlong(const Entity* entity, const float* direction, float* yRot, float* xRot)
{
    const double yaw = entity->mYRot * 0.017453292;
    const double right[3] = {-cos(yaw), 0.0, -sin(yaw)};
    const double forward[3] = {-sin(yaw), 0.0, cos(yaw)};
    double w[3];
    for (int axis = 0; axis < 3; ++axis)
        w[axis] = direction[0] * right[axis] + (axis == 1 ? direction[1] : 0.0) - direction[2] * forward[axis];
    const double len = sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2]);
    if (len < 1e-6)
    {
        *yRot = entity->mYRot;
        *xRot = entity->mXRot;
        return;
    }
    double turn = atan2(-w[0], w[2]) * 57.29577951 - entity->mYRot;
    turn -= floor((turn + 180.0) / 360.0) * 360.0;
    *yRot = (float)(entity->mYRot + turn);
    *xRot = (float)(-asin(std::max(-1.0, std::min(1.0, w[1] / len))) * 57.29577951);
}
}

bool PcModViewsAimRotation(const Entity* entity, float* yRot, float* xRot)
{
    if (!aimSet || entity == 0)
        return false;
    RotationAlong(entity, aimDirection, yRot, xRot);
    return true;
}

bool PcModViewsLookRotation(const Entity* entity, float* yRot, float* xRot)
{
    if (!lookSet || entity == 0)
        return false;
    RotationAlong(entity, lookForward, yRot, xRot);
    return true;
}

int PcModViewsBoxCount()
{
    if (FromCamera() == 0)
        return 0;
    const int cap = (int)(sizeof(views.boxes) / sizeof(views.boxes[0]));
    return views.boxCount < 0 ? 0 : (views.boxCount > cap ? cap : views.boxCount);
}

int PcModViewsWorldBox(int idx, float* viewFromBox, float* colour)
{
    if (idx < 0 || idx >= PcModViewsBoxCount() || (views.boxes[idx].depth == 0 && currentView >= 0))
        return 0;
    Multiply(FromCamera(), views.boxes[idx].model, viewFromBox);
    memcpy(colour, views.boxes[idx].colour, sizeof(float) * 4);
    return views.boxes[idx].depth == 0 ? 3 : views.boxes[idx].depth;
}

int PcModViewsTextCount()
{
    if (FromCamera() == 0)
        return 0;
    const int cap = (int)(sizeof(views.texts) / sizeof(views.texts[0]));
    return views.textCount < 0 ? 0 : (views.textCount > cap ? cap : views.textCount);
}

const char* PcModViewsText(int idx, float* viewFromText, unsigned int* colour, unsigned int* background)
{
    if (idx < 0 || idx >= PcModViewsTextCount())
        return 0;
    PcNativeViewText& text = views.texts[idx];
    text.text[sizeof(text.text) - 1] = 0;
    Multiply(FromCamera(), text.model, viewFromText);
    *colour = text.colour;
    *background = text.background;
    return text.text;
}

int PcModViewsItemCount()
{
    if (FromCamera() == 0)
        return 0;
    const int cap = (int)(sizeof(views.items) / sizeof(views.items[0]));
    return views.itemCount < 0 ? 0 : (views.itemCount > cap ? cap : views.itemCount);
}

const void* PcModViewsItem(int idx, float* viewFromItem)
{
    if (idx < 0 || idx >= PcModViewsItemCount() || views.items[idx].item == 0)
        return 0;
    Multiply(FromCamera(), views.items[idx].model, viewFromItem);
    return views.items[idx].item;
}

const float* PcModViewsFromCamera()
{
    return FromCamera();
}

void PcModViewsDraw()
{
    if (FromCamera() != 0)
        PcNativeModsViewDraw(&views);
}
