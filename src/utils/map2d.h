#pragma once
#include <assert.h>
#include <vector>
#include <utils/math_utils.h>
#include <functional>
template <typename TYPE>
struct map2d
{
	map2d() = default;
	map2d(size_t width, size_t height, TYPE default_value = TYPE{});
	void setup(size_t width, size_t height);
	void clear(TYPE default_value);
	void loop(std::function<TYPE(size_t, size_t, TYPE)> loop_function);
	TYPE get(size_t x, size_t y)const;
	TYPE get_linear(vec2 xy)const;
	void set(size_t x, size_t y, TYPE value);
	TYPE get(size_t idx)const;
	void set(size_t idx, TYPE value);

	size_t m_width;
	size_t m_height;
	std::vector<TYPE> m_values;
};

template<typename TYPE>
inline map2d<TYPE>::map2d(size_t width, size_t height, TYPE default_value)
	: m_width(width), m_height(height)
{
	setup(width, height);
	clear(default_value);
}

template<typename TYPE>
inline void map2d<TYPE>::setup(size_t width, size_t height)
{
	m_width = width;
	m_height = height;
	m_values.reserve(m_width * m_height);
}

template<typename TYPE>
inline void map2d<TYPE>::clear(TYPE default_value)
{
	m_values.clear();
	m_values.resize(m_width * m_height, default_value);
}

template<typename TYPE>
inline void map2d<TYPE>::loop(std::function<TYPE(size_t, size_t, TYPE)> loop_function)
{
	for (int y = 0; y < m_height; ++y)
		for (int x = 0; x < m_width; ++x)
			m_values[y * m_width + x] = loop_function(x, y, m_values[y * m_width + x]);
}

template<typename TYPE>
inline TYPE map2d<TYPE>::get(size_t x, size_t y) const
{
	assert(x >= 0 && y >= 0 && x < m_width && y < m_height);
	return m_values[y * m_width + x];
}

template<typename TYPE>
inline TYPE map2d<TYPE>::get_linear(vec2 xy) const
{
	assert(xy.x >= 0.0f && xy.y >= 0.0f && xy.x <= (float)m_width && xy.y <= (float)m_height);
	TYPE v_0_0 = m_values[floor_float(xy.y)*m_width + floor_float(xy.x)];
	TYPE v_0_1 = m_values[ceil_float(xy.y)*m_width + floor_float(xy.x)];
	TYPE v_1_0 = m_values[floor_float(xy.y)*m_width + ceil_float(xy.x)];
	TYPE v_1_1 = m_values[ceil_float(xy.y)*m_width + ceil_float(xy.x)];
	float x_f = xy.x - floorf(xy.x);
	float y_f = xy.y - floorf(xy.y);

	TYPE l = lerp<TYPE>(v_0_0, v_0_1, y_f);
	TYPE r = lerp<TYPE>(v_1_0, v_1_1, y_f);
	return lerp(l, r, x_f);
}

template<typename TYPE>
inline void map2d<TYPE>::set(size_t x, size_t y, TYPE value)
{
	assert(x >= 0 && y >= 0 && x < m_width && y < m_height);
	m_values[y * m_width + x] = value;
}

template<typename TYPE>
inline TYPE map2d<TYPE>::get(size_t idx) const
{
	assert(idx >= 0 && idx < m_width * m_height);
	return m_values[idx];
}

template<typename TYPE>
inline void map2d<TYPE>::set(size_t idx, TYPE value)
{
	assert(idx >= 0 && idx < m_width * m_height);
	m_values[idx] = value;
}
