#ifndef _worldgen_sortedContainer_h_
#define _worldgen_sortedContainer_h_

#include <vector>
#include <algorithm>

namespace worldgen
{

template< typename T >
typename std::vector<T>::iterator
insert_sorted(std::vector<T> &container, T const &item)
{
    return container.insert
    (
        std::upper_bound(container.begin(), container.end(), item),
        item
    );
}

template<class T>
bool contains_sorted(const std::vector<T> &container, const T &item)
{
    auto iter=std::lower_bound(
        container.begin(),
        container.end(),
        item,
        [](const T &l, const T &r){ return l<r; });
    return iter!=container.end()&&*iter==item;
}

template<class T>
size_t index_sorted(const std::vector<T> &container, const T &item)
{
    auto iter=std::lower_bound(
        container.begin(),
        container.end(),
        item,
        [](const T &l, const T &r){ return l<r; });

    if(iter!=container.end()&&*iter==item)
        return iter-container.begin();
    return std::numeric_limits<size_t>::max();
}

}//namespace worldgen

#endif //_worldgen_sortedContainer_h_