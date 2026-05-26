[[vk::binding(0, 0)]]
Texture2D<float4> g_textures[];

[[vk::binding(3, 0)]]
SamplerState g_samplers[];

[[vk::push_constant]]
cbuffer PushConstants {
    uint sampled_index;
    uint sampler_index;
};

struct VertOut {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};

[shader("vertex")]
VertOut vert_main(uint vertex_id : SV_VertexID) {
    float2 positions[3] = {
        float2(-1.0, -1.0),
        float2( 3.0, -1.0),
        float2(-1.0,  3.0),
    };
    float2 uvs[3] = {
        float2(0.0, 0.0),
        float2(2.0, 0.0),
        float2(0.0, 2.0),
    };
    VertOut o;
    o.pos = float4(positions[vertex_id], 0.0, 1.0);
    o.uv  = uvs[vertex_id];
    return o;
}

[shader("fragment")]
float4 frag_main(VertOut i) : SV_Target {
    return g_textures[sampled_index].Sample(g_samplers[sampler_index], i.uv);
}