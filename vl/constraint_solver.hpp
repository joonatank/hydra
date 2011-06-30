/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file constraint_solver.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Constraint solver.
 */

#ifndef HYDRA_CONSTRAINT_SOLVER_HPP
#define HYDRA_CONSTRAINT_SOLVER_HPP

#include "constraints.hpp"

#include "typedefs.hpp"

#include "math/transform.hpp"

#include "animation.hpp"

namespace vl
{

/// @class ConstraintSolver
class ConstraintSolver
{
public :
	/// @brief constructor
	ConstraintSolver(void);

	/// @brief destructor
	~ConstraintSolver(void);

	void addConstraint(vl::ConstraintRefPtr constraint);

	void removeConstraint(vl::ConstraintRefPtr constraint);

	bool hasConstraint(vl::ConstraintRefPtr constraint) const;

	/// @brief step the constraint solver
	/// @param t the time since last step
	void step(vl::time const &t);

	friend std::ostream &operator<<(std::ostream &os, ConstraintSolver const &solver);

private :
	typedef std::map<SceneNode *, animation::NodeRefPtr> AnimationNodeMap;

	std::vector<vl::ConstraintRefPtr> _constraints;

	animation::GraphRefPtr _graph;

	AnimationNodeMap _node_map;

};	// end class ConstraintSolver

}	// end namespace vl

#endif	// HYDRA_CONSTRAINT_SOLVER_HPP
