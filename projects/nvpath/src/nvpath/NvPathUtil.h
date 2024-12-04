/*
Copyright (c) 2023, Paul Houx Creative Coding - All rights reserved.
This code is intended for use with the Cinder C++ library: http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

	* Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <ds/util/float_util.h> // for approxEqual method.

#include "cy/cyPolynomial.h"

namespace nvpath::util {

template <typename T = float>
glm::vec<2, T> rotate(const glm::vec<2, T>& point, T angle) {
	T cosA = std::cos(angle);
	T sinA = std::sin(angle);
	return {point.x * cosA - point.y * sinA, point.x * sinA + point.y * cosA};
}

template <typename T = float>
T normalizeAngle(T angle, T range = T{2 * M_PI}, T offset = T{0}) {
	return glm::fract((angle - offset) / range) * range + offset;
}

// Check if angle is within the given range.
template <typename T = float>
bool angleInRange(T angle, T startAngle, T diffAngle) {
	const T delta = normalizeAngle(angle - startAngle);
	if (diffAngle < 0) return normalizeAngle(diffAngle) <= delta;
	return delta >= 0 && delta <= diffAngle;
}

/// Equation 5.4 of the SVG 2.0 specification: https://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
template <typename T = float>
T arcHelper(const glm::vec<2, T>& a, const glm::vec<2, T>& b) {
	const T sign = a.x * b.y - a.y * b.x < T{0} ? T{-1} : T{1};
	return sign * glm::acos(glm::clamp(glm::dot(glm::normalize(a), glm::normalize(b)), T{-1}, T{1}));
}

template <typename T = float>
struct Line {
	glm::vec<2, T> p0{};
	glm::vec<2, T> p1{};

	Line() = default;
	Line(const glm::vec<2, T>& p0, const glm::vec<2, T>& p1)
		: p0(p0)
		, p1(p1) {}
	explicit Line(const float params[4])
		: Line({params[0], params[1]}, {params[2], params[3]}) {}

	[[nodiscard]] glm::vec<2, T> evaluate(T t) const { return p0 * (T{1} - t) + p1 * t; }

	[[nodiscard]] glm::vec<2, T> normal() const { return glm::normalize(glm::vec<2, T>{p1.y - p0.y, p0.x - p1.x}); }
};

template <typename T = float>
struct QuadraticCurve {
	glm::vec<2, T> p0{};
	glm::vec<2, T> p1{};
	glm::vec<2, T> p2{};

	QuadraticCurve() = default;
	QuadraticCurve(const glm::vec<2, T>& p0, const glm::vec<2, T>& p1, const glm::vec<2, T>& p2)
		: p0(p0)
		, p1(p1)
		, p2(p2) {}
	explicit QuadraticCurve(const float params[6])
		: QuadraticCurve({params[0], params[1]}, {params[2], params[3]}, {params[4], params[5]}) {}

	[[nodiscard]] glm::vec<2, T> evaluate(T t) const {
		const T t1 = T{1} - t;
		return p0 * t1 * t1 + p1 * T{2} * t1 * t + p2 * t * t;
	}
};

template <typename T = float>
struct CubicCurve {
	glm::vec<2, T> p0{};
	glm::vec<2, T> p1{};
	glm::vec<2, T> p2{};
	glm::vec<2, T> p3{};

	CubicCurve() = default;
	CubicCurve(const glm::vec<2, T>& p0, const glm::vec<2, T>& p1, const glm::vec<2, T>& p2, const glm::vec<2, T>& p3)
		: p0(p0)
		, p1(p1)
		, p2(p2)
		, p3(p3) {}
	explicit CubicCurve(const float params[8])
		: CubicCurve({params[0], params[1]}, {params[2], params[3]}, {params[4], params[5]}, {params[6], params[7]}) {}

	[[nodiscard]] glm::vec<2, T> evaluate(T t) const {
		const T t1 = T{1} - t;
		return p0 * t1 * t1 * t1 + p1 * T{3} * t1 * t1 * t + p2 * T{3} * t1 * t * t + p3 * t * t * t;
	}
};

template <typename T = float>
struct Arc {
	glm::vec<2, T> center{};
	T			   rx;
	T			   ry;
	T			   xAxisRotation;
	T			   startAngle;
	T			   diffAngle;

	Arc() = default;
	Arc(const glm::vec<2, T>& center, float radiusX, float radiusY, float xAxisRotation, float startAngle,
		float diffAngle)
		: center(center)
		, rx(glm::abs(radiusX))
		, ry(glm::abs(radiusY))
		, xAxisRotation(xAxisRotation)
		, startAngle(startAngle)
		, diffAngle(diffAngle) {}
	Arc(const glm::vec<2, T>& p0, float radiusX, float radiusY, float xAxisRotation, bool largeFlag, bool sweepFlag,
		const glm::vec<2, T>& p1)
		: rx(glm::abs(radiusX))
		, ry(glm::abs(radiusY))
		, xAxisRotation(xAxisRotation) {
		// Implementation of https://www.w3.org/TR/SVG/implnote.html#ArcCorrectionOutOfRangeRadii
		const T				 theta = glm::radians(xAxisRotation);
		const glm::vec<2, T> p	   = rotate((p0 - p1) / T{2}, -theta);
		const glm::vec<2, T> pSqr  = p * p;

		const T sqrtLambda = glm::sqrt(glm::max(T{1}, pSqr.x / (rx * rx) + pSqr.y / (ry * ry)));
		rx *= sqrtLambda;
		ry *= sqrtLambda;

		// Implementation of https://www.w3.org/TR/SVG/implnote.html#ArcConversionEndpointToCenter

		// Equation 5.2.
		T rxSqr		  = rx * rx;
		T rySqr		  = ry * ry;
		T nominator	  = glm::max(T{0}, rxSqr * rySqr - rxSqr * pSqr.y - rySqr * pSqr.x);
		T denominator = rxSqr * pSqr.y + rySqr * pSqr.x;
		T f			  = std::sqrt(nominator / denominator);
		if (largeFlag == sweepFlag) f = -f;

		glm::vec<2, T> c = {f * rx * p.y / ry, f * -ry * p.x / rx};

		// Equation 5.3.
		center = rotate(c, theta) + (p0 + p1) / T{2};

		// Equations 5.5 and 5.6.
		startAngle = arcHelper({1, 0}, {(p.x - c.x) / rx, (p.y - c.y) / ry});
		diffAngle  = arcHelper({(p.x - c.x) / rx, (p.y - c.y) / ry}, {(-p.x - c.x) / rx, (-p.y - c.y) / ry});

		if (!sweepFlag && diffAngle > 0)
			diffAngle -= T{2.0 * M_PI};
		else if (sweepFlag && diffAngle < 0)
			diffAngle += T{2.0 * M_PI};
	}
	explicit Arc(const float params[9])
		: Arc({params[0], params[1]}, params[2], params[3], params[4], params[5], params[6], {params[7], params[8]}) {}

	[[nodiscard]] glm::vec<2, T> evaluate(T t) const {
		const T phi = startAngle + t * diffAngle;
		return center + rotate({rx * std::cos(phi), ry * std::sin(phi)}, xAxisRotation);
	}
};

template <typename T = float>
std::vector<T> solveQuadratic(T a, T b, T c) {
	T		  roots[2];
	const T	  coefficients[] = {a, b, c};
	const int count			 = cy::QuadraticRoots<T>(roots, coefficients);

	if (count) return {roots, roots + count};
	return {};
}

template <typename T = float>
std::vector<T> solveCubic(T a, T b, T c, T d) {
	T		  roots[3];
	const T	  coefficients[] = {a, b, c, d};
	const int count			 = cy::CubicRoots<T>(roots, coefficients);

	if (count) return {roots, roots + count};
	return {};
}

template <typename T = float>
std::vector<T> solveQuartic(T a, T b, T c, T d, T e) {
	T		  roots[4];
	const T	  coefficients[] = {a, b, c, d, e};
	const int count			 = cy::PolynomialRoots<4>(roots, coefficients);

	if (count) return {roots, roots + count};
	return {};
}

template <typename T = float>
T distanceLineLine(const Line<T>& lineA, const Line<T>& lineB) {
	const glm::vec<2, T> v0 = lineA.p0 - lineA.p1;
	const glm::vec<2, T> v1 = lineB.p0 - lineB.p1;

	const T det = v0.x * v1.y - v0.y * v1.x;
	if (ds::approxZero(det)) return glm::distance(lineA.p0, lineB.p0);

	const glm::vec<2, T> v2 = lineA.p0 - lineB.p0;

	const T t = (v2.x * v1.y - v2.y * v1.x) / det;
	const T u = (v2.x * v0.y - v2.y * v0.x) / det;

	if (t < 0 || t > 1 || u < 0 || u > 1) return glm::distance(lineA.p0, lineB.p0);

	return T{0};
}

/// \brief Intersect 2 lines and return the intersection points.
template <typename T = float>
std::vector<glm::vec<2, T>> intersectLineLine(const Line<T>& lineA, const Line<T>& lineB) {
	const glm::vec<2, T> v0 = lineA.p0 - lineA.p1;
	const glm::vec<2, T> v1 = lineB.p0 - lineB.p1;
	const glm::vec<2, T> v2 = lineA.p0 - lineB.p0;

	const T det = v0.x * v1.y - v0.y * v1.x;
	if (ds::approxZero(det)) { // Lines are parallel or coincident.
		const T distance = glm::dot(lineA.normal(), v2);
		if (ds::approxZero(distance)) return {lineA.p0, lineA.p1, lineB.p0, lineB.p1}; // Lines are coincident.
		return {};																	   // Lines are parallel.
	}

	const T t = (v2.x * v1.y - v2.y * v1.x) / det;
	if (t < 0 || t > 1) return {};

	const T u = (v2.x * v0.y - v2.y * v0.x) / det;
	if (u < 0 || u > 1) return {};

	return {lineA.evaluate(t)};
}

/// \brief Intersect a line with a quadratic curve segment and return the intersection points.
template <typename T = float>
std::vector<glm::vec<2, T>> intersectLineQuad(const Line<T>& line, const QuadraticCurve<T>& quad) {
	// Coefficients of the quadratic equation.
	glm::vec<2, T> d  = line.p1 - line.p0;
	glm::vec<2, T> va = quad.p0 - line.p0;
	glm::vec<2, T> vb = (quad.p1 - quad.p0) * T{2};
	glm::vec<2, T> vc = quad.p0 - quad.p1 * T{2} + quad.p2;

	T a = va.x * d.y - va.y * d.x;
	T b = vb.x * d.y - vb.y * d.x;
	T c = vc.x * d.y - vc.y * d.x;

	// Find roots.
	auto roots = solveQuadratic(a, b, c);

	// Find intersection points.
	const bool isVertical = glm::abs(d.x) < glm::abs(d.y);

	std::vector<glm::vec<2, T>> intersections;
	for (T u : roots) {
		if (u < 0 || u > 1) continue; // Ensure valid u
		const glm::vec<2, T> point = quad.evaluate(u);

		const T t = isVertical ? (point.y - line.p0.y) / d.y : (point.x - line.p0.x) / d.x; // Solve for t
		if (t >= 0 && t <= 1) {
			intersections.push_back(point);
		}
	}

	return intersections;
}

/// \brief Intersect a line with a cubic curve segment and return the intersection points.
template <typename T = float>
std::vector<glm::vec<2, T>> intersectLineCubic(const Line<T>& line, const CubicCurve<T>& cubic) {
	// Coefficients of the cubic equation.
	glm::vec<2, T> n(line.p1.y - line.p0.y, line.p0.x - line.p1.x);

	T a = glm::dot(n, cubic.p0 - line.p0);
	T b = glm::dot(n, cubic.p1 * T{3} - cubic.p0 * T{3});
	T c = glm::dot(n, cubic.p2 * T{3} - cubic.p1 * T{6} + cubic.p0 * T{3});
	T d = glm::dot(n, cubic.p3 - cubic.p2 * T{3} + cubic.p1 * T{3} - cubic.p0);

	// Find roots.
	auto roots = solveCubic(a, b, c, d);

	// Find intersection points.
	const bool isVertical = glm::abs(line.p1.x - line.p0.x) < glm::abs(line.p1.y - line.p0.y);

	std::vector<glm::vec<2, T>> intersections;
	for (T u : roots) {
		if (u < 0 || u > 1) continue; // Ensure valid u.
		const glm::vec<2, T> point = cubic.evaluate(u);

		// Solve for t.
		const T t = isVertical ? (point.y - line.p0.y) / (line.p1.y - line.p0.y)
							   : (point.x - line.p0.x) / (line.p1.x - line.p0.x);
		if (t >= 0 && t <= 1) {
			intersections.push_back(point);
		}
	}
	return intersections;
}

///
template <typename T = float>
std::vector<glm::vec<2, T>> intersectLineArc(const Line<T>& line, const Arc<T>& arc) {
	// Convert degrees to radians.
	T theta = glm::radians(arc.xAxisRotation);

	// Normalize to unit circle.
	glm::vec<2, T> scale{arc.rx, arc.ry};
	glm::vec<2, T> normOrigin	 = rotate(line.p0 - arc.center, -theta) / scale; // Offset from circle center.
	glm::vec<2, T> normDirection = rotate(line.p1 - line.p0, -theta) / scale;	 // Direction of the line.

	// Coefficients of the quadratic equation.
	T a = glm::dot(normOrigin, normOrigin) - 1;
	T b = T{2} * glm::dot(normOrigin, normDirection);
	T c = glm::dot(normDirection, normDirection);

	// Find roots.
	auto roots = solveQuadratic(a, b, c);

	// Find intersection points.
	std::vector<glm::vec<2, T>> intersections;
	for (T t : roots) {
		if (t < 0 || t > 1) continue; // Ensure t is within the line segment.
		glm::vec<2, T> point = normOrigin + normDirection * t;

		// Check if the point is within the arc's angular range.
		const T angle = std::atan2(point.y, point.x);
		if (angleInRange(angle, arc.startAngle, arc.diffAngle)) {
			intersections.push_back(arc.center + rotate(point * scale, theta));
		}
	}
	return intersections;
}

///
template <typename T = float>
std::vector<glm::vec<2, T>> intersectArcArc(const Arc<T>& arcA, const Arc<T>& arcB) {
	constexpr int n = 30;

	// For now, approximate the arcs using line segments and intersect them.
	glm::vec<2, T> a0, a1;
	glm::vec<2, T> b0, b1;

	std::vector<glm::vec<2, T>> intersections;
	for (int i = 0; i < n; ++i) {
		a0 = arcA.evaluate(T(i) / T{n});
		a1 = arcA.evaluate(T(i + 1) / T{n});
		for (int j = 0; j < n; ++j) {
			b0 = arcB.evaluate(T(j) / T{n});
			b1 = arcB.evaluate(T(j + 1) / T{n});

			if (auto result = intersectLineLine(Line<T>{a0, a1}, Line<T>{b0, b1}); !result.empty()) {
				intersections.insert(intersections.end(), result.begin(), result.end());
			}
		}
	}
	return intersections;
}

} // namespace nvpath::util