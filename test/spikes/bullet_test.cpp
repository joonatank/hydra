#include <iostream>

#include <bullet/btBulletDynamicsCommon.h>

int main (void)
{
	std::cerr << "starting bullet test." << std::endl;

	btBroadphaseInterface* broadphase = new btDbvtBroadphase();

	std::cerr << "Broadphase created." << std::endl;
	btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
	std::cerr << "CollisionConfiguration created." << std::endl;
	btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
	std::cerr << "Dispatcher " << (void *)dispatcher << " created. size = "
		<< sizeof(btCollisionDispatcher) << std::endl;

	btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

	std::cerr << "Solver " << (void *)solver << " created. size = "
		<< sizeof(btSequentialImpulseConstraintSolver) << std::endl;

	std::cerr << "Trying to create the world." << std::endl;
	btDiscreteDynamicsWorld* dynamicsWorld
		= new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
	std::cerr << "World created." << std::endl;

	dynamicsWorld->setGravity(btVector3(0,-10,0));
	std::cerr << "Gravity set." << std::endl;

	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0), btScalar(1.0));

	btCollisionShape* fallShape = new btSphereShape(1);

	std::cerr << "Ground and fall shapes created." << std::endl;

	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
	btRigidBody::btRigidBodyConstructionInfo
			groundRigidBodyCI(0,groundMotionState,groundShape,btVector3(0,0,0));
	btRigidBody* groundRigidBody = new btRigidBody(groundRigidBodyCI);
	dynamicsWorld->addRigidBody(groundRigidBody);

	std::cerr << "Ground RigidBody created and added." << std::endl;

	btDefaultMotionState* fallMotionState =
			new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0)));
	btScalar mass = 1;
	btVector3 fallInertia(0,0,0);
	fallShape->calculateLocalInertia(mass,fallInertia);
	btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,fallMotionState,fallShape,fallInertia);
	btRigidBody* fallRigidBody = new btRigidBody(fallRigidBodyCI);
	dynamicsWorld->addRigidBody(fallRigidBody);

	std::cerr << "Fall RigidBody created and added." << std::endl;

	for (int i=0 ; i<300 ; i++) {
			dynamicsWorld->stepSimulation(1/60.f,10);

			btTransform trans;
			fallRigidBody->getMotionState()->getWorldTransform(trans);

			std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
	}

	dynamicsWorld->removeRigidBody(fallRigidBody);
	delete fallRigidBody->getMotionState();
	delete fallRigidBody;

	dynamicsWorld->removeRigidBody(groundRigidBody);
	delete groundRigidBody->getMotionState();
	delete groundRigidBody;


	delete fallShape;

	delete groundShape;

	delete dynamicsWorld;
	delete solver;
	delete collisionConfiguration;
	delete dispatcher;
	delete broadphase;

	return 0;
}