//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: classes for binary serialization into binary buffers via operators << and >>
//======================================================================================================================

#include "BinaryStream.hpp"

#include "LangUtils.hpp"  // unconst
#include "CriticalError.hpp"

#include <string>
#include <algorithm>  // find


namespace own {


//======================================================================================================================
// error handling

[[noreturn]] void BinaryOutputStream::writeError( const char * typeDesc, size_t typeSize )
{
	CRITICAL_ERROR(
		"Attempted to write %s of size %zu past the buffer end, remaining size: %zu", typeDesc, typeSize, remaining()
	);
}

[[noreturn]] void BinaryOutputStream::writeArrayError( const char * elemDesc, size_t totalSize )
{
	std::string typeDesc = std::string( elemDesc ) + " array";
	writeError( typeDesc.c_str(), totalSize );
}


//======================================================================================================================
// more complex writing operations



//======================================================================================================================
// more complex reading operations

bool BinaryInputStream::readString( std::string & str, size_t size ) noexcept
{
	if (const size_t readSize = checkRead( size ))
	{
		str.resize( readSize );
		copyBytes( _curPos, unconst( reinterpret_cast< const uint8_t * >( str.data() ) ), readSize );
		_curPos += readSize;
	}
	return !_failed;
}

bool BinaryInputStream::readString0( std::string & str ) noexcept
{
	if (!_failed)
	{
		const uint8_t * strEndPos = std::find( _curPos, _endPos, '\0' );
		if (strEndPos != _endPos)
		{
			const size_t strSize = size_t( strEndPos - _curPos );
			str.resize( strSize );
			copyBytes( _curPos, unconst( reinterpret_cast< const uint8_t * >( str.data() ) ), strSize );
			_curPos += strSize + 1;
		}
		else
		{
			_failed = true;
		}
	}
	return !_failed;
}


//======================================================================================================================


} // namespace own
