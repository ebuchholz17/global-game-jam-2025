struct pixel_color {
    float2 texCoords: TEX_COORD;
    float4 color: COLOR;
    uint textureID: TEX_ID;
};

Texture2D textures[16] : register(t0);
SamplerState defaultSampler : register(s0);

float4 main (pixel_color pixel): SV_Target {
    Texture2D t = textures[NonUniformResourceIndex(pixel.textureID)];
    float2 textureSize;
    t.GetDimensions(textureSize.x, textureSize.y);

    float2 uv = pixel.texCoords;
    uv *= textureSize;

    float2 seam = floor(uv + 0.5);
    float2 dudv = fwidth(uv);
    uv = seam + clamp((uv - seam) / dudv, -0.5, 0.5);

    uv /= textureSize;

    float4 sample = t.Sample(defaultSampler, uv);

    return sample * pixel.color;
}
