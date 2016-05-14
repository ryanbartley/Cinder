#version 330

uniform sampler2D particleTex;

in vec4 vColor;

out vec4 oColor;

void main() {
	vec4 texColor = texture( particleTex, gl_PointCoord );
	oColor = vColor * texColor;
}