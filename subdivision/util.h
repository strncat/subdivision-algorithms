//
// Assignment 3
//

#ifndef UTIL_H
#define UTIL_H

#include <math.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <math.h>
#include <iostream>
#include <vector>

#define EPSILON 0.0001

struct Vector3f {
    float x, y, z;
    Vector3f() { x = 0.;  y = 0.; z = 0.; }
    Vector3f(float _x, float _y, float _z) { x = _x; y = _y; z = _z;}
    Vector3f operator*(float s) { return Vector3f(x*s, y*s, z*s); }
    Vector3f operator/(float s) { return Vector3f(x/s, y/s, z/s); }
    Vector3f operator-() { return Vector3f(-x, -y, -z); }
    Vector3f operator-(Vector3f const o) { return Vector3f(x - o.x, y - o.y, z - o.z); }
    Vector3f operator+(Vector3f const o) { return Vector3f(x + o.x, y + o.y, z + o.z); }
    float length() { return sqrtf(x*x + y*y + z*z); }
    void normalize() {
        float l = length();
        x = x/l;
        y = y/l;
        z = z/l;
    }
    bool operator == (const Vector3f& v) {
        return (-EPSILON < x - v.x && x - v.x < EPSILON)
        && (-EPSILON < y - v.y && y - v.y < EPSILON)
        && (-EPSILON < z - v.z && z - v.z < EPSILON);
    }
    void print() {
        printf("%.4f %.4f %.4f\n", x, y, z);
    }
};

void cross(Vector3f *result, Vector3f* lhs, Vector3f* rhs);

#endif
