#version 440

uniform sampler2D u_input;
uniform vec2 u_resolution;

uniform int u_direction; //0 = horizontal, 1 = vertical

uniform float u_blurSigma;

void main () {
	vec2 coordinates = gl_FragCoord.xy / u_resolution.xy;

	vec3 total = vec3(0.0);
	float weight = 0.0;

	float dx = 1.0 / u_resolution.x;
	float dy = 1.0 / u_resolution.y;

	for (int i = -BLUR_WIDTH; i <= BLUR_WIDTH; i += BLUR_STEP) {

		float offset = float(i); //ofset in pixels

		float gaussian = exp(-float(offset * offset) / u_blurSigma);
		weight += gaussian;

		if (u_direction == 0) {
			total += texture(u_input, coordinates + vec2(offset * dx, 0.0)).rgb * gaussian;
		} else {
			total += texture(u_input, coordinates + vec2(0.0, offset * dy)).rgb * gaussian;
		}

	}

	total /= weight;

	gl_FragColor = vec4(total, 1.0);
}