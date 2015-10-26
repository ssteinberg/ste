// StE
// © Shlomi Steinberg, 2015

#pragma once

#include "stdafx.h"

#include <array>

namespace StE {
namespace Graphics {

template <typename T, int samples>
class spectrum {
private:
	template <typename T, int samples>
	friend spectrum<T, samples> sqrt(const spectrum<T, samples> &rhs);
	template <typename T, int samples>
	friend spectrum<T, samples> pow(const spectrum<T, samples> &rhs, const T& exponent);
	template <typename T, int samples>
	friend spectrum<T, samples> pow(const spectrum<T, samples> &rhs, const T& exponent);
	template <typename T, int samples>
	friend spectrum<T, samples> mix(const spectrum<T, samples> &a, const spectrum<T, samples> &b, float x);
	template <typename T, int samples>
	friend spectrum<T, samples> smoothstep(const spectrum<T, samples> &a, const spectrum<T, samples> &b, float x);
	template <typename T, int samples>
	friend spectrum<T, samples> min(const spectrum<T, samples> &a, const spectrum<T, samples> &b);
	template <typename T, int samples>
	friend spectrum<T, samples> max(const spectrum<T, samples> &a, const spectrum<T, samples> &b);

protected:
	std::array<T, samples> c;

public:
	spectrum() = default;
	spectrum(const T &v) { c.fill(v); }

	spectrum &operator+=(const spectrum &rhs) {
		for (int i = 0; i < samples; ++i)
			c[i] += rhs.c[i];
		return *this;
	}
	spectrum &operator-=(const spectrum &rhs) {
		for (int i = 0; i < samples; ++i)
			c[i] -= rhs.c[i];
		return *this;
	}
	spectrum &operator*=(const spectrum &rhs) {
		for (int i = 0; i < samples; ++i)
			c[i] *= rhs.c[i];
		return *this;
	}
	spectrum &operator/=(const spectrum &rhs) {
		for (int i = 0; i < samples; ++i)
			c[i] /= rhs.c[i];
		return *this;
	}
	spectrum &operator+=(const T &val) const {
		for (int i = 0; i < samples; ++i)
			c[i] += val;
		return *this;
	}
	spectrum &operator-=(const T &val) const {
		for (int i = 0; i < samples; ++i)
			c[i] -= val;
		return *this;
	}
	spectrum &operator*=(const T &val) const {
		for (int i = 0; i < samples; ++i)
			c[i] *= val;
		return *this;
	}
	spectrum &operator/=(const T &val) const {
		for (int i = 0; i < samples; ++i)
			c[i] /= val;
		return *this;
	}

	spectrum &operator+(const spectrum &rhs) {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] + rhs.c[i];
		return ret;
	}
	spectrum &operator-(const spectrum &rhs) {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] - rhs.c[i];
		return ret;
	}
	spectrum &operator*(const spectrum &rhs) {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] * rhs.c[i];
		return ret;
	}
	spectrum &operator/(const spectrum &rhs) {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] / rhs.c[i];
		return ret;
	}
	spectrum &operator+(const T &val) const {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] + val;
		return ret;
	}
	spectrum &operator-(const T &val) const {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] - val;
		return ret;
	}
	spectrum &operator*(const T &val) const {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] * val;
		return ret;
	}
	spectrum &operator/(const T &val) const {
		spectrum<T, samples> ret;
		for (int i = 0; i < samples; ++i)
			ret.c[i] = c[i] / val;
		return ret;
	}

	bool is_black() const {
		for (int i = 0; i < samples; ++i)
			if (c[i])
				return false;
		return true;
	}

	virtual T luminance() const = 0;
};

template <typename T, int samples>
spectrum<T, samples> sqrt(const spectrum<T, samples> &rhs) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::sqrt(rhs.c[i]);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> pow(const spectrum<T, samples> &rhs, const T &exponent) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::pow(rhs.c[i], exponent);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> mix(const spectrum<T, samples> &a, const spectrum<T, samples> &b, float x) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::mix(a.c[i], b.c[i], a);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> smoothstep(const spectrum<T, samples> &a, const spectrum<T, samples> &b, float x) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::smoothstep(a.c[i], b.c[i], a);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> clamp(const spectrum<T, samples> &x, const T &min_val, const T &max_val) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::clamp(x.c[i], min_val, max_val);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> min(const spectrum<T, samples> &a, const spectrum<T, samples> &b) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::min(a.c[i], b.c[i]);
	return ret;
}

template <typename T, int samples>
spectrum<T, samples> max(const spectrum<T, samples> &a, const spectrum<T, samples> &b) {
	spectrum<T, samples> ret;
	for (int i = 0; i < samples; ++i)
		ret.c[i] = glm::max(a.c[i], b.c[i]);
	return ret;
}

}
}
