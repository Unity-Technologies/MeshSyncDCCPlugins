#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

namespace MeshSyncClient {

void BaseSyncSettings::Validate() {
    if (!BakeModifiers)
        BakeTransform = false;

}

} // namespace MeshSyncClient
