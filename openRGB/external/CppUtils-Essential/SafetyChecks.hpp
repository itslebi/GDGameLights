//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: declarations and utils for runtime safety
//======================================================================================================================

#ifndef CPPUTILS_SAFETY_CHECKS_INCLUDED
#define CPPUTILS_SAFETY_CHECKS_INCLUDED


#include "CriticalError.hpp"

#ifdef DEBUG
	#define SAFETY_CHECKS
#endif

enum class SafetyChecks
{
	Disabled,
	Enabled
};

#ifdef SAFETY_CHECKS
	constexpr SafetyChecks defaultSafetyChecks = SafetyChecks::Enabled;
#else
	constexpr SafetyChecks defaultSafetyChecks = SafetyChecks::Disabled;
#endif

// safety checks made only in some build configurations
#if defined(SAFETY_CHECKS)
	#if defined(DEBUG) && !defined(NOASSERT)
		#define SAFETY_CHECK( condition, ... ) assert_msg( condition, CPPUTILS_GET_FIRST_ARG( __VA_ARGS__ ) )
	#elif defined(CRITICALS_CATCHABLE)
		#define SAFETY_CHECK( condition, ... ) if (condition) impl::throw_critical_error( __VA_ARGS__ )
	#else
		#define SAFETY_CHECK( condition, ... ) if (condition) impl::abort_on_critical_error( __VA_ARGS__ )
	#endif
#else
	#define SAFETY_CHECK( condition, ... )
#endif


#endif // CPPUTILS_SAFETY_CHECKS_INCLUDED
