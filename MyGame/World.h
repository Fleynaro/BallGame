#pragma once

#include "Entity.h"

class World
{
public:
	World()
	{
		initPhysicWorld();
		m_dynamicsWorld->setGravity(btVector3(0, -15, 0));
	}

	void addEntity(PhysEntity* entity) {
		m_entityPool.push_back(entity);
		m_dynamicsWorld->addRigidBody(entity->m_rigidBody);
	}

	void removeEntity(PhysEntity* entity)
	{
		m_entityPool.remove(entity);
		m_dynamicsWorld->removeRigidBody(entity->m_rigidBody);
	}

	std::list<Entity*>& getPool() {
		return m_entityPool;
	}

	btDiscreteDynamicsWorld& getPhysic() {
		return *m_dynamicsWorld;
	}

	inline static World* m_world = nullptr;
private:
	void initPhysicWorld()
	{
		///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
		btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
		///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
		btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
		///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
		btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
		///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
		btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

		//create a world
		m_dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
		m_dynamicsWorld->setGravity(btVector3(0, -15, 0));
	}

private:
	btDiscreteDynamicsWorld* m_dynamicsWorld;

	std::list<Entity*> m_entityPool;
};