fifo_ptr_test : fifo_ptr_test.o fifo_test.o
	g++ -lpthread -o $@ $^

fifo_test.o : fifo_test.cpp fifo_test.hpp ../fifo_buffer.hpp
	g++ -c $<

fifo_ptr_test.o : fifo_ptr_test.cpp fifo_test.hpp ../fifo_buffer.hpp
	g++ -c $<
