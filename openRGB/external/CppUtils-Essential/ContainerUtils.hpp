//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: helpers that require declarations of STL containers
//======================================================================================================================

#ifndef CPPUTILS_CONTAINERS_INCLUDED
#define CPPUTILS_CONTAINERS_INCLUDED


#include "Essential.hpp"

#include "Span.hpp"

#include <iterator>   // advance, begin, end
#include <algorithm>  // find_if
#include <array>
#include <vector>


namespace own {


//----------------------------------------------------------------------------------------------------------------------
// range-like helpers

template< typename Container, typename Value >
typename Container::iterator find( Container & cont, const Value & val )
{
	return std::find( std::begin(cont), std::end(cont), val );
}
template< typename Container, typename Value >
typename Container::const_iterator find( const Container & cont, const Value & val )
{
	return std::find( std::begin(cont), std::end(cont), val );
}
template< typename Container, typename Predicate >
typename Container::iterator find_if( Container & cont, Predicate pred )
{
	return std::find_if( std::begin(cont), std::end(cont), pred );
}
template< typename Container, typename Predicate >
typename Container::const_iterator find_if( const Container & cont, Predicate pred )
{
	return std::find_if( std::begin(cont), std::end(cont), pred );
}
template< typename Container, typename Predicate >
typename Container::iterator find_if_not( Container & cont, Predicate pred )
{
	return std::find_if_not( std::begin(cont), std::end(cont), pred );
}
template< typename Container, typename Predicate >
typename Container::const_iterator find_if_not( const Container & cont, Predicate pred )
{
	return std::find_if_not( std::begin(cont), std::end(cont), pred );
}

// How is this still not in C++ even in 2020?
template< typename Container, typename Value >
bool contains( const Container & cont, const Value & val )
{
	return std::find( std::begin(cont), std::end(cont), val ) != std::end(cont);
}
template< typename Container, typename Predicate >
bool contains_if( const Container & cont, Predicate pred )
{
	return std::find_if( std::begin(cont), std::end(cont), pred ) != std::end(cont);
}
template< typename Container, typename Predicate >
bool contains_if_not( const Container & cont, Predicate pred )
{
	return std::find_if_not( std::begin(cont), std::end(cont), pred ) != std::end(cont);
}

template< typename Container1, typename Container2 >
bool equal( const Container1 & cont1, const Container2 & cont2 )
{
	return std::equal( std::begin(cont1), std::end(cont1), std::begin(cont2), std::end(cont2) );
}

template< typename DstContainer, typename SrcContainer >
void append( DstContainer & dstCont, const SrcContainer & srcCont )
{
	dstCont.insert( std::end(dstCont), std::begin(srcCont), std::end(srcCont) );
}


//----------------------------------------------------------------------------------------------------------------------
// misc

template< typename Element, size_t size >
std::array< Element, size > to_std_array( const Element (& cArray) [size] )
{
	std::array< Element, size > cppArray;
	std::copy( std::begin(cArray), std::end(cArray), cppArray.begin() );
	return cppArray;
}

// this allows us to write  make_array< 16 >( u"Blabla" )
// and it returns an array of 16 elements that start with the given constant and whose type is automatically deduced
template< size_t dstSize, typename Element, size_t srcSize >
std::array< Element, dstSize > make_std_array( const Element (& cArray) [srcSize] )
{
	static_assert( srcSize <= dstSize, "array is too small" );
	std::array< Element, dstSize > cppArray{};
	std::copy( cArray, cArray + srcSize, cppArray.begin() );
	return cppArray;
}

template< typename Element, REQUIRES( std::is_trivial<Element>::value ) >
size_t sizeofVector( const std::vector< Element > & vec ) noexcept
{
	return vec.size() * sizeof( Element );
}

template< typename Element, size_t Size, REQUIRES( std::is_trivial<Element>::value ) >
size_t sizeofArray( const std::array< Element, Size > & arr ) noexcept
{
	return arr.size() * sizeof( Element );
}


//----------------------------------------------------------------------------------------------------------------------


} // namespace own


#endif // CPPUTILS_CONTAINER_INCLUDED
