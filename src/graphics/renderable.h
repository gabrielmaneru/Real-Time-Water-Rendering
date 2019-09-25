/* Start Header -------------------------------------------------------
Copyright (C) 2019 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.
File Name:	renderable.h
Purpose: Renderable Base
Author: Gabriel Mañeru - gabriel.m
- End Header --------------------------------------------------------*/

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