#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;

uniform sampler3D u_velocityTexture;
uniform sampler3D u_pressure;

void main () {
	vec3 coordinates = vec3(gl_FragCoord.xy / u_resolution.xy, v_layer / u_resolution.z);
	vec3 delta = 1.0 / u_resolution;

	float center = texture(u_pressure, coordinates).r;
	float left = texture(u_pressure, coordinates + vec3(-delta.x, 0.0, 0.0)).r;
	float right = texture(u_pressure, coordinates + vec3(delta.x, 0.0, 0.0)).r;
	float bottom = texture(u_pressure, coordinates + vec3(0.0, -delta.y, 0.0)).r;
	float top = texture(u_pressure, coordinates + vec3(0.0, delta.y, 0.0)).r;
	float back = texture(u_pressure, coordinates + vec3(0.0, 0.0, -delta.z)).r;
	float front = texture(u_pressure, coordinates + vec3(0.0, 0.0, delta.z)).r;

	if (coordinates.x - delta.x < 0.0) left = center;
	if (coordinates.x + delta.x > 1.0) right = center;
	if (coordinates.y - delta.y < 0.0) bottom = center;
	if (coordinates.y + delta.y > 1.0) top = center;
	if (coordinates.z - delta.z < 0.0) back = center;
	if (coordinates.z + delta.z > 1.0) front = center;


	//compute gradient of pressure
	vec3 gradient = vec3(right - left, top - bottom, front - back) / 2.0;

	vec3 currentVelocity = texture(u_velocityTexture, coordinates).rgb;

	gl_FragColor = vec4(currentVelocity - gradient, 0.0);
}