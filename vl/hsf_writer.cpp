/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file hsf_writer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 */

#include "hsf_writer.hpp"

#include "base/exceptions.hpp"

#include "base/rapidxml_print.hpp"

#include "game_manager.hpp"
#include "game_object.hpp"
#include "scene_manager.hpp"
#include "scene_node.hpp"
#include "entity.hpp"
#include "light.hpp"
#include "camera.hpp"

#include "physics/physics_world.hpp"
#include "physics/rigid_body.hpp"
#include "physics/shapes.hpp"
#include "physics/physics_constraints.hpp"

#include "animation/kinematic_world.hpp"
#include "animation/kinematic_body.hpp"
#include "animation/constraints.hpp"

#include "mesh.hpp"

#include <iostream>

vl::HSFWriter::HSFWriter(vl::GameManagerPtr game)
	: _game(game)
{
	assert(_game);
}

vl::HSFWriter::~HSFWriter(void)
{
}

void
vl::HSFWriter::write(fs::path const &file, bool overwrite)
{
	if(!overwrite && fs::exists(file))
	{
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Not overwriting a file."));
	}

	// xml declaration
	rapidxml::xml_node<> *decl = _doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(_doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(_doc.allocate_attribute("encoding", "utf-8"));
	_doc.append_node(decl);

	rapidxml::xml_node<> *xml_root = _doc.allocate_node(rapidxml::node_element, "scene");
	xml_root->append_attribute(_doc.allocate_attribute("version", "0.1"));
	_doc.append_node(xml_root);
	writeScene(xml_root, _game);

	std::string xml_data;
	rapidxml::print(std::back_inserter(xml_data), _doc);

	// write to file
	std::ofstream ofs(file.string());
	ofs << xml_data;
}

void
vl::HSFWriter::writeScene(rapidxml::xml_node<> *xml_root, vl::GameManagerPtr game)
{
	// @todo should not write empty elements
	rapidxml::xml_node<> *env = _doc.allocate_node(rapidxml::node_element, "environment");
	xml_root->append_node(env);
	writeEnvironment(env, game);

	rapidxml::xml_node<> *nodes = _doc.allocate_node(rapidxml::node_element, "nodes");
	xml_root->append_node(nodes);
	writeNodes(nodes, game);

	rapidxml::xml_node<> *constraints = _doc.allocate_node(rapidxml::node_element, "constraints");
	xml_root->append_node(constraints);
	writeConstraints(constraints, game);
}

void
vl::HSFWriter::writeNodes(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game)
{
	GameObjectList const &objs = game->getGameObjectList();
	for(GameObjectList::const_iterator iter = objs.begin(); iter != objs.end(); ++iter)
	{
		rapidxml::xml_node<> *node = _doc.allocate_node(rapidxml::node_element, "node");
		xml_node->append_node(node);
		writeNode(node, *iter);
	}
}

void
vl::HSFWriter::writeNode(rapidxml::xml_node<> *xml_node, GameObjectRefPtr obj)
{
	assert(obj);
	
	// write name
	xml_node->append_attribute(_doc.allocate_attribute("name", obj->getName().c_str()));

	// write physics type
	bool kinematic = obj->isKinematic();
	bool dynamic = obj->getPhysicsNode();

	std::string physics_type("none");
	if(kinematic)
	{
		physics_type = "kinematic";
	}
	else if(dynamic)
	{
		physics_type = "dynamic";
	}
	char *type = _doc.allocate_string(physics_type.c_str());
	xml_node->append_attribute(_doc.allocate_attribute("physics_type", type));

	// Write dynamics
	rapidxml::xml_node<> *xml_body = _doc.allocate_node(rapidxml::node_element, "body");
	xml_node->append_node(xml_body);
	writeBody(xml_body, obj);

	// Write transformation
	rapidxml::xml_node<> *trans = _doc.allocate_node(rapidxml::node_element, "transform");
	xml_node->append_node(trans);
	_writeTransform(trans, obj->getTransform());

	// @todo write scale

	// Write collisions models
	rapidxml::xml_node<> *col = _doc.allocate_node(rapidxml::node_element, "collision");
	xml_node->append_node(col);
	
	assert(obj->isCollisionDetectionEnabled() == (bool)obj->getCollisionModel());

	char *col_enabled = _doc.allocate_string(vl::to_string(obj->isCollisionDetectionEnabled()).c_str());
	col->append_attribute(_doc.allocate_attribute("enabled", col_enabled));
	if(obj->getCollisionModel())
	{
		physics::ConvexHullShapeRefPtr hull = boost::dynamic_pointer_cast<physics::ConvexHullShape>(obj->getCollisionModel());
		// @tood not supporting other types than meshes
		assert(hull);
		char *name = _doc.allocate_string(hull->getMesh()->getName().c_str());
		col->append_attribute(_doc.allocate_attribute("model", name));
	}
	

	// Write graphics

	// @todo write visible

	SceneNodePtr sn = obj->getGraphicsNode();
	for(vl::SceneNodeList::const_iterator iter = sn->getChilds().begin();
		iter != sn->getChilds().end(); ++iter)
	{
		rapidxml::xml_node<> *child = _doc.allocate_node(rapidxml::node_element, "node");
		xml_node->append_node(child);
		writeChildNode(child, *iter);
	}

	for(vl::MovableObjectList::const_iterator iter = sn->getObjects().begin();
		iter != sn->getObjects().end(); ++iter)
	{
		if((*iter)->getTypeName() == "Entity")
		{
			EntityPtr ent = static_cast<EntityPtr>(*iter);
			rapidxml::xml_node<> *xml_ent = _doc.allocate_node(rapidxml::node_element, "entity");
			xml_node->append_node(xml_ent);
			writeEntity(xml_ent, ent);
		}
		else if((*iter)->getTypeName() == "Camera")
		{
			CameraPtr cam = static_cast<CameraPtr>(*iter);
			rapidxml::xml_node<> *xml_cam= _doc.allocate_node(rapidxml::node_element, "camera");
			writeCamera(xml_cam, cam);
		}
		else if((*iter)->getTypeName() == "Light")
		{
			LightPtr light = static_cast<LightPtr>(*iter);
			rapidxml::xml_node<> *xml_light = _doc.allocate_node(rapidxml::node_element, "light");
			writeLight(xml_light, light);
		}
		// @todo add the rest of the types
	}
}
	
void
vl::HSFWriter::writeBody(rapidxml::xml_node<> *xml_node, GameObjectRefPtr obj)
{
	// @todo should use the game objects and not rigid body
	// because we destroy the rigid body when dynamics is not in use so we will lose
	// these settings if the dynamics is disable when saving.

	physics::RigidBodyRefPtr body = obj->getPhysicsNode();
	if(body)
	{
		xml_node->append_attribute(_doc.allocate_attribute("type", "rigid"));
		char *mass = _doc.allocate_string(vl::to_string(body->getMass()).c_str());
		xml_node->append_attribute(_doc.allocate_attribute("mass", mass));

		rapidxml::xml_node<> *inertia = _doc.allocate_node(rapidxml::node_element, "inertia");
		xml_node->append_node(inertia);
		_writeVector3(inertia, body->getInertia());
	}
}

void
vl::HSFWriter::writeChildNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr child)
{
	// @todo not implemented
	// should divide the writeNode function to two parts
	// one for writing the SceneNode (and MovableObjects) and other one for
	// writing the physics. We could then use the SceneNode part recursively.
	std::clog << "vl::HSFWriter::writeChildNode" << std::endl;
}

void
vl::HSFWriter::writeEntity(rapidxml::xml_node<> *xml_node, vl::EntityPtr ent)
{
	std::string mesh_name;
	if(!ent->getMeshName().empty())
	{
		mesh_name = ent->getMeshName();
	}
	else
	{
		mesh_name = ent->getMesh()->getName();
	}

	assert(!mesh_name.empty());

	char *mn = _doc.allocate_string(mesh_name.c_str());
	xml_node->append_attribute(_doc.allocate_attribute("mesh_file", mn));

	char *name = _doc.allocate_string(ent->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("name", name));

	// @todo write material

	// @todo write cast shadows

	// @todo write instanced

	// @todo write visible
}

void
vl::HSFWriter::writeLight(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	std::clog << "vl::HSFWriter::writeLight" << std::endl;
}

void
vl::HSFWriter::writeCamera(rapidxml::xml_node<> *xml_node, vl::CameraPtr camera)
{
	std::clog << "vl::HSFWriter::writeCamera" << std::endl;
}

void
vl::HSFWriter::writeLightRange(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	std::clog << "vl::HSFWriter::writeLightRange" << std::endl;
}

void
vl::HSFWriter::writeLightAttenuation(rapidxml::xml_node<> *xml_node, vl::LightPtr light)
{
	std::clog << "vl::HSFWriter::writeLightAttenuation" << std::endl;
}

void
vl::HSFWriter::writeConstraints(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game)
{
	std::clog << "vl::HSFWriter::writeConstraints" << std::endl;

	physics::ConstraintList const &cl = game->getPhysicsWorld()->getConstraints();
	for(physics::ConstraintList::const_iterator iter = cl.begin();
		iter != cl.end(); ++iter)
	{
		rapidxml::xml_node<> *c_node = _doc.allocate_node(rapidxml::node_element, "constraint");
		xml_node->append_node(c_node);
		writeConstraint(c_node, *iter);
	}

	ConstraintList const &cl_ = game->getKinematicWorld()->getConstraints();
	for(ConstraintList::const_iterator iter = cl_.begin();
		iter != cl_.end(); ++iter)
	{
		rapidxml::xml_node<> *c_node = _doc.allocate_node(rapidxml::node_element, "constraint");
		xml_node->append_node(c_node);
		writeConstraint(c_node, *iter);
	}
}

void
vl::HSFWriter::writeConstraint(rapidxml::xml_node<> *xml_node, vl::ConstraintRefPtr constraint)
{
	std::clog << "vl::HSFWriter::writeConstraint" << std::endl;
	assert(constraint->getBodyA());
	assert(constraint->getBodyB());

	xml_node->append_attribute(_doc.allocate_attribute("physics_type", "kinematic"));

	char *str = _doc.allocate_string(constraint->getTypeName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("type", str));

	str = _doc.allocate_string(constraint->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("name", str));

	str = _doc.allocate_string(constraint->getBodyA()->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("body_a", str));

	str = _doc.allocate_string(constraint->getBodyB()->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("body_b", str));

	str = _doc.allocate_string(vl::to_string(constraint->isActuator()).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("actuator", str));

	// write local frames
	// @todo should have information if there is a frame object for this constraint
	// this object would be one that came with the CAD data
	// and it would supercede the frame information otherwise

	rapidxml::xml_node<> *frameA= _doc.allocate_node(rapidxml::node_element, "frame_a");
	xml_node->append_node(frameA);
	_writeTransform(frameA, constraint->getLocalFrameA());

	rapidxml::xml_node<> *frameB= _doc.allocate_node(rapidxml::node_element, "frame_b");
	xml_node->append_node(frameB);
	_writeTransform(frameB, constraint->getLocalFrameB());

	rapidxml::xml_node<> *limit = _doc.allocate_node(rapidxml::node_element, "limit");
	xml_node->append_node(limit);

	// Don't write the axis node to xml file here
	// only write it if we have a valid axis for the constraint
	rapidxml::xml_node<> *axis = _doc.allocate_node(rapidxml::node_element, "axis");

	// needs to be casted to get limits
	if(HingeConstraintRefPtr hinge = boost::dynamic_pointer_cast<HingeConstraint>(constraint))
	{
		limit->append_attribute(_doc.allocate_attribute("unit", "degree"));

		str = _doc.allocate_string(vl::to_string(hinge->getLowerLimit().valueDegrees()).c_str());
		limit->append_attribute(_doc.allocate_attribute("min", str));

		str = _doc.allocate_string(vl::to_string(hinge->getUpperLimit().valueDegrees()).c_str());
		limit->append_attribute(_doc.allocate_attribute("max", str));

		xml_node->append_node(axis);
		_writeVector3(axis, hinge->getAxis());
	}
	else if(SliderConstraintRefPtr slider = boost::dynamic_pointer_cast<SliderConstraint>(constraint))
	{
		str = _doc.allocate_string(vl::to_string(slider->getLowerLimit()).c_str());
		xml_node->append_attribute(_doc.allocate_attribute("min", str));

		str = _doc.allocate_string(vl::to_string(slider->getUpperLimit()).c_str());
		limit->append_attribute(_doc.allocate_attribute("max", str));

		xml_node->append_node(axis);
		_writeVector3(axis, slider->getAxis());
	}
	else
	{
		assert(boost::dynamic_pointer_cast<FixedConstraint>(constraint));
	}
}

void
vl::HSFWriter::writeConstraint(rapidxml::xml_node<> *xml_node, vl::physics::ConstraintRefPtr constraint)
{
	std::clog << "vl::HSFWriter::writeConstraint" << std::endl;
	assert(constraint->getBodyA());
	assert(constraint->getBodyB());

	xml_node->append_attribute(_doc.allocate_attribute("physics_type", "dynamic"));

	char *str = _doc.allocate_string(constraint->getTypeName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("type", str));

	str = _doc.allocate_string(constraint->getBodyA()->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("body_a", str));

	str = _doc.allocate_string(constraint->getBodyB()->getName().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("body_b", str));

	// write local frames
	// @todo should have information if there is a frame object for this constraint
	// this object would be one that came with the CAD data
	// and it would supercede the frame information otherwise

	rapidxml::xml_node<> *frameA= _doc.allocate_node(rapidxml::node_element, "frame_a");
	xml_node->append_node(frameA);
	_writeTransform(frameA, constraint->getLocalFrameA());

	rapidxml::xml_node<> *frameB= _doc.allocate_node(rapidxml::node_element, "frame_b");
	xml_node->append_node(frameB);
	_writeTransform(frameB, constraint->getLocalFrameB());

	rapidxml::xml_node<> *limit = _doc.allocate_node(rapidxml::node_element, "limit");
	xml_node->append_node(limit);
	
	// needs to be casted to get limits
	// @todo needs to write motors and damping
	if(physics::HingeConstraintRefPtr hinge = boost::dynamic_pointer_cast<physics::HingeConstraint>(constraint))
	{
		limit->append_attribute(_doc.allocate_attribute("unit", "radian"));

		str = _doc.allocate_string(vl::to_string(hinge->getLowerLimit()).c_str());
		limit->append_attribute(_doc.allocate_attribute("min", str));

		str = _doc.allocate_string(vl::to_string(hinge->getUpperLimit()).c_str());
		limit->append_attribute(_doc.allocate_attribute("max", str));
	}
	else if(physics::SliderConstraintRefPtr slider = boost::dynamic_pointer_cast<physics::SliderConstraint>(constraint))
	{
		str = _doc.allocate_string(vl::to_string(slider->getLowerLinLimit()).c_str());
		xml_node->append_attribute(_doc.allocate_attribute("min", str));

		str = _doc.allocate_string(vl::to_string(slider->getUpperLinLimit()).c_str());
		limit->append_attribute(_doc.allocate_attribute("max", str));
	}
	else
	{
		physics::SixDofConstraintRefPtr sixdof = boost::dynamic_pointer_cast<physics::SixDofConstraint>(constraint);
		assert(sixdof);

		rapidxml::xml_node<> *low_linear = _doc.allocate_node(rapidxml::node_element, "low_linear");
		xml_node->append_node(low_linear);
		_writeVector3(low_linear, sixdof->getLinearLowerLimit());
		
		rapidxml::xml_node<> *high_linear = _doc.allocate_node(rapidxml::node_element, "high_linear");
		xml_node->append_node(high_linear);
		_writeVector3(high_linear, sixdof->getLinearUpperLimit());

		rapidxml::xml_node<> *low_angular = _doc.allocate_node(rapidxml::node_element, "low_angular");
		xml_node->append_node(low_angular);
		_writeVector3(low_angular, sixdof->getAngularLowerLimit());

		rapidxml::xml_node<> *high_angular = _doc.allocate_node(rapidxml::node_element, "high_angular");
		xml_node->append_node(high_angular);
		_writeVector3(high_angular, sixdof->getAngularUpperLimit());
	}
}

void
vl::HSFWriter::_writeTransform(rapidxml::xml_node<> *xml_node, vl::Transform const &t)
{
	rapidxml::xml_node<> *vec= _doc.allocate_node(rapidxml::node_element, "position");
	xml_node->append_node(vec);
	_writeVector3(vec, t.position);

	rapidxml::xml_node<> *q= _doc.allocate_node(rapidxml::node_element, "quaternion");
	xml_node->append_node(q);
	_writeQuaternion(q, t.quaternion);
}

void
vl::HSFWriter::writeEnvironment(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game)
{
	std::clog << "vl::HSFWriter::writeEnvironment" << std::endl;
	
	assert(game->getSceneManager());

	rapidxml::xml_node<> *fog_node = _doc.allocate_node(rapidxml::node_element, "fog");
	xml_node->append_node(fog_node);
	writeFog(fog_node, game->getSceneManager()->getFog());

	rapidxml::xml_node<> *sky_node = _doc.allocate_node(rapidxml::node_element, "sky");
	xml_node->append_node(sky_node);
	writeSky(sky_node, game->getSceneManager());

	rapidxml::xml_node<> *shadow_node = _doc.allocate_node(rapidxml::node_element, "shadows");
	xml_node->append_node(shadow_node);
	writeShadows(shadow_node, game->getSceneManager()->getShadowInfo());

	// Write ambient
	rapidxml::xml_node<> *ambient_node = _doc.allocate_node(rapidxml::node_element, "ambient_light");
	xml_node->append_node(ambient_node);
	_writeColour(ambient_node, game->getSceneManager()->getAmbientLight());
}

void
vl::HSFWriter::writeFog(rapidxml::xml_node<> *xml_node, vl::FogInfo const &fog)
{
	std::clog << "vl::HSFWriter::writeFog" << std::endl;
	
	char *type = _doc.allocate_string(fog.getMode().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("type", type));

	char *density = _doc.allocate_string(vl::to_string(fog.exp_density).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("density", density));

	char *start = _doc.allocate_string(vl::to_string(fog.linear_start).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("start", start));

	char *end = _doc.allocate_string(vl::to_string(fog.linear_end).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("end", end));

	rapidxml::xml_node<> *colour = _doc.allocate_node(rapidxml::node_element, "colour");
	xml_node->append_node(colour);
	_writeColour(colour, fog.colour_diffuse);
}

void
vl::HSFWriter::writeSky(rapidxml::xml_node<> *xml_node, vl::SceneManagerPtr scene)
{
	std::clog << "vl::HSFWriter::writeSky" << std::endl;
	SkyInfo const &info = scene->getSkyInfo();
	SkyDomeInfo const &dome = scene->getSkyDome();

	std::string type("static");
	if(info.getPreset() != "none")
	{
		type = "dynamic";
	}

	char *str = _doc.allocate_string(type.c_str());
	xml_node->append_attribute(_doc.allocate_attribute("type", str));

	xml_node->append_attribute(_doc.allocate_attribute("fallback", "true"));


	/// Write dynamic parameters
	rapidxml::xml_node<> *dyn = _doc.allocate_node(rapidxml::node_element, "dynamic");
	xml_node->append_node(dyn);

	char *preset = _doc.allocate_string(info.getPreset().c_str());
	dyn->append_attribute(_doc.allocate_attribute("preset", preset));

	/// Write static parameters
	rapidxml::xml_node<> *stat = _doc.allocate_node(rapidxml::node_element, "static");
	xml_node->append_node(stat);

	stat->append_attribute(_doc.allocate_attribute("type", "dome"));

	char *mat = _doc.allocate_string(dome.material_name.c_str());
	stat->append_attribute(_doc.allocate_attribute("material", mat));
}

void
vl::HSFWriter::writeShadows(rapidxml::xml_node<> *xml_node, vl::ShadowInfo const &shadows)
{
	std::clog << "vl::HSFWriter::writeShadows" << std::endl;


	char *str = _doc.allocate_string(vl::to_string(shadows.isEnabled()).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("enabled", str));

	str = _doc.allocate_string(shadows.getCamera().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("camera", str));
	
	str = _doc.allocate_string(vl::to_string(shadows.isShelfShadowEnabled()).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("shelf_shadowing", str));

	str = _doc.allocate_string(vl::to_string(shadows.getTextureSize()).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("texture_size", str));

	str = _doc.allocate_string(vl::to_string(shadows.getMaxDistance()).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("max_distance", str));

	str = _doc.allocate_string(shadows.getShadowCasterMaterial().c_str());
	xml_node->append_attribute(_doc.allocate_attribute("caster_material", str));
}

void
vl::HSFWriter::_writeVector3(rapidxml::xml_node<> *xml_node, Ogre::Vector3 const &v)
{
	// @todo we need to round the output, zeroes seem to be turning to really small numbers
	char *x = _doc.allocate_string(vl::to_string(v.x).c_str());
	char *y = _doc.allocate_string(vl::to_string(v.y).c_str());
	char *z = _doc.allocate_string(vl::to_string(v.z).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("x", x));
	xml_node->append_attribute(_doc.allocate_attribute("y", y));
	xml_node->append_attribute(_doc.allocate_attribute("z", z));
}

void
vl::HSFWriter::_writeQuaternion(rapidxml::xml_node<> *xml_node, Ogre::Quaternion const &q)
{
	// @todo we need to round the output, zeroes seem to be turning to really small numbers
	char *w = _doc.allocate_string(vl::to_string(q.w).c_str());
	char *x = _doc.allocate_string(vl::to_string(q.x).c_str());
	char *y = _doc.allocate_string(vl::to_string(q.y).c_str());
	char *z = _doc.allocate_string(vl::to_string(q.z).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("w", w));
	xml_node->append_attribute(_doc.allocate_attribute("x", x));
	xml_node->append_attribute(_doc.allocate_attribute("y", y));
	xml_node->append_attribute(_doc.allocate_attribute("z", z));
}

void
vl::HSFWriter::_writeColour(rapidxml::xml_node<> *xml_node, Ogre::ColourValue const &col)
{
	// @todo we need to round the output, zeroes seem to be turning to really small numbers
	char *r = _doc.allocate_string(vl::to_string(col.r).c_str());
	char *g = _doc.allocate_string(vl::to_string(col.g).c_str());
	char *b = _doc.allocate_string(vl::to_string(col.b).c_str());
	char *a = _doc.allocate_string(vl::to_string(col.a).c_str());
	xml_node->append_attribute(_doc.allocate_attribute("r", r));
	xml_node->append_attribute(_doc.allocate_attribute("g", g));
	xml_node->append_attribute(_doc.allocate_attribute("b", b));
	xml_node->append_attribute(_doc.allocate_attribute("a", a));
}

