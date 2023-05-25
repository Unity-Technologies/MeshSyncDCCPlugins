#pragma once

#include "MeshSync/MeshSync.h"

namespace ms{

/// <summary>
/// Interface for managers that deal with Transforms.
/// </summary>
class ITransformManager {
public:
	virtual void add(TransformPtr transform) = 0;
	virtual void touch(const std::string& path) = 0;
	virtual void eraseStaleEntities() = 0;
	virtual void clear() = 0;
	virtual void setAlwaysMarkDirty(bool flag) = 0;
	
    virtual bool needsToApplyMirrorModifier() { return true; }
};
}