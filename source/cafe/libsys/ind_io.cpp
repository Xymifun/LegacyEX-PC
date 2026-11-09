#include <stdarg.h>

#include <coreinit/debug.h>

enum HostSyscall
{
    SYSCALL_READ = 0x40000,
    SYSCALL_WRITE = 0x40001,
    SYSCALL_OPEN = 0x30004,
    SYSCALL_OPEN2 = 0x40004,
    SYSCALL_CLOSE = 0x20005,
    SYSCALL_LSEEK = 0x40007,
    SYSCALL_UNLINK = 0x20008
};

enum OpenFlags
{
    O_CREAT = 0x100
};

enum StandardFile
{
    STDOUT_FILENO = 1,
    STDERR_FILENO = 2
};

extern "C"
{

int __ghs_syscall(int, ...);

int write(int fno, const void* buf, int size)
{
    if (fno == STDOUT_FILENO || fno == STDERR_FILENO)
    {
        OSConsoleWrite(static_cast<const char*>(buf), size);
        return size;
    }
    return __ghs_syscall(SYSCALL_WRITE, fno, buf, size);
}

int read(int fno, void* buf, int size)
{
    return __ghs_syscall(SYSCALL_READ, fno, buf, size);
}

int open(const char* filename, int flags, ...)
{
    if (flags & O_CREAT)
    {
        int mode;
        va_list va;

        va_start(va, flags);
        mode = va_arg(va, int);
        va_end(va);
        return __ghs_syscall(SYSCALL_OPEN2, filename, flags, mode);
    }
    else
    {
        return __ghs_syscall(SYSCALL_OPEN, filename, flags);
    }
}

int close(int fno)
{
    return __ghs_syscall(SYSCALL_CLOSE, fno);
}

long lseek(int fno, long offset, int end)
{
    return __ghs_syscall(SYSCALL_LSEEK, fno, offset, end);
}

int unlink(const char* name)
{
    return __ghs_syscall(SYSCALL_UNLINK, name);
}
}
