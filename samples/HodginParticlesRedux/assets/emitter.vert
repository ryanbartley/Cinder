#version 330

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat4	ciModelMatrix;
uniform mat3	ciNormalMatrix;
uniform sampler2D	uHeightTex;
uniform float		uTime;
uniform float		uSOffset;
uniform float		uTOffset;
uniform float		uRotSpeed;

in vec4			ciPosition;
in vec3			ciNormal;
in vec2			ciTexCoord0;
in vec3			ciTangent;

out vec4		Position;
out vec2		TexCoord;
out vec3		Normal;
out vec3		Tangent;
out vec3		Bitangent;

void main()
{
	TexCoord		= ciTexCoord0;
	vec2 tc			= vec2( TexCoord.x, pow( TexCoord.y, 2.0 ) ) * vec2( 2.0, 1.0 ) + vec2( -uSOffset, uTOffset );
	Position		= ciModelView * ciPosition;
	
	Normal			= normalize( ciNormalMatrix * ciNormal );
	Tangent			= normalize( ciNormalMatrix * ciTangent );
	Bitangent		= normalize( cross( Tangent, Normal ) );
	
	vec4 pos		= ciPosition;
	pos.y			*= ( 1.0 - uRotSpeed ) * 0.3 + 0.7;
	
	float height	= texture( uHeightTex, tc ).r;
	
	// make less pronounced at poles
	float adjHeight	= height * ( sin( ( Normal.y * 0.5 + 0.5 ) * 3.14159 ) * 0.7 + 0.3 );
	pos.xyz			+= ciNormal * adjHeight * ( 1.0 - ( pow( uRotSpeed, 3.0 ) * 0.3 + 0.7 ) ) * 2.0;
	
	gl_Position		= ciModelViewProjection * pos;
	
}
