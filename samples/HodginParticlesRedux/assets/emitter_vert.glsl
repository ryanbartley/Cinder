#version 330

in vec4 ciPosition;
in vec3 ciNormal;
in vec2 ciTexCoord0;

uniform mat4 ciModelViewProjection;
uniform mat3 ciNormalMatrix;

out vec3 normal;
out vec3 position;
out vec2 texcoord;

void main()
{
	normal = normalize( ciNormalMatrix * ciNormal );
	position = ciPosition.xyz;
	gl_Position = ciModelViewProjection * ciPosition;
	texcoord = vec2(ciTexCoord0) - vec2( .5 );
}
