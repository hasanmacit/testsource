const char* lobby_terrainShadowsPS = R"(
struct PS_INPUT
{
    float4 Position : POSITION;
    float2 UV : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 LightViewPosition1 : TEXCOORD2;
    float4 LightViewPosition2 : TEXCOORD3;
    float4 WorldPos : TEXCOORD4;
    float3 Tangent : TEXCOORD5;
};

#define USE_SHADOWS
#define SHADOW_PCF_RADIUS 1
#define SHADOW_PCF_NUM_SAMPLES 30 // 7

float3 MaterialDiffuse : register(c0);
float3 LightDirection : register(c1);
float4 LightDiffuse : register(c2);
float4 AmbientColor : register(c3);
float3 CameraPos : register(c4);
float4 FogColor : register(c5);
float2 FogRange : register(c6);
float3 MaterialSpecular : register(c7);
float Shininess : register(c8);
float3 LightSpecular : register(c9);
int ShadowOpacity : register(c10); // 0..100?
float SpecularLight : register(c11);
float Time : register(c12); // Add this to the constants at the bottom
float reduceLightIntensity : register(c15);
float enableEffects : register(c13);

sampler2D DiffuseTexture : register(s0);
sampler2D ShadowMap1 : register(s1);   // big shadowmap
sampler2D ShadowMap2 : register(s2);   // local (small) shadowmap

static const float PI = 3.1416f;
static const float2 pcfOffsets[30] = {
    float2(0.0000, 0.0000),
    float2(0.5000, 0.0000),
    float2(0.3536, 0.3536),
    float2(0.0000, 0.5000),
    float2(-0.3536, 0.3536),
    float2(-0.5000, 0.0000),
    float2(-0.3536, -0.3536),
    float2(-0.0000, -0.5000),
    float2(0.3536, -0.3536),
    float2(1.0000, 0.0000),
    float2(0.8660, 0.5000),
    float2(0.5000, 0.8660),
    float2(0.0000, 1.0000),
    float2(-0.5000, 0.8660),
    float2(-0.8660, 0.5000),
    float2(-1.0000, 0.0000),
    float2(-0.8660, -0.5000),
    float2(-0.5000, -0.8660),
    float2(-0.0000, -1.0000),
    float2(0.5000, -0.8660),
    float2(0.8660, -0.5000),
    float2(1.5000, 0.0000),
    float2(1.1491, 0.9642),
    float2(0.2605, 1.4772),
    float2(-0.7500, 1.2990),
    float2(-1.4095, 0.5130),
    float2(-1.4095, -0.5130),
    float2(-0.7500, -1.2990),
    float2(0.2605, -1.4772),
    float2(1.1491, -0.9642)
};

static const float3 pointLightPositionLobby[62] = 
{
    float3(4599.603027, -17126.146484, 17760.044922 + 100.0f),
    float3(7430.978516, -20678.500000, 17751.500000 + 100.0f),
    float3(3438.737549, -21313.423828, 17748.615234 + 100.0f),
    float3(4381.654297, -24431.451172, 17750.894531 + 100.0f),
    float3(1942.597900, -26004.628906, 17750.775391 + 100.0f),
    float3(4712.081055, -27615.705078, 17738.699219 + 100.0f),
    float3(6651.171387, -25468.330078, 17747.373047 + 100.0f),
    float3(9415.385742, -21994.339844, 17737.849609 + 100.0f),
    float3(10916.774414, -24449.775391, 17750.435547 + 100.0f),
    float3(13162.217773, -26191.349609, 17751.097656 + 100.0f),
    float3(17775.595703, -27018.378906, 17748.669922 + 100.0f),
    float3(22596.466797, -31026.160156, 17833.960938 + 100.0f),
    float3(21213.281250, -27952.957031, 18256.546875 + 100.0f),
    float3(17370.451172, -21125.531250, 18257.925781 + 100.0f),
    float3(21766.947266, -13792.586914, 18262.000000 + 100.0f),
    float3(24398.091797, -12785.338867, 18262.000000 + 100.0f),
    float3(32067.482422, -11593.994141, 18258.000000 + 100.0f),
    float3(33560.625000, -15502.521484, 18258.810547 + 100.0f),
    float3(34679.726562, -18100.246094, 18258.589844 + 100.0f),
    float3(33014.984375, -24310.093750, 18261.484375 + 100.0f),
    float3(30726.216797, -27894.417969, 18255.337891 + 100.0f),
    float3(26108.285156, -29796.980469, 18321.591797 + 100.0f),
    float3(29283.658203, -31470.113281, 17922.437500 + 100.0f),
    float3(35984.324219, -34262.367188, 17751.500000 + 100.0f),
    float3(36708.996094, -25498.898438, 17751.500000 + 100.0f),
    float3(38397.316406, -25517.166016, 17755.818359 + 100.0f),
    float3(40258.160156, -25416.595703, 17751.500000 + 100.0f),
    float3(41618.023438, -24188.832031, 17751.500000 + 100.0f),
    float3(41653.285156, -22228.701172, 17751.167969 + 100.0f),
    float3(41706.812500, -19855.302734, 17757.214844 + 100.0f),
    float3(41532.597656, -17117.337891, 17751.500000 + 100.0f),
    float3(39378.097656, -14581.868164, 17756.574219 + 100.0f),
    float3(38344.785156, -13483.138672, 17750.210938 + 100.0f),
    float3(39533.429688, -12115.005859, 17751.574219 + 100.0f),
    float3(43448.054688, -11921.946289, 17750.873047 + 100.0f),
    float3(46706.105469, -12455.190430, 17751.779297 + 100.0f),
    float3(45311.273438, -23971.099609, 17748.525391 + 100.0f),
    float3(45095.050781, -29883.931641, 17750.187500 + 100.0f),
    float3(42651.496094, -39310.863281, 18227.500000 + 100.0f),
    float3(39919.343750, -34403.769531, 17887.500000 + 100.0f),
    float3(26710.029297, -33993.070312, 17770.089844 + 100.0f),
    float3(22767.683594, -42266.218750, 17809.960938 + 100.0f),
    float3(27761.738281, -39817.953125, 17806.984375 + 100.0f),
    float3(35066.031250, -41150.187500, 17761.234375 + 100.0f),
    float3(18927.271484, -43828.000000, 19642.230469 + 100.0f),
    float3(19086.314453, -38804.750000, 20104.814453 + 100.0f),
    float3(17116.355469, -39065.199219, 20090.144531 + 100.0f),
    float3(10594.347656, -35856.707031, 20982.677734 + 100.0f),
    float3(14384.308594, -47258.136719, 19679.519531 + 100.0f),
    float3(16300.496094, -47234.082031, 19683.164062 + 100.0f),
    float3(18820.548828, -47207.785156, 19682.089844 + 100.0f),
    float3(20602.144531, -47199.726562, 19685.162109 + 100.0f),
    float3(22618.007812, -47177.621094, 19683.605469 + 100.0f),
    float3(24900.587891, -47117.597656, 19689.132812 + 100.0f),
    float3(26540.480469, -47123.945312, 19686.779297 + 100.0f),
    float3(6000.000000, -13100.000000, 16725.500000 + 2000.0f),
    float3(5400.000000, -10200.000000, 16725.500000 + 2000.0f),
    float3(10300.000000, -8800.000000, 16725.500000 + 2000.0f),
    float3(6007.584473, -22254.117188, 17747.890625 + 20.0f),
    float3(3227.504150, -29173.126953, 17749.382812 + 20.0f),
    float3(5240.759766, -29621.611328, 17747.964844 + 20.0f),
    float3(13300.000000, -9200.000000, 16725.500000 + 2000.0f)
};

float2 clipSpaceToTextureSpace(float4 clipSpace)
{
    float2 cs = clipSpace.xy;
    return float2(0.5f * cs.x, -0.5f * cs.y) + 0.5f;
}

// Utility function to compute flickering effect
float ComputeFlicker(float Time)
{
    float flicker = sin(Time * 10.0f) * 0.5f + 0.5f;
    flicker += sin(Time * 12.34f) * 0.25f;
    flicker = saturate(flicker);
    return 1.0f + (flicker - 1.0f) / 8.0f;
}

// Utility function to compute point light intensity
float ComputePointLightIntensity(float3 position, float3 lightPosition, float radius)
{
    float3 toLightPosition = position - lightPosition;
    float distanceToLight = length(toLightPosition);
    return pow(saturate(1.0f - distanceToLight / radius), 2);
}



float4 main(PS_INPUT input) : SV_TARGET
{
    float4 Color = tex2D(DiffuseTexture, input.UV);

    // Fetch and transform normal from the diffuse map (pretending it's a normal map)
    float3 NM = normalize((tex2D(DiffuseTexture, input.UV).rgb * 2.0f) - 1.0f);
    float3 N = normalize(input.Normal * NM);
    float3 L = -LightDirection;

    float OriginalLightIntensity = saturate(dot(N, L));
    float LightIntensity = 0.3f + 0.2f * OriginalLightIntensity;
    float3 LightResult = (AmbientColor.rgb + (MaterialDiffuse * LightDiffuse.rgb) * LightIntensity);

#ifdef USE_SHADOWS

    float totalPointLightIntensity = 0.0f;
    if (SpecularLight > 0.5f)
    {
        float flicker = sin(Time / 2.0f) * 0.5f + 0.5f;
        flicker += sin(Time * 12.34f) * 0.25f;
        flicker = saturate(flicker);
        flicker = 1.0f + (flicker - 1.0f) / 35.0f; // This line will reduce the intensity of the flickering
		
		float flicker2 = sin(Time / 250.0f) * 0.25f + 0.75f;  // Slower base flicker
		flicker2 += sin(Time / 50.0f) * 0.1f;                 // Slower and smaller magnitude secondary fluctuation
		flicker2 = saturate(flicker2);                        // Ensure it's clamped between 0 and 1
		flicker2 *= 0.85f;                                    // Reduce overall intensity to make it darker

        if (enableEffects < 0.5)
        {
    
            float3 colorFactor = float3(1.0f, 0.5f, 0.2f);
            int lightCount = 62;
        
            for (int i = 0; i < lightCount; ++i)
            {
                float pointLightIntensity1 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionLobby[i], 4000.0f) * flicker;
                float pointLightIntensity2 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionLobby[i], 2000.0f) * flicker;
                totalPointLightIntensity += pointLightIntensity1;
                Color.rgb += pointLightIntensity2 * colorFactor * 6.0f / reduceLightIntensity;
            }

            float3 colorFactor2 = float3(1.0f, 0.0f, 0.0f);
            int lightCount2 = 3;

            totalPointLightIntensity = saturate(totalPointLightIntensity); // clamp between [0, 1]
        }
    }

    const float ShadowBias = 0.0001f;
    const float ShadowBias2 = 0.00004f;
    const float2 LightTexCoord1 = clipSpaceToTextureSpace(input.LightViewPosition1);
    const float2 LightTexCoord2 = clipSpaceToTextureSpace(input.LightViewPosition2);
    const float fCurrDepth1 = input.LightViewPosition1.z - ShadowBias;
    const float fCurrDepth2 = input.LightViewPosition2.z - ShadowBias2;
	 
    float bigShadow = 0.0f;
    float localShadow = 0.0f;	 
    const float texelScale1 = 1.0f / 2500.0f;
    const float texelScale2 = 1.0f / 1200.0f;
	 const float increment = 1.0f / SHADOW_PCF_NUM_SAMPLES;
   
    for (int i = 0; i < SHADOW_PCF_NUM_SAMPLES; ++i) {		 		
            float2 offset = pcfOffsets[i];				
				bigShadow   += (fCurrDepth1 > tex2D(ShadowMap1, LightTexCoord1 + offset * texelScale1).r ? increment : 0.0f);
				localShadow += (fCurrDepth2 > tex2D(ShadowMap2, LightTexCoord2 + offset * texelScale2).r ? increment : 0.0f);      
    }

	 // Distance to the inner shadow border (0.0 .. 0.5)
    float borderDist = 0.5f - max(abs(0.5f - LightTexCoord2.x), abs(0.5f - LightTexCoord2.y));
    float fade = smoothstep(0.0f, 0.2f, borderDist);
	 
	 /*
	 // Visualize map grid (for debug)	 
	 float2 pix = LightTexCoord1 * 2048.0f;
	 pix = pix - floor(pix);
	 pix = 0.5f - abs(pix - 0.5f);
 	 float gridVal = max(pix.x, pix.y);
	 gridVal = smoothstep(0.45f, 0.5f, gridVal);
	 */

    float finalShadow = lerp(bigShadow, localShadow, fade);//(shadow2 + (shadow1 * fade)) / 2.0f;
	 //finalShadow = bigShadow + gridVal * 0.8f;
	 float InvertedOpacity = 1.0f - (ShadowOpacity / 120.0f);

	finalShadow = lerp(bigShadow, localShadow, fade); 
	finalShadow *= InvertedOpacity;  // Here, if ShadowOpacity is 100 (full), InvertedOpacity will be 0, hence transparent.


	if(reduceLightIntensity < 1.1f)
		finalShadow *= (1.0f - totalPointLightIntensity);
	

	LightResult *= (1.0f - finalShadow);

	if (SpecularLight > 0.5f)
	{	
		if (finalShadow < 0.30)
		{
			float3 V = normalize(CameraPos - input.WorldPos.xyz);
			float3 R = reflect(-L, N);
			float SpecularIntensity = pow(saturate(dot(V, R)), Shininess);
			LightResult += (SpecularIntensity * MaterialSpecular * LightSpecular.rgb)/1.5f;
		}
	}
	

#endif

    Color.rgb *= LightResult;

    // Fog
    const float fogStart = FogRange.x;
    const float fogEnd = FogRange.y;
    float distance = length(input.WorldPos.xyz - CameraPos);
    float fogFactor = saturate((distance - fogStart) / (fogEnd - fogStart));
    return lerp(Color, FogColor, fogFactor);
}

)";
