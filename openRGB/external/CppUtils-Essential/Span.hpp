//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: own span substituting span from C++20
//======================================================================================================================

#ifndef CPPUTILS_SPAN_INCLUDED
#define CPPUTILS_SPAN_INCLUDED


#include "Essential.hpp"

#include "TypeTraits.hpp"
#include "CriticalError.hpp"

#include <array>    // fixed_span construction, size_t, begin, end, ...


namespace own {


//======================================================================================================================
/// Generalization over continuous-memory containers.
/** To be used instead of pair of buffer pointer and size. */

template< typename Element >
class span
{
	Element * _begin;
	Element * _end;

 public:

	span() noexcept : _begin( nullptr ), _end( nullptr ) {}

	// construct manually from pair of pointers
	span( Element * begin, Element * end ) noexcept : _begin( begin ), _end( end ) {}

	// construct manually from a data pointer and size
	span( Element * data, size_t size ) noexcept : _begin( data ), _end( data + size ) {}

	// construct from the same span type
	span( const span & other ) noexcept = default;

	// construct from a compatible span type
	// Allows constructing  span< const char >  from  span< char >  or  span< BaseClass >  from  span< SubClass >.
	template< typename OtherElem, REQUIRES( !std::is_same< OtherElem, Element >::value ) >
	span( span< OtherElem > other ) noexcept : span( other.begin(), other.end() ) {}

	// deduce from a generic container
	template< typename Container, REQUIRES( is_contiguous_range< Container >::value && !std::is_const< Element >::value ) >
	span( Container & cont ) noexcept : span( fut::data(cont), fut::size(cont) ) {}
	template< typename Container, REQUIRES( is_contiguous_range< Container >::value && std::is_const< Element >::value ) >
	span( const Container & cont ) noexcept : span( fut::data(cont), fut::size(cont) ) {}

	// assign
	span & operator=( const span & other ) noexcept = default;

	Element * begin() const noexcept   { return _begin; }
	Element * end() const noexcept     { return _end; }
	Element * data() const noexcept    { return _begin; }
	size_t size() const noexcept       { return size_t( _end - _begin ); }
	bool empty() const noexcept        { return _begin == _end; }

	Element & operator[]( int index ) const noexcept  { return _begin[ index ]; }

	span shorter( size_t newSize ) const noexcept
	{
		if (newSize > size())
			critical_error( "attempted to increase span size from %zu to %zu", size(), newSize );
		return span( _begin, newSize );
	}

	template< typename OtherElem >
	span< OtherElem > interpret_as() const noexcept
	{
		static_assert( sizeof(Element) % sizeof(OtherElem) == 0,
			"you can only cast to a span whose element size divide the original element size"
		);
		return { reinterpret_cast< OtherElem * >( _begin ), reinterpret_cast< OtherElem * >( _end ) };
	}

	span< typename corresponding_constness< Element, uint8_t >::type > as_bytes() const noexcept
	{
		return interpret_as< typename corresponding_constness< Element, uint8_t >::type >();
	}

	// specialized function only available for some ElemTypes
	template< typename Elem = Element, REQUIRES( std::is_same< Elem, Element >::value && is_byte_alike< Element >::value ) >
	span< typename corresponding_constness< Element, char >::type > as_chars() const noexcept
	{
		return interpret_as< typename corresponding_constness< Element, char >::type >();
	}

 private:

	// Members of this class with different template arguments are not normally accessible.
	template< typename OtherType >
	friend class span;
};


//======================================================================================================================
/// Variant of own::span with compile-time length.
/** Use this when you know how long the span is at compile-time. */

template< typename Element, size_t size_ >
class fixed_span
{
	Element * _begin;

	// span< char >        ->  std::array< char >
	// span< const char >  ->  const std::array< char >
	using CorrespondingArray = typename corresponding_constness<
		Element, std::array< typename std::remove_const<Element>::type, size_ >
	>::type;

 public:

	fixed_span() noexcept : _begin( nullptr ) {}

	// construct from static containers
	fixed_span( Element (& arr) [size_] ) noexcept : _begin( arr ) {}

	fixed_span( CorrespondingArray & arr ) noexcept : _begin( arr.data() ) {}

	// construct from the same span type
	fixed_span( const fixed_span & other ) noexcept = default;

	// construct from a compatible span type
	// Allow constructing  span< const char >  from  span< char >  or  span< BaseClass >  from  span< SubClass >.
	template< typename OtherElem, REQUIRES( !std::is_same< OtherElem, Element >::value ) >
	fixed_span( fixed_span< OtherElem, size_ > other ) noexcept : _begin( other.begin() ) {}

	// assign
	fixed_span & operator=( const fixed_span & other ) noexcept = default;

	Element * begin() const noexcept   { return _begin; }
	Element * end() const noexcept     { return _begin + size_; }
	Element * data() const noexcept    { return _begin; }
	size_t size() const noexcept       { return size_; }
	bool empty() const noexcept        { return size_ == 0; }

	Element & operator[]( int index ) const noexcept  { return _begin[ index ]; }

	// convert to dynamic span
	span< Element > to_dynamic() const noexcept  { return span< Element >( _begin, size_ ); }
	operator span< Element >() const noexcept    { return to_dynamic(); }

	template< size_t newSize >
	fixed_span< Element, newSize > shorter() const noexcept
	{
		static_assert( newSize <= size_, "newSize must be smaller than current size" );
		return from_ptr< Element, newSize >( _begin );
	}

	template< typename OtherElem >
	fixed_span< OtherElem, size_ > interpret_as() const noexcept
	{
		static_assert( sizeof(Element) % sizeof(OtherElem) == 0,
			"you can only cast to a span whose element size divide the original element size"
		);
		return from_ptr< OtherElem, size_ >( reinterpret_cast< OtherElem * >( _begin ) );
	}

	fixed_span< typename corresponding_constness< Element, uint8_t >::type, size_ > as_bytes() const noexcept
	{
		return interpret_as< typename corresponding_constness< Element, uint8_t >::type >();
	}

	// specialized function only available for some ElemTypes
	template< typename Elem = Element, REQUIRES( std::is_same< Elem, Element >::value && is_byte_alike< Element >::value ) >
	fixed_span< typename corresponding_constness< Element, char >::type, size_ > as_chars() const noexcept
	{
		return interpret_as< typename corresponding_constness< Element, char >::type >();
	}

 private:

	// Members of this class with different template arguments are not normally accessible.
	template< typename OtherElem, size_t otherSize >
	friend class fixed_span;

	template< typename OtherElem, size_t otherSize >
	static fixed_span< OtherElem, otherSize > from_ptr( OtherElem * begin ) noexcept
	{
		fixed_span< OtherElem, otherSize > s;
		s._begin = begin;
		return s;
	}
};


//======================================================================================================================
// aliases

using byte_span       = span< uint8_t >;
using const_byte_span = span< const uint8_t >;
using char_span       = span< char >;
using const_char_span = span< const char >;

template< size_t size_ > using fixed_byte_span       = fixed_span< uint8_t, size_ >;
template< size_t size_ > using fixed_const_byte_span = fixed_span< const uint8_t, size_ >;
template< size_t size_ > using fixed_char_span       = fixed_span< char, size_ >;
template< size_t size_ > using fixed_const_char_span = fixed_span< const char, size_ >;


//======================================================================================================================
// automatic template argument deduction

// from pointers
template< typename Element >
auto make_span( Element * begin, Element * end ) noexcept
 -> span< Element >
{
	return { begin, end };
}
template< typename Element >
auto make_span( Element * data, size_t size ) noexcept
 -> span< Element >
{
	return { data, size };
}

// from generic containers
template< typename Container >
auto make_span( Container & cont ) noexcept
 -> span< typename range_value<Container>::type >
{
	return { fut::data(cont), fut::size(cont) };
}
template< typename Container >
auto make_span( const Container & cont ) noexcept
 -> span< const typename range_value<Container>::type >
{
	return { fut::data(cont), fut::size(cont) };
}

// from static containers
template< typename Element, size_t size_ >
auto make_fixed_span( Element (& arr) [size_] ) noexcept
 -> fixed_span< Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_span( const Element (& arr) [size_] ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_span( std::array< Element, size_ > & arr ) noexcept
 -> fixed_span< Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_span( const std::array< Element, size_ > & arr ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}


//======================================================================================================================
// automatic template argument deduction with forced const

// from pointers
template< typename Element >
auto make_const_span( Element * begin, Element * end ) noexcept
 -> span< const Element >
{
	return { begin, end };
}
template< typename Element >
auto make_const_span( Element * data, size_t size ) noexcept
 -> span< const Element >
{
	return { data, size };
}

// from generic containers
template< typename Container >
auto make_const_span( Container & cont ) noexcept
 -> span< const typename range_value<Container>::type >
{
	return { fut::data(cont), fut::size(cont) };
}
template< typename Container >
auto make_const_span( const Container & cont ) noexcept
 -> span< const typename range_value<Container>::type >
{
	return { fut::data(cont), fut::size(cont) };
}

// from static containers
template< typename Element, size_t size_ >
auto make_fixed_const_span( Element (& arr) [size_] ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_const_span( const Element (& arr) [size_] ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_const_span( std::array< Element, size_ > & arr ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}
template< typename Element, size_t size_ >
auto make_fixed_const_span( const std::array< Element, size_ > & arr ) noexcept
 -> fixed_span< const Element, size_ >
{
	return { arr };
}


//======================================================================================================================


} // namespace own


#endif // CPPUTILS_SPAN_INCLUDED
