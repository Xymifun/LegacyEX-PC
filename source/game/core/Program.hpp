#ifndef GAME_CORE_PROGRAM_HPP
#define GAME_CORE_PROGRAM_HPP

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>

class BufferedInputStream;
class Effect;
class ResourceManager;

class Program
{
public:
    class Type
    {
    public:
        Type(const std::wstring& name, const std::wstring& extension, int glType);
        std::wstring getName();
        std::wstring getExtension();
        int getGlType();
        boost::unordered_map<std::wstring, Program*>& getPrograms();

    private:
        std::wstring mName;
        std::wstring mExtension;
        int mGlType;
        boost::unordered_map<std::wstring, Program*> mPrograms;
    };

    Program(Type* type, int id, const std::wstring& name);
    void attachToEffect(Effect* effect);
    void detachFromEffect(Effect* effect);
    std::wstring getName();
    static Program* getOrCreate(ResourceManager* resourceManager, Type* type, const std::wstring& name);
    std::vector<unsigned char> getBytes(BufferedInputStream* stream);
};

#endif
