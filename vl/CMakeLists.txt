PROJECT( "eqOgre" CXX )

set( SOURCES
	eq_ogre/ogre_root.cpp
	eq_ogre/ogre_root.hpp
	base/args.hpp
	base/args.cpp
	settings.cpp
	settings.hpp
	tracker.hpp
	vrpn_tracker.cpp
	vrpn_tracker.hpp
	base/exceptions.hpp
	dotscene_loader.cpp
	dotscene_loader.hpp
	)

add_library( eqOgre ${SOURCES} )
