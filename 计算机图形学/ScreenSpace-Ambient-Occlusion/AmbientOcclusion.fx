//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

texture g_DepthTexture;              // Base color texture
texture g_noiseTexture;


// Light parameters:
float3 g_LightDir;								// Light's direction in world space
float4 g_LightDiffuse;							// Light's diffuse color
float4 g_LightAmbient;						// Light's ambient color

float4   g_vEye;									// Camera's location
float    g_fBaseTextureRepeat;				// The tiling factor for base and normal map textures
float    g_fHeightMapScale;					// Describes the useful range of values for the height field

// Matrices:
float4x4 g_mWorld;							// World matrix for object
float4x4 g_mWorldViewProjection;		// World * View * Projection matrix
float4x4 g_mView;								// View matrix 
float4x4 g_mWorldView;
float4x4	 g_mProjection;
float4x4	 g_mITWorldView;

float4	g_vFustrumCorner;



int      g_nMinSamples;				// The minimum number of samples for sampling the height field profile
int      g_nMaxSamples;				// The maximum number of samples for sampling the height field profile

float near;
float far;
float sampleRadius = 0.12;
float distanceScale = 50;
int g_nSamples = 16;




//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------

sampler2D depthSampler = sampler_state
{
	Texture = <g_DepthTexture>;
    ADDRESSU = CLAMP;
	ADDRESSV = CLAMP;
	MAGFILTER = LINEAR;
	MINFILTER = LINEAR;
};

sampler2D RandNormal = sampler_state
{
	Texture = <g_noiseTexture>;
    ADDRESSU = WRAP;
	ADDRESSV = WRAP;
	MAGFILTER = LINEAR;
	MINFILTER = LINEAR;
};

//--------------------------------------------------------------------------------------
// This technique performs zbuffer writing
//--------------------------------------------------------------------------------------

struct VS_OUTPUT_NZ
{
   float4 Position: POSITION0;
   float3 Normal : TEXCOORD0;
   float4 vPositionVS : TEXCOORD1;
};  


VS_OUTPUT_NZ VS_NZ_Encode(				float4 inPosition		: POSITION, 
															float2 inTexCoord	: TEXCOORD0,
															float3 inNormal		: NORMAL )
{
   VS_OUTPUT_NZ Output; 
   Output.Position		= mul( inPosition, g_mWorldViewProjection);
   Output.vPositionVS = mul(  inPosition, g_mWorldView);
   Output.Normal		= mul( inNormal, g_mITWorldView);
   
   return Output;
}  

float4 PS_NZ_Encode(	VS_OUTPUT_NZ IN):COLOR
{ 
    float depth = IN.vPositionVS.z / far;
	IN.Normal = normalize(IN.Normal);
	return float4(IN.Normal.x, IN.Normal.y, IN.Normal.z, depth);
	//return depth;
}   


//----------------------------------------------------------------------z buffer transform end --------------------------------------------------------------------------------

struct VS_OUTPUT
{
    float4 pos				: POSITION;
    float2 TexCoord			: TEXCOORD0;
    float3 viewDirection	: TEXCOORD1;
};

VS_OUTPUT SSAOVertexShader(
    float4 Position : POSITION, float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;

    Out.pos =  mul( Position, g_mWorldViewProjection);

	Position = Out.pos;
	Position.xy /= Position.w;
	float2 uv = (float2(Position.x, -Position.y) + float2( 1.0f, 1.0f ) ) * 0.5f;
	

    Out.TexCoord = uv; 
	//float3   direction = mul(Position, g_mWorldView).xyz - g_vEye;
	float3   direction =  mul(Position, g_mWorldView).xyz ;
	Out.viewDirection =  direction;
    
    return Out;
}

float occlusion(const float ZD) {
    return 1.0 / (1.0 + ZD * ZD);
}

float4 SSAOPixelShader(VS_OUTPUT IN) : COLOR
{
	float4 samples[16] =
	{
		float4(0.355512, 	-0.709318, 	-0.102371,	0.0 ),
		float4(0.534186, 	0.71511, 		-0.115167,	0.0 ),
		float4(-0.87866, 		0.157139, 		-0.115167,	0.0 ),
		float4(0.140679, 	-0.475516, 	-0.0639818,	0.0 ),
		float4(-0.0796121, 	0.158842, 		-0.677075,	0.0 ),
		float4(-0.0759516, 	-0.101676, 	-0.483625,	0.0 ),
		float4(0.12493, 		-0.0223423,	-0.483625,	0.0 ),
		float4(-0.0720074, 	0.243395, 		-0.967251,	0.0 ),
		float4(-0.207641, 	0.414286, 		0.187755,	0.0 ),
		float4(-0.277332, 	-0.371262, 	0.187755,	0.0 ),
		float4(0.63864, 		-0.114214, 	0.262857,	0.0 ),
		float4(-0.184051, 	0.622119, 		0.262857,	0.0 ),
		float4(0.110007, 	-0.219486, 	0.435574,	0.0 ),
		float4(0.235085, 	0.314707, 		0.696918,	0.0 ),
		float4(-0.290012, 	0.0518654, 	0.522688,	0.0 ),
		float4(0.0975089, 	-0.329594, 	0.609803,	0.0 )
	};
	

	normalize (IN.viewDirection);
	float depth = tex2D(depthSampler, IN.TexCoord).a;
	float3 se = depth * IN.viewDirection;
	
	float3 randNormal = tex2D( RandNormal, IN.TexCoord  ).rgb;

	float3 normal = tex2D(depthSampler, IN.TexCoord).rgb;
	float finalColor = 0.0f;
	
	for (int i = 0; i < g_nSamples; i++)
	{
		float3 ray = reflect(samples[i].xyz,randNormal) * sampleRadius;
					
		float4 sample = float4(se + ray, 1.0f);
		float4 ss = mul(sample, g_mProjection);
		ss.xy /= ss.w;
		float2 uv = (float2(ss.x, -ss.y) + float2( 1.0f, 1.0f ) ) * 0.5f;

		float2 sampleTexCoord = uv;

		
		float sampleDepth = tex2D(depthSampler, sampleTexCoord).a;
		
		
		//float ZD = distanceScale* max(sampleDepth - depth, 0.0f);
		float ZD = distanceScale* max(depth - sampleDepth, 0.0f);
		const float MAX_DIFF = 1;
		const float NO_OCC = 2;
		finalColor += (ZD < distanceScale * MAX_DIFF ? occlusion(ZD) : NO_OCC);

	}
	
	finalColor = finalColor / 16;
    finalColor = lerp(0.3, 1, finalColor);
    return float4(finalColor.xxx, 1); 

	//return float4(finalColor/16, finalColor/16, finalColor/16, 1.0f);
	//return 0.5;
}





//--------------------------------------------------------------------------------------
// Renders SSAO scene to render target
//--------------------------------------------------------------------------------------
technique RenderSceneWithSSAO
{
    pass P0
    {          
        VertexShader = compile vs_3_0 VS_NZ_Encode();
        PixelShader    = compile ps_3_0 PS_NZ_Encode(); 
    }
   
   pass P1
    {          
        VertexShader = compile vs_3_0 SSAOVertexShader();
        PixelShader    = compile ps_3_0 SSAOPixelShader(); 
    } 
   
}


