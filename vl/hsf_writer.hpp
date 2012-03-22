/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file hsf_writer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 */

#ifndef HYDRA_HSF_WRITER_HPP
#define HYDRA_HSF_WRITER_HPP

// Necessary for HYDRA_API
#include "defines.hpp"
// Necessary for types
#include "typedefs.hpp"
// Necessary for XML parsing
#include "base/rapidxml.hpp"

#include "base/filesystem.hpp"

#include "math/types.hpp"

namespace vl
{

class HYDRA_API HSFWriter
{
public :
	HSFWriter(vl::GameManagerPtr game);

	~HSFWriter(void);

	/// @brief Write the complete game context to string
	/// @param file the path the the file to write
	/// @param game GameContext to write
	void write(fs::path const &file, bool overwrite = false);

private :
	/// Scene nodes
	void writeScene(rapidxml::xml_node<> *xml_root, vl::GameManagerPtr game);

	void writeNodes(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game);
	
	void writeNode(rapidxml::xml_node<> *xml_node, GameObjectRefPtr obj);

	void writeBody(rapidxml::xml_node<> *xml_node, GameObjectRefPtr obj);

	void writeChildNode(rapidxml::xml_node<> *xml_node, vl::SceneNodePtr child);

	void writeEntity(rapidxml::xml_node<> *xml_node, vl::EntityPtr ent);

	void writeLight(rapidxml::xml_node<> *xml_node, vl::LightPtr light);

	void writeCamera(rapidxml::xml_node<> *xml_node, vl::CameraPtr camera);

	void writeLightRange(rapidxml::xml_node<> *xml_node, vl::LightPtr light);

	void writeLightAttenuation(rapidxml::xml_node<> *xml_node, vl::LightPtr light);

	/// Constraint nodes
	void writeConstraints(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game);

	void writeConstraint(rapidxml::xml_node<> *xml_node, vl::ConstraintRefPtr constraint);
	void writeConstraint(rapidxml::xml_node<> *xml_node, vl::physics::ConstraintRefPtr constraint);


	/// Environment nodes
	void writeEnvironment(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game);

	void writeFog(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game);

	void writeSky(rapidxml::xml_node<> *xml_node, vl::GameManagerPtr game);

	void _writeVector3(rapidxml::xml_node<> *xml_node, Ogre::Vector3 const &v);
	void _writeQuaternion(rapidxml::xml_node<> *xml_node, Ogre::Quaternion const &q);

	vl::GameManagerPtr _game;

	rapidxml::xml_document<> _doc;

};	// class HsfWriter

}	// namespace vl

#endif // HYDRA_HSF_WRITER_HPP