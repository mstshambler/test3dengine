/*
 * Per-pixel Phong Lighting.
 * One Light with Diffuse settings
 */

varying vec3 normal;
varying vec3 vertex;

uniform sampler3D tex;

uniform int lightType;

// point lighting
void PointLighting(in vec3 norm, inout vec4 ambient, inout vec4 diffuse) {
	// Calculate ambient
	ambient = gl_LightSource[1].ambient;

	// Calculate diffuse
	vec3 distV = gl_LightSource[1].position.xyz - vertex;
	vec3 distN = normalize(distV);
	float dist = length(distV);
	float attenuation = 1.0 / (gl_LightSource[1].constantAttenuation +
	              gl_LightSource[1].linearAttenuation * dist +
	              gl_LightSource[1].quadraticAttenuation * dist * dist);

	float nDotL = max(0.0, dot(norm, distN));

	if (nDotL > 0.0) {
		diffuse = gl_LightSource[1].diffuse  * attenuation * nDotL;
	}

	// TODO: add specular here
}

void SphereLighting(inout vec4 ambient, inout vec4 diffuse) {
	// Calculate ambient
	ambient = gl_LightSource[1].ambient;

	// Calculate diffuse
	vec3 distV = gl_LightSource[1].position.xyz - vertex;
	float dist = length(distV);
	float attenuation = 1.0 / (gl_LightSource[1].constantAttenuation +
	              gl_LightSource[1].linearAttenuation * dist +
	              gl_LightSource[1].quadraticAttenuation * dist * dist);

	diffuse = gl_LightSource[1].diffuse  * attenuation * 0.5;

	// TODO: add specular here
}

void main() {
	vec3 norm = normalize(normal);	
	vec4 diffuse = vec4(0.0);
	vec4 ambient = vec4(0.0);

	if (lightType == 1)
		PointLighting(norm, ambient, diffuse);
	else
		SphereLighting(ambient, diffuse);
	
	// Get tex coords
	vec4 texColor = texture3D(tex, gl_TexCoord[0].stp);

	// Color it using precalculated lighting
	vec4 color = ambient + diffuse * texColor;

	// Copy alpha
//	color[3]=gl_Color[3];

	// Clamp color to range 0..1
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color;
}

