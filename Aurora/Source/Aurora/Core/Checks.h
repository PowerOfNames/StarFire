#pragma once

#include "Aurora/Core/Logging.h"
#include "Substrate/CheckMacros.h"


#ifndef AURORA_CHECK_LEVEL
	#if defined(AURORA_DEBUG_MODE)
		#define AURORA_CHECK_LEVEL 3
	#elif defined(AURORA_PROFILING_MODE)
		#define AURORA_CHECK_LEVEL 2
	#elif defined(AURORA_RELEASE_MODE)
		#define AURORA_CHECK_LEVEL 1
	#else
		#error "No AURORA_*_MODE define - cannot derive AURORA_CHECK_LEVEL"
	#endif
#endif

#ifndef AURORA_CHECK_SLOW_ENABLED
	#define AURORA_CHECK_SLOW_ENABLED 0
#endif

#if (AURORA_CHECK_LEVEL >= 3)
	#define AURORA_ASSERT(expr, ...)				SUBSTRATE_HALT_IMPL(expr, AURORA_CRITICAL, __VA_ARGS__)
	#define AURORA_VALIDATE(expr, ...)				SUBSTRATE_HALT_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE(expr, ...)				SUBSTRATE_ENSURE_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_FAILS(expr, ...)			SUBSTRATE_ENSURE_NOT_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_ALL(expr, ...)			SUBSTRATE_ENSURE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_FAILS_ALL(expr, ...)		SUBSTRATE_ENSURE_NOT_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
#else
	// The break is a desk-time tool, so it goes. The message still belongs in a shipping log.
	#define AURORA_ASSERT(expr, ...)				SUBSTRATE_LOG_IMPL(expr, AURORA_CRITICAL, __VA_ARGS__)
	// The only tier that vanishes. Safe because a VALIDATE site always has a working fallback below it.
	#define AURORA_VALIDATE(expr, ...)				(void(0))
	#define AURORA_REQUIRE(expr, ...)				SUBSTRATE_ENSURE_NOBREAK_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_FAILS(expr, ...)			SUBSTRATE_ENSURE_NOT_NOBREAK_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_ALL(expr, ...)			SUBSTRATE_ENSURE_NOBREAK_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_REQUIRE_FAILS_ALL(expr, ...)		SUBSTRATE_ENSURE_NOT_NOBREAK_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
#endif

// _FAILS takes the same good condition but inverts the return, so it reads
// `if (AURORA_REQUIRE_FAILS(cond, ...)) return;` with no leading `!`.
// _ALL drops the once-latch so one pass over a loop names every bad element.

#if(AURORA_CHECK_LEVEL >= 1)
	#define AURORA_CHECK(expr, ...)					SUBSTRATE_LOG_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
	#define AURORA_CHECK_ALL(expr, ...)				SUBSTRATE_LOG_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
#else
	#define AURORA_CHECK(expr, ...)					SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
	#define AURORA_CHECK_ALL(expr, ...)				SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
#endif

// Level 1 should be the lowest level, with exception for special builds not yet introduced.
#if(AURORA_CHECK_LEVEL >= 1 && AURORA_CHECK_SLOW_ENABLED)
	#define AURORA_CHECK_SLOW(expr, ...)			SUBSTRATE_LOG_ONCE_IMPL(expr, AURORA_ERROR, __VA_ARGS__)
#else
	#define AURORA_CHECK_SLOW(expr, ...)			SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
#endif