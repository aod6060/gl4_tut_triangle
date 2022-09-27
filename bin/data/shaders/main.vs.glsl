/*
    main.vs.glsl

    This vertex shader
*/
#version 400

// Attributes
layout(location = 0) in vec3 vertices;
layout(location = 1) in vec4 colors;

// Uniforms
uniform mat4 proj; // Projection Matrix
uniform mat4 view; // View Matrix
uniform mat4 model; // Model Matrix

// Varying
out vec4 v_Color;

void main() {
    gl_Position = proj * view * model * vec4(vertices, 1.0);
    v_Color = colors;
}