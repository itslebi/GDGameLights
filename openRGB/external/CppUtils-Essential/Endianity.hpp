//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: types and functions dealing with endianity
//======================================================================================================================

#ifndef CPPUTILS_ENDIANITY_INCLUDED
#define CPPUTILS_ENDIANITY_INCLUDED


#include "Essential.hpp"

#include "TypeTraits.hpp"  // REQUIRES
#include "MemAccessUtils.hpp"


namespace own {


enum class Endianity
{
	Little,
	Big
};


//======================================================================================================================
// endianity detection

// Taken from https://github.com/Tencent/rapidjson/blob/a95e013b97ca6523f32da23f5095fcc9dd6067e5/include/rapidjson/rapidjson.h#L245-L282

#define CPPUTILS_LITTLE_ENDIAN 0
#define CPPUTILS_BIG_ENDIAN 1

// Detect with GCC 4.6's macro
#ifdef __BYTE_ORDER__
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_LITTLE_ENDIAN
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_BIG_ENDIAN
	#else
		#error "Unknown CPU endianity"
	#endif
// Detect with GLIBC's endian.h
#elif defined(__GLIBC__)
	#include <endian.h>
	#if (__BYTE_ORDER == __LITTLE_ENDIAN)
		#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_LITTLE_ENDIAN
	#elif (__BYTE_ORDER == __BIG_ENDIAN)
		#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_BIG_ENDIAN
	#else
		#error "Unknown CPU endianity"
	#endif
// Detect with _LITTLE_ENDIAN and _BIG_ENDIAN macro
#elif defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
	#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
	#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_BIG_ENDIAN
// Detect with architecture macros
#elif defined(__sparc) || defined(__sparc__) || defined(_POWER) || defined(__powerpc__) || defined(__ppc__) || defined(__hpux) || defined(__hppa) || defined(_MIPSEB) || defined(_POWER) || defined(__s390__)
	#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_BIG_ENDIAN
#elif defined(__i386__) || defined(__alpha__) || defined(__ia64) || defined(__ia64__) || defined(_M_IX86) || defined(_M_IA64) || defined(_M_ALPHA) || defined(__amd64) || defined(__amd64__) || defined(_M_AMD64) || defined(__x86_64) || defined(__x86_64__) || defined(_M_X64) || defined(__bfin__)
	#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_LITTLE_ENDIAN
#elif defined(_MSC_VER) && (defined(_M_ARM) || defined(_M_ARM64))
	#define CPPUTILS_THIS_CPU_ENDIANITY CPPUTILS_LITTLE_ENDIAN
#else
	#error "Unknown CPU endianity"
#endif

constexpr Endianity c_thisCpuEndianity
	= (CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_LITTLE_ENDIAN) ? Endianity::Little : Endianity::Big;


//======================================================================================================================
// integer conversion - private implementation details

namespace impl {

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) != 1 ) >
inline void writeConvertedLittleEndian( uint8_t * const bufferPos, Type native ) noexcept
{
	using IntType = typename int_type< Type >::type;
	IntType nativeInt = IntType( native );

	// indexed variant is more optimizable than variant with pointers https://godbolt.org/z/McT3Yb
	size_t offset = 0;
	while (offset < sizeof( nativeInt )) {
		bufferPos[ offset ] = uint8_t( nativeInt & 0xFF );
		nativeInt >>= 8;
		++offset;
	}
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) != 1 ) >
inline void writeConvertedBigEndian( uint8_t * const bufferPos, Type native ) noexcept
{
	using IntType = typename int_type< Type >::type;
	IntType nativeInt = IntType( native );

	// indexed variant is more optimizable than variant with pointers https://godbolt.org/z/McT3Yb
	size_t offset = sizeof( nativeInt );
	while (offset > 0) {  // can't use traditional for-loop approach, because index is unsigned
		--offset;         // so we can't check if it's < 0
		bufferPos[ offset ] = uint8_t( nativeInt & 0xFF );
		nativeInt >>= 8;
	}
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) != 1 ) >
inline Type readConvertedLittleEndian( const uint8_t * const bufferPos ) noexcept
{
	using IntType = typename int_type< Type >::type;
	IntType nativeInt = 0;

	// indexed variant is more optimizable than variant with pointers https://godbolt.org/z/McT3Yb
	size_t offset = sizeof( IntType );
	while (offset > 0) {  // can't use traditional for-loop approach, because index is unsigned
		--offset;         // so we can't check if it's < 0
		nativeInt <<= 8;
		nativeInt = IntType( nativeInt | bufferPos[ offset ] );
	}

	return Type( nativeInt );
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) != 1 ) >
inline Type readConvertedBigEndian( const uint8_t * const bufferPos ) noexcept
{
	using IntType = typename int_type< Type >::type;
	IntType nativeInt = 0;

	// indexed variant is more optimizable than variant with pointers https://godbolt.org/z/McT3Yb
	size_t pos = 0;
	while (pos < sizeof( IntType )) {
		nativeInt <<= 8;
		nativeInt = IntType( nativeInt | bufferPos[ pos ] );
		++pos;
	}

	return Type( nativeInt );
}

// It is useful to provide these functions for all integer sizes, for possible generic code that doesn't know the type.
// But if the Type is single-byte, shift by 8 is undefined, so we need to have separate overloads.

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) == 1 ) >
inline void writeConvertedLittleEndian( uint8_t * const bufferPos, Type native ) noexcept
{
	*bufferPos = uint8_t( native );
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) == 1 ) >
inline void writeConvertedBigEndian( uint8_t * const bufferPos, Type native ) noexcept
{
	*bufferPos = uint8_t( native );
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) == 1 ) >
inline Type readConvertedLittleEndian( const uint8_t * const bufferPos ) noexcept
{
	return Type( *bufferPos );
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value && sizeof(Type) == 1 ) >
inline Type readConvertedBigEndian( const uint8_t * const bufferPos ) noexcept
{
	return Type( *bufferPos );
}

// Direct write and read optimizations in case the target endianity equals to the current endianity.
// The best compilers can do this automatically, but not under all circumstances. This way we are sure.

template< typename Type, REQUIRES( is_int_or_enum<Type>::value ) >
inline void writeIntDirectly_unaligned( uint8_t * bufferPos, Type native ) noexcept
{
	Type toWrite = native;
	std::copy(
		reinterpret_cast< const uint8_t * >( &toWrite ),
		reinterpret_cast< const uint8_t * >( &toWrite ) + sizeof( toWrite ),
		bufferPos
	);
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value ) >
inline void writeIntDirectly_aligned( uint8_t * bufferPos, Type native ) noexcept
{
	*reinterpret_cast< Type * >( bufferPos ) = native;
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value ) >
inline Type readIntDirectly_unaligned( const uint8_t * bufferPos ) noexcept
{
	Type toRead;
	std::copy( bufferPos, bufferPos + sizeof( toRead ), reinterpret_cast< uint8_t * >( &toRead ) );
	return toRead;
}

template< typename Type, REQUIRES( is_int_or_enum<Type>::value ) >
inline Type readIntDirectly_aligned( const uint8_t * bufferPos ) noexcept
{
	return *reinterpret_cast< const Type * >( bufferPos );
}


} // namespace impl


//======================================================================================================================
// integer conversion - public API
// NOTE: The following functions perform no boundary checking, caller must ensure there is enough space in the buffer
//       to do the read or write. Preffer using BinaryStream.h whenever possible.

/// Converts an arbitrary integral number from native format to little endian and writes it into the buffer.
template< typename Type >
inline void writeLittleEndian( uint8_t * bufferPos, Type native ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_LITTLE_ENDIAN
	impl::writeIntDirectly_unaligned( bufferPos, native );
 #else
	impl::writeConvertedLittleEndian( bufferPos, native );
 #endif
}

/// Converts an arbitrary integral number from native format to little endian and writes it into the buffer.
/** Variant manually optimized for cases where the \p bufferPos is divisible by sizeof(Type) */
template< typename Type >
inline void writeLittleEndian_aligned( uint8_t * bufferPos, Type native ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_LITTLE_ENDIAN
	impl::writeIntDirectly_aligned( bufferPos, native );
 #else
	impl::writeConvertedLittleEndian( bufferPos, native );
 #endif
}

/// Converts an arbitrary integral number from native format to big endian and writes it into the buffer.
template< typename Type >
inline void writeBigEndian( uint8_t * bufferPos, Type native ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_BIG_ENDIAN
	impl::writeIntDirectly_unaligned( bufferPos, native );
 #else
	impl::writeConvertedBigEndian( bufferPos, native );
 #endif
}

/// Converts an arbitrary integral number from native format to big endian and writes it into the buffer.
template< typename Type >
inline void writeBigEndian_aligned( uint8_t * bufferPos, Type native ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_BIG_ENDIAN
	impl::writeIntDirectly_aligned( bufferPos, native );
 #else
	impl::writeConvertedBigEndian( bufferPos, native );
 #endif
}

/// Reads an arbitrary integral number from the buffer and converts it from little endian to native format.
template< typename Type >
inline Type readLittleEndian( const uint8_t * bufferPos ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_LITTLE_ENDIAN
	return impl::readIntDirectly_unaligned< Type >( bufferPos );
 #else
	return impl::readConvertedLittleEndian< Type >( bufferPos );
 #endif
}

/// Reads an arbitrary integral number from the buffer and converts it from little endian to native format.
template< typename Type >
inline Type readLittleEndian_aligned( const uint8_t * bufferPos ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_LITTLE_ENDIAN
	return impl::readIntDirectly_aligned< Type >( bufferPos );
 #else
	return impl::readConvertedLittleEndian< Type >( bufferPos );
 #endif
}

/// Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
template< typename Type >
inline Type readBigEndian( const uint8_t * bufferPos ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_BIG_ENDIAN
	return impl::readIntDirectly_unaligned< Type >( bufferPos );
 #else
	return impl::readConvertedBigEndian< Type >( bufferPos );
 #endif
}

/// Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
template< typename Type >
inline Type readBigEndian_aligned( const uint8_t * bufferPos ) noexcept
{
 #if CPPUTILS_THIS_CPU_ENDIANITY == CPPUTILS_BIG_ENDIAN
	return impl::readIntDirectly_aligned< Type >( bufferPos );
 #else
	return impl::readConvertedBigEndian< Type >( bufferPos );
 #endif
}


//======================================================================================================================


} // namespace own


#endif // CPPUTILS_ENDIANITY_INCLUDED
