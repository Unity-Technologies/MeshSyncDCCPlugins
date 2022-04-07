#pragma once
#include <BKE_context.h> //bContext
#include "MeshUtils/muMath.h"

namespace blender
{
    /// <summary>
    /// A wrapper around the RNA callbacks for accessing properties and collections related 
    /// to the Depsgraph.
    /// </summary>
    class BlenderPyDepsgraph
    {
    public:

        /// <summary>
        /// Creates a wrapper from the given Depsgraph pointer.
        /// </summary>
        BlenderPyDepsgraph(Depsgraph* depsgraph);

        /// <summary>
        /// Sets the iterator to the begin of the object instances of Depsgraph.
        /// </summary>
        void object_instances_begin(CollectionPropertyIterator* it);

        /// <summary>
        /// Sets the iterator to the end of the object instances of Depsgraph.
        /// Will also cleanup resources used by the iterator.
        /// </summary>
        void object_instances_end(CollectionPropertyIterator* it);

        /// <summary>
        /// Moves the iterator to the next element of the object instances of Depsgraph.
        /// </summary>
        void object_instances_next(CollectionPropertyIterator* it);

        /// <summary>
        /// Fetches the iterator value
        /// </summary>
        PointerRNA object_instances_get(CollectionPropertyIterator* it);

    private:
        Depsgraph* m_depsgraph;
    };

}