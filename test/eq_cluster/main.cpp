#include "eq_cluster/client.hpp"
//#include "root.hpp"
//#include "camera.hpp"
//#include "initData.hpp"
#include "eq_cluster/nodeFactory.hpp"

int main( int const argc, char **argv )
{
	// Redirect equalizer logger to file, prevents flooding std::cerr,
	// which we use for network code testing
	std::ofstream log_file( "equalize.log" );
	eq::base::Log::setOutput( log_file );

	// 3. Equalizer initialization
	eqOgre::NodeFactory nodeFactory;
	if( !eq::init( argc, argv, &nodeFactory ))
	{
		EQERROR << "Equalizer init failed" << std::endl;
		return EXIT_FAILURE;
	}

	// 4. initialization of local client node
	eq::base::RefPtr< eqOgre::Client > client = new eqOgre::Client( );
	if( !client->initLocal( argc, argv ))
	{
		EQERROR << "Can't init client" << std::endl;
		eq::exit();
		return EXIT_FAILURE;
	}

	// 5. run client
	const int ret = client->run();

	// 6. cleanup and exit
	client->exitLocal();

	// TODO EQASSERTINFO( client->getRefCount() == 1, client->getRefCount( ));
	client = 0;

	eq::exit();

	log_file.close();

	return ret;
}

