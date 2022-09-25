uniform mat4 mat_view_proj;


#if COMPILE_VERTEX_SHADER == 1

layout(location = 0) in vec3 vert_position;
layout(location = 1) in vec2 vert_uv;
layout(location = 2) in vec4 vert_color;

out vec2 vert_to_frag_uv;
out vec4 vert_to_frag_color;

void main() {
    gl_Position = mat_view_proj * vec4(vert_position, 1.0);
    vert_to_frag_uv = vert_uv;
    vert_to_frag_color = vert_color;
}

#elif COMPILE_FRAGMENT_SHADER == 1

layout(binding = 0) uniform sampler2D uColorTexture;

in vec2 vert_to_frag_uv;
in vec4 vert_to_frag_color;

out vec4 frag_color;

void main() {
    frag_color = texture(uColorTexture, vert_to_frag_uv);
}
#endif