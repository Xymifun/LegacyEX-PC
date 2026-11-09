#ifndef SOURCE_PLATFORM_WIIU_NINTENDOFILESYSTEM_WIIU_HPP
#define SOURCE_PLATFORM_WIIU_NINTENDOFILESYSTEM_WIIU_HPP

#include <stdint.h>

#include "platform/wiiu/C4JStorage.hpp"

int ReadDir(int* directory, FSDirEntry* entry);
int CloseDir(int* directory);
void CloseHandle(int file);
int CreateDirectoryA(const char* pathName, void* securityAttributes);
int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode, void* securityAttributes,
                unsigned int creationDisposition, unsigned int flagsAndAttributes, int templateFile);

#endif
