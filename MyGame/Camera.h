#pragma once


#include "Entity.h"
#include "World.h"


class Camera : public PhysEntity
{
public:
	PhysEntity* m_entity = nullptr;
	btScalar m_dist = 10.0f;		//расстояние от камеры до сущности
	btScalar m_alpha = 0.0f;	//вращение вокруг объекта вправо/влево
	btScalar m_beta = 0.0f;		//вращение вокруг объекта вниз/вверх

	Camera()
		: PhysEntity(nullptr)
	{}

	void setEntityToLookAt(PhysEntity* entity) {
		m_entity = entity;
	}

	void setAngles(btScalar alpha, btScalar beta) {
		m_alpha = alpha;
		m_beta = beta;
	}

	void updateViewMatrix() {
		if (m_entity == nullptr)
			return;

		auto trans = m_entity->getTransform();


		XMVECTOR vec = XMVector3Transform(
			XMVectorSet(0.0, 0.0, m_dist, 1.0),
			XMMatrixRotationX(m_beta) * XMMatrixRotationY(m_alpha) //порядок произведения вращательных матриц также имеет значение!!!
		);

		XMVECTOR At = XMVectorSet(
			trans.getOrigin().getX(),
			trans.getOrigin().getY(),
			trans.getOrigin().getZ(),
			1.0
		);

		auto from = MathConvert::toBtVector(At);
		auto to = MathConvert::toBtVector(At) + MathConvert::toBtVector(vec);

		//check the camera on contact to objects
		btCollisionWorld::ClosestRayResultCallback RayCallback(from, to);
		World::m_world->getPhysic().rayTest(from, to, RayCallback);

		XMVECTOR Eye;
		if (RayCallback.hasHit())
		{
			Eye = MathConvert::toXmVector(RayCallback.m_hitPointWorld);
		}
		else {
			Eye = XMVectorAdd(At, vec);
		}

		XMVECTOR Up = XMVectorSet(0.0, 1.0, 0.0, 1.0);
		g_direct3d->m_View = XMMatrixLookAtLH(Eye, At, Up);
	}

	void draw() override {
	}
};