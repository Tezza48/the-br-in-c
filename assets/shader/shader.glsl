#if COMPILE_VERTEX_SHADER == 1

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec2 vert_uv;

out vec2 vert_to_frag_uv;

void main() {
    gl_Position = vec4(vert_position, 1.0);
    vert_to_frag_uv = vert_uv;
}

#elif COMPILE_FRAGMENT_SHADER == 1
in vec2 vert_to_frag_uv;

out vec4 frag_color;

void main() {
    frag_color = vec4(vert_to_frag_uv, 0.0, 1.0);
}
#endif