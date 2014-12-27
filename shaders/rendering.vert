#version 440

in layout(location = 0) vec2 a_position;

out vec3 v_position;

uniform vec3 u_resolution;

uniform float u_cameraDistance;

uniform mat4 u_projectionMatrix;

void main () {
	vec3 position = vec3(a_position.xy * u_resolution.xy, u_resolution.z);
	v_position = position;

	gl_Position = u_projectionMatrix * vec4(position + vec3(0.0, 0.0, -u_cameraDistance), 1.0);
}