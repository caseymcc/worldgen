#ifndef _worldgen_worldDescriptors_h_
#define _worldgen_worldDescriptors_h_

#include "worldgen/export.h"
#include <glm/glm.hpp>

#include <string>

namespace worldgen
{

class WORLDGEN_EXPORT WorldDescriptors
{
public:
    const glm::ivec3 &getSize() const{ return m_size; }
    void setSize(const glm::ivec3 &size){ m_size=size; }

    const glm::ivec3 &getRegionSize() const{ return m_regionSize; }
    void setRegionSize(const glm::ivec3 &size){ m_regionSize=size; }
    
    const glm::ivec3 &getChunkSize() const{ return m_chunkSize; }
    void setChunkSize(const glm::ivec3 &size){ m_chunkSize=size; }
    
    std::string &getGeneratorArgs(){ return m_generatorArgs; }

private:
    glm::ivec3 m_size;
    glm::ivec3 m_regionSize;
    glm::ivec3 m_chunkSize;

    std::string m_generatorArgs;
};

}//namespace worldgen

#endif//_worldgen_worldDescriptors_h_