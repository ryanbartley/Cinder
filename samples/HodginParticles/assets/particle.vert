#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in float radius;

uniform mat4 ciModelView;
uniform mat4 ciProjectionMatrix;

out vec4 vColor;

uniform float uScreenSize = 1000.0;

void main() {
	vec4 eyePos = ciModelView * vec4( position, 1.0 );
	
	gl_PointSize = 10;
	gl_Position = ciProjectionMatrix * eyePos;
	vColor = color;
}