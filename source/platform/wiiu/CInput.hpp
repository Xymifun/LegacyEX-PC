#ifndef PLATFORM_WIIU_CINPUT_HPP
#define PLATFORM_WIIU_CINPUT_HPP

#include <stdint.h>

struct _STRING_VERIFY_RESPONSE;
struct FSClient;
struct VPADStatus;
struct KPADStatus;

class C_4JInput
{
public:
    enum ETouchType
    {
        ETouchType_0 = 0,
        ETouchType_1 = 1,
        ETouchType_2 = 2
    };

    enum EKeyboardMode
    {
        EKeyboardMode_0 = 0
    };

    typedef int (*ErrorViewerCallback)(void*);
    typedef int (*InputCallback)(void*);
    typedef int (*KeyboardCallback)(void*, bool);

    virtual void Initialise(int controllerCount, unsigned char gameMapCount, unsigned char menuMapCount) = 0;
    virtual void Tick() = 0;
    virtual void SetDeadzoneAndMovementRange(unsigned int deadzone, unsigned int maximum,
                                             unsigned int movementRange) = 0;
    virtual void SetGameControllerTypeJoypadMaps(int controllerType, unsigned char map, unsigned int value) = 0;
    virtual void SetGameJoypadMaps(int pad, unsigned char map, unsigned int value) = 0;
    virtual unsigned int GetGameJoypadMaps(int pad, unsigned char map) = 0;
    virtual void SetJoypadMapArraySetupComplete() = 0;
    virtual void SetJoypadSensitivity(int pad, float sensitivity) = 0;
    virtual unsigned int GetValue(int value, unsigned char type, bool flags) = 0;
    virtual void SetFakePadValue(int pad, unsigned char map) = 0;
    virtual bool ButtonPressed(int pad, unsigned char map, unsigned int mask) = 0;
    virtual bool ButtonReleased(int pad, unsigned char map) = 0;
    virtual bool ButtonDown(int pad, unsigned char map) = 0;
    virtual void SetJoypadStickAxisMap(int axis, unsigned int negativeMask, unsigned int positiveMask) = 0;
    virtual void SetJoypadStickTriggerMap(int trigger, unsigned int mask, unsigned int threshold) = 0;
    virtual void SetKeyRepeatRate(float initialDelay, float repeatDelay) = 0;
    virtual void SetDebugSequence(const char* sequence, InputCallback callback, void* context) = 0;
    virtual double GetIdleSeconds(int pad) = 0;
    virtual unsigned char IsPadConnected(int pad) = 0;
    virtual void SetCircleCrossSwapped(bool swapped) = 0;
    virtual unsigned char IsCircleCrossSwapped() = 0;
    virtual void MapTouchInput(int pad, unsigned int mask) = 0;
    virtual float GetJoypadStick_Menu_LX(int value) = 0;
    virtual float GetJoypadStick_Menu_LY(int value) = 0;
    virtual float GetJoypadStick_Menu_RX(int value) = 0;
    virtual float GetJoypadStick_Menu_RY(int value) = 0;
    virtual unsigned char GetJoypadLTrigger_Menu(int value) = 0;
    virtual unsigned char GetJoypadRTrigger_Menu(int value) = 0;
    virtual float GetJoypadStick_LX(int pad, bool checkMenu) = 0;
    virtual float GetJoypadStick_LY(int pad, bool checkMenu) = 0;
    virtual float GetJoypadStick_RX(int pad, bool checkMenu) = 0;
    virtual float GetJoypadStick_RY(int pad, bool checkMenu) = 0;
    virtual unsigned char GetJoypadLTrigger(int pad, bool raw) = 0;
    virtual unsigned char GetJoypadRTrigger(int pad, bool raw) = 0;
    virtual unsigned short* GetTouchPadData(int pad, ETouchType touchType, bool previous) = 0;
    virtual void SetMenuDisplayed(int pad, bool displayed) = 0;
    virtual unsigned int RequestKeyboard(const wchar_t* title, const wchar_t* initialText, unsigned int pad,
                                         unsigned int maximumLength, KeyboardCallback callback, void* context,
                                         EKeyboardMode mode) = 0;
    virtual void RequestErrorViewer(unsigned int value, unsigned int type, ErrorViewerCallback callback,
                                    void* context) = 0;
    virtual void RequestErrorViewer(unsigned int value, wchar_t* title, wchar_t* message, ErrorViewerCallback callback,
                                    void* context) = 0;
    virtual void DestroyErrorViewer() = 0;
    virtual void DestroyKeyboard() = 0;
    virtual void GetText(wchar_t* text, int length) = 0;
    virtual void SetLocalMultiplayer(bool enabled) = 0;
    virtual unsigned int IsLocalMultiplayerAvailable() = 0;
    virtual unsigned int UsingRemoteVita() = 0;
    virtual unsigned int VerifyStrings(wchar_t** strings, int count, int (*callback)(void*, _STRING_VERIFY_RESPONSE*),
                                       void* context) = 0;
    virtual void CancelQueuedVerifyStrings(int (*callback)(void*, _STRING_VERIFY_RESPONSE*), void* context) = 0;
    virtual void CancelAllVerifyInProgress() = 0;
    virtual unsigned int LockPad(int pad, int controllerId) = 0;
    virtual int UnlockPad(int pad) = 0;
    virtual unsigned int IsPadLocked(int pad) = 0;
    virtual int GetControllerID(int pad) = 0;
    virtual unsigned char IsPadDRCController(int pad) = 0;
    virtual unsigned int IsPadClassicController(int pad) = 0;
    virtual unsigned char IsDRCConnected() = 0;
    virtual void AcquiredForeground() = 0;
};

class CInput : public C_4JInput
{
public:
    enum eErrorViewerProcessState
    {
        ERROR_VIEWER_NONE = 0,
        ERROR_VIEWER_REQUESTED = 1,
        ERROR_VIEWER_ACTIVE = 2,
        ERROR_VIEWER_DESTROYED = 3,
        ERROR_VIEWER_DISAPPEARING = 4
    };

    enum eKeyboardProcessState
    {
        eKeyboardProcessState_0 = 0,
        eKeyboardProcessState_1 = 1,
        eKeyboardProcessState_2 = 2
    };

    CInput();
    class JOYPAD
    {
    public:
        unsigned char sampleIndex;
        unsigned char previousSampleIndex;
        unsigned char connected;
        unsigned char disconnected;
        unsigned char newlyConnected;
        unsigned char connectionPadding[3];
        unsigned int touchValue;
        unsigned int rawValue;
        unsigned int fakeValue;
        unsigned int fakeTimer;
        unsigned int previousValue;
        unsigned int pressedValue;
        unsigned int releasedValue;
        int rawRightStickX;
        int rawRightStickY;
        int rawLeftStickX;
        int rawLeftStickY;
        int leftStickX;
        int leftStickY;
        int rightStickX;
        int rightStickY;
        float leftX;
        float leftY;
        float rightX;
        float rightY;
        unsigned char hasTouch;
        unsigned char touchPressed;
        unsigned char valuePadding[2];
        float* leftXSource;
        float* leftYSource;
        float* rightXSource;
        float* rightYSource;
        float sensitivity;
        unsigned char* leftTriggerSource;
        unsigned char* rightTriggerSource;
        signed char statusIndex;
        unsigned char statusPadding[3];
        union STATUS_STORAGE
        {
            struct VPAD_STORAGE
            {
                VPADStatus* statuses;
                void* padding;
            } vpad;
            struct KPAD_STORAGE
            {
                void* padding;
                KPADStatus* statuses;
            } kpad;
        } statusStorage;
        int controllerId;
        int lockedController;
        void setConnected(bool connected);
    };

    struct TIME_VALUE
    {
        unsigned int high;
        unsigned int low;
    };

    struct BUTTON_TIME
    {
        TIME_VALUE lastUpdate;
        TIME_VALUE elapsed;
        float heldTime;
        float repeatTime;
        unsigned char repeated;
        unsigned char padding[7];
    };
    unsigned int reservedBeforeJoypads[3];
    JOYPAD joypads[5];
    unsigned int reservedAfterJoypads;
    int padRemap[4];
    unsigned int controllerCount;
    unsigned int controllerOptions;
    unsigned int deadzone;
    unsigned int maximumInput;
    unsigned int movementRange;
    unsigned int movementSpan;
    float movementSpanFloat;
    unsigned char controllerConfiguration[3];
    unsigned char reservedAfterControllerConfiguration;
    unsigned int* gameJoypadMaps[5];
    unsigned int* alternateJoypadMaps[5];
    unsigned char joypadMapsReady;
    unsigned char reservedAfterJoypadMaps[7];
    unsigned char menuDisplayed[4];
    unsigned int inputTick;
    float timeScale;
    float initialRepeatDelay;
    float repeatDelay;
    unsigned char circleCrossSwapped;
    unsigned char reservedBeforeButtonTimes[7];
    BUTTON_TIME buttonTimes[5][24];
    TIME_VALUE idleStartTimes[5];
    char* debugSequence;
    unsigned int debugSequenceLength;
    InputCallback debugSequenceCallback;
    void* debugSequenceContext;
    unsigned int reservedAfterDebugSequence[2];
    bool localMultiplayer;
    unsigned char reservedBeforeKeyboardState[3];
    eKeyboardProcessState keyboardState;
    FSClient* keyboardFileClient;
    void* keyboardErrorViewerWorkMemory;
    C_4JInput::EKeyboardMode keyboardMode;
    unsigned int reservedAfterKeyboardMode[2];
    KeyboardCallback keyboardCallback;
    void* keyboardContext;
    int keyboardMaximumLength;
    int keyboardPad;
    wchar_t keyboardText[258];
    unsigned int keyboardLanguage;
    unsigned int reservedAfterKeyboardLanguage;
    unsigned int keyboardControllerType;
    unsigned int keyboardRegion;
    unsigned char destroyKeyboardRequested;
    unsigned char keyboardVisible;
    unsigned char reservedAfterKeyboardFlags[2];
    ErrorViewerCallback errorViewerCallback;
    void* errorViewerContext;
    unsigned char errorViewerVisible;
    unsigned char reservedBeforeErrorViewerState[3];
    eErrorViewerProcessState errorViewerState;
    FSClient* errorViewerFileClient;
    unsigned int reservedBeforeErrorViewerController;
    unsigned int errorViewerController;
    unsigned int errorViewerCode;
    wchar_t* errorViewerTitle;
    wchar_t* errorViewerMessage;
    unsigned int reservedAtEnd;
    void InitTime();
    static void ConnectCallback(int channel, int result);
    virtual void Initialise(int controllerCount, unsigned char gameMapCount, unsigned char menuMapCount);
    void ReadAllPads();
    void ClearJoypadValues(JOYPAD* value);
    void JoypadConnected(int value);
    void JoypadRemoved(int value);
    void SetDRCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetJoypadValuesDRC(JOYPAD* joypad);
    void GetStartTime(int value, int type);
    void UpdateTime(int value, int type);
    bool UpdateDRCPad();
    void SetClassicJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetFreestyleJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetURCCJoypadValue(JOYPAD* value, unsigned int type, unsigned int flags);
    void SetJoypadValues(JOYPAD* joypad);
    bool UpdateJoypads();
    virtual void Tick();
    virtual void SetDeadzoneAndMovementRange(unsigned int deadzone, unsigned int maximum, unsigned int movementRange);
    virtual void SetGameControllerTypeJoypadMaps(int controllerType, unsigned char map, unsigned int value);
    bool RemapIpad(int& pad);
    virtual void SetGameJoypadMaps(int pad, unsigned char map, unsigned int value);
    virtual unsigned int GetGameJoypadMaps(int pad, unsigned char map);
    virtual void SetJoypadMapArraySetupComplete();
    virtual void SetJoypadSensitivity(int pad, float sensitivity);
    unsigned int only_one_bit_set(unsigned int value);
    unsigned int allowInput(int value, unsigned char type, unsigned int flags);
    virtual unsigned int GetValue(int value, unsigned char type, bool flags);
    virtual void SetFakePadValue(int pad, unsigned char map);
    virtual bool ButtonPressed(int pad, unsigned char map, unsigned int mask);
    virtual bool ButtonReleased(int pad, unsigned char map);
    virtual bool ButtonDown(int pad, unsigned char map);
    virtual void SetJoypadStickAxisMap(int axis, unsigned int negativeMask, unsigned int positiveMask);
    virtual void SetJoypadStickTriggerMap(int trigger, unsigned int mask, unsigned int threshold);
    virtual void SetKeyRepeatRate(float initialDelay, float repeatDelay);
    virtual void SetDebugSequence(const char* sequence, InputCallback callback, void* context);
    virtual double GetIdleSeconds(int pad);
    virtual unsigned char IsPadConnected(int pad);
    virtual void SetCircleCrossSwapped(bool swapped);
    virtual unsigned char IsCircleCrossSwapped();
    virtual void MapTouchInput(int pad, unsigned int mask);
    virtual float GetJoypadStick_Menu_LX(int value);
    virtual float GetJoypadStick_Menu_LY(int value);
    virtual float GetJoypadStick_Menu_RX(int value);
    virtual float GetJoypadStick_Menu_RY(int value);
    virtual unsigned char GetJoypadLTrigger_Menu(int value);
    virtual unsigned char GetJoypadRTrigger_Menu(int value);
    virtual float GetJoypadStick_LX(int pad, bool checkMenu);
    virtual float GetJoypadStick_LY(int pad, bool checkMenu);
    virtual float GetJoypadStick_RX(int pad, bool checkMenu);
    virtual float GetJoypadStick_RY(int pad, bool checkMenu);
    virtual unsigned char GetJoypadLTrigger(int pad, bool raw);
    virtual unsigned char GetJoypadRTrigger(int pad, bool raw);
    virtual unsigned short* GetTouchPadData(int pad, C_4JInput::ETouchType touchType, bool previous);
    virtual void SetMenuDisplayed(int pad, bool displayed);
    virtual unsigned int RequestKeyboard(const wchar_t* title, const wchar_t* initialText, unsigned int pad,
                                         unsigned int maximumLength, KeyboardCallback callback, void* context,
                                         C_4JInput::EKeyboardMode mode);
    virtual void RequestErrorViewer(unsigned int value, unsigned int type, ErrorViewerCallback callback, void* context);
    virtual void RequestErrorViewer(unsigned int value, wchar_t* title, wchar_t* message, ErrorViewerCallback callback,
                                    void* context);
    virtual void DestroyErrorViewer();
    virtual void DestroyKeyboard();
    virtual void GetText(wchar_t* text, int length);
    virtual void SetLocalMultiplayer(bool enabled);
    virtual unsigned int IsLocalMultiplayerAvailable();
    virtual unsigned int UsingRemoteVita();
    virtual unsigned int VerifyStrings(wchar_t** strings, int count, int (*callback)(void*, _STRING_VERIFY_RESPONSE*),
                                       void* context);
    virtual void CancelQueuedVerifyStrings(int (*callback)(void*, _STRING_VERIFY_RESPONSE*), void* context);
    virtual void CancelAllVerifyInProgress();
    virtual unsigned int LockPad(int pad, int controllerId);
    virtual int UnlockPad(int pad);
    virtual unsigned int IsPadLocked(int pad);
    virtual int GetControllerID(int pad);
    virtual unsigned char IsPadDRCController(int pad);
    virtual unsigned int IsPadClassicController(int pad);
    virtual unsigned char IsDRCConnected();
    virtual void AcquiredForeground();
    bool IsSet(int value, unsigned char type);
    intptr_t GetVPadStatus();
    intptr_t GetKPadStatus(int value);
    unsigned char IsShowingOnscreenKeyboard();
    void SetKeyboardMode(eKeyboardProcessState value);
    static int ProcFont(int value, void* context);
    static int ProcPredict(int value, void* context);
    void KeyboardOnCreate();
    unsigned int GetKeyboardControllerType();
    unsigned int GetKeyboardMode();
    unsigned char GetLanguageType();
    void TickKeyboard();
    void InitialiseKeyboard();
    unsigned char IsShowingErrorViewer();
    void SetErrorViewerMode(eErrorViewerProcessState value);
    void ErrorViewerOnCreate();
    void InitialiseErrorViewer();
    unsigned int GetErrorViewerControllerType();
    void TickErrorViewer();
    ~CInput();
    void SetSigninJoypadMask(unsigned int);
    static const wchar_t* getDeviceTypeName(unsigned int);
    int GetControllerType(int);
    void FinaliseKeyboard();
    void FinaliseErrorViewer();
    void DebugPrintf(const char*, ...);
};

extern CInput* g_pInput;
extern CInput* g_pInputInstance;

int XEnableGuestSignin(int enable);

#endif
