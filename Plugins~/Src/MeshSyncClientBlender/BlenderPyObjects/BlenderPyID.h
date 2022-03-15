#pragma once

#include "msblenMacros.h" //MSBLEN_BOILERPLATE2
#include "DNA_ID.h" //ID

struct Depsgraph;

namespace blender
{

class BlenderPyID
{
public:
    MSBLEN_BOILERPLATE2(BlenderPyID, ID)

    const char *name() const;
    bool is_updated() const;
    bool is_updated_data() const;
    ID* evaluated_get(Depsgraph *depsgraph);
    void update_tag();
};


} // namespace blender
