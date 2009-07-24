varying vec3 L;
const vec4 lightPos = vec4(0.0, 5.0, 5.0, 1);  // make this uniform

void main() 
{
	// normal map computing
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	
	L = vec3(lightPos - gl_ModelViewMatrix * gl_Vertex);
	L = normalize(L);	
	
	vec3 N = normalize(gl_NormalMatrix * gl_Normal);
	vec3 T = gl_Color.xyz;	
	vec3 B = cross(T,N); 	
	
	mat3 TBN = mat3(T,B,N);
	
	L = TBN * L;
	gl_Position = ftransform();
}