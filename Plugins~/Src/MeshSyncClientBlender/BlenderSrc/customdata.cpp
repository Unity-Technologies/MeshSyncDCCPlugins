//TODO-sin: 2020-8-28: This file holds picked functions from Blender source code that are used to build MeshSync plugin.
//It should be possible to download and use Blender source directly.
//Paths of the original source code:
//- source\blender\blenkernel\intern\customdata.c


#include "pch.h"

int CustomData_get_layer_index(const CustomData *data, int type)
{
    BLI_assert(customdata_typemap_is_valid(data));
    return data->typemap[type];
}

int CustomData_get_layer_index_n(const struct CustomData *data, int type, int n)
{
    int i = CustomData_get_layer_index(data, type);

    if (i != -1) {
        BLI_assert(i + n < data->totlayer);
        i = (data->layers[i + n].type == type) ? (i + n) : (-1);
    }

    return i;
}

#if BLENDER_VERSION >= 305
const
#endif
void *CustomData_get_layer_n(const CustomData *data, int type, int n)
{
    /* get the layer index of the active layer of type */
    int layer_index = CustomData_get_layer_index_n(data, type, n);
    if (layer_index == -1) {
        return NULL;
    }

    return data->layers[layer_index].data;
}

int CustomData_number_of_layers(const CustomData *data, int type)
{
    int i, number = 0;
    for (i = 0; i < data->totlayer; i++) {
        if (data->layers[i].type == type) {
            number++;
        }
    }

    return number;
}

#if BLENDER_VERSION >= 305
const
#endif
void* CustomData_get_layer_named(const CustomData* data, const int type, const char* name)
{
    int layer_index = CustomData_get_named_layer_index(data, type, name);
    if (layer_index == -1) {
        return nullptr;
    }

    return data->layers[layer_index].data;
}

int CustomData_get_named_layer_index(const CustomData* data, const int type, const char* name)
{
    for (int i = 0; i < data->totlayer; i++) {
        if (data->layers[i].type == type) {
            if (STREQ(data->layers[i].name, name)) {
                return i;
            }
        }
    }

    return -1;
}