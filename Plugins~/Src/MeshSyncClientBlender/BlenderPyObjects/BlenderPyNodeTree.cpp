#include "pch.h"
#include "BlenderPyNodeTree.h"

namespace blender {

	PropertyRNA* BlenderPyNodeTree_inputs = nullptr;
	PropertyRNA* BlenderPyNodeTree_nodes = nullptr;
	PropertyRNA* BlenderPyNodeTree_outputs = nullptr;
	StructRNA* BlenderPyNodeTree_Tree = nullptr;

	void begin(CollectionPropertyIterator* it, const bNodeTree* nodeTree, PropertyRNA* prop) {
		auto cprop = (CollectionPropertyRNA*)prop;
		PointerRNA rna;
		rna.data = (void*)nodeTree;
		cprop->begin(it, &rna);
	}

	void next(CollectionPropertyIterator* it, PropertyRNA* prop) {
		auto cprop = (CollectionPropertyRNA*)prop;
		cprop->next(it);
	}

	void end(CollectionPropertyIterator* it, PropertyRNA* prop) {
		auto cprop = (CollectionPropertyRNA*)prop;
		cprop->end(it);
	}

	PointerRNA get(CollectionPropertyIterator* it, PropertyRNA* prop) {
		auto cprop = (CollectionPropertyRNA*)prop;
		return cprop->get(it);
	}

	/**Inputs**/
	void BlenderPyNodeTree::inputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree)
	{
		begin(it , nodeTree, BlenderPyNodeTree_inputs);
	}
	void BlenderPyNodeTree::inputs_next(CollectionPropertyIterator* it)
	{
		next(it, BlenderPyNodeTree_inputs);
	}
	void BlenderPyNodeTree::inputs_end(CollectionPropertyIterator* it)
	{
		end(it, BlenderPyNodeTree_inputs);
	}

	PointerRNA BlenderPyNodeTree::inputs_get(CollectionPropertyIterator* it)
	{
		return get(it, BlenderPyNodeTree_inputs);
	}

	void BlenderPyNodeTree::outputs_begin(CollectionPropertyIterator* it, bNodeTree* nodeTree)
	{
		begin(it, nodeTree, BlenderPyNodeTree_outputs);
	}

	void BlenderPyNodeTree::outputs_next(CollectionPropertyIterator* it)
	{
		next(it, BlenderPyNodeTree_outputs);
	}

	void BlenderPyNodeTree::outputs_end(CollectionPropertyIterator* it)
	{
		end(it, BlenderPyNodeTree_outputs);
	}

	PointerRNA BlenderPyNodeTree::outputs_get(CollectionPropertyIterator* it)
	{
		return get(it, BlenderPyNodeTree_outputs);
	}

	/*Outputs*/
	

	/*Nodes*/
	void BlenderPyNodeTree::nodes_begin(CollectionPropertyIterator* it, const bNodeTree* nodeTree) {
		begin(it, nodeTree, BlenderPyNodeTree_nodes);
	}

	void BlenderPyNodeTree::nodes_next(CollectionPropertyIterator* it) {
		next(it, BlenderPyNodeTree_nodes);
	}

	void BlenderPyNodeTree::nodes_end(CollectionPropertyIterator* it) {
		end(it, BlenderPyNodeTree_nodes);
	}

	PointerRNA BlenderPyNodeTree::nodes_get(CollectionPropertyIterator* it) {
		return get(it, BlenderPyNodeTree_nodes);
	}
}
