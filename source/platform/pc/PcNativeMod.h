#ifndef PLATFORM_PC_PCNATIVEMOD_H
#define PLATFORM_PC_PCNATIVEMOD_H

/* Native mods (tools/pc/README.md, Native mods): a mod's native/win64.dll (or native/win32.dll for the
 * 32-bit build) is C++ built against this game's sources, with this game's compiler and flags
 * (pc_add_native_mod in tools/pc/CMakeLists.txt). It runs only on the exact build it was made for: the
 * game reads the record below from the DLL's file before it loads it, and refuses a DLL made for
 * another build.
 *
 * A mod calls the game directly: it includes the game's headers and links libpcgame.a, an import
 * library for every function and variable of the game, as if the game were a DLL named pcgame.dll. The
 * game loads the mod without resolving those imports, fills them in from its own symbol table, and only
 * then starts the mod's C runtime and calls PcNativeModMain. malloc, free, operator new and delete come
 * from the game too, so memory passes freely between the game and the mod.
 */

#define PC_NATIVE_ABI 1
#define PC_NATIVE_ENTRY "PcNativeModMain"

#ifdef __cplusplus
extern "C" {
#endif

/* In the DLL's ".pcmod" section; PC_NATIVE_MOD_RECORD makes it. */
typedef struct PcNativeModRecord
{
    char magic[8];          /* "PCNATIVE" */
    unsigned int abi;       /* PC_NATIVE_ABI */
    unsigned int pointerBits;
    char gameBuild[24];     /* the game version it was built for (16 hex digits) */
} PcNativeModRecord;

typedef void (*PcNativeCallback)(void* user);

/* Views of the mod's own: the world is drawn from a pair of views (a first and a second, each with its own
 * projection and placement), into images the game owns, instead of once for the window. Matrices are
 * column-major, as OpenGL takes them. Distances are in blocks. "Camera space" is the player's camera
 * before a view's matrix: -z along the player's yaw, y up, in blocks from the player's eyes. */
typedef struct PcNativeView
{
    int width;              /* set by the mod: the view's image in pixels */
    int height;
    float tanLeft;          /* set by the mod: the view's field of view as tangents from its direction; */
    float tanRight;         /* left and down are negative */
    float tanDown;
    float tanUp;
    float fromCamera[16];   /* set by the mod: from camera space to this view, applied before the game's own
                               camera (the player's position, yaw and pitch) */
    unsigned int texture;   /* set by the game before onViewDone: the OpenGL texture holding the view's image */
} PcNativeView;

/* The menus and HUD on a flat panel in each view: what the window would show over the world. */
typedef struct PcNativeViewPanel
{
    int show;               /* set by the mod: 1 to draw the panel */
    float model[16];        /* set by the mod: the panel's square x and y from -1 to 1 (z 0, facing +z) into
                               camera space */
    int width;              /* set by the game: the size of the window's image the panel shows, */
    int height;             /* for its shape (0 until the first frame) */
} PcNativeViewPanel;

/* A part of the panel's image on a square of its own (the HUD's hotbar somewhere else), optionally cut out
 * of the panel. Rectangles are left, top, right, bottom as shares of the image's size from its top left. */
typedef struct PcNativeViewPart
{
    float source[4];        /* the part of the image to show */
    float model[16];        /* the part's square x and y from -1 to 1 (z 0, facing +z) into camera space */
    int cut;                /* 1: the panel leaves this part of the image out */
} PcNativeViewPart;

/* A solid box drawn in each view, for markers and pointers: the cube from -1 to 1 into camera space. Boxes
 * hide behind the panel and each other, not behind the world; with depth 1 a box is drawn in the world
 * instead (behind blocks, with the items), its sides shaded, and the panel draws over it; there an alpha
 * below 1 is see-through and drawn in the order given (solid ones first). With depth 2 the same, but every
 * side the same colour (not shaded). */
typedef struct PcNativeViewBox
{
    float model[16];
    float colour[4];        /* red, green, blue, alpha (0 to 1) */
    int depth;
} PcNativeViewBox;

/* An item drawn in each view in the world's depth and light, as an item frame shows it (a block fills
 * about half the square from -1 to 1): model places it in camera space. item points to a
 * boost::shared_ptr<ItemInstance> (not_null_ptr is one) the mod keeps until onViewDone. */
typedef struct PcNativeViewItem
{
    float model[16];
    const void* item;
} PcNativeViewItem;

/* Text drawn in each view in the world, after the world boxes: model places it in camera space, its middle at
 * the origin, x to its right and y up, one unit one pixel of the game's font (a line is 8 tall). */
typedef struct PcNativeViewText
{
    float model[16];
    char text[48];          /* UTF-8 */
    unsigned int colour;    /* 0xAARRGGBB */
    unsigned int background; /* 0xAARRGGBB, a box behind it; 0 for none */
} PcNativeViewText;

typedef struct PcNativeViews
{
    int inWorld;            /* set by the game: 1 in a world, 0 in menus outside one (only the panel is drawn) */
    int menuOpen;           /* set by the game: 1 while a menu or inventory takes the pointer (also outside a world) */
    int viewCount;          /* set by the mod: 2 to draw this frame from the two views, 0 (as given) for the
                               normal view */
    int showHand;           /* 1: draw the held item in each view, as the normal view does */
    int keepWindowView;     /* 1: also draw the normal view for the window (a third pass); 0: it shows the first
                               view */
    float renderScale;      /* the world is drawn at this share of each view's size, then scaled up to it
                               (0 means 1), for a mod that does not scale a smaller image itself */
    PcNativeViewPanel panel;
    int aim;                /* set by the mod: 1 to aim (mine, place, hit) along aimOrigin and aimDirection instead
                               of the player's view; used from the next frame */
    float aimOrigin[3];     /* in camera space, in blocks */
    float aimDirection[3];  /* the same space, length 1 */
    float aimHit;           /* set by the game: how far along the aim the last pick hit a block or mob (0: nothing) */
    int hideCrosshair;      /* set by the mod: 1 to hide the HUD's crosshair (it marks the view's centre) */
    int showArms;           /* set by the mod: 1 to draw the player's arms (with the skin) and held items at hands */
    int handValid[2];       /* set by the mod: 0 left, 1 right */
    float hands[2][16];     /* set by the mod: the hands in camera space; -z points forward, the arm's hand end
                               is at the origin and the arm reaches back along +z */
    int boxCount;           /* set by the mod: how many of boxes to draw */
    PcNativeViewBox boxes[192];
    PcNativeView view[2];
    float hudRect[4];       /* set by the game: the HUD's hotbar slots in the panel's image, as a part's source
                               (all 0 while the HUD shows no hotbar); hearts, food and experience sit above */
    int partCount;          /* set by the mod: how many of parts to draw */
    PcNativeViewPart parts[4];
    float hudOffhandRect[4]; /* set by the game: the HUD's offhand slot, as hudRect (all 0 while not shown) */
    int hideHints;          /* set by the mod: 1 to hide the button hints (tooltips) while no menu is open */
    double camera[3];       /* set by the game: where camera space's origin is in the world this frame (the
                               camera at the player's eyes, as drawn); 0 outside a world */
    float cameraYaw;        /* set by the game: the yaw (degrees, as Entity::mYRot) camera space's -z faces;
                               x is to its right and y up, so a world point w is at
                               x = (w - camera) . (-cos yaw, 0, -sin yaw), y = w.y - camera.y,
                               z = -(w - camera) . (-sin yaw, 0, cos yaw) */
    int itemCount;          /* set by the mod: how many of items to draw */
    PcNativeViewItem items[96];
    int textEntry;          /* set by the game: 1 while a line of text is being typed (chat, a sign, a name) */
    int textCount;          /* set by the mod: how many of texts to draw */
    PcNativeViewText texts[128];
} PcNativeViews;

typedef void (*PcNativeViewCallback)(PcNativeViews* views, void* user);

/* One more controller for the first player, as the Wii U GamePad's buttons the game reads: in a world
 * A jump, B drop, X inventory, Y crafting, L and R the hotbar, ZL use, ZR attack, STICK_R sneak,
 * STICK_L camera view, PLUS pause, MINUS players; in menus A select, B back, L and R tabs. */
enum
{
    PC_NATIVE_PAD_A = 1 << 0,
    PC_NATIVE_PAD_B = 1 << 1,
    PC_NATIVE_PAD_X = 1 << 2,
    PC_NATIVE_PAD_Y = 1 << 3,
    PC_NATIVE_PAD_L = 1 << 4,
    PC_NATIVE_PAD_R = 1 << 5,
    PC_NATIVE_PAD_ZL = 1 << 6,
    PC_NATIVE_PAD_ZR = 1 << 7,
    PC_NATIVE_PAD_STICK_L = 1 << 8,
    PC_NATIVE_PAD_STICK_R = 1 << 9,
    PC_NATIVE_PAD_PLUS = 1 << 10,
    PC_NATIVE_PAD_MINUS = 1 << 11,
    PC_NATIVE_PAD_UP = 1 << 12,
    PC_NATIVE_PAD_DOWN = 1 << 13,
    PC_NATIVE_PAD_LEFT = 1 << 14,
    PC_NATIVE_PAD_RIGHT = 1 << 15,
    PC_NATIVE_PAD_CHAT = 1 << 16,   /* not a GamePad button: opens the chat line, as T does */
    PC_NATIVE_PAD_SNEAK = 1 << 17   /* not a GamePad button: sneaks while held, as Shift does (STICK_R toggles) */
};

typedef struct PcNativePad
{
    unsigned int buttons;   /* PC_NATIVE_PAD_*, held */
    float leftX;            /* sticks, -1 to 1, right and up positive */
    float leftY;
    float rightX;
    float rightY;
    int pointer;            /* 1: pointerX and pointerY point into the window's image (0 to 1 across and down),
                               used in menus and inventories as the mouse is; ZL and ZR are not used there */
    float pointerX;
    float pointerY;
    int pointerPress;       /* 1 while pressed, as the left mouse button */
} PcNativePad;

/* What the game gives PcNativeModMain. Fields are only ever added at the end; size says how many. */
typedef struct PcNativeHost
{
    unsigned int abi;
    unsigned int size;
    const char* gameBuild;
    const char* modId;
    const char* modDirectory;   /* the mod's folder, for its own files */
    int dedicatedServer;

    /* "[pc] native ID: text" in the log */
    void (*log)(const char* text);
    /* the address of a game function or variable by its (mangled) symbol name, or 0 */
    void* (*findSymbol)(const char* name);

    /* main thread, once a frame (on a dedicated server too) */
    void (*onFrame)(PcNativeCallback callback, void* user);
    /* server thread: a world starts (after it loaded), each server tick, the world stops */
    void (*onHostStart)(PcNativeCallback callback, void* user);
    void (*onHostTick)(PcNativeCallback callback, void* user);
    void (*onHostStop)(PcNativeCallback callback, void* user);
    /* main thread, each frame before the world is drawn, with one local player in a world: the mod fills in
     * its views, or leaves viewCount at 0. Outside a world (inWorld 0) it is called before the window's
     * image is shown, and only the panel is drawn in the views. Not called on a dedicated server. */
    void (*onViewFrame)(PcNativeViewCallback callback, void* user);
    /* main thread, after both views were drawn and before the HUD and menus; the view textures are set */
    void (*onViewDone)(PcNativeViewCallback callback, void* user);
    /* main thread, once a frame: the controller's state until the next call (zeroed: no input) */
    void (*setPad)(const PcNativePad* pad);
    /* main thread, while the HUD is drawn (in a world, one local player): draw with drawRect and drawText in
     * 640 x 360 units over the whole window image, as the debug screen and mod scripts do */
    void (*onHud)(PcNativeCallback callback, void* user);
    void (*drawRect)(int x1, int y1, int x2, int y2, unsigned int argb);
    void (*drawText)(const char* utf8, int x, int y, unsigned int argb);  /* with a shadow; 9 units high */
    int (*textWidth)(const char* utf8);                                    /* in the same units */
    /* main thread: while on, a container the server opens for the first player (chest, furnace, brewing
     * stand, hopper, dispenser, dropper, shulker box, and the crafting table's 3x3 grid menu,
     * CraftingMenu) gets its menu (mContainerMenu) but no screen; the
     * mod reads its slots and clicks them (MultiPlayerGameMode::handleInventoryMouseClick) and closes it
     * (closeContainer). Other menus still show. */
    void (*setHideMenus)(int on);
    /* main thread: a key of the keyboard pressed (down 1) or let go (0), as Windows' virtual-key codes, and
     * a typed character, as the keyboard gives them to the game */
    void (*sendKey)(int key, int down);
    void (*sendChar)(unsigned int character);
} PcNativeHost;

/* The mod's entry point: 0 when it started; anything else and the game unloads it. */
typedef int (*PcNativeModMainFn)(const PcNativeHost* host);

#ifdef __cplusplus
}
#endif

#ifdef PC_GAME_BUILD
#define PC_NATIVE_MOD_RECORD                                                                                    \
    extern "C" __attribute__((section(".pcmod"), used)) const PcNativeModRecord pcNativeModRecord = {         \
        {'P', 'C', 'N', 'A', 'T', 'I', 'V', 'E'}, PC_NATIVE_ABI, (unsigned int)(sizeof(void*) * 8), PC_GAME_BUILD}; \
    extern "C" __declspec(dllexport) int PcNativeModMain(const PcNativeHost* host)
#endif

#endif
