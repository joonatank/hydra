/* Joonatan Kuosa
 * 2010-01
 * First test structures for event or command driven rendering
 *
 * Design completely flawed... these are almost impossible to expand
 * in any easy way.
 * TODO redesign after testing the initial version.
 *
 * We use fifo_buffer to pass pointers to these structures from
 * appThread to pipeThread (e.g. rendering thread).
 */

#ifndef EQ_OGRE_COMMAND
#define EQ_OGRE_COMMAND

#include <eq/base/debug.h>	// needed for EQASSERT

#include <vmmlib/vector.hpp>
#include <vmmlib/quaternion.hpp>

#include "base/typedefs.hpp"


//#include <OGRE/OgreCommon.h>

//#include "math/vector.hpp"
//#include "math/quaternion.hpp"

//#include <OgreString.h>
//#include <OgreVector3.h>
//#include <OgreQuaternion.h>

namespace vl
{

namespace base
{

	const uint32_t ID_INVALID = 0;

	enum
	{
		CMD_CREATE = CMD_UNUSED,
		CMD_DELETE,
		CMD_MOVE,
		CMD_ROTATE,
		CMD_LOOKAT
	};

	enum TransformSpace
	{
		TS_LOCAL = 0,
		TS_PARENT,
		TS_WORLD
	};

	class CommandStruct
	{
		public :
			CommandStruct( CMD_TYPE cmd = CMD_INVALID,
					std::string const &name1 = std::string(),
					std::string const &typeName1 = std::string(),
					std::string const &creator1 = std::string(),
					std::string const &parent1 = std::string(),
					vmml::vec3d const &vec = vmml::vec3d::ZERO,
					vmml::quaterniond const quat = vmml::quaterniond::IDENTITY,
					TransformSpace space1 = TS_LOCAL,
					vl::NamedValuePairList param = vl::NamedValuePairList() )
				: cmdType(cmd), name(name1), typeName(typeName1), creator(creator1),
				  parent(parent1), position(vec), rotation(quat),
				  space(space1), params(param)
			{}

			vl::base::CMD_TYPE cmdType;
			std::string name;
			std::string typeName;
			std::string creator;
			std::string parent;
			vmml::vec3d position;
			vmml::quaterniond rotation;
			TransformSpace space;
			vl::NamedValuePairList params;
	};

	inline
	std::ostream &operator<<( std::ostream &os, vl::base::CommandStruct const &cmd )
	{
		os << "type = " << cmd.cmdType << std::endl
			<< "name = " << cmd.name << std::endl
			<< "type name = " << cmd.typeName << std::endl
			<< "creator = " << cmd.creator << std::endl
			<< "position = " << cmd.position << std::endl
			<< "rotation quaternion = " << cmd.rotation << std::endl
			<< "space = " << cmd.space << std::endl;

		return os;
	}

	// Dummy class for client and Server commands
	class Command
	{
		public :
			virtual ~Command( void ) {}

	};

	}	// namespace base 

};	// namespace vl

#endif
