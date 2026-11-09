#ifndef PLATFORM_PC_PCKEYBOARDGLYPHS_HPP
#define PLATFORM_PC_PCKEYBOARDGLYPHS_HPP

struct PcKeyboardGlyph
{
    const char* name;
    const unsigned char* data;
    unsigned int size;
};

extern const PcKeyboardGlyph kPcKeyboardGlyphs[];
extern const unsigned int kPcKeyboardGlyphCount;

#endif
