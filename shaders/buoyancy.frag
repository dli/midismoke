#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;
uniform sampler3D u_velocityTexture;
uniform sampler3D u_temperatureTexture;

uniform float u_deltaTime;

uniform vec3 u_position;

void main () {

	vec3 coordinates = vec3(gl_FragCoord.xy, v_layer);
	vec3 currentVelocity = texture(u_velocityTexture, coordinates / u_resolution).rgb;
	float temperature = texture(u_temperatureTexture, coordinates / u_resolution).r;

	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 force = temperature * up * u_deltaTime;

	gl_FragColor = vec4(currentVelocity + force, 0.0);

}