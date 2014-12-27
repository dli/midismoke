#version 440

precision highp float;

varying float v_layer;

uniform vec3 u_resolution;
uniform sampler3D u_velocityTexture;
uniform sampler3D u_dataTexture; //data to be advected

uniform sampler3D u_phin1hatTexture;
uniform sampler3D u_phinhatTexture;

uniform float u_dissipation;

uniform float u_deltaTime;

void main () {
	vec3 coordinates = vec3(gl_FragCoord.xy, v_layer);
	vec3 currentVelocity = texture(u_velocityTexture, coordinates / u_resolution).rgb;
	vec3 npos = coordinates - currentVelocity * u_deltaTime;

	npos = floor(npos); //integer bottom-left-back corner (in the middle of the 8 texels we want)
	npos = (npos + 0.5) / u_resolution;

	vec3 offset = 0.5 / u_resolution;

	vec4 value0 = texture(u_dataTexture, npos + vec3(-offset.x, -offset.y, -offset.z));
	vec4 value1 = texture(u_dataTexture, npos + vec3(-offset.x, -offset.y, offset.z));
	vec4 value2 = texture(u_dataTexture, npos + vec3(-offset.x, offset.y, -offset.z));
	vec4 value3 = texture(u_dataTexture, npos + vec3(-offset.x, offset.y, offset.z));
	vec4 value4 = texture(u_dataTexture, npos + vec3(offset.x, -offset.y, -offset.z));
	vec4 value5 = texture(u_dataTexture, npos + vec3(offset.x, -offset.y, offset.z));
	vec4 value6 = texture(u_dataTexture, npos + vec3(offset.x, offset.y, -offset.z));
	vec4 value7 = texture(u_dataTexture, npos + vec3(offset.x, offset.y, offset.z));

	vec4 dataMin = min(min(min(min(min(min(min(
	value0, value1), value2), value3), 
	value4), value5), value6), value7);

	vec4 dataMax = max(max(max(max(max(max(max(
	value0, value1), value2), value3),
	value4), value5), value6), value7);

	vec4 phin1hat = texture(u_phin1hatTexture, (coordinates - currentVelocity * u_deltaTime) / u_resolution);
	vec4 phinhat = texture(u_phinhatTexture, coordinates / u_resolution);
	vec4 phin = texture(u_dataTexture, coordinates / u_resolution);

	vec4 value = (phin1hat + 0.5 * (phin - phinhat));

	gl_FragColor = clamp(value, dataMin, dataMax) * pow((1.0 - u_dissipation), u_deltaTime);
}