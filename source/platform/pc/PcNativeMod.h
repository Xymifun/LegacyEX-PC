#ifndef PLATFORM_PC_PCNATIVEMOD_H
#define PLATFORM_PC_PCNATIVEMOD_H

/* Native mods: a mod's native/win64.dll (or native/win32.dll for the 32-bit build) is C++ built
 * against this game's sources, with this game's compiler and flags (pc_add_native_mod in
 * tools/pc/CMakeLists.txt; tools/pc/native-example/ExampleMod.cpp is a working mod that uses all of
 * this header). The game reads the record below from the DLL's file before it loads it. It runs on
 * the build it was made for, and on other builds only when every game header it was compiled against
 * is unchanged there (the .pcabi section pc_add_native_mod adds); any other DLL is refused.
 *
 * A mod calls the game directly: it includes the game's headers and links libpcgame.a, an import
 * library for every function and variable of the game, as if the game were a DLL named pcgame.dll. The
 * game loads the mod without resolving those imports, fills them in from its own symbol table, and only
 * then starts the mod's C runtime and calls PcNativeModMain. malloc, free, operator new and delete come
 * from the game too, so memory passes freely between the game and the mod.
 */

#define PC_NATIVE_ABI 2
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

/* Drawing in the world and views of the mod's own (onViewFrame, every frame with one local player). A mod can
 * draw boxes, items, text and the player's arms in the world, and aim along a ray of its own, in the normal
 * view. It can also have the world drawn from one or two views of its own (each with its own projection and
 * placement) into images the game owns, instead of once for the window; the menus and HUD then go on a panel
 * it places. Matrices are column-major, as OpenGL takes them. Distances are in blocks. "Camera space" is the
 * player's camera turned by its yaw only: -z along the player's yaw, x to its right, y up, in blocks from
 * the player's eyes. */
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

/* A part of the panel's image on a square of its own, optionally cut out of the panel (PcNativeHost's hudRect
 * says where the HUD's parts are). Rectangles are left, top, right, bottom as shares of the image's size from
 * its top left. */
typedef struct PcNativeViewPart
{
    float source[4];        /* the part of the image to show */
    float model[16];        /* the part's square x and y from -1 to 1 (z 0, facing +z) into camera space */
    int cut;                /* 1: the panel leaves this part of the image out */
} PcNativeViewPart;

/* A solid box, for markers and pointers: the cube from -1 to 1 into camera space. depth 0: drawn over
 * the world (in the mod's views over the panel too, hidden only by the panel and other boxes); depth 1: in
 * the world, behind blocks, its sides shaded, and an alpha below 1 is see-through (drawn in the order given,
 * solid ones first); depth 2: the same, every side one colour. */
typedef struct PcNativeViewBox
{
    float model[16];
    float colour[4];        /* red, green, blue, alpha (0 to 1) */
    int depth;
} PcNativeViewBox;

/* An item in the world's depth and light, as an item frame shows it (a block fills about half the square
 * from -1 to 1): model places it in camera space. item points to a boost::shared_ptr<ItemInstance>
 * (not_null_ptr is one) the mod keeps until the frame is drawn (the next onViewFrame). */
typedef struct PcNativeViewItem
{
    float model[16];
    const void* item;
} PcNativeViewItem;

/* Text in the world, after the world boxes: model places it in camera space, its middle at the origin, x to
 * its right and y up, one unit one pixel of the game's font (a line is 8 tall). */
typedef struct PcNativeViewText
{
    float model[16];
    char text[48];          /* UTF-8 */
    unsigned int colour;    /* 0xAARRGGBB */
    unsigned int background; /* 0xAARRGGBB, a box behind it; 0 for none */
} PcNativeViewText;

/* One of the player's arms, with the skin and the item it holds, as the player model has it. */
typedef struct PcNativeViewArm
{
    int show;               /* 1 to draw it */
    float model[16];        /* in camera space: the hand's end at the origin, the arm reaching back along +z,
                               -z the way the hand points, y the back of the hand */
} PcNativeViewArm;

typedef struct PcNativeViews
{
    /* set by the game */
    int inWorld;            /* 1 in a world, 0 in menus outside one (then only views and their panel are drawn) */
    int menuOpen;           /* 1 while a menu or inventory takes the pointer (also outside a world) */
    int textEntry;          /* 1 while a line of text is being typed (chat, a sign, a name) */
    double camera[3];       /* where camera space's origin is in the world this frame (the camera at the
                               player's eyes, as drawn); 0 outside a world */
    float cameraYaw;        /* the yaw (degrees, as Entity::mYRot) camera space's -z faces; a world point w is
                               at x = (w - camera) . (-cos yaw, 0, -sin yaw), y = w.y - camera.y,
                               z = -(w - camera) . (-sin yaw, 0, cos yaw) */
    float cameraPitch;      /* the player's pitch (degrees, as Entity::mXRot; down is positive): the normal
                               view looks this far below camera space's -z */
    float aimHit;           /* how far along the aim the last pick hit a block or mob (0: nothing) */

    /* set by the mod: aiming (mining, placing, hitting) along a ray of its own instead of the player's view;
       used from the next frame */
    int aim;
    float aimOrigin[3];     /* in camera space */
    float aimDirection[3];  /* the same space, length 1 */

    /* set by the mod: things drawn in the world, in its views or else in the normal view */
    PcNativeViewArm arms[2];    /* 0 the left arm, 1 the right */
    int boxCount;
    PcNativeViewBox boxes[192];
    int itemCount;
    PcNativeViewItem items[96];
    int textCount;
    PcNativeViewText texts[128];

    /* set by the mod: views of its own; viewCount 0 (as given) keeps the normal view */
    int viewCount;          /* 1 or 2 */
    int showHand;           /* 1: draw the held item in each view, as the normal view does */
    int keepWindowView;     /* 1: also draw the normal view for the window (one more pass); 0: it shows the first
                               view */
    float renderScale;      /* the world is drawn at this share of each view's size, then scaled up to it
                               (0 means 1), for a mod that does not scale a smaller image itself */
    PcNativeView view[2];
    PcNativeViewPanel panel;
    int partCount;
    PcNativeViewPart parts[4];
    int windowViewOnly;     /* 1: the window shows the first view as it is, without the menus and HUD drawn flat
                               over it (in menus too), as a recording of the views would */
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

/* parts of the HUD, for hideHud and hudRect */
enum
{
    PC_NATIVE_HUD_CROSSHAIR = 1 << 0,
    PC_NATIVE_HUD_HINTS = 1 << 1,       /* the button hints (tooltips) shown while no menu is open */
    PC_NATIVE_HUD_HOTBAR = 1 << 2,      /* the hotbar's nine slots (hearts, food and experience sit above) */
    PC_NATIVE_HUD_OFFHAND = 1 << 3      /* the offhand slot beside the hotbar */
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

/* --- new content: PcNativeModRegister, while the game starts --- */

#define PC_NATIVE_REGISTER "PcNativeModRegister"

/* creative menu tabs for addBlock and addItem */
enum
{
    PC_NATIVE_TAB_NONE,
    PC_NATIVE_TAB_BUILDING,
    PC_NATIVE_TAB_DECORATION,
    PC_NATIVE_TAB_REDSTONE,
    PC_NATIVE_TAB_TRANSPORT,
    PC_NATIVE_TAB_MATERIALS,
    PC_NATIVE_TAB_FOOD,
    PC_NATIVE_TAB_TOOLS,
    PC_NATIVE_TAB_BREWING,
    PC_NATIVE_TAB_MISC
};

/* atlases for addTexture */
enum
{
    PC_NATIVE_ATLAS_BLOCKS = 0,
    PC_NATIVE_ATLAS_ITEMS = 1
};

/* the type a mod block entity puts in its ClientboundBlockEntityDataPacket (getUpdatePacket): players' games
   then give the packet's tag to that block entity's load() */
#define PC_NATIVE_BLOCK_ENTITY_PACKET 127

/* these return a new object of the game's class, as void* so this file stays plain C */
typedef void* (*PcNativeMakeBlock)(int id);        /* a Block (its constructor must call DerivedInit()) */
typedef void* (*PcNativeMakeItem)(int id);         /* an Item */
typedef void* (*PcNativeMakeBlockEntity)(void);    /* a BlockEntity */
typedef void* (*PcNativeMakeEntity)(void* level);  /* an Entity, for that Level */
typedef void* (*PcNativeMakeRenderer)(void* dispatcher);  /* an EntityRenderer, for the EntityRenderDispatcher */
typedef void* (*PcNativeMakeEffect)(void);         /* a MobEffect */
typedef void* (*PcNativeMakeEnchantment)(void);    /* an Enchantment */
typedef void* (*PcNativeMakeStructure)(void* level, void* levelSource);  /* a StructureFeature, for that world */
typedef void* (*PcNativeMakeStructureStart)(void);  /* an empty StructureStart, to load a saved one into */
typedef void* (*PcNativeMakeStructurePiece)(void);  /* an empty StructurePiece, to load a saved one into */
/* what spawns at pos (a BlockPos) in this structure (its StructureFeature) for that MobCategory: a
   std::vector<Biome::MobSpawnerData*>* the mod keeps, or 0 for the biome's own mobs */
typedef void* (*PcNativeStructureMobs)(void* feature, void* category, const void* pos);

/* dimensions for addStructure */
enum
{
    PC_NATIVE_OVERWORLD = 1,
    PC_NATIVE_NETHER = 2,
    PC_NATIVE_END = 4
};

/* What the game gives PcNativeModRegister. Names are the mod's own ("copper_lamp"); the game puts the mod's id
   in front ("mymod:copper_lamp"). Blocks and items get ids like content.lua's do, and worlds keep them the same
   way. Entities and block entities are saved by name, so a world can lose a mod and keep loading. Only a mod
   with "side": "both" can add blocks, items, entities, block entities, effects, enchantments, structures and
   sounds (every player's game needs them); for any other mod those return 0 and log why. */
typedef struct PcNativeRegister
{
    unsigned int abi;
    unsigned int size;
    const char* gameBuild;
    const char* modId;
    const char* modDirectory;
    void (*log)(const char* text);

    void (*addBlock)(const char* name, const char* displayName, int tab, PcNativeMakeBlock make);
    void (*addItem)(const char* name, const char* displayName, int tab, PcNativeMakeItem make);
    /* the mod's textures/FILE.png in an atlas; returns the sprite name for IconRegister::registerIcon */
    const wchar_t* (*addTexture)(int atlas, const char* file);
    /* family is a type of the game's (BlockEntityType::CONTAINER, EntityType::MONSTER...): the game's "is it a ..."
       checks see the new type as one of those. Returns the new type, which the class's GetType() must return. */
    unsigned long long (*addBlockEntity)(const char* name, unsigned long long family, PcNativeMakeBlockEntity make);
    unsigned long long (*addEntity)(const char* name, unsigned long long family, PcNativeMakeEntity make);
    void (*addEntityRenderer)(unsigned long long type, PcNativeMakeRenderer make);

    /* Replaces a game function with the mod's own, for every caller, virtual calls
       included. name is the function as `llvm-nm -C` prints it: "Zombie::doHurtTarget" when only one function
       has that name, else the full signature "Zombie::doHurtTarget(boost::shared_ptr<Entity>)"; a mangled
       name works too. replacement is a plain function with the same return type and arguments, and for a
       member function the object first: bool MyHit(Zombie* self, boost::shared_ptr<Entity> target).
       *original gets the function to call for the game's own code (or for the hook another mod put there
       first). Returns 1 when hooked, 0 when not (the log says why). Calls the compiler inlined into other
       functions of the same source file skip the hook, and a subclass's override is a function of its own. */
    int (*hookFunction)(const char* name, void* replacement, void** original);

    /* A status effect (a MobEffect, usually a subclass with its own applyEffectTick). The game makes it right
       after its own effects and registers it as "modid:name" (MobEffect::byName finds it); displayName shows
       in the inventory. Its id is 64 to 255, from a hash of the name, the same on every game with the same
       mods; saved effects also keep the name. For an icon the effect calls setIcon with one of the game's.
       Returns 1 when added, 0 when not. */
    int (*addEffect)(const char* name, const char* displayName, PcNativeMakeEffect make);
    /* An enchantment (an Enchantment subclass: its category and canEnchant pick the items, its rarity, levels
       and costs how often the enchanting table offers it). Registered as "modid:name" after the game's own, so
       the enchanting table, books, the anvil, loot and the creative menu find it. Items keep its id (1000 to
       30999, from a hash of the name). Returns 1 when added, 0 when not. */
    int (*addEnchantment)(const char* name, const char* displayName, PcNativeMakeEnchantment make);

    /* The mod's settings: config/MODID.cfg in the player's game folder (next to pcsettings.txt), "key=value"
       lines a player can edit while the game is closed. A key the file does not have yet gets defaultValue,
       and is written into the file with its comment (one line or several, without the #), so players see
       every setting. getConfig's text stays valid until setConfig changes that key. Here and in PcNativeHost. */
    const char* (*getConfig)(const char* key, const char* defaultValue, const char* comment);
    int (*getConfigInt)(const char* key, int defaultValue, const char* comment);
    void (*setConfig)(const char* key, const char* value);
    /* A key of the mod's own, listed under "Mods" in Controls (keyboard) where players rebind it; kept in
       keybinds.txt. defaultKey is a Windows virtual-key code ('G', 0x70 for F1...) or a PC_NATIVE_MOUSE_*
       button; 0 leaves it unbound. Returns the key for PcNativeHost's keyDown and keyPressed, or 0. */
    int (*addKey)(const char* name, const char* label, int defaultKey);

    /* A structure spread over chunks, as villages and temples are: a StructureFeature
       subclass. Every world generator of those dimensions (PC_NATIVE_*; normal worlds, not flat ones) makes
       one with make, and runs it as its own structures when "Generate Structures" is on: isFeatureChunk picks
       the chunks that start one, createStructureStart lays out its pieces (kept in the world's saved data
       under getFeatureName, best "modid_name"), postProcess builds them chunk by chunk.
       Level::findNearestMapFeature and isPosInFeature find it as "modid:name" (explorer maps, the mod's own
       items). mobs (or 0) picks what spawns inside it. Returns 1 when added. */
    int (*addStructure)(const char* name, int dimensions, PcNativeMakeStructure make, PcNativeStructureMobs mobs);
    /* The mod's StructureStart and StructurePiece classes, saved by name: what their GetType() must return
       (as EStructureStart / EStructurePiece). make returns an empty one the game loads saved data into.
       0 when not added. */
    unsigned int (*addStructureStart)(const char* name, PcNativeMakeStructureStart make);
    unsigned int (*addStructurePiece)(const char* name, PcNativeMakeStructurePiece make);

    /* A sound of the mod's own: a SoundEvent named "modid:name" that plays the mod's
       sounds/FILE instead of one of the game's. files is one file or several split by ';' ("step1.ogg;
       step2.ogg"), one picked at random each time: .wav (PCM or ADPCM), .ogg (Vorbis) or .mp3, mono or
       stereo, up to 16 MB each (an ogg or mp3 up to about 3 minutes, kept decoded). range is how far
       it carries in blocks at volume 1 (0 for the game's usual 16). Returns the SoundEvent* to play like the
       game's (Level::playSound sends it to every player near it), or 0. A looping or moving sound
       (SoundInstance) plays once, where it starts. */
    void* (*addSound)(const char* name, const char* files, float range);
} PcNativeRegister;

/* mouse buttons for addKey */
enum
{
    PC_NATIVE_MOUSE_LEFT = 0x101,
    PC_NATIVE_MOUSE_RIGHT = 0x102,
    PC_NATIVE_MOUSE_MIDDLE = 0x103,
    PC_NATIVE_MOUSE_4 = 0x104,
    PC_NATIVE_MOUSE_5 = 0x105
};

/* optional, as well as PcNativeModMain: 0 when it registered; anything else and the mod is not loaded */
typedef int (*PcNativeModRegisterFn)(const PcNativeRegister* reg);

/* --- runtime hooks (PcNativeHost) --- */

typedef void* (*PcNativeMakeScreen)(int pad, void* data, void* layer);  /* a UIScene, for that UILayer */
/* player's game: the server opened a container of this type for player (a MultiplayerLocalPlayer); make the menu
   (player->mContainerMenu, with m_containerId) and show the screen. Return 1 if it did. */
typedef int (*PcNativeOpenContainer)(void* player, int containerId, int size, const wchar_t* title, int entityId,
                                     void* user);
typedef void (*PcNativeChunkCallback)(void* level, int chunkX, int chunkZ, int dimension, void* user);
typedef void (*PcNativeMessageCallback)(void* player, const char* channel, const void* data, int size, void* user);

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
     * what it draws in the world and its views, or leaves viewCount at 0 for the normal view. Outside a world
     * (inWorld 0) it is called before the window's image is shown, and only the panel is drawn in the views.
     * Not called on a dedicated server. */
    void (*onViewFrame)(PcNativeViewCallback callback, void* user);
    /* main thread, after the mod's views were drawn and before the HUD and menus; the view textures are set
     * (only in frames with views) */
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

    /* main thread: a screen of the mod's own. Returns its id, for gConsoleUIController.NavigateToScene(pad,
       (EUIScene)id, data, ...). container 1: it counts as a container menu (as a chest's does). */
    int (*addScreen)(PcNativeMakeScreen make, int container);
    /* a container type of the mod's own: the number (100 to 199) its InteractionObject::getTypeId returns on the
       host. The same name gives the same number on every game. */
    int (*addContainerType)(const char* name, PcNativeOpenContainer open, void* user);
    /* host: a new chunk was just decorated (ores, trees, structures); place the mod's own things in it. Runs on
       the thread that made the chunk. */
    void (*onChunkDecorated)(PcNativeChunkCallback callback, void* user);
    /* messages between the mod on the host and on the players' games: channel up to 32 characters, data up to
       30000 bytes. Channels are the mod's own: a message only reaches the same mod on the other side, so two
       mods can use the same channel name. onMessage gets the sending ServerPlayer on the host, and 0 on a
       player's game. sendToHost drops the message while the player is not connected to a world (in menus, or
       while one loads). */
    void (*sendToHost)(const char* channel, const void* data, int size);
    void (*sendToPlayer)(void* serverPlayer, const char* channel, const void* data, int size);  /* 0: everyone */
    void (*onMessage)(PcNativeMessageCallback callback, void* user);

    /* as in PcNativeRegister */
    const char* (*getConfig)(const char* key, const char* defaultValue, const char* comment);
    int (*getConfigInt)(const char* key, int defaultValue, const char* comment);
    void (*setConfig)(const char* key, const char* value);
    /* host: bytes the mod keeps with the world it runs (from onHostStart until onHostStop), saved in its
       level.dat whenever the world saves. getWorldData gives them and their size (0 and size 0 if there are
       none); the pointer stays valid until the next setWorldData. setWorldData replaces them (size 0 removes
       them; up to 4 MB). On a player's game there is no world of the host's: 0. */
    const void* (*getWorldData)(int* size);
    void (*setWorldData)(const void* data, int size);
    /* host: the same for one player (a ServerPlayer), saved with the player and kept through death and
       changing dimension */
    const void* (*getPlayerData)(void* serverPlayer, int* size);
    void (*setPlayerData)(void* serverPlayer, const void* data, int size);
    /* main thread: a key from addKey. keyDown is 1 while it is held during play (not in menus, chat or while
       typing); keyPressed is how many times it was pressed during play since the last call. */
    int (*keyDown)(int key);
    int (*keyPressed)(int key);
    /* main thread: hide parts of the HUD (PC_NATIVE_HUD_CROSSHAIR and _HINTS) until called again; 0 shows them.
       A part stays hidden while any mod hides it. */
    void (*hideHud)(int parts);
    /* main thread: where one part of the HUD (PC_NATIVE_HUD_HOTBAR or _OFFHAND) was drawn last frame, as left,
       top, right, bottom shares of the window's image from its top left. 0 (and the rect all 0) when it was
       not drawn. */
    int (*hudRect)(int part, float* rect);
    /* main thread: called while the mod's things are drawn in the world, in each of its views and in the
       normal view (and in its views while only menus show), after its arms and items and before its boxes (so
       see-through boxes show over what it draws). The modelview is set
       so that what the mod draws is in camera space, lit as entities are; draw with the game's own classes
       (ModelPart::render, Tesselator, GlStateManager) and leave the state as it was. */
    void (*onViewDraw)(PcNativeViewCallback callback, void* user);
    /* main thread: the mod's textures/FILE (a PNG) as a texture for GlStateManager::bindTexture, loaded once;
       0 when it can not be read */
    int (*loadTexture)(const char* file);
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
