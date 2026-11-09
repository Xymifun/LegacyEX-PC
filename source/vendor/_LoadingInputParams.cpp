#include "vendor/_LoadingInputParams.hpp"

_LoadingInputParams::_LoadingInputParams()
{
    m_threadProc = 0;
    m_threadParam = 0;
    m_completionData = 0;
    m_cancelText = -1;
    m_cancelFunc = 0;
    m_completeFunc = 0;
    m_cancelFuncParam = 0;
    m_completeFuncParam = 0;
    m_waitForThreadToDelete = false;
    m_miniGameId = eMiniGameId_Any;
}
