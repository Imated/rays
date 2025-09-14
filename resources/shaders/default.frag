#version 460 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D tex;

void main() {
    vec3 c = texture(tex, uv).rgb;
    FragColor = vec4(c, 1.0);
}