#include <BlenderPyObjects/BlenderPyDepsgraph.h>"
#include <BLI_listbase.h>

namespace blender {

    PropertyRNA* BlenderPyDepsgraph_object_instances = nullptr;

    BlenderPyDepsgraph::BlenderPyDepsgraph(Depsgraph* depsgraph)
    {
        m_depsgraph = depsgraph;
    }

    void BlenderPyDepsgraph::object_instances_begin(CollectionPropertyIterator* it) {

        PointerRNA rna;
        rna.data = m_depsgraph;

        CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyDepsgraph_object_instances;

        cprop->begin(it, &rna);
    }

    void BlenderPyDepsgraph::object_instances_end(CollectionPropertyIterator* it) {

        CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyDepsgraph_object_instances;
        cprop->end(it);
    }

    void BlenderPyDepsgraph::object_instances_next(CollectionPropertyIterator* it) {
        CollectionPropertyRNA* cprop = (CollectionPropertyRNA*)BlenderPyDepsgraph_object_instances;
        cprop->next(it);
    }

    PointerRNA BlenderPyDepsgraph::object_instances_get(CollectionPropertyIterator* it) {

        auto collectionProp = (CollectionPropertyRNA*)BlenderPyDepsgraph_object_instances;
        return collectionProp->get(it);
    }
}