#version 440
in vec2 var_uv;
out vec4 out_color;

const int MAX_STEPS = 100;
const float MAX_DIST = 100.;
const float SURF_DIST = .001;

uniform int highlight_factor;
uniform float dt;
uniform float blend_factor;
uniform vec3 eye;
uniform vec3 vec_front;
uniform vec3 vec_right;
uniform vec3 vec_up;

float coef(float value, float min_, float max_)
{
	return (value - min_)/(max_-min_);
}

float map(float value, float in_min, float in_max, float out_min, float out_max)
{
	return mix(out_min, out_max, coef(value, in_min, in_max));
}
vec3 map(float value, float in_min, float in_max, vec3 out_min, vec3 out_max)
{
	return out_min + (out_max-out_min) * coef(value, in_min, in_max);
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 ab = b-a;
    vec3 ap = p-a;
    
    float t = dot(ab, ap) / dot(ab, ab);
    t = clamp(t, 0., 1.);
    
    vec3 c = a + t*ab;
    
    return length(p-c)-r;
}
float sdCylinder(vec3 p, vec3 a, vec3 b, float r)
{
	vec3 ab = b-a;
    vec3 ap = p-a;
    
    float t = dot(ab, ap) / dot(ab, ab);
    //t = clamp(t, 0., 1.);
    
    vec3 c = a + t*ab;
    
    float x = length(p-c)-r;
    float y = (abs(t-.5)-.5)*length(ab);
    float e = length(max(vec2(x, y), 0.));
    float i = min(max(x, y), 0.);
    
    return e+i;
}
float sdTorus(vec3 p, vec2 r)
{
	float x = length(p.xz)-r.x;
    return length(vec2(x, p.y))-r.y;
}
float dBox(vec3 p, vec3 s)
{
	return length(max(abs(p)-s, 0.));
}
float sdSphere(vec3 p, vec3 s, float r)
{
	return length(p-s)-r;
}
float sdPlane(vec3 p, float h)
{
	return p.y-h;
}

float min_smooth(float a, float b)
{
	float k = blend_factor;
	float h = 0.0f;
	if(k > 0.0f)
		h = max(k-abs(a-b), 0)/k;
	return min(a,b) - h*h*h*k/6.0;
}

float GetDist(vec3 p) {
    float sd = sdSphere(p, vec3(3*sin(dt),1 +sin(0.5*dt),6), 1);
    float td = sdTorus(p-vec3(0,0,6), vec2(2, .5));
    float bd = dBox(p-vec3(1 +sin(0.5*dt), 1+3* sin(dt), 6), vec3(1,.75,1));
    
    float d1 = min_smooth(sd, td);
    float d2 = min_smooth(d1, bd);
    return d2;
}
float GetDist(vec3 p, out vec3 color) {
    float sd = sdSphere(p, vec3(3*sin(dt),1 +sin(0.5*dt),6), 1);
    float td = sdTorus(p-vec3(0,0,6), vec2(2, .5));
    float bd = dBox(p-vec3(1 +sin(0.5*dt), 1+3* sin(dt), 6), vec3(1,.75,1));
    
    float d1 = min_smooth(sd, td);
	color = map(d1, sd, td, vec3(1., .0, .0), vec3(.0, 1., .0));
	
    float d2 = min_smooth(d1, bd);
    color = map(d2, d1, bd, color, vec3(.0, .0, 1.));
    return d2;
}

float RayMarch(vec3 ro, vec3 rd) {
	float dO=0.;
    
    for(int i=0; i<MAX_STEPS; i++) {
    	vec3 p = ro + rd*dO;
        float dS = GetDist(p);
        dO += dS;
        if(dO>MAX_DIST || dS<SURF_DIST) break;
    }
    
    return dO;
}
float RayMarch(vec3 ro, vec3 rd, out int steps) {
	float dO=0.;
    
    for(steps=0; steps<MAX_STEPS; steps++) {
    	vec3 p = ro + rd*dO;
        float dS = GetDist(p);
        dO += dS;
        if(dO>MAX_DIST || dS<SURF_DIST) break;
    }
    return dO;
}

vec3 GetNormal(vec3 p) {
	float d = GetDist(p);
    vec2 e = vec2(.001, 0);
    
    vec3 n = d - vec3(
        GetDist(p-e.xyy),
        GetDist(p-e.yxy),
        GetDist(p-e.yyx));
    
    return normalize(n);
}

vec3 GetLight(vec3 p) {
    vec3 lightPos = vec3(3, 3, -3);
    vec3 l = normalize(lightPos-p);
    vec3 n = GetNormal(p);
    
	vec3 color;
	GetDist(p, color);
    color *= clamp(dot(n, l), 0.0, 1.);
    float d = RayMarch(p+n*SURF_DIST*2., l);
    return color;
}

void main()
{
	vec3 ro = eye;
	float rat = 1080./1920.;
    vec3 rd = vec_front + map(var_uv.x, 0., 1., -vec_right, vec_right) + map(var_uv.y, 0., 1., -vec_up, vec_up);
	int steps;
    float d = RayMarch(ro, rd, steps);
    
    vec3 p = ro + rd * d;
    
    vec3 dif = GetLight(p);
	if(steps > highlight_factor)
		dif = vec3(1.0f);
	out_color = vec4(dif, 1.0f);
}