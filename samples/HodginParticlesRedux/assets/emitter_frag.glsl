#version 330

uniform float counter;
uniform sampler2D texDiffuse;
uniform sampler2D texNormal;
uniform sampler2D texHeight;
uniform sampler2D texSpec;

uniform vec3 lightDir;
uniform float heat;
uniform float spinSpeed;
uniform vec3 mouseVel;
uniform float minHeight;

in vec3 normal;
in vec3 position;
in vec2 texcoord;

out vec4 oColor;

void main()
{
	float rotSpeed			= 0.025 * heat * heat + 0.005;

	float tOffset			= sin( counter*rotSpeed ) * spinSpeed;
	
	vec2 texCoord			= vec2( ( texcoord.s * ( 1.0 - spinSpeed ) ) + counter*rotSpeed * spinSpeed + mouseVel.x, 1.0 - texcoord.t + tOffset + mouseVel.y );
	
	vec3 diffuseSample		= texture( texDiffuse, texCoord ).rgb;
	vec3 normalSample		= ( texture( texNormal, texCoord ).rgb - vec3( 0.5, 0.5, 0.5 ) ) * 2.0;
	float heightValue		= pow( 1.0 - texture( texHeight, texCoord ).r, 5.0 );
	float specValue			= texture( texSpec, texCoord ).r;
	
	vec3 ppNormal			= normalize( normal + normalSample );
	float ppDiffuse			= abs( dot( ppNormal, lightDir ) );
	float ppFresnel			= pow( ( 1.0 - ppDiffuse ), 5.0 );
	float ppSpecular		= pow( ppDiffuse, 15.0 ) * specValue;
	float ppSpecularBright	= pow( ppSpecular, 50.0 ) * specValue;
	
	vec3 heatColor			= vec3( heightValue * heat, heightValue * heat * 0.1, 0.0 ) * heat;
	vec3 heatBottomColor	= pow( texCoord.t - tOffset, 3.0 ) * heatColor;
	vec3 heatCoreColor		= ppDiffuse * heatColor;
	
	vec3 landFinal			= ppSpecularBright + heatColor * ppSpecular + heatCoreColor;
	vec3 oceanFinal			= diffuseSample + ppSpecularBright + heatCoreColor;
	
	if( minHeight < 0.01 ){
		oColor.rgb	= landFinal + oceanFinal;
		oColor.a		= 1.0;
	} else {
		oColor.rgb	= ( landFinal + oceanFinal ) * heatColor * ppSpecular * ( 1.0 - heightValue );
		if( ( 1.0 - heightValue ) <= minHeight ){
			oColor.a	= 0.0;
		} else {
			oColor.a	= minHeight * 0.0001;
		}
	}
}

