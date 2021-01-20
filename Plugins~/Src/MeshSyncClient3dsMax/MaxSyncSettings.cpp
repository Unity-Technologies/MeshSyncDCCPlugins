#pragma once

#include "MaxSyncSettings.h"

void MaxSyncSettings::validate() {
    if (!BakeModifiers)
        BakeTransform = false;
}
