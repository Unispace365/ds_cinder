#pragma once
#ifndef DS_MATH_RANDOM_H
#define DS_MATH_RANDOM_H
#include "math_func.h"
#include <cstdlib>

namespace ds {
namespace math {

//random value from 0.0 through 1.0
inline double random()
{
  return double(rand()) / RAND_MAX;
}

// random value from 0 through end
inline int random(int end)
{
  return int(round(random() * end));
}

template < typename T >
inline T random(T end)
{
  return T(round(random() * end));
}

// random value from start through end
inline int random(int start, int end)
{
  return int(round(random() * (end - start)) + start);
}

template < typename T >
inline T random(T start, T end)
{
  return T(round(random() * (end - start)) + start);
}

}
}

#endif//DS_MATH_RANDOM_H
