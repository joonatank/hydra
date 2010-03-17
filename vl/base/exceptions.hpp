#ifndef VL_EXCEPTIONS_HPP
#define VL_EXCEPTIONS_HPP

namespace vl
{
	class exception
	{
		public :
			exception(const char *wha, const char *wher)
				: what(wha), where(wher)
			{}

			const char *what;
			const char *where;
	};

	class null_pointer : public exception
	{
		public :
			null_pointer( const char *wher )
				: exception( "null_pointer", wher )
			{}
	};

	class empty_param : public exception
	{
		public :
			empty_param( const char *wher )
				: exception( "empty_param", wher )
			{}

	};

	class no_object : public exception
	{
		public :
			no_object( const char *wher )
				: exception( "no_object", wher )
			{}

	};

	class scale_quaternion : public exception
	{
		public :
			scale_quaternion( const char *wher )
				: exception( "scale quaternion", wher )
			{}
	};

	class zero_scale : public exception
	{
		public :
			zero_scale( const char *wher )
				: exception( "zero scale", wher )
			{}
	};

	class duplicate : public exception
	{
		public :
			duplicate( const char *wher )
				: exception( "duplicate", wher )
			{}
	};

	class fifo_full : public exception
	{
		public :
			fifo_full( const char *wher )
				: exception( "fifo_full", wher )
			{}
	};

}	// namespace vl

#endif
