#include "header.h"

float mapfloat(float x, long in_min, long in_max, long out_min, long out_max)
{
    return (float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min;
}

int mapint(float x, long in_min, long in_max, long out_min, long out_max)
{
    if (in_max == in_min) return out_min;
    return (int)((float)(x - in_min) * (float)(out_max - out_min) / (float)(in_max - in_min) + out_min);
}