#version 330 core

in mediump float Transp;

out mediump vec4 FragColor;

const mediump vec3 orange = vec3( 1.0, .5, .25 );

void main() {
	highp float L = length( gl_PointCoord.xy - vec2( .5 ) );
	highp float alpha = pow( exp( - pow( L, 2.0 ) ), 16.0 );
	FragColor = vec4( orange, Transp * alpha );
}