#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 TexCoords;

out vec2 uv;
out vec3 direction;
uniform uvec2 uResolution;

void main() 
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    uv = TexCoords;
    direction = normalize(vec3(uResolution * uv - uResolution / 2u, uResolution.y / 2u));
}
