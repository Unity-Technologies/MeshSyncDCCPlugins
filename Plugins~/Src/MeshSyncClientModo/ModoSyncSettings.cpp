#pragma once

#include "ModoSyncSettings.h"

void ModoSyncSettings::validate() {

    if (!BakeModifiers)
        BakeTransform = false;
}

