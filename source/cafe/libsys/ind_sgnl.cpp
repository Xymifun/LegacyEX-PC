#include <signal.h>
#include <stdlib.h>

#include <coreinit/debug.h>

typedef void (*SignalHandler)(int);

extern "C"
{

static SignalHandler static_SignalHandlers[_SIGMAX];

static int __gh_signal_initialized;

void __gh_signal_init(SignalHandler* SignalHandlers)
{
    int i;
    for (i = 1; i <= _SIGMAX; i++)
        SignalHandlers[i - 1] = SIG_DFL;
    for (i = SIGSTOP; i <= SIGIO; i++)
        SignalHandlers[i - 1] = SIG_IGN;
    for (i = SIGPROF; i <= SIGUSR2; i++)
        SignalHandlers[i - 1] = SIG_IGN;

    __gh_signal_initialized = 1;
}

void __ghs_default_signal_handler(int sig)
{
    if (sig == SIGKILL)
        _Exit(EXIT_FAILURE);
    OSPanic("ind_sgnl.c", 105, "unhandled signal %d\n", sig);
}

int raise(int sig)
{
    SignalHandler temp, *handlers;

    if (sig <= 0 || sig > _SIGMAX)
        return -1;
    handlers = static_SignalHandlers;

    if (__gh_signal_initialized != 1)
    {
        __gh_signal_init(handlers);
    }

    temp = handlers[sig - 1];
    if (sig == SIGKILL || temp == SIG_DFL)
        __ghs_default_signal_handler(sig);
    else if (temp != SIG_IGN)
    {
        handlers[sig - 1] = SIG_DFL;
        (*temp)(sig);
    }
    return 0;
}
}
