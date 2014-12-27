#version 440

in layout(location = 0) vec2 a_position;

void main () {
	gl_Position = vec4(a_position, 0.0, 1.0);
}