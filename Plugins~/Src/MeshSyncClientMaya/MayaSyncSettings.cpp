#pragma once

#include "MayaSyncSettings.h"

void MayaSyncSettings::validate() {

    if (!BakeModifiers)
        BakeTransform = false;
}

