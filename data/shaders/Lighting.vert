/*
 * Per-pixel Phong Lighting.
 * One Light with Diffuse settings
 */

varying vec3 normal;
varying vec3 vertex;

void main() {
	// Calculate the normal
	normal = normalize(gl_NormalMatrix * gl_Normal);
	
	// Transform the vertex position to eye space
	vertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
} 
