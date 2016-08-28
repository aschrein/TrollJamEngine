#pragma once
namespace Options
{
	template< typename F , typename S >
	class Optional
	{
	private:
		F first;
		S second;
		bool var;
	public:
		Optional() = default;
		void setFirst( F &&f )
		{
			first = std::move( f );
			var = true;
		}
		void setSecond( S &&s )
		{
			second = std::move( s );
			var = false;
		}
		void setFirst( F const &f )
		{
			first = f;
			var = true;
		}
		void setSecond( S const &s )
		{
			second = s;
			var = false;
		}
		bool isFirst() const
		{
			return var;
		}
		F &getFirst()
		{
			return first;
		}
		S &getSecond()
		{
			return second;
		}
		F const &getFirst() const
		{
			return first;
		}
		S const &getSecond() const
		{
			return second;
		}
	};
	template< typename V >
	class Result
	{
	private:
		V val;
		bool present = false;
	public:
		Result() = default;
		Result( Result const & ) = delete;
		Result &operator=( Result const & ) = delete;
		Result( Result && ) = default;
		Result &operator=( Result && ) = default;
		Result( V &&v ):
			val( std::move( v ) )
		{
			present = true;
		}
		Result( V const &v ) :
			val( v )
		{
			present = true;
		}
		bool isPresent() const
		{
			return present;
		}
		V getValue()
		{
			return std::move( val );
		}
	};
}