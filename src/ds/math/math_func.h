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
  return (T)(a + (b - a) * t);
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

template <typename T>
inline T convertRange(const T originalStart, const T originalEnd, const T newStart, const T newEnd, const T val) {
	T originalRange = originalEnd - originalStart;
	if(originalRange == 0.0){
		return val;
	}

	return (val - originalStart) * ((newEnd - newStart) / originalRange) + newStart;
}

// Surely somewhere in of there's been a rounding function defined??  Well, use
// symmetric rounding, which I believe will be implemented in C+xx10
inline double round(const double d)
{
  return d > 0.0 ? floor(d + 0.5) : ceil(d - 0.5);
}

inline unsigned getNextPowerOf2( unsigned number )
{
  unsigned pos = sizeof(unsigned) * 8;
  while (pos-->0 && !(number&1<<pos));
  return 1<<++pos;
}

inline double dist(const double x0, const double y0, const double x1, const double y1)
{
	return sqrt(((x0-x1)*(x0-x1)) + ((y0-y1)*(y0-y1)));
}

inline double distSquared(const double x0, const double y0, const double x1, const double y1)
{
	return ((x0-x1)*(x0-x1)) + ((y0-y1)*(y0-y1));
}

inline double slope(const double x0, const double y0, const double x1, const double y1)
{
	return (y1 - y0) / (x1 - x0);
}

// 0 = right, clockwise (90 = down)
double degree(const double x2, const double y2);

ci::Vec3f randomUnitVector();

} // namespace math
} // namespace ds

#endif//DS_MATH_MATH_FUNC_H
