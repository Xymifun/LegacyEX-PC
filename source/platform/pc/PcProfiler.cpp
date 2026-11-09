#include "platform/pc/PcProfiler.hpp"

#include "platform/pc/PcOs.hpp"
#include "platform/wiiu/C4JThread.hpp"

#include <algorithm>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

namespace
{
const unsigned int MAX_DEPTH = 64;
const unsigned int SCOPE_SLOTS = 1024;
const unsigned int SLOW_FRAME_ROWS = 12;
const unsigned int WINDOW_ROWS = 6;
const unsigned long long WINDOW_NS = 5000000000ULL;

struct ScopeStats
{
    const char* name;
    unsigned long long self;
    unsigned long long total;
    unsigned long long max;
    unsigned int calls;
};

struct ScopeTable
{
    ScopeStats slots[SCOPE_SLOTS];
    unsigned int used;
    unsigned long long busy;
};

struct ThreadStats
{
    unsigned char lock[kPcOsLockBytes];
    char name[64];
    bool main;
    ScopeTable window;
    ScopeTable frame;
};

struct OpenScope
{
    const char* name;
    unsigned long long start;
    unsigned long long child;
};

bool profilerOn;
bool syncGpuOn;
unsigned long long slowFrameNs;
unsigned char registryLock[kPcOsLockBytes];
std::vector<ThreadStats*>* allThreads;
unsigned long long frameStart;
unsigned long long frameNumber;
unsigned long long windowStart;
unsigned long long windowFrames;
unsigned long long windowTotal;
unsigned long long windowMax;
unsigned long long windowSlow;

unsigned char outputLock[kPcOsLockBytes];
std::string* pendingOutput;

__thread OpenScope scopeStack[MAX_DEPTH];
__thread unsigned int scopeDepth;
__thread unsigned int overflowCount;
__thread ThreadStats* myThread;
__thread bool isMainThread;

int NameLength(const char* name)
{
    int len = (int)strcspn(name, "\r\n");
    while (len > 0 && name[len - 1] == ' ')
        --len;
    return len;
}

ThreadStats* CurrentThread()
{
    if (myThread != 0)
        return myThread;
    const char* name = isMainThread ? "Main" : C4JThread::getCurrentThreadName();
    if (strncmp(name, "(4J) ", 5) == 0)
        name += 5;
    PcOsLockAcquire(registryLock);
    if (allThreads == 0)
        allThreads = new std::vector<ThreadStats*>;
    ThreadStats* t = 0;
    for (size_t i = 0; i < allThreads->size(); ++i)
    {
        if (strcmp((*allThreads)[i]->name, name) == 0)
            t = (*allThreads)[i];
    }
    if (t == 0)
    {
        t = new ThreadStats;
        memset(t, 0, sizeof(*t));
        snprintf(t->name, sizeof(t->name), "%.*s", NameLength(name), name);
        t->main = isMainThread;
        allThreads->push_back(t);
    }
    PcOsLockRelease(registryLock);
    myThread = t;
    return t;
}

void Record(ScopeTable* tbl, const char* name, unsigned long long self, unsigned long long total, bool outermost)
{
    unsigned int h = (unsigned int)(reinterpret_cast<uintptr_t>(name)) * 2654435761u;
    for (unsigned int p = 0; p < SCOPE_SLOTS; ++p)
    {
        ScopeStats& s = tbl->slots[(h + p) & (SCOPE_SLOTS - 1)];
        if (s.name == 0)
        {
            if (tbl->used + 1 >= SCOPE_SLOTS)
                return;
            s.name = name;
            ++tbl->used;
        }
        if (s.name != name)
            continue;
        s.self += self;
        if (outermost)
            s.total += total;
        if (total > s.max)
            s.max = total;
        ++s.calls;
        return;
    }
}

bool BySelfTime(const ScopeStats& a, const ScopeStats& b)
{
    return a.self > b.self;
}

double Milliseconds(unsigned long long ns)
{
    return (double)ns / 1e6;
}

void Append(std::string* out, const char* fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0)
        out->append(buf, (size_t)len < sizeof(buf) ? (size_t)len : sizeof(buf) - 1);
}

void* OutputThread(void*)
{
    PcOsSetCurrentThreadName("Profiler output");
    std::string buf;
    for (;;)
    {
        PcOsSleepMilliseconds(25);
        PcOsLockAcquire(outputLock);
        buf.swap(*pendingOutput);
        PcOsLockRelease(outputLock);
        if (buf.empty())
            continue;
        fwrite(buf.data(), 1, buf.size(), stdout);
        fflush(stdout);
        buf.clear();
    }
    return 0;
}

void Output(const std::string& text)
{
    PcOsLockAcquire(outputLock);
    pendingOutput->append(text);
    PcOsLockRelease(outputLock);
}

void TakeScopes(ScopeTable* tbl, std::vector<ScopeStats>* out)
{
    out->clear();
    for (unsigned int i = 0; i < SCOPE_SLOTS; ++i)
    {
        if (tbl->slots[i].name != 0)
            out->push_back(tbl->slots[i]);
    }
    memset(tbl, 0, sizeof(*tbl));
}

void AppendScopes(std::string* out, std::vector<ScopeStats>* list, unsigned int maxRows)
{
    std::sort(list->begin(), list->end(), BySelfTime);
    Append(out, "[pc] prof:     self ms    incl ms   calls     max ms  scope\n");
    for (size_t i = 0; i < list->size() && i < maxRows; ++i)
    {
        const ScopeStats& s = (*list)[i];
        Append(out, "[pc] prof:  %10.2f %10.2f %7u %10.2f  %.*s\n", Milliseconds(s.self),
               Milliseconds(s.total), s.calls, Milliseconds(s.max), NameLength(s.name), s.name);
    }
}

void ReportSlowFrame(std::vector<ScopeStats>* scopes, unsigned long long busy, unsigned long long frameNs)
{
    std::string out;
    Append(&out, "[pc] prof: frame %llu took %.2f ms\n", frameNumber, Milliseconds(frameNs));
    AppendScopes(&out, scopes, SLOW_FRAME_ROWS);
    unsigned long long outside = frameNs > busy ? frameNs - busy : 0;
    Append(&out, "[pc] prof:  %10.2f                               (main thread outside any scope)\n",
           Milliseconds(outside));
    Output(out);
}

void ReportWindow(unsigned long long now)
{
    unsigned long long windowNs = now - windowStart;
    double seconds = (double)windowNs / 1e9;
    std::string out;
    Append(&out, "[pc] prof: last %.1f s: %llu frames (%.1f fps), average %.2f ms, worst %.2f ms, %llu over %.0f ms\n",
           seconds, windowFrames, (double)windowFrames / seconds,
           windowFrames != 0 ? Milliseconds(windowTotal) / (double)windowFrames : 0.0,
           Milliseconds(windowMax), windowSlow, Milliseconds(slowFrameNs));
    PcOsLockAcquire(registryLock);
    std::vector<ThreadStats*> list = *allThreads;
    PcOsLockRelease(registryLock);
    std::vector<ScopeStats> scopes;
    for (size_t i = 0; i < list.size(); ++i)
    {
        ThreadStats* t = list[i];
        PcOsLockAcquire(t->lock);
        unsigned long long busy = t->window.busy;
        TakeScopes(&t->window, &scopes);
        PcOsLockRelease(t->lock);
        if (scopes.empty())
            continue;
        Append(&out, "[pc] prof: thread %s: %.0f%% of the time inside scopes\n", t->name,
               100.0 * (double)busy / (double)windowNs);
        AppendScopes(&out, &scopes, WINDOW_ROWS);
    }
    Output(out);
    windowStart = now;
    windowFrames = 0;
    windowTotal = 0;
    windowMax = 0;
    windowSlow = 0;
}
}

void PcProfilerEnable(double slowMs, bool syncGpu)
{
    isMainThread = true;
    slowFrameNs = (unsigned long long)(slowMs * 1e6);
    syncGpuOn = syncGpu;
    pendingOutput = new std::string;
    pthread_t t;
    pthread_create(&t, 0, OutputThread, 0);
    pthread_detach(t);
    profilerOn = true;
}

bool PcProfilerEnabled()
{
    return profilerOn;
}

bool PcProfilerSyncGpu()
{
    return profilerOn && syncGpuOn;
}

void PcProfilerBegin(const char* name)
{
    if (scopeDepth >= MAX_DEPTH)
    {
        ++overflowCount;
        return;
    }
    OpenScope& s = scopeStack[scopeDepth++];
    s.name = name;
    s.child = 0;
    s.start = PcOsMonotonicNanoseconds();
}

void PcProfilerEnd()
{
    if (overflowCount != 0)
    {
        --overflowCount;
        return;
    }
    if (scopeDepth == 0)
        return;
    unsigned long long now = PcOsMonotonicNanoseconds();
    const OpenScope& s = scopeStack[--scopeDepth];
    unsigned long long total = now - s.start;
    unsigned long long self = total > s.child ? total - s.child : 0;
    if (scopeDepth != 0)
        scopeStack[scopeDepth - 1].child += total;
    bool outer = true;
    for (unsigned int i = 0; i < scopeDepth; ++i)
    {
        if (scopeStack[i].name == s.name)
            outer = false;
    }
    ThreadStats* t = CurrentThread();
    PcOsLockAcquire(t->lock);
    Record(&t->window, s.name, self, total, outer);
    t->window.busy += self;
    if (t->main)
    {
        Record(&t->frame, s.name, self, total, outer);
        t->frame.busy += self;
    }
    PcOsLockRelease(t->lock);
}

void PcProfilerFrameEnd()
{
    if (!profilerOn)
        return;
    unsigned long long now = PcOsMonotonicNanoseconds();
    ThreadStats* mainStats = CurrentThread();
    std::vector<ScopeStats> scopes;
    PcOsLockAcquire(mainStats->lock);
    unsigned long long busy = mainStats->frame.busy;
    TakeScopes(&mainStats->frame, &scopes);
    PcOsLockRelease(mainStats->lock);
    if (frameStart == 0)
    {
        frameStart = now;
        windowStart = now;
        return;
    }
    unsigned long long frameNs = now - frameStart;
    frameStart = now;
    ++frameNumber;
    ++windowFrames;
    windowTotal += frameNs;
    if (frameNs > windowMax)
        windowMax = frameNs;
    if (frameNs >= slowFrameNs)
    {
        ++windowSlow;
        ReportSlowFrame(&scopes, busy, frameNs);
    }
    if (now - windowStart >= WINDOW_NS)
        ReportWindow(now);
}
