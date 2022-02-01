//Note-sin: 2022-2-1: This file holds picked functions from Blender source code (material.c) 
//that are used to build MeshSync plugin. 
//These functions might be slightly modified to satisfy project requirements, such as explicit type conversions, etc
//
//Path of the original source code:
//- source\blender\blenkernel\intern\material.c

#include "pch.h"

#undef rad2 //required before including "DNA_meta_types.h" 
#include "DNA_meta_types.h" //MetaBall


Material ***BKE_object_material_array_p(Object *ob)
{
  if (ob->type == OB_MESH) {
    Mesh *me = reinterpret_cast<Mesh*>(ob->data);
    return &(me->mat);
  }
  if (ELEM(ob->type, OB_CURVE, OB_FONT, OB_SURF)) {
    Curve *cu = reinterpret_cast<Curve*>(ob->data);
    return &(cu->mat);
  }
  if (ob->type == OB_MBALL) {
    MetaBall *mb = reinterpret_cast<MetaBall*>(ob->data);
    return &(mb->mat);
  }
  if (ob->type == OB_GPENCIL) {
    bGPdata *gpd = reinterpret_cast<bGPdata*>(ob->data);
    return &(gpd->mat);
  }
  if (ob->type == OB_HAIR) {
    Hair *hair = reinterpret_cast<Hair*>(ob->data);
    return &(hair->mat);
  }
  if (ob->type == OB_POINTCLOUD) {
    PointCloud *pointcloud = reinterpret_cast<PointCloud*>(ob->data);
    return &(pointcloud->mat);
  }
  if (ob->type == OB_VOLUME) {
    Volume *volume = reinterpret_cast<Volume*>(ob->data);
    return &(volume->mat);
  }
  return NULL;
}



short *BKE_object_material_len_p(Object *ob)
{
  if (ob->type == OB_MESH) {
    Mesh *me = reinterpret_cast<Mesh*>(ob->data);
    return &(me->totcol);
  }
  if (ELEM(ob->type, OB_CURVE, OB_FONT, OB_SURF)) {
    Curve *cu =reinterpret_cast<Curve*>( ob->data);
    return &(cu->totcol);
  }
  if (ob->type == OB_MBALL) {
    MetaBall *mb = reinterpret_cast<MetaBall*>(ob->data);
    return &(mb->totcol);
  }
  if (ob->type == OB_GPENCIL) {
    bGPdata *gpd = reinterpret_cast<bGPdata*>(ob->data);
    return &(gpd->totcol);
  }
  if (ob->type == OB_HAIR) {
    Hair *hair = reinterpret_cast<Hair*>(ob->data);
    return &(hair->totcol);
  }
  if (ob->type == OB_POINTCLOUD) {
    PointCloud *pointcloud = reinterpret_cast<PointCloud*>(ob->data);
    return &(pointcloud->totcol);
  }
  if (ob->type == OB_VOLUME) {
    Volume *volume = reinterpret_cast<Volume*>(ob->data);
    return &(volume->totcol);
  }
  return NULL;
}
