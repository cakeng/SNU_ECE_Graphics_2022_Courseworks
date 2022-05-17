#version 330
out vec4 FragColor;

in vec2 TexCoords;


// You can change the code whatever you want



uniform samplerCube environmentMap;


struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
    // phong shading coefficients
    vec3 Ka;
    vec3 Kd;
    vec3 Ks;
    float shininess;

    // reflect / refract
    vec3 R0; // Schlick approximation
    float ior; // index of refration

    // for refractive material
    vec3 extinction_constant;
    vec3 shadow_attenuation_constant;

    // 0 : phong
    // 1 : refractive dielectric
    // add more
    int material_type;
};

const int material_type_phong = 0;
const int material_type_refractive = 1;

// Just consider point light
struct Light{
    vec3 position;
    vec3 color;
    bool castShadow;
};
uniform vec3 ambientLightColor;

// hit information
struct HitRecord{
    float t;        // distance to hit point
    vec3 p;         // hit point
    vec3 normal;    // hit point normal
    Material mat;   // hit point material
};

// Geometry
struct Sphere {
    vec3 center;
    float radius;
    Material mat;
};

struct Plane {
    vec3 normal;
    vec3 p0;
    Material mat;
};

struct Box {
    vec3 box_min;
    vec3 box_max;
    Material mat;
};

struct Square {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 v3;
};

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    // we didn't add material to triangle because it requires so many uniform memory when we use mesh...
};


const int mat_phong = 0;
const int mat_refractive = 1;

uniform Material material_ground;
uniform Material material_box;
uniform Material material_gold;
uniform Material material_dielectric_glass;
uniform Material material_mirror;
uniform Material material_lambert;
uniform Material material_mesh;


Sphere spheres[] = Sphere[](
  Sphere(vec3(1,0.5,-1), 0.5, material_gold),
  Sphere(vec3(-1,0.5,-1), 0.5, material_gold),
  Sphere(vec3(0,0.5,1), 0.5, material_lambert),
  Sphere(vec3(1,0.5,0), 0.5, material_lambert)
);

Box boxes[] = Box[](
  //Box(vec3(0,0,0), vec3(0.5,1,0.5), dielectric),
  Box(vec3(2,0,-3), vec3(3,1,-2), material_box)
);

Plane groundPlane = Plane(vec3(0,1,0), vec3(0,0,0), material_ground);
Triangle mirrorTriangle = Triangle( vec3(-3,0,0), vec3(0,0,-4), vec3(-1, 4, -2));

Light lights[] = Light[](
    Light(vec3(3,5,3), vec3(1,1,1), true),
    Light(vec3(-3,5,3), vec3(0.5,0,0), false),
    Light(vec3(-3,5,-3), vec3(0,0.5,0), false),
    Light(vec3(3,5,-3), vec3(0,0,0.5), false)
);

// use this for mesh
/*
layout (std140) uniform mesh_vertices_ubo
{
    vec3 mesh_vertices[500];
};

layout (std140) uniform mesh_tri_indices_ubo
{
    ivec3 mesh_tri_indices[500];
};

uniform int meshTriangleNumber;*/

// Math functions
/* returns a varying number between 0 and 1 */
float rand(vec2 co) {
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

float max3 (vec3 v) {
  return max (max (v.x, v.y), v.z);
}

float min3 (vec3 v) {
  return min (min (v.x, v.y), v.z);
}

float drand48(vec2 co) {
  return 2 * fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453) - 1;
}

vec3 random_in_unit_disk(vec2 co) {
  vec3 p;
  int n = 0;
  do {
    p = vec3(drand48(co.xy), drand48(co.yx), 0);
    n++;
  } while (dot(p,p) >= 1.0 && n < 3);
  return p;
}

#define M_PI 3.1415926535897932384626433832795
uniform vec3 cameraPosition;
uniform vec3 cameraFront;
uniform mat3 cameraToWorldRotMatrix;
uniform float fovY; //set to 45
uniform float H;
uniform float W;

Ray getRay(vec2 uv){
    // TODO:
    float s = uv.x;
    float t = uv.y;
    float aspect = W/H;
    float theta = fovY * M_PI / 180.0;
    float half_height = tan (theta / 2.0);
    float half_width = aspect * half_height;
    float focus_dist = 5.0;
    float aperture = 0.1;
    float camera_lens_radius = aperture / 2.0;

    vec3 camera_origin = cameraPosition;
    vec3 w = normalize( -cameraFront );
    vec3 u = normalize( cross(w, vec3 (0.0, -1.0, 0.0)) );
    vec3 v = -cross(u, w);
    vec3 camera_lower_left_corner = camera_origin - (u * half_width * focus_dist) - (v * half_height * focus_dist) - (w * focus_dist);
    vec3 camera_horizontal  = u *  2 * half_width * focus_dist;
    vec3 camera_vertical  = v * 2 * half_height * focus_dist;

    vec3 rd = camera_lens_radius * random_in_unit_disk(vec2(s,t)) * 0.12;
    vec3 offset = vec3(s * rd.x, t * rd.y, 0);
    return Ray(camera_origin + offset, camera_lower_left_corner + s * camera_horizontal + t * camera_vertical - camera_origin - offset);
}


const float bias = 0.0001; // to prevent point too close to surface.

vec3 point_at_parameter(Ray r,float t) {
  return r.origin + t * r.direction;
}

bool sphere_hit(Sphere sp, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    vec3 oc = r.origin - sp.center;
    float a = dot(r.direction, r.direction);
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - sp.radius * sp.radius;
    float discriminant = b*b - a*c;
    if (discriminant > 0) {
        float temp = (-b - sqrt(b*b-a*c)) /a;
        if (temp < t_max && temp > t_min) {
        hit.t = temp;
        hit.p = point_at_parameter(r, hit.t);
        hit.normal = (hit.p - sp.center) / sp.radius;
        hit.mat = sp.mat;
        return true;
        }
        temp = (-b + sqrt(b*b-a*c)) /a;
        if (temp < t_max && temp > t_min) {
        hit.t = temp;
        hit.p = point_at_parameter(r, hit.t);
        hit.normal = (hit.p - sp.center) / sp.radius;
        hit.mat = sp.mat;
        return true;
        }
    }
    return false;
}

bool plane_hit(Plane p, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    float t = (p.p0.y - r.origin.y) / r.direction.y;
    if (t < t_min || t > t_max) return false;
    hit.t = t;
    hit.p = point_at_parameter(r, t);
    hit.mat = p.mat;
    hit.normal = vec3(0, 1, 0);
    return true;
}

bool triangle_hit(Triangle tri, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
    vec3 v0 = tri.v0;
    vec3 v1 = tri.v1;
    vec3 v2 = tri.v2;
    vec3 orig = r.origin;
    vec3 dir = r.direction;

    vec3 v0v1 = v1 - v0; 
    vec3 v0v2 = v2 - v0; 
    // no need to normalize
    vec3 N = cross(v0v1, v0v2);  //N 
 
    // Step 1: finding P
 
    // check if ray and plane are parallel ?
    float NdotRayDirection = dot(N, dir); 
    if (abs(NdotRayDirection) < 0.0001)  //almost 0 
        return false;  //they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float d = -dot(N, v0); 
 
    // compute t (equation 3)
    float t = -(dot(N, orig) + d) / NdotRayDirection; 
 
    // check if the triangle is in behind the ray
    if (t < 0) return false;  //the triangle is behind 
 
    // compute the intersection point using equation 1
    vec3 P = orig + t * dir; 

    if (t < t_min || t > t_max) return false;
 
    // Step 2: inside-outside test
    vec3 C;  //vector perpendicular to triangle's plane 
 
    // edge 0
    vec3 edge0 = v1 - v0; 
    vec3 vp0 = P - v0; 
    C = cross(edge0, vp0); 
    if (dot(N, C) < 0) return false;  //P is on the right side 
 
    // edge 1
    vec3 edge1 = v2 - v1; 
    vec3 vp1 = P - v1; 
    C = cross(edge1, vp1); 
    if (dot(N, C) < 0)  return false;  //P is on the right side 
 
    // edge 2
    vec3 edge2 = v0 - v2; 
    vec3 vp2 = P - v2; 
    C = cross(edge2, vp2); 
    if (dot(N, C) < 0) return false;  //P is on the right side; 

    hit.t = t;
    hit.p = P;
    hit.normal = N;
    if (dot (N, dir) > 0.0)
    {
        hit.normal = -N;
    }
    return true;
}

bool square_hit(Square tri, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
    vec3 v0 = tri.v0;
    vec3 v1 = tri.v1;
    vec3 v2 = tri.v2;
    vec3 v3 = tri.v3;
    vec3 orig = r.origin;
    vec3 dir = r.direction;

    vec3 v0v1 = v1 - v0; 
    vec3 v0v2 = v2 - v0; 
    // no need to normalize
    vec3 N = cross(v0v1, v0v2);  //N 
 
    // Step 1: finding P
 
    // check if ray and plane are parallel ?
    float NdotRayDirection = dot(N, dir); 
    if (abs(NdotRayDirection) < 0.0001)  //almost 0 
        return false;  //they are parallel so they don't intersect ! 
 
    // compute d parameter using equation 2
    float d = -dot(N, v0); 
 
    // compute t (equation 3)
    float t = -(dot(N, orig) + d) / NdotRayDirection; 
 
    // check if the triangle is in behind the ray
    if (t < 0) return false;  //the triangle is behind 
 
    // compute the intersection point using equation 1
    vec3 P = orig + t * dir; 

    if (t < t_min || t > t_max) return false;
 
    // Step 2: inside-outside test
    vec3 C;  //vector perpendicular to triangle's plane 
 
    // edge 0
    vec3 edge0 = v1 - v0; 
    vec3 vp0 = P - v0; 
    C = cross(edge0, vp0); 
    if (dot(N, C) < 0) return false;  //P is on the right side 
 
    // edge 1
    vec3 edge1 = v2 - v1; 
    vec3 vp1 = P - v1; 
    C = cross(edge1, vp1); 
    if (dot(N, C) < 0)  return false;  //P is on the right side 
 
    // edge 2
    vec3 edge2 = v3 - v2; 
    vec3 vp2 = P - v2; 
    C = cross(edge2, vp2); 
    if (dot(N, C) < 0) return false;  //P is on the right side; 

    // edge 3
    vec3 edge3 = v0 - v3; 
    vec3 vp3 = P - v3; 
    C = cross(edge3, vp3); 
    if (dot(N, C) < 0) return false;  //P is on the right side; 

    hit.t = t;
    hit.p = P;
    hit.normal = N;
    if (dot (N, dir) > 0.0)
    {
        hit.normal = -N;
    }
    return true;
}

bool box_hit(Box b, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    vec3 p0 = b.box_min;
    vec3 p7 = b.box_max;
    vec3 p1 = b.box_min + vec3(b.box_max.x - b.box_min.x, 0.0, 0.0);
    vec3 p2 = b.box_min + vec3(0.0, b.box_max.y - b.box_min.y, 0.0);
    vec3 p3 = b.box_min + vec3(0.0, 0.0, b.box_max.z - b.box_min.z);
    vec3 p4 = b.box_min + vec3(b.box_max.x - b.box_min.x, b.box_max.y - b.box_min.y, 0.0);
    vec3 p5 = b.box_min + vec3(0.0, b.box_max.y - b.box_min.y, b.box_max.z - b.box_min.z);
    vec3 p6 = b.box_min + vec3(b.box_max.x - b.box_min.x, 0.0, b.box_max.z - b.box_min.z);

    Square sq0 = Square (p0, p1, p4, p2);
    Square sq1 = Square (p0, p1, p6, p3);
    Square sq2 = Square (p0, p2, p5, p3);
    Square sq3 = Square (p1, p4, p7, p6);
    Square sq4 = Square (p2, p4, p7, p5);
    Square sq5 = Square (p3, p5, p7, p6);

    bool result = false;
    float temp_max = t_max;
    HitRecord temp_hit;

    if (square_hit (sq0, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }
    if (square_hit (sq1, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }
    if (square_hit (sq2, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }
    if (square_hit (sq3, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }
    if (square_hit (sq4, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }
    if (square_hit (sq5, r, t_min, temp_max, temp_hit))
    {
        hit = temp_hit;
        temp_max = hit.t;
        result = true;
    }

    if (result)
    {
        hit.mat = b.mat;
    }
    return result;
}


float schlick(float cosine, float r0) {
    // TODO:

    return 0.0;
}

vec3 schlick(float cosine, vec3 r0) {
    // TODO:

    return vec3 (0.0, 0.0, 0.0);
}

bool trace(Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO: trace single ray.
    HitRecord temp_hit;
    bool hit_anything = false;
    float closest_so_far = t_max;
    
    if (triangle_hit(mirrorTriangle, r, t_min, closest_so_far, temp_hit)) {
        hit_anything = true;
        hit = temp_hit;
        hit.mat = material_mirror;
        closest_so_far = temp_hit.t;
    }
    for (int i = 0; i < spheres.length(); i++) {
        if (sphere_hit(spheres[i], r, t_min, closest_so_far, temp_hit)) {
        hit_anything = true;
        hit = temp_hit;
        closest_so_far = temp_hit.t;
        }
    }
    for (int i = 0; i < boxes.length(); i++) {
        if (box_hit(boxes[i], r, t_min, closest_so_far, temp_hit)) {
        hit_anything = true;
        hit = temp_hit;
        closest_so_far = temp_hit.t;
        }
    }
    
    if (plane_hit(groundPlane, r, t_min, closest_so_far, temp_hit)) {
        hit_anything = true;
        hit = temp_hit;
        closest_so_far = temp_hit.t;
    }
    
    return hit_anything;
}

bool dispatch_scatter(in Ray r, HitRecord hit, out vec3 attenuation, out Ray scattered) {
    attenuation = vec3 (0.0, 0.0, 0.0);
    for (int i = 0; i < lights.length(); i++)
    {
        float shadow = 0.0f;

        vec3 lightDir = normalize(lights[i].position - hit.p);
        vec3 lightCol = lights[i].color;

        vec3 ambient = lightCol * 0.3 * hit.mat.Ka;
        vec3 specular = vec3 (0.0, 0.0, 0.0);
        vec3 diffuse = vec3 (0.0, 0.0, 0.0);

        if (ambient[0] < 0.0f)
        {
            ambient[0] = 0.0;
        }
        if (ambient[1] < 0.0f)
        {
            ambient[1] = 0.0;
        }
        if (ambient[2] < 0.0f)
        {
            ambient[2] = 0.0;
        }

        float diff = max(dot(hit.normal, lightDir), 0.0);
        diffuse = lightCol * diff * hit.mat.Kd; 
        if (diffuse[0] < 0.0f)
        {
            diffuse[0] = 0.0;
        }
        if (diffuse[1] < 0.0f)
        {
            diffuse[1] = 0.0;
        }
        if (diffuse[2] < 0.0f)
        {
            diffuse[2] = 0.0;
        }
        
        vec3 viewDir = normalize(r.origin - hit.p);
        vec3 reflectDir = reflect(-lightDir, hit.normal);  
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), hit.mat.shininess);
        specular = lightCol * spec * hit.mat.Ks;  
        if (specular[0] < 0.0f)
        {
            specular[0] = 0.0;
        }
        if (specular[1] < 0.0f)
        {
            specular[1] = 0.0;
        }
        if (specular[2] < 0.0f)
        {
            specular[2] = 0.0;
        }
        if (lights[i].castShadow)
        {
            Ray shadowRay = Ray (hit.p, lightDir);
            HitRecord shadowRec;
            if (trace (shadowRay, 0.01, 99, shadowRec))
            {
                shadow = 1.0;
            }
        }
        attenuation += ambient + (1.0 - shadow) * (diffuse + specular);
    }
    scattered.origin = hit.p;
    scattered.direction = reflect(r.direction, hit.normal);  
//   if(hit.mat.scatter_function == mat_dielectric) {
//     return dielectric_scatter(hit.mat, r, hit, attenuation, scattered);
//   } else if (hit.mat.scatter_function == mat_metal) {
//     return metal_scatter(hit.mat, r, hit, attenuation, scattered);
//   } else {
//     return lambertian_scatter(hit.mat, r, hit, attenuation, scattered);
//   }
    return true;
}

const int MAX_DEPTH = 4; // maximum bounce

vec3 castRay(Ray r){
    // TODO: trace ray in iterative way.
    HitRecord hit;
    vec3 col = vec3(0.0, 0.0, 0.0); /* visible color */
    vec3 reflectivity [MAX_DEPTH];
    vec3 colors [MAX_DEPTH];
    int bounce = 0;
  for (; bounce < MAX_DEPTH ; bounce++) {

    if (trace(r, bias, 1.0 / 0.0, hit)) 
    {
      /* create a new reflected ray */
      Ray scattered;
      dispatch_scatter(r, hit, colors[bounce], scattered);
      reflectivity[bounce] = hit.mat.R0;
        r = scattered;
    } else {
      /* background hit (light source) */
      vec3 unit_dir = normalize(r.direction);
      float t = 0.5 * (unit_dir.y + 1.0);
      colors[bounce] = ((1.0-t)*vec3(1.0,1.0,1.0)+t*vec3(0.5,0.7,1.0));
      reflectivity[bounce] = vec3(0.0);
    }
  }
  for (int i = MAX_DEPTH-1; i >= 0; i--)
  {
      col = col*reflectivity[i] + colors[i]*(vec3(1.0)-reflectivity[i]);
  }
  return col;
}

void main()
{
    // TODO:
    const int nsamples = 1;
    vec3 color = vec3(0);
    Ray r = getRay(TexCoords);
    color += castRay(r);
    color /= nsamples;
    // color = r.direction;
    FragColor = vec4(color, 1.0);
}
