//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: unrecoverable error notification
//======================================================================================================================

#ifndef CPPUTILS_CRITICAL_ERROR_INCLUDED
#define CPPUTILS_CRITICAL_ERROR_INCLUDED

#if defined(DEBUG) && !defined(NOASSERT)
	#include <cassert>
#endif

#if defined(NO_EXCEPTIONS) && defined(CRITICALS_CATCHABLE)
	#error "You cannot define NO_EXCEPTIONS and CRITICALS_CATCHABLE at the same time"
#endif

namespace impl
{
	void print_critical_error( const char * format, ... );

	[[noreturn]] void abort_on_critical_error( const char * format, ... );
#ifndef NO_EXCEPTIONS
	[[noreturn]] void throw_critical_error( const char * format, ... );
#endif
}

#define assert_msg( condition, message ) assert(( (void)message, (condition) ))

#define CPPUTILS_GET_FIRST_ARG( arg, ... ) arg

// unconditional critical error notification
#if defined(DEBUG) && !defined(NOASSERT)
	#define CRITICAL_ERROR( ... ) \
		do { \
			::impl::print_critical_error( __VA_ARGS__ ); \
			assert_msg( false, CPPUTILS_GET_FIRST_ARG( __VA_ARGS__ ) ); \
		} while (false)
#elif defined(CRITICALS_CATCHABLE)
	#define CRITICAL_ERROR( ... ) \
		::impl::throw_critical_error( __VA_ARGS__ )
#else
	#define CRITICAL_ERROR( ... ) \
		::impl::abort_on_critical_error( __VA_ARGS__ )
#endif

#define SHOULDNT_HAPPEN( ... ) CRITICAL_ERROR( __VA_ARGS__ )

#define TODO CRITICAL_ERROR( "unfinished feature!" );


#endif // CPPUTILS_CRITICAL_ERROR_INCLUDED
