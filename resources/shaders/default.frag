#version 330 core
out vec4 FragColor;

struct Material {
    vec3 color;
    vec3 emissiveColor;
    float emissiveStrength;
    float smoothness;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Sphere {
    vec3 pos;
    float radius;
    Material material;
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

Sphere spheres[3];
uniform uvec2 uResolution;
uniform sampler2D uPrevFrame;
uniform float uFocalLength;
uniform uint renderedFrames;
uniform vec3 cameraPosition;
uniform mat3 cameraRotation;
uniform int samplesPerPixel;

float RandomValue(inout uint rngState) {
    rngState = rngState * 747796405u + 2891336453u;
    uint result = ((rngState >> ((rngState >> 28u) + 4u)) ^ rngState) * 277803737u;
    result = (result >> 22u) ^ result;
    return result / 4294967295.0;
}
float RandomValueNormalDistribution(inout uint rngState) {
    float theta = 2 * 3.1415926 * RandomValue(rngState);
    float rho = sqrt(-2 * log(RandomValue(rngState)));
    return rho * cos(theta);
}
vec3 RandomDirection(inout uint rngState) {
    float x = RandomValueNormalDistribution(rngState);
    float y = RandomValueNormalDistribution(rngState);
    float z = RandomValueNormalDistribution(rngState);
    return normalize(vec3(x, y, z));
}
vec3 RandomHemisphereDirection(vec3 normal, inout uint rngState) {
    vec3 dir = RandomDirection(rngState);
    return dir * sign(dot(normal, dir));
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
            hitInfo.material = sphere.material;
        }
    }
    return hitInfo;
}

HitInfo calculateRayIntersection(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.distance = 1.0 / 0.0;

    for (int i = 0; i < 3; i++) {
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
    return mix(vec3(0.8, 0.8, 0.8), vec3(0.5, 0.7, 1.0), a);
}

uniform int maxBounces;
vec3 traceRay(Ray ray, inout uint rngState) {
    vec3 inLight = vec3(0.0);
    vec3 rayColor = vec3(1.0);
    for(int i = 0; i <= maxBounces; i++) {
        HitInfo info = calculateRayIntersection(ray);
        if(info.didHit) {
            ray.origin = info.hitPos;
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
            //inLight += GetEnvironmentLight(ray) * rayColor;
            break;
        }
    }
    return inLight;
}

void main() {
    uvec2 pixelCoord = uvec2(uv * uResolution);
    uint rngState = pixelCoord.x * uResolution.x + pixelCoord.y + renderedFrames * 719393u;
    vec3 dir = cameraRotation * normalize(vec3(uv * uResolution - uResolution * 0.5, uFocalLength));
    Ray ray = Ray(cameraPosition, dir);
    Sphere sphere0 = Sphere(vec3(0.0, 0.0, 0.0), 1.0, Material(vec3(0, 1, 0), vec3(0), 0, 0));
    Sphere sphere1 = Sphere(cameraPosition, 1.0, Material(vec3(0, 0, 1), vec3(1, 0, 1), 4, 0));
    Sphere sphere2 = Sphere(vec3(0.0, -20.0, -1.0), 20.0, Material(vec3(0, 0, 1), vec3(0), 0, 0));
    spheres[0] = sphere0;
    spheres[1] = sphere1;
    spheres[2] = sphere2;

    vec3 prev = texture(uPrevFrame, uv).rgb;
    vec3 curr = vec3(0);
    for(int rayIndex = 0; rayIndex <= samplesPerPixel; rayIndex++)
        curr += traceRay(ray, rngState);
    curr /= samplesPerPixel;
    float alpha = 1.0 / float(renderedFrames + 1u);
    vec3 blended = mix(prev, curr, alpha);
    FragColor = vec4(blended, 1.0);
}