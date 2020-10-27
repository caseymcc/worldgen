#ifndef _worldgen_generator_h_
#define _worldgen_generator_h_

#include "voxigen/voxigen_export.h"
#include "voxigen/classFactory.h"
#include "voxigen/updateQueue.h"
#include "voxigen/progress.h"

#include <memory>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace worldgen
{

template<typename _Chunk>
class ChunkHandle;

class Generator
{
public:
    Generator() {}
    virtual ~Generator() {}

    virtual void create(IGridDescriptors *descriptors, LoadProgress &progress)=0;
    virtual bool load(IGridDescriptors *descriptors, const std::string &directoryName, LoadProgress &progress)=0;
    virtual void save(const std::string &directoryName)=0;

    virtual void loadDescriptors(IGridDescriptors *descriptors)=0;
    virtual void saveDescriptors(IGridDescriptors *descriptors)=0;
    //    virtual void terminate()=0;

    //    virtual void generateChunk(unsigned int hash, void *buffer, size_t size)=0;
    virtual unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod)=0;
    virtual unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &size, void *buffer, size_t bufferSize, size_t lod)=0;

    //used to get the general height at a location, may not be exact
    //exepected limited use
    //pos grid coordinates
    virtual int getBaseHeight(const glm::vec2 &pos)=0;
};

typedef std::shared_ptr<Generator> SharedGenerator;

template<typename _Generator, typename _FileIO>
class GeneratorTemplate:public RegisterClass<GeneratorTemplate<_Generator, _FileIO>, Generator>
{
public:
    GeneratorTemplate():m_generator(new _Generator()) {}
    virtual ~GeneratorTemplate() {}

    static const char *typeName() { return _Generator::typeName(); }

    void create(IGridDescriptors *descriptors, Progress &progress) override { m_generator->create(descriptors, progress); }
    bool load(IGridDescriptors *descriptors, const std::string &directoryName, LoadProgress &progress) override { return m_generator->load<_FileIO>(descriptors, directoryName, progress); }
    void save(const std::string &directoryName) override { m_generator->save<_FileIO>(directoryName); }

    void loadDescriptors(IGridDescriptors *descriptors) override { m_generator->loadDescriptors(descriptors); }
    void saveDescriptors(IGridDescriptors *descriptors) override { m_generator->saveDescriptors(descriptors); }
    //    void terminate() { m_generator->terminate(); }

    //    void generateChunk(unsigned int hash, void *buffer, size_t size) { m_generator->generateChunk(hash, buffer, size); };
    unsigned int generateChunk(const glm::vec3 &startPos, const glm::ivec3 &chunkSize, void *buffer, size_t bufferSize, size_t lod) override { return m_generator->generateChunk(startPos, chunkSize, buffer, bufferSize, lod); };
    unsigned int generateRegion(const glm::vec3 &startPos, const glm::ivec3 &size, void *buffer, size_t bufferSize, size_t lod) override { return m_generator->generateRegion(startPos, size, buffer, bufferSize, lod); };

    int getBaseHeight(const glm::vec2 &pos) override { return m_generator->getBaseHeight(pos); };

    _Generator *get() { return m_generator.get(); }
private:
    std::unique_ptr<_Generator> m_generator;
};

}//namespace voxigen

#include "voxigen/volume/chunkHandle.h"

#endif //_worldgen_generator_h_