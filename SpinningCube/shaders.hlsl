
cbuffer cb {
    float4x4 world_view_proj;
};

struct VS_IN {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_OUT {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


VS_OUT VS(VS_IN vin) {
    VS_OUT vout;
    vout.position = mul(world_view_proj, float4(vin.position, 1.0));
    vout.color = vin.color;
    return vout;
}

float4 PS(VS_OUT pin) : SV_TARGET {
    return pin.color;
}
