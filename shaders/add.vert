#version 440

in layout(location = 0) vec2 a_position;

out int v_instance;

uniform vec3 u_position;
uniform vec3 u_resolution;
uniform float u_radius;

void main () {
	v_instance = gl_InstanceID;

	vec2 center = (u_position.xy / u_resolution.xy) * 2.0 - 1.0;

	gl_Position = vec4(center + a_position * 2.0 * u_radius / u_resolution.xy, 0.0, 1.0);

}