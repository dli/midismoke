#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;

uniform sampler3D u_dyeTexture;

uniform vec3 u_lightDirection;

uniform float u_absorption;
uniform float u_ambient;

uniform float u_stepSize;

void main () {

	vec3 coordinates = vec3(gl_FragCoord.xy, v_layer);
	vec3 delta = 1.0 / u_resolution;

	vec3 rayPosition = vec3(gl_FragCoord.xy, v_layer);

	float totalDensity = 0.0;

	for (int i = 0; i < RAY_STEPS; ++i) {
		rayPosition += normalize(u_lightDirection) * u_stepSize;

		if (rayPosition.x > 0.0 && rayPosition.x < u_resolution.x && rayPosition.y > 0.0 && rayPosition.y < u_resolution.y && rayPosition.z > 0.0 && rayPosition.z < u_resolution.z) {

			float density = dot(texture(u_dyeTexture, rayPosition / u_resolution).rgb, vec3(1.0));
			totalDensity += density;

		} else {
			break;
		}

	}

	float transparency = exp(-totalDensity * u_absorption);

	gl_FragColor = vec4(vec3(transparency) + u_ambient, 0.0);
	
}