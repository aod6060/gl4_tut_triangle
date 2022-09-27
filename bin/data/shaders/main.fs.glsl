/*
    main.fs.glsl

    This our fragment shader
*/
#version 400

in vec4 v_Color;

out vec4 out_Color;

void main() {
    //out_Color = v_Color;


    vec3 color = 1.0 - v_Color.rgb;

    out_Color = vec4(color, v_Color.a);
}