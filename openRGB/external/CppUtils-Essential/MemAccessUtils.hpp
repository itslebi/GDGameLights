//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: utils for fast memory access (filling, copying, comparing, ...)
//======================================================================================================================

#ifndef CPPUTILS_MEMACCESS_INCLUDED
#define CPPUTILS_MEMACCESS_INCLUDED


#include "Essential.hpp"

//#include "TypeTraits.hpp"  // REQUIRES

#include <algorithm>


// This should work for most of the compilers, including MSVC. Others will have to find their alternative.
#define RESTRICT_PTR __restrict

#if __cplusplus >= 202002L  // C++20
	#include <memory>
	#define ASSUME_ALIGNED( variable, blockSize ) std::assume_aligned< blockSize >( variable )
#elif defined(__GNUC__)  // gcc or clang
	#define ASSUME_ALIGNED( variable, blockSize ) reinterpret_cast< decltype(variable) >( __builtin_assume_aligned( variable, blockSize ) )
#else  // in worst case we'll just lose some optimizations
	#define ASSUME_ALIGNED( variable, blockSize )
#endif


namespace own {


//======================================================================================================================
// initialization

// variants of byte array initialization procedures optimized for different use-cases

/// Zeroes all bytes in a memory range starting at \p dst and ending at \p dst + \p count.
/** General purpose variant suitable for most use-cases. */
inline void zeroBytes( uint8_t * dst, size_t count ) noexcept
{
	std::fill( dst, dst + count, 0 );
}

/// Zeroes all bytes in a memory range starting at \p dst and ending at \p dst + \p count.
/** Variant optimized for large chunks for the cost of a function call. */
void zeroBytes_large( uint8_t * dst, size_t count ) noexcept;

/// Zeroes all bytes in a memory range starting at \p dst and ending at \p dst + \p count to.
/** Variant optimized for small, fixed size memory ranges that are aligned to a multiple of their size. */
template< size_t count > inline void zeroBytes_aligned( uint8_t * dst ) noexcept
{
	std::fill( dst, dst + count, 0 );  // fallback for unknown size
}
// The compiler can probably generate these optimizations automatically under the right circumstances,
// but this way we have more control.
template<> inline void zeroBytes_aligned< 2 >( uint8_t * dst ) noexcept
{
	*reinterpret_cast< uint16_t * >( dst ) = 0;
}
template<> inline void zeroBytes_aligned< 4 >( uint8_t * dst ) noexcept
{
	*reinterpret_cast< uint32_t * >( dst ) = 0;
}
template<> inline void zeroBytes_aligned< 8 >( uint8_t * dst ) noexcept
{
	*reinterpret_cast< uint64_t * >( dst ) = 0;
}
template<> inline void zeroBytes_aligned< 16 >( uint8_t * dst ) noexcept
{
	reinterpret_cast< uint64_t * >( dst )[0] = 0;
	reinterpret_cast< uint64_t * >( dst )[1] = 0;
}


//======================================================================================================================
// copying

// variants of byte-copying procedures optimized for different use-cases
// https://stackoverflow.com/questions/4707012/is-it-better-to-use-stdmemcpy-or-stdcopy-in-terms-to-performance

/// Copies \p count bytes from memory range starting at \p src to range starting at \p dst.
/** General purpose variant suitable for most use-cases. Requires non-overlapping memory ranges. */
inline void copyBytes( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst, size_t count ) noexcept
{
	std::copy( src, src + count, dst );
}

/// Copies \p count bytes from memory range starting at \p src to range starting at \p dst.
/** Variant for memory ranges that may overlap. */
inline void copyBytes_overlapping( const uint8_t * src, uint8_t * dst, size_t count ) noexcept
{
	std::copy( src, src + count, dst );
}

/// Copies \p count bytes from memory range starting at \p src to range starting at \p dst.
/** Variant optimized for large chunks for the cost of a function call. Memory ranges must not overlap. */
void copyBytes_large( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst, size_t count ) noexcept;

/// Copies \p count bytes from memory range starting at \p src to range starting at \p dst.
/** Variant optimized for large chunks for the cost of a function call. Memory ranges can overlap. */
void copyBytes_large_overlapping( const uint8_t * src, uint8_t * dst, size_t count ) noexcept;

/// Copies \p count bytes from memory range starting at \p src to range starting at \p dst.
/** Variant optimized for small, fixed size memory ranges that are aligned to a multiple of their size.
  * The ranges must not overlap. */
template< size_t count > inline void copyBytes_aligned( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst ) noexcept
{
	std::copy( src, src + count, dst );  // fallback for unknown size
}
// The compiler can probably generate these optimizations automatically under the right circumstances,
// but this way we have more control.
template<> inline void copyBytes_aligned< 2 >( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst ) noexcept
{
	*reinterpret_cast< uint16_t * >( dst ) = *reinterpret_cast< const uint16_t * >( src );
}
template<> inline void copyBytes_aligned< 4 >( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst ) noexcept
{
	*reinterpret_cast< uint32_t * >( dst ) = *reinterpret_cast< const uint32_t * >( src );
}
template<> inline void copyBytes_aligned< 8 >( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst ) noexcept
{
	*reinterpret_cast< uint64_t * >( dst ) = *reinterpret_cast< const uint64_t * >( src );
}
template<> inline void copyBytes_aligned< 16 >( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst ) noexcept
{
	reinterpret_cast< uint64_t * >( dst )[0] = reinterpret_cast< const uint64_t * >( src )[0];
	reinterpret_cast< uint64_t * >( dst )[1] = reinterpret_cast< const uint64_t * >( src )[1];
}


//======================================================================================================================
// comparison

inline int compareBytes( const uint8_t * a1, const uint8_t * a2, size_t count ) noexcept
{
	return std::lexicographical_compare( a1, a1 + count, a2, a2 + count );
}


//======================================================================================================================
// reading/writing fundamental types and POD structures

// TODO: is this needed?
/*
template< typename Type, REQUIRES( std::is_trivial< Type >::value && std::is_standard_layout< Type >::value ) >
void writeAsBytes( uint8_t * dst, const Type & obj )
{
	const auto srcMemBlockBegin = reinterpret_cast< const uint8_t * >( obj );
	const auto srcMemBlockEnd = reinterpret_cast< const uint8_t * >( obj + sizeof(obj) );
	const auto dstMemBlockBegin = dst;
	std::copy( srcMemBlockBegin, srcMemBlockEnd, dstMemBlockBegin );
}

template< typename Type, REQUIRES( std::is_trivial< Type >::value && std::is_standard_layout< Type >::value ) >
void readAsBytes( const uint8_t * src, Type & obj )
{
	const auto srcMemBlockBegin = src;
	const auto srcMemBlockEnd = src + sizeof(obj);
	const auto dstMemBlockBegin = reinterpret_cast< const uint8_t * >( obj );
	std::copy( srcMemBlockBegin, srcMemBlockEnd, dstMemBlockBegin );
}

template<
	typename SrcType, REQUIRES( std::is_trivial< SrcType >::value && std::is_standard_layout< SrcType >::value ),
	typename DstType, REQUIRES( std::is_trivial< DstType >::value && std::is_standard_layout< DstType >::value )
>
void copyAsBytes( const SrcType * firstSrcObj, DstType * firstDstObj, size_t numOfObjects )
{
	static_assert( sizeof(SrcType) == sizeof(DstType), "source object and destination object cannot have different size" );
	const auto srcMemBlockBegin = reinterpret_cast< const uint8_t * >( firstSrcObj );
	const auto srcMemBlockEnd = reinterpret_cast< const uint8_t * >( firstSrcObj + numOfObjects );
	const auto dstMemBlockBegin = reinterpret_cast< uint8_t * >( firstDstObj );
	std::copy( srcMemBlockBegin, srcMemBlockEnd, dstMemBlockBegin );
}
*/

//======================================================================================================================


} // namespace own


#endif // CPPUTILS_MEMACCESS_INCLUDED
