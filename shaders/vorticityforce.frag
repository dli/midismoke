#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;

uniform sampler3D u_velocityTexture;
uniform sampler3D u_vorticityTexture;

uniform float u_vorticity;

void main () {
	vec3 coordinates = vec3(gl_FragCoord.xy / u_resolution.xy, v_layer / u_resolution.z);
	vec3 delta = 1.0 / u_resolution;

	vec3 center = texture(u_vorticityTexture, coordinates).rgb;
	vec3 left = texture(u_vorticityTexture, coordinates + vec3(-delta.x, 0.0, 0.0)).rgb;
	vec3 right = texture(u_vorticityTexture, coordinates + vec3(delta.x, 0.0, 0.0)).rgb;
	vec3 bottom = texture(u_vorticityTexture, coordinates + vec3(0.0, -delta.y, 0.0)).rgb;
	vec3 top = texture(u_vorticityTexture, coordinates + vec3(0.0, delta.y, 0.0)).rgb;
	vec3 back = texture(u_vorticityTexture, coordinates + vec3(0.0, 0.0, -delta.z)).rgb;
	vec3 front = texture(u_vorticityTexture, coordinates + vec3(0.0, 0.0, delta.z)).rgb;

	if (coordinates.x - delta.x < 0.0) left = center;
	if (coordinates.x + delta.x > 1.0) right = center;
	if (coordinates.y - delta.y < 0.0) bottom = center;
	if (coordinates.y + delta.y > 1.0) top = center;
	if (coordinates.z - delta.z < 0.0) back = center;
	if (coordinates.z + delta.z > 1.0) front = center;

	vec3 gradient = vec3(length(right) - length(left), length(top) - length(bottom), length(front) - length(back)) / 2.0;

	//safe normalize
	float epsilon = 0.0001;
	float magnitudeSquared = max(epsilon, dot(gradient, gradient));
	gradient /= sqrt(magnitudeSquared);

	vec3 force = u_vorticity * cross(gradient, center);

	vec3 currentVelocity = texture(u_velocityTexture, coordinates).rgb;

	gl_FragColor = vec4(currentVelocity + force, 0.0);
}