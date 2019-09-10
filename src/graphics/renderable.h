#pragma once
#include "transform3d.h"
#include "model.h"
class renderable
{
public:
	transform3d m_transform;
	const Model * m_model{nullptr};
};