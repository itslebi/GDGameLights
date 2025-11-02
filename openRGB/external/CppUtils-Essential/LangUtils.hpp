//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: template helpers, generic container helpers and backports from newer C++ standards
//======================================================================================================================

#ifndef CPPUTILS_LANGUAGE_INCLUDED
#define CPPUTILS_LANGUAGE_INCLUDED


#include "Essential.hpp"

#include "TypeTraits.hpp"
#include "SafetyChecks.hpp"

#include <iterator>  // advance, begin, end
#include <memory>
#include <functional>


//======================================================================================================================
// workarounds that depend on the current C++ standard
//
// The active C++ standard version can be detected using the following values of pre-processor value __cplusplus:
// C++11: 201103L
// C++14: 201402L
// C++17: 201703L
// C++20: 202002L
// C++23: 202302L


//======================================================================================================================
// utils from standard library of a newer C++ standard

namespace fut {


// C++23
template< typename Enum, REQUIRES( std::is_enum< Enum >::value ) >
constexpr typename std::underlying_type< Enum >::type to_underlying( Enum num ) noexcept
{
	return static_cast< typename std::underlying_type< Enum >::type >( num );
}

// C++14
template< typename Type, typename ... Args >
std::unique_ptr< Type > make_unique( Args && ... args )
{
	return std::unique_ptr< Type >( new Type( std::forward< Args >( args ) ... ) );
}

// C++14
template< typename Iter >
constexpr std::reverse_iterator< Iter > make_reverse_iterator( Iter it ) noexcept
{
    return std::reverse_iterator< Iter >( it );
}

#if __cplusplus >= 201703L  // requires optional and variant
// simplified versions of C++23 std::expected and std::unexpected
template< typename Error >
class unexpected
{
 public:

	unexpected( Error error ) : _error( std::move(error) ) {}

	template< typename ... Args >
	unexpected( Args && ... args ) : _error( std::forward< Args >( args ) ... ) {}

	      Error & error()        { return _error; }
	const Error & error() const  { return _error; }

 private:

	Error _error;
};
template< typename Value, typename Error >
class expected
{
 public:

	using unexpected_type = unexpected< Error >;

	template< typename ... Args >
	expected( Args && ... args ) : _data( std::forward< Args >( args ) ... ) {}

	expected( unexpected_type unexp ) : _data( std::move( unexp ) ) {}

	bool has_value() const           { return _data.index() == 0; }
	explicit operator bool () const  { return has_value(); }

	      Value & value()         { return std::get< 0 >( _data ); }
	const Value & value() const   { return std::get< 0 >( _data ); }

	      Error & error()        { return std::get< 1 >( _data ).error(); }
	const Error & error() const  { return std::get< 1 >( _data ).error(); }

	template< typename UniRef >
	Value value_or( UniRef && defaultVal ) const &
	{
		return has_value() ? value() : Value( std::forward< UniRef >( defaultVal ) );
	}
	template< typename UniRef >
	Value value_or( UniRef && defaultVal ) &&
	{
		return has_value() ? std::move( value() ) : Value( std::forward< UniRef >( defaultVal ) );
	}

	      Value & operator * ()         { return value(); }
	const Value & operator * () const   { return value(); }
	      Value * operator -> ()        { return &value(); }
	const Value * operator -> () const  { return &value(); }

 private:

	std::variant< Value, unexpected_type > _data;
};
template< typename Error >
class expected< void, Error >
{
 public:

	using unexpected_type = unexpected< Error >;

	expected() = default;
	expected( unexpected_type unexp ) : _unexp( std::move( unexp ) ) {}

	bool has_value() const           { return !_unexp.has_value(); }
	explicit operator bool () const  { return !static_cast< bool >( _unexp ); }

	      Error & error()        { return _unexp->error(); }
	const Error & error() const  { return _unexp->error(); }

 private:

	std::optional< unexpected_type > _unexp;
};
#endif


} // namespace fut

//======================================================================================================================


namespace own {


//======================================================================================================================
// scope guards

template< typename EndFunc >
class scope_guard
{
	EndFunc _atEnd;

 public:

	scope_guard( const EndFunc & endFunc ) : _atEnd( endFunc ) {}
	scope_guard( EndFunc && endFunc ) : _atEnd( move(endFunc) ) {}
	~scope_guard() noexcept  { _atEnd(); }
};

template< typename EndFunc >
class optional_scope_guard
{
	EndFunc _atEnd;
	bool _enabled;

 public:

	optional_scope_guard( const EndFunc & endFunc ) : _atEnd( endFunc ), _enabled( true ) {}
	optional_scope_guard( EndFunc && endFunc ) : _atEnd( move(endFunc) ), _enabled( true ) {}

	~optional_scope_guard() noexcept
	{
		if (_enabled)
			_atEnd();
	}

	void dismiss()  { _enabled = false; }

	bool isEnabled() const  { return _enabled; }

	void execute()
	{
		if (_enabled)
		{
			_atEnd();
			_enabled = false;
		}
	}
};

template< typename EndFunc >
scope_guard< EndFunc > at_scope_end_do( const EndFunc & endFunc )
{
	return scope_guard< EndFunc >( endFunc );
}

template< typename EndFunc >
scope_guard< EndFunc > at_scope_end_do( EndFunc && endFunc )
{
	return scope_guard< EndFunc >( move(endFunc) );
}


//======================================================================================================================
// custom iterators

/// Iterator wrapper that converts a key,value pair of a map into just a key (as if it was a set)
template< typename WrappedIter >
class KeyIterator
{
	WrappedIter wrappedIter;
 public:
	KeyIterator( const WrappedIter & origIter ) : wrappedIter( origIter ) {}
	auto operator*() -> decltype( **wrappedIter ) const   { return wrappedIter->first; }
	auto operator->() -> decltype( *wrappedIter ) const   { return &wrappedIter->first; }
	KeyIterator & operator++()    { ++wrappedIter; return *this; }
	KeyIterator operator++(int)   { auto prev = *this; ++wrappedIter; return prev; }
	friend bool operator==( const KeyIterator & a, const KeyIterator & b )  { return a.wrappedIter == b.wrappedIter; }
	friend bool operator!=( const KeyIterator & a, const KeyIterator & b )  { return a.wrappedIter != b.wrappedIter; }
};

/// Wrapper around iterator to container of pointers that skips the additional needed dereference and returns a reference directly
template< typename WrappedIter >
class DerefIterator
{
	WrappedIter wrappedIter;
 public:
	DerefIterator( const WrappedIter & origIter ) : wrappedIter( origIter ) {}
	auto operator*() -> decltype( **wrappedIter ) const   { return **wrappedIter; }
	auto operator->() -> decltype( *wrappedIter ) const   { return *wrappedIter; }
	DerefIterator & operator++()    { ++wrappedIter; return *this; }
	DerefIterator operator++(int)   { auto tmp = *this; ++wrappedIter; return tmp; }
	friend bool operator==( const DerefIterator & a, const DerefIterator & b )  { return a.wrappedIter == b.wrappedIter; }
	friend bool operator!=( const DerefIterator & a, const DerefIterator & b )  { return a.wrappedIter != b.wrappedIter; }
};


//======================================================================================================================
// miscellaneous

template< typename Type >
constexpr Type & unconst( const Type & x ) noexcept
{
	return const_cast< Type & >( x );
}

template< typename Type >
constexpr Type * unconst( const Type * x ) noexcept
{
	return const_cast< Type * >( x );
}

template< typename Type >
constexpr const Type & as_const( Type & t ) noexcept
{
    return t;
}

template< typename Type >
constexpr const Type * as_const( Type * t ) noexcept
{
    return t;
}

template< typename Type, typename Deleter >
std::unique_ptr< Type, Deleter > make_unique_with_deleter( Type * newObj , Deleter && deleter )
{
	return std::unique_ptr< Type, Deleter >( newObj, std::forward< Deleter >( deleter ) );
}

/// return value variant of std::advance
template< typename Iterator, typename Distance >
Iterator advance( Iterator it, Distance n )
{
	std::advance( it, n );
	return it;
}

class tribool
{
	using rep = int8_t;
	static constexpr rep empty_rep = -1;

	rep _val;

 public:

	struct empty_t {};
	static constexpr empty_t empty{};

	tribool() : _val( empty_rep ) {}
	tribool( empty_t ) : _val( empty_rep ) {}
	tribool( bool bval ) : _val( static_cast<rep>(bval) ) {}
	tribool( const tribool & other ) = default;

	bool has_value() const  { return _val >= 0; }
	bool value() const
	{
		SAFETY_CHECK( has_value(), "value() called when tribool is empty" );
		return static_cast<bool>(_val);
	}

	bool operator==( const tribool & other ) const  { return _val == other._val; }
};


//======================================================================================================================


} // namespace own


#endif // CPPUTILS_LANGUAGE_INCLUDED
