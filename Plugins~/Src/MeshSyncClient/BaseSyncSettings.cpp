#pragma once

#include "MeshSyncClient/BaseSyncSettings.h"

namespace MeshSyncClient {

void BaseSyncSettings::validate() {
    if (!BakeModifiers)
        BakeTransform = false;

}

} // namespace MeshSyncClient
