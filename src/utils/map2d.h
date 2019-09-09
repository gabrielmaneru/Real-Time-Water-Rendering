#pragma once
#include <assert.h>
#include <vector>
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
