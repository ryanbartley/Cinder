#version 330

uniform sampler2D	uDiffuseTex;
uniform sampler2D	uAoTex;
uniform sampler2D	uNormalTex;
uniform sampler2D	uHeightTex;
uniform sampler2D	uReflOccTex;

uniform float		uTime;
uniform float		uSOffset;
uniform float		uTOffset;
uniform float		uRotSpeed;

in vec4			Position;
in vec2			TexCoord;
in vec3			Normal;
in vec3			Tangent;
in vec3			Bitangent;

out vec4		FragColor;

void main()
{
	vec2 tc			= vec2( TexCoord.x, pow( TexCoord.y, 2.0 ) ) * vec2( 2.0, 1.0 ) + vec2( -uSOffset, uTOffset );
	
	vec3 diffCol	= texture( uDiffuseTex,		tc ).rgb;
	float aoVal		= texture( uAoTex,			tc ).r;
	float height	= texture( uHeightTex,		tc ).r;
	vec3 normalCol	= texture( uNormalTex,		tc ).rgb * 2.0 - 1.0;
	float roVal		= texture( uReflOccTex,		tc ).r;
	
	vec3 N			= normalize( ( Tangent * normalCol.x ) + ( Bitangent * normalCol.y ) + ( Normal * normalCol.z ) );
	vec3 C			= normalize( -Position.xyz );
	vec3 L			= normalize( vec3( 0.0, 1.0, 1.0 ) );
	vec3 R			= normalize( -reflect( L, N ) );
	
	float D			= max( dot( N, L ), 0.0 );
	float S			= pow( max( dot( R, C ), 0.0 ), 125.0 );
	
	float eyeDiff	= max( dot( N, C ), 0.0 );
	float eyeRim	= pow( 1.0 - eyeDiff, 4.0 );
	
	vec3 finalDiff	= diffCol * D - aoVal * 0.3 + S * roVal;
	vec3 hotGlow	= vec3( diffCol - 0.4 ) * uRotSpeed + diffCol * diffCol * uRotSpeed;
	vec3 spinGlow	= vec3( height * diffCol.r + eyeRim * 0.6 + eyeDiff * 2.0 ) * pow( uRotSpeed, 2.0 ) * vec3( 1.0, 0.4, 0.0 );
	vec3 crackGlow	= vec3( 1.0, 0.4, 0.0 ) * pow( 1.0 - height, 4.0 ) * uRotSpeed;
	
	FragColor.rgb	= finalDiff + hotGlow + spinGlow + crackGlow * 3.0;
	FragColor.a		= 1.0;
}