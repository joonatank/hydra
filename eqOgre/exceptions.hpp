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

	class fifo_full : public exception
	{
		public :
			fifo_full( const char *wher )
				: exception( "fifo_full", wher )
			{}
	};
}
