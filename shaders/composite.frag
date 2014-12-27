#version 440

uniform sampler2D u_unblurredTexture;
uniform sampler2D u_blurredTexture;
uniform vec2 u_resolution;

uniform float u_time;

uniform float u_unblurredWeight;
uniform float u_blurredWeight;

void main () {
	vec2 coordinates = gl_FragCoord.xy / u_resolution.xy;

	vec3 color = texture(u_unblurredTexture, coordinates).rgb * u_unblurredWeight + texture(u_blurredTexture, coordinates).rgb * u_blurredWeight;

	gl_FragColor = vec4(color, 1.0);
}