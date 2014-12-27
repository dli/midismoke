#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;

uniform sampler3D u_velocityTexture;

void main () {
	vec3 coordinates = vec3(gl_FragCoord.xy / u_resolution.xy, v_layer / u_resolution.z);
	vec3 delta = 1.0 / u_resolution;

	vec3 center = texture(u_velocityTexture, coordinates).rgb;
	vec3 left = texture(u_velocityTexture, coordinates + vec3(-delta.x, 0.0, 0.0)).rgb;
	vec3 right = texture(u_velocityTexture, coordinates + vec3(delta.x, 0.0, 0.0)).rgb;
	vec3 bottom = texture(u_velocityTexture, coordinates + vec3(0.0, -delta.y, 0.0)).rgb;
	vec3 top = texture(u_velocityTexture, coordinates + vec3(0.0, delta.y, 0.0)).rgb;
	vec3 back = texture(u_velocityTexture, coordinates + vec3(0.0, 0.0, -delta.z)).rgb;
	vec3 front = texture(u_velocityTexture, coordinates + vec3(0.0, 0.0, delta.z)).rgb;

	if (coordinates.x - delta.x < 0.0) left = vec3(0.0);
	if (coordinates.x + delta.x > 1.0) right = vec3(0.0);
	if (coordinates.y - delta.y < 0.0) bottom = vec3(0.0);
	if (coordinates.y + delta.y > 1.0) top = vec3(0.0);
	if (coordinates.z - delta.z < 0.0) back = vec3(0.0);
	if (coordinates.z + delta.z > 1.0) front = vec3(0.0);

	vec3 vorticity = vec3(
	(top.z - bottom.z) - (front.y - back.y),
	(front.x - back.x) - (right.z - left.z),
	(right.y - left.y) - (top.x - bottom.x)
	);

	gl_FragColor = vec4(vorticity, 0.0);
}