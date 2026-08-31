#include "imguiHelpers.h"

#include <cstring>

namespace mapgen
{

std::vector<char> packVectorString(const std::vector<std::string> &values)
{
    std::vector<char> packed;

    size_t size=0;
    for(const std::string &value:values)
        size+=value.size()+1;//null terminator

    packed.resize(size+1);

    size=0;
    for(const std::string &value:values)
    {
        memcpy(&packed[size], value.c_str(), value.size());
        size+=value.size();
        packed[size]=0;
        size++;
    }
    packed[size]=0;

    return packed;
}

}//namespace mapgen
