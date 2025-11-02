//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: unrecoverable error notification
//======================================================================================================================

#include "CriticalError.hpp"

#include <cstdio>
#include <cstdarg>
#include <stdexcept>


namespace impl {


static void print_critical_error_v( const char * format, va_list args )
{
	vfprintf( stderr, format, args );
	fputc( '\n', stderr );
	fflush( stderr );
}

[[noreturn]] static void abort_on_critical_error_v( const char * format, va_list args )
{
	print_critical_error_v( format, args );
	abort();
}

#ifndef NO_EXCEPTIONS
[[noreturn]] static void throw_critical_error_v( const char * format, va_list args )
{
	char message [1024];
	print_critical_error_v( format, args );
	vsnprintf( message, sizeof(message), format, args );
	throw std::logic_error( message );
}
#endif

void print_critical_error( const char * format, ... )
{
	va_list args;
	va_start( args, format );
	print_critical_error_v( format, args );
	va_end( args );
}

[[noreturn]] void abort_on_critical_error( const char * format, ... )
{
	va_list args;
	va_start( args, format );
	abort_on_critical_error_v( format, args );
	va_end( args );
}

#ifndef NO_EXCEPTIONS
[[noreturn]] void throw_critical_error( const char * format, ... )
{
	va_list args;
	va_start( args, format );
	throw_critical_error_v( format, args );
	va_end( args );
}
#endif


} // namespace impl
