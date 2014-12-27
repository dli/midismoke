#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;
uniform sampler3D u_velocityTexture;
uniform sampler3D u_dataTexture; //data to be advected

uniform float u_deltaTime;

void main () {
	vec3 coordinates = vec3(gl_FragCoord.xy, v_layer);
	vec3 currentVelocity = texture(u_velocityTexture, coordinates / u_resolution).rgb;
	vec4 data = texture(u_dataTexture, (coordinates - currentVelocity * u_deltaTime) / u_resolution);
	gl_FragColor = data;
};