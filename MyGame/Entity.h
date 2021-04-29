#pragma once


#include "Model.h"
#include <bullet/btBulletDynamicsCommon.h>


class Entity
{
public:
	virtual btVector3 getPos() = 0;
	virtual btQuaternion getRot() = 0;
	virtual void setPos(btVector3 pos) = 0;
	virtual void setRot(btQuaternion q) = 0;

	virtual void draw() = 0;

	Model* m_model = nullptr;
};


class PhysEntity : public Entity
{
public:
	PhysEntity(btRigidBody* rigidBody)
		: m_rigidBody(rigidBody)
	{}

	btRigidBody* m_rigidBody;
	btCollisionShape* m_colShape = nullptr;

	btTransform getTransform()
	{
		btTransform trans;

		if (m_rigidBody && m_rigidBody->getMotionState())
		{
			m_rigidBody->getMotionState()->getWorldTransform(trans);
		}
		else
		{
			trans = m_rigidBody->getWorldTransform();
		}

		return trans;
	}

	void setTransform(btTransform trans)
	{
		if (m_rigidBody && m_rigidBody->getMotionState())
		{
			m_rigidBody->getMotionState()->setWorldTransform(trans);
		}
		m_rigidBody->setWorldTransform(trans);
	}

	btVector3 getPos() override {
		return getTransform().getOrigin();
	}

	void setPos(btVector3 pos) override {
		auto trans = getTransform();
		trans.setOrigin(pos);
		setTransform(trans);
	}

	btQuaternion getRot() override {
		return getTransform().getRotation();
	}

	void setRot(btQuaternion q) override {
		auto trans = getTransform();
		trans.setRotation(q);
		setTransform(trans);
	}

	void draw() override {
		auto trans = getTransform();

		g_direct3d->m_World *=
			XMMatrixRotationQuaternion(
				XMVectorSet(
					trans.getRotation().getX(),
					trans.getRotation().getY(),
					trans.getRotation().getZ(),
					trans.getRotation().getW()
				)
			)
			* XMMatrixTranslation(
				trans.getOrigin().getX(),
				trans.getOrigin().getY(),
				trans.getOrigin().getZ()
			);

		g_direct3d->setDefaultToDraw();
		m_model->draw();
	}

	static btTransform getDefaultTransform() {
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(0.f, 0.f, 0.f));
		return trans;
	}
};


class Base : public PhysEntity
{
public:
	Base(btVector3 size, btScalar mass = 0.0f)
		: m_size(size), PhysEntity(nullptr)
	{
		m_colShape = new btBoxShape(size);

		btVector3 localInertia(0, 0, 0);
		if (mass > 0)
			m_colShape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* MotionState = new btDefaultMotionState(getDefaultTransform());

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, MotionState, m_colShape, localInertia);
		m_rigidBody = new btRigidBody(rbInfo);
		
		m_model = Models::LoadCubeModel();
		m_tex = Textures::LoadSand();
	}

	void draw() override {
		auto size = ((btBoxShape*)m_colShape)->getHalfExtentsWithoutMargin();

		g_direct3d->m_World =
			XMMatrixScaling(
				size.getX(),
				size.getY(),
				size.getZ()
			);
		m_tex->draw();
		PhysEntity::draw();
	}
private:
	btVector3 m_size;
	Texture* m_tex;
};


class IShooted
{
public:
	virtual void shootAt(btVector3 vel, float size, float mass, float speed) = 0;
};

class ITemporary
{
public:

	void passFrame()
	{
		m_lifeFrameDuration--;
	}

	bool checkToRemove()
	{
		if (m_lifeFrameDuration <= 0) {
			return true;
		}
		return false;
	}
private:
	int m_lifeFrameDuration = GAME_FPS * 3;
};


class IBall : public PhysEntity
{
public:
	IBall(btScalar size = 1.0f, btScalar mass = 1.0f)
		: m_size(size), PhysEntity(nullptr)
	{
		m_colShape = new btSphereShape(m_size);

		btVector3 localInertia(0, 0, 0);
		if (mass > 0)
			m_colShape->calculateLocalInertia(mass, localInertia);

		btDefaultMotionState* MotionState = new btDefaultMotionState(getDefaultTransform());

		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, MotionState, m_colShape, localInertia);
		m_rigidBody = new btRigidBody(rbInfo);
	}

	void draw() override {
		/*static float t = 0.f;
		t += (float)XM_PI * 0.0125f / 50.f;

		g_direct3d->setDefaultLightDir();
		XMMATRIX mRotate = XMMatrixRotationY(t);
		auto vLightDir = XMLoadFloat4(&g_direct3d->m_vLightDir);
		vLightDir = XMVector3Transform(vLightDir, mRotate);
		XMStoreFloat4(&g_direct3d->m_vLightDir, vLightDir);*/

		auto r = ((btSphereShape*)m_colShape)->getRadius();

		g_direct3d->m_World =
			XMMatrixScaling(
				r, r, r
			);
		m_tex->draw();
		PhysEntity::draw();
	}
protected:
	btScalar m_size;
	Texture* m_tex;
};


class BallBullet : public IBall, public ITemporary
{
public:
	BallBullet(btScalar size = 0.5f, btScalar mass = 1.3f)
		: IBall(size, mass)
	{
		m_model = Models::LoadSphereModel();
		m_tex = Textures::LoadBallStrike();
	}
};


class Ball : public IBall, public IShooted
{
public:
	Ball(btScalar size = 1.0f, btScalar mass = 1.0f)
		: IBall(size, mass)
	{
		m_model = Models::LoadSphereModel();
		m_tex = Textures::LoadBallFleynaro();
	}

	std::list<BallBullet*> m_bullets;
	void draw() override;
	void shootAt(btVector3 vel, float size = 0.3, float mass = 0.4, float speed = 50) override;
};