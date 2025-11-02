//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: declarations that should be built-in features of the languge, but aren't
//======================================================================================================================

#ifndef CPPUTILS_ESSENTIAL_INCLUDED
#define CPPUTILS_ESSENTIAL_INCLUDED


#include <cstddef>
#include <cstdint>
#include <climits>
#include <utility>      // move, forward, swap

using uint = unsigned int;
using ushort = unsigned short;
using ulong = unsigned long;
using ullong = unsigned long long;
//using byte = uint8_t;

using std::move;
using std::forward;


// These things are so commonly used that they are moved from LangUtils here.
namespace fut {


// C++17
template< typename Range >
constexpr size_t size( const Range & range ) noexcept
{
	return range.end() - range.begin();
}
// In C++11 std::begin and std::end is not constexpr so we have to make a specialization
template< typename Element, size_t size_ >
constexpr size_t size( const Element (&) [size_] ) noexcept
{
	return size_;
}

// C++17
template< typename Range >
constexpr auto data( Range & range ) noexcept -> decltype( range.data() )
{
    return range.data();
}
template< typename Range >
constexpr auto data( const Range & range ) noexcept -> decltype( range.data() )
{
    return range.data();
}
template< typename Element, size_t size_ >
constexpr Element * data( Element (& array) [size_] ) noexcept
{
    return array;
}


} // namespace fut


namespace own {



} // namespace own


// Standard-dependent or compiler-dependent way to silence unused variable warnings.
/* If the project is limited to C++11 this is to be used instead of [[maybe_unused]]. This works in gcc and clang.
 * Users of other compilers will have to deal with occasional warnings or disable them with command line argument. */
#if __cplusplus >= 201703L
	[[maybe_unused]]
#else
	#ifdef __GNUC__
		#define MAYBE_UNUSED __attribute__((unused))
	#else
		#define MAYBE_UNUSED
	#endif
#endif

// Since C++17 there is no 'if constexpr', but if the expression is really constant,
// any reasonable compiler will be able to evaluate it at compile time and eliminate the dead branches.
#if __cplusplus >= 201703L
	#define IF_CONSTEXPR if constexpr
#else
	#define IF_CONSTEXPR if
#endif


#endif // CPPUTILS_ESSENTIAL_INCLUDED
