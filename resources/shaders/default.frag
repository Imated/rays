#version 460 core
out vec4 FragColor;

struct Material {
    vec3 color;
    float smoothness;
    vec3 emissiveColor;
    float emissiveStrength;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 pos;
    float radius;
    vec3 color;
    float smoothness;
    vec3 emissiveColor;
    float emissiveStrength;
};
layout (std430, binding = 0) buffer SphereBuffer {
    Sphere spheres[];
};

struct HitInfo {
    bool didHit;
    float distance; // t?  the multiple of rayDir (basically distance in units) hm
    vec3 hitPos;
    vec3 normal;
    Material material;
};

in vec2 uv;
in vec3 direction;

uniform uvec2 uResolution;
uniform sampler2D uPrevFrame;
uniform float uFocalLength;
uniform uint renderedFrames;
uniform vec3 cameraPosition;
uniform mat3 cameraRotation;
uniform int samplesPerPixel;
uniform bool shouldAccumulate;

float RandomValue(inout uint rngState) {
    rngState = rngState * 747796405u + 2891336453u;
    uint result = ((rngState >> ((rngState >> 28u) + 4u)) ^ rngState) * 277803737u;
    result = (result >> 22u) ^ result;
    return result / 4294967295.0;
}
float RandomValueNormalDistribution(inout uint rngState) {
    float u = clamp(RandomValue(rngState), 1e-7, 1.0 - 1e-7);
    float theta = 6.2831853 * RandomValue(rngState);
    float rho   = sqrt(-2.0 * log(u));
    return rho * cos(theta);
}
vec3 RandomDirection(inout uint rngState) {
    float x = RandomValueNormalDistribution(rngState);
    float y = RandomValueNormalDistribution(rngState);
    float z = RandomValueNormalDistribution(rngState);
    return normalize(vec3(x, y, z));
}

HitInfo intersectRaySphere(Ray ray, Sphere sphere) {
    vec3 oc = sphere.pos - ray.origin;
    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - sphere.radius*sphere.radius;
    float discriminant = b*b - 4.0 * a * c;

    HitInfo hitInfo;
    hitInfo.didHit = false;

    if (discriminant >= 0.0) {
        hitInfo.distance = (-b - sqrt(discriminant)) / (2.0 * a);

        if (hitInfo.distance > 0.0) {
            hitInfo.didHit = true;
            hitInfo.hitPos = ray.origin + ray.direction * hitInfo.distance;
            hitInfo.normal = normalize(hitInfo.hitPos - sphere.pos);
            Material material;
            material.color = sphere.color;
            material.emissiveColor = sphere.emissiveColor;
            material.emissiveStrength = sphere.emissiveStrength;
            material.smoothness = sphere.smoothness;
            hitInfo.material = material;
        }
    }
    return hitInfo;
}

HitInfo calculateRayIntersection(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.distance = 1.0 / 0.0;

    for (int i = 0; i < spheres.length(); i++) {
        Sphere sphere = spheres[i];
        HitInfo hitInfo = intersectRaySphere(ray, sphere);
        if (hitInfo.didHit && hitInfo.distance < closestHit.distance) {
            closestHit = hitInfo;
        }
    }
    return closestHit;
}

vec3 GetEnvironmentLight(Ray ray) {
    float a = 0.5*(ray.direction.y + 1.0);
    return mix(vec3(1), vec3(0.5, 0.7, 1.0), a);
}

uniform int maxBounces;
vec3 traceRay(Ray ray, inout uint rngState) {
    vec3 inLight = vec3(0.0);
    vec3 rayColor = vec3(1.0);
    for(int i = 0; i <= maxBounces; i++) {
        HitInfo info = calculateRayIntersection(ray);
        if(info.didHit) {
            ray.origin = info.hitPos + info.normal * 1e-4;
            Material material = info.material;
            vec3 diffuseDir = normalize(info.normal + RandomDirection(rngState));
            vec3 specularDir = reflect(normalize(ray.direction), info.normal);
            ray.direction = normalize(mix(diffuseDir, specularDir, clamp(material.smoothness, 0.0, 1.0)));
            vec3 emittedLight = material.emissiveColor * material.emissiveStrength;
            inLight += emittedLight * rayColor;
            rayColor *= material.color;
        }
        else
        {
            inLight += GetEnvironmentLight(ray) * rayColor;
            break;
        }
    }
    return inLight;
}

void main() {
    uvec2 pixelCoord = uvec2(uv * uResolution);
    uint rngState = pixelCoord.x * uResolution.x + pixelCoord.y + renderedFrames * 71933u;
    vec3 dir = cameraRotation * normalize(vec3(uv * uResolution - uResolution * 0.5, uFocalLength));
    Ray ray = Ray(cameraPosition, dir);

    vec3 curr = vec3(0);
    for(int rayIndex = 0; rayIndex < samplesPerPixel; rayIndex++)
        curr += traceRay(ray, rngState);
    curr /= float(samplesPerPixel);
    if(shouldAccumulate) {
        vec3 prev = texture(uPrevFrame, uv).rgb;
        float alpha = 1.0 / float(renderedFrames + 1u);
        vec3 blended = mix(prev, curr, alpha);
        FragColor = vec4(blended, 1.0);
    }
    else
        FragColor = vec4(curr, 1.0);
}