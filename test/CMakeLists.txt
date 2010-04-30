project("tests" CXX)

cmake_minimum_required(VERSION 2.6)

include_directories( util )

#subdirs( util math eq_graph spikes eq_ogre synchronization dotscene_loader
#	vl_dotscene_loader )
subdirs( util math eq_graph spikes eq_ogre synchronization
	vl_dotscene_loader )