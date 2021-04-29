//--------------------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Константные буферы
//--------------------------------------------------------------------------------------
Texture2D txDiffuse : register( t0 );		// Буфер текстуры
SamplerState samLinear : register( s0 );	// Буфер образца

cbuffer CMatrixBuffer : register( b0 )
{
	matrix World;			// Матрица мира
	matrix View;			// Матрица вида
	matrix Projection;		// Матрица проекции
}

cbuffer CLightBuffer : register( b1 )
{
	float4 vLightDir;		// Направление источника света
	float4 vLightColor;		// Цвет источника света
}


//--------------------------------------------------------------------------------------
struct VS_INPUT					// Входящие данные вершинного шейдера
{
    float4 Pos : POSITION;		// Позиция по X, Y, Z
    float3 Norm : NORMAL;		// Нормаль по X, Y, Z
	float2 Tex : TEXCOORD0;		// Координаты текстуры по tu, tv
};

struct PS_INPUT					// Входящие данные пиксельного шейдера
{
    float4 Pos : SV_POSITION;	// Позиция пикселя в проекции (экранная)
	float2 Tex : TEXCOORD0;		// Координаты текстуры по tu, tv
    float3 Norm : TEXCOORD1;	// Относительная нормаль пикселя по tu, tv
};


//--------------------------------------------------------------------------------------
// Вершинный шейдер
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( input.Pos, World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Norm = mul( input.Norm, World );
	output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Пиксельный шейдер для куба
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
	float ambient = 0.6;
    float4 finalColor = float4(ambient, ambient, ambient, 1.0);
    
    // складываем освещенность пикселя от всех источников света
    finalColor += saturate( dot( (float3)vLightDir, input.Norm ) * vLightColor );
    finalColor.a = 1;
	finalColor *= txDiffuse.Sample( samLinear, input.Tex );
	
    return finalColor;
}
