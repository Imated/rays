#version 330 core
out vec4 FragColor;

struct Material {
    vec3 color;
    vec3 emissiveColor;
    float emissiveStrength;
};

struct Ray {
    vec3 position;
    vec3 direction;
};

struct Sphere {
    vec3 pos;
    float radius;
    Material material;
};

struct HitInfo {
    bool didHit;
    float t; // t?  the multiple of rayDir (basically distance in units) hm
    vec3 hitPos;
    vec3 normal;
    Material material;
};

in vec2 uv;
in vec3 direction;

Sphere spheres[2];
uniform uvec2 uResolution;
uniform sampler2D uPrevFrame;
uniform uint renderedFrames;

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
    vec3 oc = sphere.pos - ray.position;
    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - sphere.radius*sphere.radius;
    float discriminant = b*b - 4.0 * a * c;

    HitInfo hitInfo;
    hitInfo.didHit = false;
    hitInfo.t = 1e20;

    if (discriminant >= 0.0) {
        float t0 = (-b - sqrt(discriminant)) / (2.0 * a);
        float t1 = (-b + sqrt(discriminant)) / (2.0 * a);

        hitInfo.t = t0;
        if (hitInfo.t <= 0.0) hitInfo.t = t1;

        if (hitInfo.t > 0.0) {
            hitInfo.didHit = true;
            hitInfo.hitPos = ray.position + ray.direction * hitInfo.t;
            hitInfo.normal = normalize(hitInfo.hitPos - sphere.pos);
            hitInfo.material = sphere.material;
        }
    }
    return hitInfo;
}

HitInfo calculateRayIntersection(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.t = 1.0 / 0.0;

    for (int i = 0; i < 2; i++) {
        Sphere sphere = spheres[i];
        HitInfo hitInfo = intersectRaySphere(ray, sphere);
        if (hitInfo.didHit && hitInfo.t < closestHit.t) {
            closestHit = hitInfo;
        }
    }
    return closestHit;
}

vec3 GetEnvironmentLight(Ray ray) {
    float a = 0.5*(ray.direction.y + 1.0);
    return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), a);
}

uniform int maxBounces;
vec3 traceRay(Ray ray, inout uint rngState) {
    vec3 inLight = vec3(0.0);
    vec3 rayColor = vec3(1.0);
    for(int i = 0; i <= maxBounces; i++) {
        HitInfo info = calculateRayIntersection(ray);
        if(info.didHit) {
            ray.position = info.hitPos;
            ray.direction = info.normal + RandomDirection(rngState);

            Material material = info.material;
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
    uint rngState = pixelCoord.x * uResolution.x + pixelCoord.y + renderedFrames * 719393u;
    Ray ray = Ray(vec3(0.0, 0.0, -5.0), normalize(direction));
    Sphere sphere0 = Sphere(vec3(0.0), 1.0, Material(vec3(0, 1, 0), vec3(0), 0));
    Sphere sphere1 = Sphere(vec3(0.0, 2.0, -1.0), 1.0, Material(vec3(1.0), vec3(1, 1, 1), 2));
    spheres[0] = sphere0;
    spheres[1] = sphere1;

    vec3 prev = texture(uPrevFrame, uv).rgb;
    vec3 curr = traceRay(ray, rngState);
    float alpha = 1.0 / float(renderedFrames + 1u);
    vec3 blended = mix(prev, curr, alpha);
    FragColor = vec4(blended, 1.0);
}