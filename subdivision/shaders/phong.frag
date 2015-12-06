//
// phong.frag
//
//

#version 120

#define NUM_LIGHTS 2

varying vec3 lightColor;
varying vec3 eyeNormal;
varying vec3 eyeDirection;
varying vec3 lightDirection[NUM_LIGHTS];

// an ambient reflection constant, the ratio of reflection of the ambient
// term present in all points in the scene rendered
uniform vec3 Ka;

// diffuse reflection constant, the ratio of reflection of the diffuse
// term of incoming light
uniform vec3 Kd;

// specular reflection constant, the ratio of reflection of the specular
// term of incoming light
uniform vec3 Ks;

float shininess = 100.0f;

void main() {
    vec3 N = normalize(eyeNormal);
    vec3 V = normalize(eyeDirection);

    // ambient term
    vec3 ambient = Ka * lightColor;

    // initial color
    vec3 color = ambient;

    for (int i = 0; i < NUM_LIGHTS; i++) {
        vec3 L = normalize(lightDirection[i]);
        vec3 H = normalize(L+V); // using the half vector instead of dot(V,R)

        // diffuse term
        float diffuseLight = max(dot(N, L), 0);
        vec3 diffuse = Kd * lightColor * diffuseLight;

        // specular term
        float specularLight = pow(max(dot(N, H), 0.0), shininess);
        if (diffuseLight < 0) { specularLight = 0; }
        vec3 specular = Ks * lightColor * specularLight;

        color += (diffuse + specular);
    }

    // blinnPhong
    gl_FragColor = vec4(color, 1.0);
}
