#version 440

in layout(location = 0) vec2 a_position;

out int v_instance;

void main () {
	v_instance = gl_InstanceID;
	gl_Position = vec4(a_position, 0.0, 1.0);
};