#version 440

precision highp float;

in vec3 v_position;

uniform vec3 u_resolution;
uniform sampler3D u_input;
uniform sampler3D u_transparencyTexture;

uniform float u_cameraDistance;

uniform float u_densityScale;
uniform float u_colorScale;

uniform float u_stepSize;

void main () {
	vec3 cameraPosition = vec3(0.0, 0.0, u_cameraDistance);

	vec3 rayDirection = normalize(v_position - cameraPosition);

	vec3 currentPosition = v_position;

	vec4 finalColor = vec4(0.0);

	for (int i = 0; i < RAY_STEPS; ++i) {

		currentPosition += rayDirection * u_stepSize;

		if (currentPosition.x > -u_resolution.x && currentPosition.x < u_resolution.x && currentPosition.y > -u_resolution.y && currentPosition.y < u_resolution.y && currentPosition.z > -u_resolution.z && currentPosition.z < u_resolution.z) {

			vec3 color = texture(u_input, (currentPosition / u_resolution) * 0.5 + 0.5).rgb;
			float density = color.r + color.g + color.b;

			density *= u_densityScale;

			color *= u_colorScale;

			float transparency = texture(u_transparencyTexture, (currentPosition / u_resolution) * 0.5 + 0.5).r;

			color *= transparency;

			finalColor.rgb += color * density * (1.0 - finalColor.a);
			finalColor.a += density * (1.0 - finalColor.a);

		} else {
			break;
		}

	}

	gl_FragColor = vec4(finalColor.rgb, 1.0);
}