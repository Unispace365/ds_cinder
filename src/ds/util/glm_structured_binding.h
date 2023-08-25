#pragma once
#ifndef GLM_STRUCTURED_BINDING
#define GLM_STRUCTURED_BINDING

#include <glm/glm.hpp>
#include <utility>

namespace glm {
template <std::size_t I, auto N, class T, auto Q>
constexpr auto& get(glm::vec<N, T, Q>& v) noexcept {
	return v[I];
}

template <std::size_t I, auto N, class T, auto Q>
constexpr const auto& get(const glm::vec<N, T, Q>& v) noexcept {
	return v[I];
}

template <std::size_t I, auto N, class T, auto Q>
constexpr auto&& get(glm::vec<N, T, Q>&& v) noexcept {
	return std::move(v[I]);
}

template <std::size_t I, auto N, class T, auto Q>
constexpr const auto&& get(const glm::vec<N, T, Q>&& v) noexcept {
	return std::move(v[I]);
}
} // namespace glm

namespace std {
template <auto N, class T, auto Q>
struct tuple_size<glm::vec<N, T, Q>> : std::integral_constant<std::size_t, N> {};

template <std::size_t I, auto N, class T, auto Q>
struct tuple_element<I, glm::vec<N, T, Q>> {
	using type = decltype(get<I>(declval<glm::vec<N, T, Q>>()));
};
} // namespace std

#endif