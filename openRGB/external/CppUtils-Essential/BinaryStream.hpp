//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: classes for binary serialization into binary buffers via operators << and >>
//======================================================================================================================

#ifndef CPPUTILS_BINARY_STREAM_INCLUDED
#define CPPUTILS_BINARY_STREAM_INCLUDED


#include "Essential.hpp"

#include "TypeTraits.hpp"  // is_int_or_enum
#include "Span.hpp"
#include "Endianity.hpp"
#include "MemAccessUtils.hpp"
#include "SafetyChecks.hpp"

#include <string>
#include <vector>  // toByteVector, fromByteVector
#include <typeinfo>


namespace own {


class BinaryOutputStreamLE;
class BinaryOutputStreamBE;
class BinaryInputStreamLE;
class BinaryInputStreamBE;


//======================================================================================================================
/// Binary buffer output stream allowing serialization via operator<< .
/** This is a binary alternative of std::ostringstream. First you allocate a buffer, then you construct this stream
  * object, and then you write all the data you need using operator<< or named methods. */

class BinaryOutputStream
{

 protected:

	uint8_t * _begPos;  ///< position of the beginning of the buffer
	uint8_t * _curPos;  ///< current position in the buffer
	uint8_t * _endPos;  ///< position of the end of the buffer

 public:

	BinaryOutputStream( const BinaryOutputStream & ) = delete;
	BinaryOutputStream( BinaryOutputStream && ) = default;
	BinaryOutputStream & operator=( const BinaryOutputStream & other ) = delete;
	BinaryOutputStream & operator=( BinaryOutputStream && other ) = default;

	/// Initializes a binary output stream operating over any byte container with continuous memory.
	/** WARNING: The class takes non-owning reference to a buffer. You are responsible for making sure the buffer exists
	  * at least as long as this object and for allocating the storage big enough for all write operations to fit in. */
	BinaryOutputStream( byte_span buffer ) noexcept
	{
		reset( buffer );
	}

	void reset( byte_span buffer ) noexcept
	{
		_begPos = buffer.data();
		_curPos = buffer.data();
		_endPos = buffer.data() + buffer.size();
	}

	//-- atomic elements -----------------------------------------------------------------------------------------------

	/// Writes a single byte into the buffer.
	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >  // same code for char, uint8_t, std::byte, ...
	void put( Byte b )
	{
		checkWrite< Byte >();
		*_curPos = uint8_t( b );
		_curPos++;
	}

	/// Writes bytes of an object as they are in memory, without any byte conversion or deep serialization.
	template< typename Type >
	void writeRaw( const Type & obj )
	{
		const size_t writeSize = checkWrite< Type >();
		copyBytes( reinterpret_cast< const uint8_t * >( &obj ), _curPos, writeSize );
		_curPos += writeSize;
	}

	//-- integers ------------------------------------------------------------------------------------------------------

	/// Converts an arbitrary integral number from native format to little endian and writes it into the buffer.
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	void writeLittleEndian( Int native )
	{
		const size_t writeSize = checkWrite< Int >();
		own::writeLittleEndian( _curPos, native );
		_curPos += writeSize;
	}

	/// Converts an arbitrary integral number from native format to big endian and writes it into the buffer.
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	void writeBigEndian( Int native )
	{
		const size_t writeSize = checkWrite< Int >();
		own::writeBigEndian( _curPos, native );
		_curPos += writeSize;
	}

	inline BinaryOutputStreamLE & getLittleEndianStream();
	inline BinaryOutputStreamBE & getBigEndianStream();

	//-- arrays and strings --------------------------------------------------------------------------------------------

	/// Writes specified number of bytes from a continuous memory storage to the buffer.
	template< typename Range, REQUIRES( is_range_of_byte_alikes<Range>::value && has_contiguous_data<Range>::value ) >  // same code for char, uint8_t, std::byte, ...
	void writeBytes( const Range & bytes )
	{
		const size_t writeSize = checkWrite( "span of bytes", fut::size( bytes ) );
		copyBytes( reinterpret_cast< const uint8_t * >( fut::data( bytes ) ), _curPos, writeSize );
		_curPos += writeSize;
	}

	/// Writes bytes of an arbitrary array as they are in memory, without any byte conversion or deep serialization.
	template< typename Range, REQUIRES( is_trivial_range<Range>::value && has_contiguous_data<Range>::value ) >
	void writeTrivialArray( const Range & array )
	{
		using Element = typename range_element< Range >::type;
		const size_t writeSize = checkWrite< Element >( fut::size( array ) );
		copyBytes( reinterpret_cast< const uint8_t * >( fut::data( array ) ), _curPos, writeSize );
		_curPos += writeSize;
	}

	/// Writes a string WITHOUT its null terminator to the buffer.
	void writeString( const std::string & str )
	{
		const size_t writeSize = checkWrite( "string", str.size() );
		copyBytes( reinterpret_cast< const uint8_t * >( str.data() ), _curPos, writeSize );
		_curPos += writeSize;
	}

	/// Writes a string WITH its null terminator to the buffer.
	void writeString0( const std::string & str )
	{
		const size_t writeSize = checkWrite( "string", str.size() + 1 );
		copyBytes( reinterpret_cast< const uint8_t * >( str.data() ), _curPos, writeSize );
		_curPos += writeSize;
	}

	/// Writes specified number of zero bytes to the buffer.
	void writeZeroBytes( size_t numZeroBytes )
	{
		const size_t writeSize = checkWrite( "zero bytes", numZeroBytes );
		zeroBytes( _curPos, writeSize );
		_curPos += writeSize;
	}

	//-- convenience operators -----------------------------------------------------------------------------------------

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >  // same code for char, uint8_t, std::byte, ...
	BinaryOutputStream & operator<<( Byte b )
	{
		put( b );
		return *this;
	}

	/// Accepts C array, std::array, std::vector, std::basic_string, own::span or any custom range-based container
	/// whose elements are char, unsigned char, char8_t, uint8_t, std::byte, ...
	template< typename Range, REQUIRES( is_range_of_byte_alikes<Range>::value && has_contiguous_data<Range>::value ) >
	BinaryOutputStream & operator<<( const Range & bytes )
	{
		writeBytes( bytes );
		return *this;
	}

	//-- position manipulation -----------------------------------------------------------------------------------------

	/// Returns how many bytes is the current stream position from the beginning of the buffer.
	size_t offset() const noexcept
	{
		return size_t( _curPos - _begPos );
	}

	/// Returns how many bytes is the current stream position from the end of the buffer.
	size_t remaining() const noexcept
	{
		return size_t( _endPos - _curPos );
	}

	/// Returns true when the position in the stream reaches the end.
	bool isAtEnd() const noexcept
	{
		return _curPos >= _endPos;
	}

 private:

	template< typename Type >
	inline size_t checkWrite()
	{
		constexpr size_t numBytes = sizeof( Type );
	 #ifdef SAFETY_CHECKS
		if (_curPos + numBytes > _endPos)
		{
			writeError( typeid( Type ).name(), numBytes );
		}
	 #endif
		return numBytes;
	}

	template< typename Element >
	inline size_t checkWrite( size_t elemCount )
	{
		const size_t numBytes = elemCount * sizeof( Element );
	 #ifdef SAFETY_CHECKS
		if (_curPos + numBytes > _endPos)
		{
			writeArrayError( typeid( Element ).name(), numBytes );
		}
	 #endif
		return numBytes;
	}

	inline size_t checkWrite( MAYBE_UNUSED const char * typeDesc, size_t numBytes )
	{
	 #ifdef SAFETY_CHECKS
		if (_curPos + numBytes > _endPos)
		{
			writeError( typeDesc, numBytes );
		}
	 #endif
		return numBytes;
	}

	[[noreturn]] void writeError( const char * typeDesc, size_t typeSize );

	[[noreturn]] void writeArrayError( const char * elemDesc, size_t totalSize );

};


//======================================================================================================================
/// Binary buffer input stream allowing deserialization via operator>> .
/** This is a binary alternative of std::istringstream. First you allocate a buffer, then you construct this stream
  * object, and then you read the data you expect using operator>> or named methods.
  * If an attempt to read past the end of the input buffer is made, the stream sets its internal error flag
  * and returns default values for any further read operations. The error flag can be checked with failed(). */

class BinaryInputStream
{

 protected:

	const uint8_t * _begPos;  ///< position of the beginning of the buffer
	const uint8_t * _curPos;  ///< current position in the buffer
	const uint8_t * _endPos;  ///< position of the end of the buffer
	bool _failed;  ///< the end was reached while attemting to read from the buffer

 public:

	BinaryInputStream( const BinaryInputStream & ) = delete;
	BinaryInputStream( BinaryInputStream && ) = default;
	BinaryInputStream & operator=( const BinaryInputStream & other ) = delete;
	BinaryInputStream & operator=( BinaryInputStream && other ) = default;

	/// Initializes a binary input stream operating over any byte container with continuous memory.
	/** WARNING: The class takes non-owning reference to a buffer.
	  * You are responsible for making sure the buffer exists at least as long as this object. */
	BinaryInputStream( const_byte_span buffer ) noexcept
	{
		reset( buffer );
	}

	void reset( const_byte_span buffer ) noexcept
	{
		_begPos = buffer.data();
		_curPos = buffer.data();
		_endPos = buffer.data() + buffer.size();
		_failed = false;
	}

	//-- atomic elements -----------------------------------------------------------------------------------------------

	/// Reads a single byte from the buffer.
	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >  // same code for char, uint8_t, std::byte, ...
	Byte get() noexcept
	{
		if (checkRead< Byte >())
			return Byte( *(_curPos++) );
		else
			return Byte( 0 );
	}

	uint8_t getByte() noexcept
	{
		return get< uint8_t >();
	}
	char getChar() noexcept
	{
		return get< char >();
	}

	/// Reads bytes of an object as they are in memory, without any byte conversion or deep deserialization.
	/** (output parameter variant) */
	template< typename Type >
	bool readRaw( Type & obj ) noexcept
	{
		if (const size_t readSize = checkRead< Type >())
		{
			copyBytes( _curPos, reinterpret_cast< uint8_t * >( &obj ), readSize );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads bytes of an object as they are, without any byte conversion or deep deserialization.
	/** (return value variant) */
	template< typename Type >
	Type readRaw() noexcept
	{
		Type obj;
		readRaw( obj );
		return obj;
	}

	//-- integers ------------------------------------------------------------------------------------------------------

	/// Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
	/** (output parameter variant) */
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	bool readLittleEndian( Int & native ) noexcept
	{
		if (const size_t readSize = checkRead< Int >())
		{
			native = own::readLittleEndian< Int >( _curPos );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads an arbitrary integral number from the buffer and converts it from little endian to native format.
	/** (return value variant) */
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	Int readLittleEndian() noexcept
	{
		auto native = Int(0);
		if (const size_t readSize = checkRead< Int >())
		{
			native = own::readLittleEndian< Int >( _curPos );
			_curPos += readSize;
		}
		return native;
	}

	/// Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
	/** (output parameter variant) */
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	bool readBigEndian( Int & native ) noexcept
	{
		if (const size_t readSize = checkRead< Int >())
		{
			native = own::readBigEndian< Int >( _curPos );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
	/** (return value variant) */
	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	Int readBigEndian() noexcept
	{
		auto native = Int(0);
		if (const size_t readSize = checkRead< Int >())
		{
			native = own::readBigEndian< Int >( _curPos );
			_curPos += readSize;
		}
		return native;
	}

	inline BinaryInputStreamLE & getLittleEndianStream();
	inline BinaryInputStreamBE & getBigEndianStream();

	//-- arrays and strings --------------------------------------------------------------------------------------------

	// We must have overload for both generic container and span,
	// because temporary rvalue of span will not fit into the non-const range reference
	// and because containers won't convert to the templated span implicitly.

	/// Reads a range of bytes from the buffer into a given pre-allocated container.
	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	bool readBytes( span< Byte > bytes ) noexcept
	{
		if (const size_t readSize = checkRead( bytes.size() ))
		{
			copyBytes( _curPos, reinterpret_cast< uint8_t * >( bytes.data() ), readSize );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads a range of bytes from the buffer into a given pre-allocated container.
	template< typename Cont, REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value ) >
	bool readBytes( Cont & cont ) noexcept
	{
		return readBytes( make_span( cont ) );
	}

	/// Reads bytes of an arbitrary array as they are in memory, without any byte conversion or deep deserialization.
	template< typename Element, REQUIRES( std::is_trivial<Element>::value ) >
	bool readTrivialArray( span< Element > array ) noexcept
	{
		if (const size_t readSize = checkRead< Element >( array.size() ))
		{
			copyBytes( _curPos, reinterpret_cast< uint8_t * >( array.data() ), readSize );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads bytes of an arbitrary array as they are in memory, without any byte conversion or deep deserialization.
	template< typename Cont, REQUIRES( is_trivial_range<Cont>::value && has_contiguous_data<Cont>::value ) >
	bool readTrivialArray( Cont & cont ) noexcept
	{
		return readTrivialArray( make_span( cont ) );
	}

	/// Reads bytes of an arbitrary array as they are in memory into a resizable container.
	/** The container is automatically resized before copying the data into it. */
	template< typename Cont,
		REQUIRES( is_trivial_range<Cont>::value && has_contiguous_data<Cont>::value && is_resizable<Cont>::value ) >
	bool readResizableTrivialArray( Cont & cont, size_t size ) noexcept
	{
		using Element = typename range_value< Cont >::type;
		if (const size_t readSize = checkRead< Element >( size ))
		{
			cont.resize( size );
			copyBytes( _curPos, reinterpret_cast< uint8_t * >( cont.data() ), readSize );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads a range of bytes from the buffer into a resizable container.
	/** The container is automatically resized before copying the bytes into it. */
	template< typename Cont,
		REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value && is_resizable<Cont>::value ) >
	bool readResizableByteArray( Cont & cont, size_t size ) noexcept
	{
		return readResizableTrivialArray( cont, size );
	}

	/// Reads the remaining data from the current position until the end of the buffer to a resizable container.
	/** The container is automatically resized before copying the bytes into it. */
	template< typename Cont,
		REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value && is_resizable<Cont>::value ) >
	bool readRemaining( Cont & cont ) noexcept
	{
		if (!_failed)
		{
			const size_t readSize = remaining();
			cont.resize( readSize );
			copyBytes( _curPos, reinterpret_cast< uint8_t * >( cont.data() ), readSize );
			_curPos += readSize;
		}
		return !_failed;
	}

	/// Reads a string of specified size from the buffer.
	/** (output parameter variant) */
	bool readString( std::string & str, size_t size ) noexcept;

	/// Reads a string of specified size from the buffer.
	/** (return value variant) */
	std::string readString( size_t size ) noexcept
	{
		std::string str;
		readString( str, size );
		return str;
	}

	/// Reads a string from the buffer until a null terminator is found.
	/** (output parameter variant) */
	bool readString0( std::string & str ) noexcept;

	/// Reads a string from the buffer until a null terminator is found.
	/** (return value variant) */
	std::string readString0() noexcept
	{
		std::string str;
		readString0( str );
		return str;
	}

	//-- convenience operators -----------------------------------------------------------------------------------------

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >  // same code for char, uint8_t, std::byte, ...
	BinaryInputStream & operator>>( Byte & b ) noexcept
	{
		b = get< Byte >();
		return *this;
	}

	/// Accepts span of char, unsigned char, char8_t, uint8_t, std::byte, ...
	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryInputStream & operator>>( span< Byte > bytes ) noexcept
	{
		readBytes( bytes );
		return *this;
	}

	/// Accepts C array, std::array or any custom range-based non-resizable container
	/// whose elements are char, unsigned char, char8_t, uint8_t, std::byte, ...
	template< typename Cont,
		REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value && !is_resizable<Cont>::value ) >
	BinaryInputStream & operator>>( Cont & cont )
	{
		return *this >> make_span( cont );
	}

	//-- position manipulation -----------------------------------------------------------------------------------------

	/// Returns how many bytes is the current stream position from the beginning of the buffer.
	size_t offset() const noexcept
	{
		return size_t( _curPos - _begPos );
	}

	/// Returns how many bytes is the current stream position from the end of the buffer.
	size_t remaining() const noexcept
	{
		return size_t( _endPos - _curPos );
	}

	/// Returns true when the position in the stream reaches the end.
	bool isAtEnd() const
	{
		return _curPos >= _endPos;
	}

	/// Moves over specified number of bytes without returning them to the user.
	bool skip( size_t numBytes ) noexcept
	{
		if (const size_t readSize = checkRead( numBytes ))
		{
			_curPos += numBytes;
		}
		return !_failed;
	}

	void rewind( size_t numBytes ) noexcept
	{
		_failed = _begPos + numBytes > _curPos;
		if (!_failed)
		{
			_curPos -= numBytes;
		}
	}

	void rewindToBeginning() noexcept
	{
		_curPos = _begPos;
		_failed = false;
	}

	//-- error handling ------------------------------------------------------------------------------------------------

	bool failed() const noexcept  { return _failed; }
	void setFailed() noexcept     { _failed = true; }
	void resetFailed() noexcept   { _failed = false; }

 private:

	template< typename Type >
	inline size_t checkRead() noexcept
	{
		return checkRead( sizeof( Type ) );
	}

	template< typename Element >
	inline size_t checkRead( size_t elemCount ) noexcept
	{
		return checkRead( elemCount * sizeof( Element ) );
	}

	// returns readSize, or 0 if we can't read that much
	inline size_t checkRead( size_t readSize ) noexcept
	{
		// the _failed flag can be true already from the previous call, in that case it will stay failed
		_failed |= _curPos + readSize > _endPos;
		return size_t( !_failed ) * readSize;
	}

};


//======================================================================================================================
// endianity convenience helpers

class BinaryOutputStreamLE : public BinaryOutputStream
{
 public:

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	void writeInt( Int native )
	{
		writeLittleEndian( native );
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value && sizeof(Int) != 1 ) >
	BinaryOutputStreamLE & operator<<( Int native )
	{
		writeLittleEndian( native );
		return *this;
	}

	// overrides with different return value

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryOutputStreamLE & operator<<( Byte b )
	{
		return static_cast< BinaryOutputStreamLE & >( BinaryOutputStream::operator<<( b ) );
	}

	template< typename Range, REQUIRES( is_range_of_byte_alikes<Range>::value && has_contiguous_data<Range>::value ) >
	BinaryOutputStreamLE & operator<<( const Range & bytes )
	{
		return static_cast< BinaryOutputStreamLE & >( BinaryOutputStream::operator<<( bytes ) );
	}
};

BinaryOutputStreamLE & BinaryOutputStream::getLittleEndianStream()
{
	return static_cast< BinaryOutputStreamLE & >( *this );
}

class BinaryOutputStreamBE : public BinaryOutputStream
{
 public:

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	void writeInt( Int native )
	{
		writeBigEndian( native );
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value && sizeof(Int) != 1 ) >
	BinaryOutputStreamBE & operator<<( Int native )
	{
		writeBigEndian( native );
		return *this;
	}

	// overrides with different return value

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryOutputStreamBE & operator<<( Byte b )
	{
		return static_cast< BinaryOutputStreamBE & >( BinaryOutputStream::operator<<( b ) );
	}

	template< typename Range, REQUIRES( is_range_of_byte_alikes<Range>::value && has_contiguous_data<Range>::value ) >
	BinaryOutputStreamBE & operator<<( const Range & bytes )
	{
		return static_cast< BinaryOutputStreamBE & >( BinaryOutputStream::operator<<( bytes ) );
	}
};

BinaryOutputStreamBE & BinaryOutputStream::getBigEndianStream()
{
	return static_cast< BinaryOutputStreamBE & >( *this );
}

class BinaryInputStreamLE : public BinaryInputStream
{
 public:

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	bool readInt( Int & native ) noexcept
	{
		return readLittleEndian( native );
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	Int readInt() noexcept
	{
		return readLittleEndian< Int >();
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value && sizeof(Int) != 1 ) >
	BinaryInputStream & operator>>( Int & native )
	{
		readLittleEndian( native );
		return *this;
	}

	// overrides with different return value

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryInputStreamLE & operator>>( Byte & b ) noexcept
	{
		return static_cast< BinaryInputStreamLE & >( BinaryInputStream::operator>>( b ) );
	}

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryInputStreamLE & operator>>( span< Byte > bytes ) noexcept
	{
		return static_cast< BinaryInputStreamLE & >( BinaryInputStream::operator>>( bytes ) );
	}

	template< typename Cont,
		REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value && !is_resizable<Cont>::value ) >
	BinaryInputStreamLE & operator>>( Cont & cont )
	{
		return static_cast< BinaryInputStreamLE & >( BinaryInputStream::operator>>( cont ) );
	}
};

BinaryInputStreamLE & BinaryInputStream::getLittleEndianStream()
{
	return static_cast< BinaryInputStreamLE & >( *this );
}

class BinaryInputStreamBE : public BinaryInputStream
{
 public:

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	bool readInt( Int & native ) noexcept
	{
		return readBigEndian( native );
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value ) >
	Int readInt() noexcept
	{
		return readBigEndian< Int >();
	}

	template< typename Int, REQUIRES( is_int_or_enum<Int>::value && sizeof(Int) != 1 ) >
	BinaryInputStreamBE & operator>>( Int & native )
	{
		readBigEndian( native );
		return *this;
	}

	// overrides with different return value

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryInputStreamBE & operator>>( Byte & b ) noexcept
	{
		return static_cast< BinaryInputStreamBE & >( BinaryInputStream::operator>>( b ) );
	}

	template< typename Byte, REQUIRES( is_byte_alike<Byte>::value ) >
	BinaryInputStreamBE & operator>>( span< Byte > bytes ) noexcept
	{
		return static_cast< BinaryInputStreamBE & >( BinaryInputStream::operator>>( bytes ) );
	}

	template< typename Cont,
		REQUIRES( is_range_of_byte_alikes<Cont>::value && has_contiguous_data<Cont>::value && !is_resizable<Cont>::value ) >
	BinaryInputStreamBE & operator>>( Cont & cont )
	{
		return static_cast< BinaryInputStreamBE & >( BinaryInputStream::operator>>( cont ) );
	}
};

BinaryInputStreamBE & BinaryInputStream::getBigEndianStream()
{
	return static_cast< BinaryInputStreamBE & >( *this );
}


//======================================================================================================================
// misc utils

template< typename Type, Endianity endianity >
std::vector< uint8_t > toByteVector( const Type & obj )
{
	std::vector< uint8_t > bytes( obj.size() );
	BinaryOutputStream stream( bytes );
	stream << obj;
	return bytes;
}

template< typename Type, Endianity endianity >
bool fromBytes( const_byte_span bytes, Type & obj )
{
	BinaryInputStream stream( bytes );
	stream >> obj;
	return !stream.failed();
}


//======================================================================================================================


} // namespace own


#endif // CPPUTILS_BINARY_STREAM_INCLUDED
