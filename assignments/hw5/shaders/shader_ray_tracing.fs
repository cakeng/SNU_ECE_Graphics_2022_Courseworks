#version 330
out vec4 FragColor;

in vec2 TexCoords;


// You can change the code whatever you want


const int MAX_DEPTH = 4; // maximum bounce
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

struct Triangle {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    Material mat;
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
Triangle mirrorTriangle = Triangle( vec3(-3,0,0), vec3(0,0,-4), vec3(-1, 4, -2), material_gold);

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
    vec3 w = normalize( cameraFront );
    vec3 u = normalize( cross(w, vec3 (0.0, 1.0, 0.0)) );
    vec3 v = cross(u, w);
    vec3 camera_lower_left_corner = camera_origin - (u * half_width * focus_dist) - (v * half_height * focus_dist) - (w * focus_dist);
    vec3 camera_horizontal  = u *  2 * half_width * focus_dist;
    vec3 camera_vertical  = v * 2 * half_height * focus_dist;

    vec3 rd = camera_lens_radius * random_in_unit_disk(vec2(s,t)) * 0.1;
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
    float t = (-0.5 - r.origin.y) / r.direction.y;
    if (t < t_min || t > t_max) return false;
    hit.t = t;
    hit.p = point_at_parameter(r, t);
    hit.mat = p.mat;
    hit.normal = vec3(0, 1, 0);
    return true;
}

bool box_hit(Box b, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:


    return false;
}

bool triangle_hit(Triangle tri, Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO:
    vec3 tri_normal = normalize(cross((tri.v0 - tri.v1), (tri.v2 - tri.v1)));
    if (dot (tri_normal, r.direction) > 0.0)
    {
        tri_normal = -tri_normal;
    }
    vec3 r1 = r.origin + t_min*r.direction;
    vec3 r2 = r.origin + t_max*r.direction;
    float d1 = dot ((r1 - tri.v0), tri_normal);
    float d2 = dot ((r2 - tri.v0), tri_normal);
    if (d1*d2 >= 0)
    {
        return false;
    }
    if (d1 == d2)
    {
        return false;
    }
    vec3 intersect = r1 + (r2-r1) * (-d1/(d2-d1));
    vec3 test = normalize(cross(tri_normal, tri.v1 - tri.v0));
    if (dot (test, intersect - tri.v0) < 0.0)
    {
        return false;
    }
    test = normalize(cross(tri_normal, tri.v2 - tri.v1));
    if (dot (test, intersect - tri.v1) < 0.0)
    {
        return false;
    }
    test = normalize(cross(tri_normal, tri.v0 - tri.v2));
    if (dot (test, intersect - tri.v2) < 0.0)
    {
        return false;
    }

    hit.t = dot (r.direction, intersect - r.origin);
    hit.p = intersect;
    hit.normal = tri_normal;
    hit.mat = tri.mat;
    return true;
}

float schlick(float cosine, float r0) {
    // TODO:

    return 0.0;
}

vec3 schlick(float cosine, vec3 r0) {
    // TODO:

    return vec3 (0.0, 0.0, 0.0);
}

bool dispatch_scatter(in Ray r, HitRecord hit, out vec3 attenuation, out Ray scattered) {
//   if(hit.mat.scatter_function == mat_dielectric) {
//     return dielectric_scatter(hit.mat, r, hit, attenuation, scattered);
//   } else if (hit.mat.scatter_function == mat_metal) {
//     return metal_scatter(hit.mat, r, hit, attenuation, scattered);
//   } else {
//     return lambertian_scatter(hit.mat, r, hit, attenuation, scattered);
//   }
    attenuation = hit.mat.Kd;
    scattered = r;
    return true;
}

bool trace(Ray r, float t_min, float t_max, out HitRecord hit){
    // TODO: trace single ray.
    HitRecord temp_hit;
    bool hit_anything = false;
    float closest_so_far = t_max;

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
    if (triangle_hit(mirrorTriangle, r, t_min, closest_so_far, temp_hit)) {
        hit_anything = true;
        hit = temp_hit;
        closest_so_far = temp_hit.t;
    }
    return hit_anything;
}

vec3 castRay(Ray r){
    // TODO: trace ray in iterative way.
    HitRecord hit;
    vec3 col = vec3(0.2, 0.1, 0.2); /* visible color */
    vec3 total_attenuation = vec3(1.0, 1.0, 1.0); /* reduction of light transmission */

  for (int bounce = 0; bounce < 1; bounce++) {

    if (trace(r, bias, 1.0 / 0.0, hit)) {
      /* create a new reflected ray */
      Ray scattered;
      vec3 local_attenuation;
      if (dispatch_scatter(r, hit, local_attenuation, scattered)) {
        total_attenuation *= local_attenuation;
        r = scattered;
        col = total_attenuation;
      } else {
        total_attenuation *= vec3(0,0,0);
      }
    } else {
      /* background hit (light source) */
      vec3 unit_dir = normalize(r.direction);
      float t = 0.5 * (unit_dir.y + 1.0);
      col = total_attenuation * ((1.0-t)*vec3(1.0,1.0,1.0)+t*vec3(0.5,0.7,1.0));
      break;
    }
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
