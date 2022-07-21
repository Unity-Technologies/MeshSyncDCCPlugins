#pragma once

#include "../MeshSyncClientBlender/msblenContextState.h"
#include "BlenderSyncSettings.h"
#include "msblenEntityHandler.h"

#include "MeshSync/SceneGraph/msCurve.h"

#include "MeshSyncClient/AsyncTasksController.h"

class msblenCurveHandler : msblenEntityHandler {
private:
	void doExtractCurveData(msblenContextState& state, BlenderSyncSettings& settings, ms::Curve& dst, const Object* obj, Curve* data, mu::float4x4 world);

public:
	ms::CurvePtr exportCurve(msblenContextState& state, msblenContextPathProvider& paths, BlenderSyncSettings& settings, const Object* obj, MeshSyncClient::AsyncTasksController& asyncTasksController);
	void importCurve(ms::Curve* curve);
};

