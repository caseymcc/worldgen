#ifndef _FastNoise_Easing_h_
#define _FastNoise_Easing_h_

#include <cmath>

namespace easing
{

//CompileTime pow
template<int Exp>
float tPow(float base)
{   return TPow<Exp-1>(base)*TPow<1>(base);}

template<>
float tPow<1>(float value)
{   return value;}

//Integer pow
inline int iPow(int base, int exp)
{
    int result=1;
    
    for (;;)
    {
        if(exp & 1)
            result*=base;
        exp>>=1;
        
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

//Float pow using IPow, 99% accurate [0.0f, 1.0f]
inline float fakePow(float base, float exp)
{
    float expFloor=std::floorf(exp);
    float weight=exp-expFloor;
    int iExp=(int)expFloor;
    
    return ((1.0f-weight)*IPow(base, iExp))+(weight*IPow(base, iExp+1));
}

inline float flip(float value)
{
    return 1.0f-value;
}

template<typename FuncA, typename FuncB, typename ... Types>
float mix(float weight, Types ... args)
{
    return ((1.0f-weight)*FuncA(args...))+(weight*FuncB(args...));
}

template<typename FuncA, typename FuncB, typename ... Types>
float crossfade(float value, Types ... args)
{
    return ((1.0f-value)*FuncA(value, args...))+(value*FuncB(value, args...));
}

template<typename Func, typename ... Types>
float scale(float value, Types ... args)
{
    return value*Func(value, args...);
}

template<typename Func, typename ... Types>
float reverseScale(float value, Types ... args)
{
    return (1.0f-value)*Func(value, args...);
}

template<int Exp=2>
float smoothStart(float value)
{
    return tPow<Exp>(value);
}

template<int Exp=2>
float smoothStop(float value)
{
    return 1.0f-tPow<Exp>(1.0f-value);
}

template<unsigned int Exp=2>
float smoothStep(float value)
{
    retrun crossfade<smoothStart<Exp>, smoothStop<Exp>>(value);
}

template<unsigned int Exp=2>
float arch(float value)
{
    return scale<flip>(value);
}

template<unsigned int Exp=2>
float bellCurve(float value)
{
    return smoothStop<Exp/2>(value)*smoothStart<Exp/2>(value);
}

inline float bounceBottom(float value)
{
    return fabs(value);
}

inline float bounceTop(float value)
{
    return 1.0f-fabs(1.0f-value);
}

inline float bounce(float value)
{
    return bounceTop(bounceBottom(value));
}

}//namespace FastNoise

#endif//_FastNoise_Easing_h_

