#pragma once
#include "transform3d.h"
#include "model.h"
class renderable
{
public:
	transform3d m_transform;
	const Model * m_model{nullptr};
	renderable(const transform3d& tr, const Model* p_M)
		: m_transform(tr), m_model(p_M) {}
	virtual ~renderable() = default;
};