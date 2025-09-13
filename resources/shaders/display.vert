#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 TexCoords;

out vec2 uv;
out vec3 direction;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    uv = TexCoords;
    direction = normalize(vec3(800*uv.x-400, 600*uv.y-300, 300)); // :thumbsup:real👍🏿
}
