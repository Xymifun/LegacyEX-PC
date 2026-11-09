#ifndef SOURCE_VENDOR_LOADINGINPUTPARAMS_HPP
#define SOURCE_VENDOR_LOADINGINPUTPARAMS_HPP

#include "game/minigame/EMiniGameId.hpp"

class _LoadingInputParams
{
public:
    _LoadingInputParams();

    int (*m_threadProc)(void*);
    void* m_threadParam;
    void* m_completionData;
    int m_cancelText;
    void (*m_cancelFunc)(void*);
    int (*m_completeFunc)(void*);
    void* m_cancelFuncParam;
    void* m_completeFuncParam;
    bool m_waitForThreadToDelete;
    EMiniGameId m_miniGameId;
};

#endif
