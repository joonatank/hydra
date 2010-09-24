#include <iostream>

#include "udp/client.hpp"

char const *HOST = "localhost";
uint16_t const PORT = 2244;

int main(int argc, char **argv)
{
    std::cout << "Starting UDP client" << std::endl;

	std::cout << "Client connecting to host : " << HOST << " "
		<< "using port = " << PORT << std::endl;
	
	try
	{
		vl::udp::Client client( HOST, PORT );
		
		std::cout << "Client created" << std::endl;

		for( size_t j = 0; j < 3; ++j )
		{
			std::vector<double> test_vec(9+j);
			for( size_t i = 0; i < test_vec.size(); ++i )
			{
				test_vec.at(i) = i;
			}

			client.send( test_vec );

			std::cout << "Message sent" << std::endl;
		}
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception : " << e.what() << std::endl;
	}

	return 0;
}