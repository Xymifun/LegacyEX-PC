#ifndef PLATFORM_PC_PCDEBUGSCHEMATIC_HPP
#define PLATFORM_PC_PCDEBUGSCHEMATIC_HPP

#include <string>

class Level;

struct PcSchematicCopy
{
    int x0, y0, z0, x1, y1, z1;
};

struct PcSchematicPaste
{
    int x, y, z;
    int rotation;
    int mirror;
};

void PcDebugSchematicCopy(Level* level, const PcSchematicCopy& copy);
void PcDebugSchematicPaste(Level* level, const PcSchematicPaste& paste);
void PcDebugSchematicUndo(Level* level);
void PcDebugSchematicSave(const std::wstring& name);
void PcDebugSchematicLoad(const std::wstring& name);
void PcDebugSchematicTick();

std::wstring PcDebugSchematicStatus(unsigned int* revision);

#endif
