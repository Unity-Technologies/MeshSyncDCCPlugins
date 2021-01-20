#pragma once

#include "BlenderSyncSettings.h"

void BlenderSyncSettings::validate() {
    if (!BakeModifiers)
        BakeTransform = false;
}

