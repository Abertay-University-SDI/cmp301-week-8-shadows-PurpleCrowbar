
Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture[2] : register(t1);

SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler[2] : register(s1);

cbuffer LightBuffer : register(b0)
{
	float4 ambient[2];
	float4 diffuse[2];
	float4 direction[2];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos1 : TEXCOORD1;
    float4 lightViewPos2 : TEXCOORD2;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    return saturate(diffuse * intensity); // return final colour
}

// Is the geometry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, SamplerState sSampler, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(sSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.015f;
    float4 colour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);

    // Prepare light view positions and projective coordinates
    float4 lightViewPos[2] = { input.lightViewPos1, input.lightViewPos2 };
    float2 pTexCoords[2] = { getProjectiveCoords(input.lightViewPos1), getProjectiveCoords(input.lightViewPos2) };

    // Initialise shadow factor and combine ambient lighting
    float finalShadow = 1.f;
    float4 combinedAmbient = float4(0.f, 0.f, 0.f, 1.f);

    for (int i = 0; i < 2; i++)
    {
        // Combine ambient light contributions
        combinedAmbient += ambient[i];

        // Check if in shadow for this light
        if (hasDepthData(pTexCoords[i]) && isInShadow(depthMapTexture[i], shadowSampler[i], pTexCoords[i], lightViewPos[i], shadowMapBias))
        {
            finalShadow *= 0.f; // In shadow
        }
    }

    // If not in shadow, calculate lighting contribution for both lights
    if (finalShadow > 0.f)
    {
        for (int i = 0; i < 2; i++)
        {
            colour += calculateLighting(-direction[i], input.normal, diffuse[i]);
        }
    }

    // Add combined ambient light and apply texture
    colour = saturate(colour + combinedAmbient);
    return saturate(colour) * textureColour;
}