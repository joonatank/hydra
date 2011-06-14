/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file constraints.hpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Defines basic non-physical constraints. These are based on the common
 *	constraints in physics engines, but do not do the dynamics simulation.
 */

#ifndef HYDRA_CONSTRAINTS_HPP
#define HYDRA_CONSTRAINTS_HPP

namespace vl
{

class Constraint
{
public :

};	// class Constraint

class SixDofConstraint : public Constraint
{
public :

};	// class SixDofConstraint

class SliderConstraint : public Constraint
{
public :

};	// class SliderConstraint

class HingeConstraint : public Constraint
{
public :

};	// class HingeConstraint

}	// namespace vl

#endif	// HYDRA_CONSTRAINTS_HPP
