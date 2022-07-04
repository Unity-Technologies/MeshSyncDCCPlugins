#include "pch.h"
#include "msblenBinder.h"
#include "msblenCurveHandler.h"
#include "msblenUtils.h"

namespace bl = blender;

ms::CurvePtr msblenCurveHandler::exportCurve(msblenContextState& state,
	msblenContextPathProvider& paths,
	BlenderSyncSettings& settings,
	const Object* src,
	MeshSyncClient::AsyncTasksController& asyncTasksController)
{
	std::shared_ptr<ms::Curve> ret = ms::Curve::create();
	
	Curve* data = (Curve*)src->data;

	ms::Curve& dst = *ret;
	dst.path = paths.get_path(src);
	
	// transform
	extractTransformData(settings, src, dst);

	if (settings.sync_curves) {
		auto task = [this, ret, src, data, &state, &settings]() {
			auto& dst = *ret;
			doExtractCurveData(state, settings, dst, src, data, dst.world_matrix);
			state.manager.add(ret);
		};

		if (settings.multithreaded)
			asyncTasksController.AddTask(std::launch::async, task);
		else
			task();
	}

	return ret;
}

void msblenCurveHandler::doExtractCurveData(msblenContextState& state, BlenderSyncSettings& settings, ms::Curve& dst, const Object* obj, Curve* data, mu::float4x4 world)
{
	if (settings.sync_curves) {
		const bool is_editing = data->editnurb != nullptr;

		ListBase nurbs;
		if (is_editing) {
			nurbs = data->editnurb->nurbs;
		}
		else {
			nurbs = data->nurb;
		}

		dst.splines.clear();

		for (auto nurb : blender::list_range((Nurb*)nurbs.first)) {
			dst.splines.push_back(ms::CurveSpline::create());
			auto curveSpline = dst.splines.back();

			curveSpline->closed = (nurb->flagu & CU_NURB_CYCLIC) != 0;

			curveSpline->cos.resize_discard(nurb->pntsu);
			curveSpline->handles_left.resize_discard(nurb->pntsu);
			curveSpline->handles_right.resize_discard(nurb->pntsu);

			for (int i = 0; i < nurb->pntsu; i++) {
				BezTriple* bezt = &nurb->bezt[i];

				if (bezt) {
					curveSpline->cos[i] = (mu::float3&)bezt->vec[1];
					curveSpline->handles_left[i] = (mu::float3&)bezt->vec[0];
					curveSpline->handles_right[i] = (mu::float3&)bezt->vec[2];
				}
			}
		}
	}
}

void msblenCurveHandler::importCurve(ms::Curve* curve) {
	auto obj = msblenUtils::get_object_from_path(curve->path);

	// Make sure the object still exists:
	if (!obj) {
		return;
	}

	auto data = (Curve*)obj->data;
	bl::BCurve bcurve(data);

	bcurve.clear_splines();

	for (auto& spline : curve->splines) {
		auto newSpline = bcurve.new_spline();

		auto bSpline = bl::BNurb(newSpline);

		int knotCount = spline->cos.size();

		// -1 because a new spline already has 1 point:
		bSpline.add_bezier_points(knotCount - 1, obj);

		for (int knotIndex = 0; knotIndex < knotCount; knotIndex++)
		{
			BezTriple* bezt = &newSpline->bezt[knotIndex];

			auto& cos = spline->cos[knotIndex];
			copyFloatVector(bezt->vec[1], cos);

			auto& handles_left = spline->handles_left[knotIndex];
			copyFloatVector(bezt->vec[0], handles_left);

			auto& handles_right = spline->handles_right[knotIndex];
			copyFloatVector(bezt->vec[2], handles_right);
		}

		if (spline->closed) {
			bSpline.m_ptr->flagu |= CU_NURB_CYCLIC;
		}
	}
}