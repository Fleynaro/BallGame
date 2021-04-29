#include "World.h"
#include "Entity.h"


void Ball::shootAt(btVector3 vel, float size, float mass, float speed)
{
	auto bullet = new BallBullet(size, mass);
	bullet->setPos(
		getPos() + vel.normalize() * (
		((btSphereShape*)m_colShape)->getRadius() + ((btSphereShape*)bullet->m_rigidBody)->getRadius()
			)
	);

	bullet->m_rigidBody->setFriction(10.9);
	bullet->m_rigidBody->setRestitution(1.0);
	bullet->m_rigidBody->setDamping(0.5, 0.5);
	bullet->m_rigidBody->setAngularVelocity(btVector3(0, 5, 0));
	bullet->m_rigidBody->setLinearVelocity(vel * speed);
	World::m_world->addEntity(bullet);
	m_bullets.push_back(bullet);
}

void Ball::draw()
{
	for (auto it : m_bullets) {
		if (it->checkToRemove()) {
			m_bullets.remove(it);
			World::m_world->removeEntity(it);
			delete it;
			break;
		}
		it->passFrame();
	}
	IBall::draw();
}