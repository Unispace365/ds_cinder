#pragma once
#ifndef DS_MATH_MATH_FUNC_H
#define DS_MATH_MATH_FUNC_H
#include "cinder/Vector.h"

namespace ds {
namespace math {

bool isEqual( float a, float b );
bool isEqual( double a, double b );
bool isEqual( float a, double b );
bool isEqual( double a, float b );

template <typename T>
T max(T a, T b)
{
  if (a > b)
    return a;
  return b;
}

template <typename T>
T min(T a, T b)
{
  if (a < b)
    return a;
  return b;
}

template <typename T>
T clamp(T v, T min, T max)
{
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

bool intersect2D(const ci::Vec3f &start0, const ci::Vec3f &end0, const ci::Vec3f &start1, const ci::Vec3f &end1);

template <typename T>
T lerp(const T &a, const T &b, float t)
{
  return a + (b - a) * t;
}

//wrapping modulus
inline int mod(int x, int m)
{
  return ( x % m + m  ) % m;
}

template <typename T>
inline T mod(T x, T m)
{
  return fmod(fmod(x, m) + m, m);
}

/**
 * wraps v to values from a to b.
 *     value of b will result in a.
 */
template <typename T>
inline T wrap(const T a, const T b, const T v)
{
    T top = b;
    if (double(a) < 0.0)
        top = b + a;
    else if (double(a) > 0.0)
        top = b - a;

    T m = mod( v, top );

    T result = m;
    if (double(a) == m)
        result = m;
    else if (m == 0.0)
        result = b - 1;
    else if (double(a) < 0.0)
        result = b + m;
    else if ( double(a) > 0.0)
        result = b - m;

    return result;
}

// Surely somewhere in of there's been a rounding function defined??  Well, use
// symmetric rounding, which I believe will be implemented in C+xx10
inline double round(const double d)
{
  return d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5);
}

} // namespace math
} // namespace ds

#endif//DS_MATH_MATH_FUNC_H
