#version 440

layout(triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in int v_instance[3];
out float v_layer;

void main () {
	gl_Layer = v_instance[0];
	v_layer = float(v_instance[0]) + 0.5;
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	EndPrimitive();
};