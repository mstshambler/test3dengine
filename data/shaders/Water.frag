varying vec3 normal;
varying vec3 vertex;

uniform sampler3D tex;
uniform sampler2D tex2;

uniform float time;
uniform float waterTex;

void main() {
	vec3 norm = normalize(normal);
	vec4 color;
	vec3 coords;

	// Get tex coords
	if (abs(waterTex - gl_TexCoord[0].p) < 0.01) {
		coords.s = gl_TexCoord[0].s - (1.0 - (0.995 + sin(time + gl_TexCoord[0].s*2.0) / 20.0));
		coords.t = gl_TexCoord[0].t - (1.0 - (0.995 + sin(time + 1.0 + gl_TexCoord[0].t*5.0) / 30.0));
		coords.p = gl_TexCoord[0].p;
	} else
		coords = gl_TexCoord[0].stp;
	vec4 texColor = texture3D(tex, coords);

	color = texColor;
	// Clamp color to range 0..1
	color = clamp(color, 0.0, 1.0);

	gl_FragColor = color;
}

