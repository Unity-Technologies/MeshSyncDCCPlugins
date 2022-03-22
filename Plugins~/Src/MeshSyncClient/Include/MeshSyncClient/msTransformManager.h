#pragma once

#include "pch.h"
#include "MeshSync/MeshSync.h"

namespace ms{
	class TransformManager {
	public:
		virtual void add(TransformPtr transform) = 0;
		virtual void touch(const std::string& path) = 0;
	};
}