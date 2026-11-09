#ifndef GAME_CORE_MEMORYTRACKER_HPP
#define GAME_CORE_MEMORYTRACKER_HPP

class ByteBuffer;
class FloatBuffer;
class IntBuffer;

class MemoryTracker
{
public:
    static int genLists(int count);
    static void releaseLists(int list);
    static int genTextures();
    static void releaseTextures();
    static void release();

    static ByteBuffer* createByteBuffer(int capacity);
    static IntBuffer* createIntBuffer(int capacity);
    static FloatBuffer* createFloatBuffer(int capacity);
};

#endif
