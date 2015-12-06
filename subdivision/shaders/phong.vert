//
// phong.vert
//
//

#version 120

#define NUM_LIGHTS 2

attribute vec3 in_position;
attribute vec3 in_normal;

//uniform mat4 modelMatrix;
//uniform mat4 viewMatrix;
//uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelViewProjectionMatrix;

uniform vec3 eyePosition;
uniform vec3 lightPosition[NUM_LIGHTS];
uniform vec3 color;

varying vec3 lightColor;
varying vec3 eyeNormal;
varying vec3 eyeDirection;
varying vec3 lightDirection[NUM_LIGHTS];

void main() {
    // final position of the vertex
    gl_Position = modelViewProjectionMatrix * vec4(in_position, 1.0);

    // normal vector
    eyeNormal = normalize(normalMatrix * in_normal);

    // color
    lightColor = color;

    // position in camera/eye space
    // vec4 positionC = viewMatrix * modelMatrix * vec4(in_position, 1.0);

    // eye direction
    eyeDirection = normalize(eyePosition - in_position); // -in_position

    // light direction
    for (int i = 0; i < NUM_LIGHTS; i++) {
        lightDirection[i] = normalize(lightPosition[i] - in_position);
    }
}
