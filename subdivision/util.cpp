//
//  util.cpp
//  Reference: Assignment 3

#include "util.h"

void cross(Vector3f *result, Vector3f* lhs, Vector3f* rhs) {
    result->x = lhs->y * rhs->z - lhs->z * rhs->y;
    result->y = lhs->z * rhs->x - lhs->x * rhs->z;
    result->z = lhs->x * rhs->y - lhs->y * rhs->x;
}
