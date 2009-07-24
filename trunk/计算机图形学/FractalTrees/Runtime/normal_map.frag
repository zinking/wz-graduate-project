varying vec3 L;
uniform sampler2D textureMap1;
uniform sampler2D normalMap1;


void main() 
{
	vec3 rgb	= texture2D(textureMap1, gl_TexCoord[0]).rgb;
	vec3 normal = texture2D(normalMap1 , gl_TexCoord[0]).xyz;
	normal = (normal - 0.5) * 2;
	L = normalize(L);
	float NdotL = dot(normal,L);
	
	gl_FragColor.rgb = rgb * NdotL;
}
