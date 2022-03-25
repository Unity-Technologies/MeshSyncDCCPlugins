#pragma once

#include "pch.h"
#include "MeshSync/MeshSync.h"

namespace ms{
	class TransformManager {
	public:
		virtual void add(TransformPtr transform) = 0;
		virtual void touch(const std::string& path) = 0;
		virtual void eraseStaleEntities() = 0;
		virtual void clear() = 0;
		virtual void setAlwaysMarkDirty(bool flag) = 0;

        TransformManager() {}
        ~TransformManager() {}

        TransformManager& operator=(const TransformManager& v)
        {
            return *this;
        }


        TransformManager(TransformManager&& v)
        {
            *this = std::move(v);
        }

        TransformManager& operator=(TransformManager&& v)
        {
          
            return *this;
        }

        TransformManager(const TransformManager& v){ *this = v; }
	};
}