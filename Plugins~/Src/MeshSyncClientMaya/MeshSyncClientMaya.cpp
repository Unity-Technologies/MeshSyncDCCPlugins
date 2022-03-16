#include "pch.h"

#ifdef _WIN32
#pragma comment(lib, "Foundation.lib")
#pragma comment(lib, "OpenMaya.lib")
#pragma comment(lib, "OpenMayaAnim.lib")
#endif


void msmayaInitialize(MObject& obj);
void msmayaUninitialize();

#if MAYA_YEAR == 2017
msAPI
#endif
MStatus initializePlugin(MObject obj)
{
    msmayaInitialize(obj);
    return MS::kSuccess;
}

#if MAYA_YEAR == 2017
msAPI
#endif
MStatus uninitializePlugin(MObject obj)
{
    msmayaUninitialize();
    return MS::kSuccess;
}
