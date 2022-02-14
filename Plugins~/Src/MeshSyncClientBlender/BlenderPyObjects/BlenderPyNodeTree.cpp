#include "pch.h"
#include "BlenderPyNodeTree.h"

namespace blender {
	PropertyRNA* BlenderPyNodeTree_inputs = nullptr;

	void blender::BlenderPyNodeTree::inputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree)
	{
		auto cprop = (CollectionPropertyRNA*)BlenderPyNodeTree_inputs;
		PointerRNA rna;
		rna.data = nodeTree;
		cprop->begin(it, &rna);
	}
	void BlenderPyNodeTree::inputs_next(CollectionPropertyIterator* it)
	{
		auto cprop = (CollectionPropertyRNA*)BlenderPyNodeTree_inputs;
		cprop->next(it);

	}
	void BlenderPyNodeTree::inputs_end(CollectionPropertyIterator* it)
	{
		auto cprop = (CollectionPropertyRNA*)BlenderPyNodeTree_inputs;
		cprop->end(it);
	}
	PointerRNA BlenderPyNodeTree::inputs_get(CollectionPropertyIterator* it)
	{
		auto cprop = (CollectionPropertyRNA*)BlenderPyNodeTree_inputs;
		return cprop->get(it);
	}
}
