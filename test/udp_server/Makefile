udp_server_test : udp_server_test.o udp_server.o server_command.o
	g++ -L/usr/local/lib/ -lRakNet -lOgreMain -leq -o $@ $^

udp_server_test.o : udp_server_test.cpp ../../udp_server.hpp
	g++ -c $<

udp_server.o : ../../udp_server.cpp ../../udp_server.hpp ../../fifo_buffer.hpp\
	../../server_command.hpp
	g++ -c $<

server_command.o : ../../server_command.cpp ../../server_command.hpp ../../command.hpp
	g++ -c $<
