struct screen_dims {
    float width;
    float height;
};
ConstantBuffer<screen_dims> screenDims: register(b0);

struct vertex_sprite {
    float3 pos: POSITION;
    float2 texCoords: TEX_COORD;
    float4 color: COLOR;
    uint textureID: TEX_ID;
};

struct vs_output {
    float2 texCoords: TEX_COORD;
    float4 color: COLOR;
    uint textureID: TEX_ID;
    float4 pos: SV_Position;
};

vs_output main (vertex_sprite vertex) {
    vs_output result;

    result.pos = float4(vertex.pos.x * (1.0f / screenDims.width) * 2.0f - 1.0f, 
                        vertex.pos.y * (1.0f / screenDims.height) * 2.0f - 1.0f, 
                        1.0f, 
                        1.0f);
    result.pos.y = -result.pos.y;
    result.texCoords = vertex.texCoords;
    result.color = vertex.color;
    result.textureID = vertex.textureID;

    return result;
}
