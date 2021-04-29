#pragma once


#include <bullet/LinearMath/btVector3.h>
#include <DirectXMath.h>

class MathConvert
{
public:

	static btVector3 toBtVector(DirectX::XMVECTOR v)
	{
		DirectX::XMFLOAT4 f;
		DirectX::XMStoreFloat4(&f, v);
		return btVector3(f.x / f.w, f.y / f.w, f.z / f.w);
	}

	static DirectX::XMVECTOR toXmVector(btVector3 v)
	{
		return DirectX::XMVectorSet(v.getX(), v.getY(), v.getZ(), 1.0);
	}
};