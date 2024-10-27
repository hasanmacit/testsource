const char* razador_terrainShadowsPS = R"(
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

static const float3 pointLightPositionGalbeni[88] = 
{
    float3(27332.939453, -9137.156250, 16555.929688 + 100.0f),
    float3(22770.380859, -8602.360352, 15764.730469 + 100.0f),
    float3(22638.466797, -11624.431641, 16768.886719 + 100.0f),
    float3(24411.439453, -14290.979492, 15724.527344 + 100.0f),
    float3(21977.578125, -15735.047852, 15288.264648 + 100.0f),
    float3(26214.861328, -20753.636719, 15714.932617 + 100.0f),
    float3(28670.671875, -25605.226562, 15332.577148 + 100.0f),
    float3(25317.390625, -24486.337891, 15341.512695 + 100.0f),
    float3(20029.398438, -21897.021484, 15524.740234 + 100.0f),
    float3(18647.947266, -25868.572266, 15531.969727 + 100.0f),
    float3(21678.886719, -27048.126953, 15707.974609 + 100.0f),
    float3(30006.937500, -27517.300781, 15549.500000 + 100.0f),
    float3(26409.919922, -28575.501953, 15795.132812 + 100.0f),
    float3(25731.119141, -31155.515625, 15618.719727 + 100.0f),
    float3(24652.378906, -32662.667969, 15601.382812 + 100.0f),
    float3(18980.306641, -30877.570312, 18258.199219 + 100.0f),
    float3(17474.029297, -32346.085938, 18020.078125 + 100.0f),
    float3(20868.240234, -30674.048828, 17478.814453 + 100.0f),
    float3(12171.211914, -40868.488281, 18060.351562 + 100.0f),
    float3(13376.086914, -42077.980469, 18240.421875 + 100.0f),
    float3(15339.157227, -42705.394531, 17944.500000 + 100.0f),
    float3(18498.296875, -40385.425781, 15741.692383 + 100.0f),
    float3(26304.746094, -39321.703125, 15651.500000 + 100.0f),
    float3(24797.052734, -35031.882812, 15629.617188 + 100.0f),
    float3(25257.146484, -38030.742188, 15653.764648 + 100.0f),
    float3(23201.472656, -39761.503906, 14890.120117 + 100.0f),
    float3(19237.623047, -38954.664062, 15690.387695 + 100.0f),
    float3(20158.332031, -41805.566406, 15676.487305 + 100.0f),
    float3(20205.771484, -47773.691406, 15678.525391 + 100.0f),
    float3(15544.453125, -47714.769531, 15664.000000 + 100.0f),
    float3(17346.603516, -50215.343750, 15675.304688 + 100.0f),
    float3(26720.732422, -47507.453125, 18103.460938 + 100.0f),
    float3(28806.792969, -46012.925781, 17984.841797 + 100.0f),
    float3(28657.882812, -43231.316406, 18019.894531 + 100.0f),
    float3(29531.302734, -44512.128906, 18074.662109 + 100.0f),
    float3(26486.435547, -39675.890625, 15654.089844 + 100.0f),
    float3(28271.597656, -41215.277344, 15607.179688 + 100.0f),
    float3(31394.810547, -41806.515625, 15690.759766 + 100.0f),
    float3(34605.835938, -40847.980469, 15728.707031 + 100.0f),
    float3(36489.941406, -38091.093750, 15639.559570 + 100.0f),
    float3(33548.449219, -37728.113281, 18090.484375 + 100.0f),
    float3(31707.613281, -37375.687500, 18093.279297 + 100.0f),
    float3(32237.246094, -36454.156250, 18081.439453 + 100.0f),
    float3(32581.591797, -35749.484375, 18047.599609 + 100.0f),
    float3(33357.937500, -34087.570312, 17845.595703 + 100.0f),
    float3(33178.902344, -31433.248047, 18007.150391 + 100.0f),
    float3(35599.308594, -28934.923828, 18303.578125 + 100.0f),
    float3(31594.302734, -26924.728516, 15549.500000 + 100.0f),
    float3(32181.664062, -24443.787109, 15346.990234 + 100.0f),
    float3(31943.123047, -22893.421875, 16284.942383 + 100.0f),
    float3(33083.382812, -17984.085938, 16008.915039 + 100.0f),
    float3(33564.156250, -20695.218750, 15449.282227 + 100.0f),
    float3(31204.615234, -18591.701172, 15342.594727 + 100.0f),
    float3(30277.947266, -16204.231445, 15174.870117 + 100.0f),
    float3(30613.664062, -13759.099609, 15515.490234 + 100.0f),
    float3(29521.429688, -11920.050781, 15700.190430 + 100.0f),
    float3(27111.136719, -9126.108398, 16588.054688 + 100.0f),
    float3(27003.632812, -6264.538086, 15665.089844 + 100.0f),
    float3(37434.246094, -18464.468750, 15572.415039 + 100.0f),
    float3(37707.363281, -15625.314453, 15404.105469 + 100.0f),
    float3(40678.046875, -17322.523438, 15421.837891 + 100.0f),
    float3(41565.859375, -21251.835938, 15555.507812 + 100.0f),
    float3(38423.925781, -24183.822266, 15422.487305 + 100.0f),
    float3(41462.539062, -24989.802734, 15595.632812 + 100.0f),
    float3(44117.378906, -23447.695312, 15564.195312 + 100.0f),
    float3(45952.871094, -21325.953125, 15765.450195 + 100.0f),
    float3(44843.160156, -18726.054688, 15567.427734 + 100.0f),
    float3(43169.074219, -16386.513672, 15394.169922 + 100.0f),
    float3(46855.570312, -17411.470703, 15660.615234 + 100.0f),
    float3(49052.921875, -14051.779297, 15667.312500 + 100.0f),
    float3(45751.996094, -13670.289062, 15645.660156 + 100.0f),
    float3(42371.292969, -35537.433594, 15688.037109 + 100.0f),
    float3(44853.835938, -36560.082031, 15666.900391 + 100.0f),
    float3(49173.753906, -37073.308594, 15734.919922 + 100.0f),
    float3(48736.445312, -40562.199219, 15703.290039 + 100.0f),
    float3(52020.925781, -41429.804688, 15674.184570 + 100.0f),
    float3(53489.898438, -36707.613281, 15702.969727 + 100.0f),
    float3(40533.351562, -41941.949219, 15668.224609 + 100.0f),
    float3(38888.226562, -44342.191406, 15664.000000 + 100.0f),
    float3(36062.425781, -45952.968750, 15666.594727 + 100.0f),
    float3(33166.000000, -46310.781250, 15668.727539 + 100.0f),
    float3(39664.187500, -51187.300781, 15781.445312 + 100.0f),
    float3(43360.789062, -50275.210938, 15701.620117 + 100.0f),
    float3(42658.156250, -47008.941406, 15770.587891 + 100.0f),
    float3(34273.246094, -28769.896484, 16776.500000),
    float3(25000.000000, -55000.000000, 15694.000000 + 100.0f),
    float3(20000.000000, -52500.000000, 15737.750000 + 100.0f),
    float3(30000.000000, -55000.000000, 15694.000000 + 100.0f)
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
        float flicker = sin(Time / 2.5f) * 0.5f + 0.5f;
        flicker += sin(Time * 12.34f) * 0.25f;
        flicker = saturate(flicker);
        flicker = 1.0f + (flicker - 1.0f) / 35.0f; // This line will reduce the intensity of the flickering
		
		float flicker2 = sin(Time / 250.0f) * 0.25f + 0.75f;  // Slower base flicker
		flicker2 += sin(Time / 50.0f) * 0.1f;                 // Slower and smaller magnitude secondary fluctuation
		flicker2 = saturate(flicker2);                        // Ensure it's clamped between 0 and 1
		flicker2 *= 0.85f;                                    // Reduce overall intensity to make it darker

        if (SpecularLight > 0.5f)
        {
    
            float3 colorFactor = float3(1.0f, 0.5f, 0.2f);
            int lightCount = 88;
        
            for (int i = 0; i < lightCount; ++i)
            {
                float pointLightIntensity1 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionGalbeni[i], 6000.0f) * flicker;
                float pointLightIntensity2 = ComputePointLightIntensity(input.WorldPos.xyz, pointLightPositionGalbeni[i], 3000.0f) * flicker;
                totalPointLightIntensity += pointLightIntensity1;
                Color.rgb += pointLightIntensity2 * colorFactor * 5.0f / reduceLightIntensity;
            }
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
