#include "BlenderPyDepsgraphUpdate.h"
#include "BlenderPyCommon.h"

namespace blender {

	PropertyRNA* BlenderPyDepsgraphUpdate_updates = nullptr;
	FunctionRNA* BlenderPyDepsgraphUpdate_id_type_updated = nullptr;
	extern bContext* g_context;


	CollectionPropertyRNA* get_property() {
		return (CollectionPropertyRNA*)BlenderPyDepsgraphUpdate_updates;
	}

	void blender::BlenderPyDepgraphUpdate::updates_begin(CollectionPropertyIterator* it, Depsgraph* graph)
	{
		PointerRNA rna;
		rna.data = graph;
		get_property()->begin(it, &rna);
	}

	void blender::BlenderPyDepgraphUpdate::updates_next(CollectionPropertyIterator* it)
	{
		get_property()->next(it);
	}

	void blender::BlenderPyDepgraphUpdate::updates_end(CollectionPropertyIterator* it)
	{
		get_property()->end(it);
	}

	PointerRNA blender::BlenderPyDepgraphUpdate::updates_get(CollectionPropertyIterator* it)
	{
		return get_property()->get(it);
	}

	bool blender::BlenderPyDepgraphUpdate::id_type_updated(int type, Depsgraph* graph) {
		
		return call<Depsgraph, bool, int>(g_context, graph, BlenderPyDepsgraphUpdate_id_type_updated, type);
	}
}
