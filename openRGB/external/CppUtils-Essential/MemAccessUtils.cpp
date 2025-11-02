//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: utils for fast memory access (filling, copying, comparing, ...)
//======================================================================================================================

#include "MemAccessUtils.hpp"

#include <cstring>


namespace own {

void fillBytes_large( uint8_t * dst, size_t count, uint8_t value ) noexcept
{
	std::memset( dst, value, count );
}

void zeroBytes_large( uint8_t * dst, size_t count ) noexcept
{
	std::memset( dst, 0, count );
}

void copyBytes_large( const uint8_t * RESTRICT_PTR src, uint8_t * RESTRICT_PTR dst, size_t count ) noexcept
{
	std::memcpy( dst, src, count );
}

void copyBytes_large_overlapping( const uint8_t * src, uint8_t * dst, size_t count ) noexcept
{
	std::memmove( dst, src, count );
}

} // namespace own
