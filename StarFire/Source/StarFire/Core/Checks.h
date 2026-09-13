#pragma once

#include "StarFire/Core/Logging.h"
#include "Substrate/CheckMacros.h"


#ifndef STARFIRE_CHECK_LEVEL
#	if defined(STARFIRE_DEBUG_MODE)
#		define STARFIRE_CHECK_LEVEL 3
#	elif defined(STARFIRE_PROFILING_MODE)
#		define STARFIRE_CHECK_LEVEL 2
#	elif defined(STARFIRE_RELEASE_MODE)
#		define STARFIRE_CHECK_LEVEL 1
#	else
#		error "No STARFIRE_*_MODE define - cannot derive STARFIRE_CHECK_LEVEL"
#endif
#endif

#ifndef STARFIRE_CHECK_SLOW_ENABLED
#	define STARFIRE_CHECK_SLOW_ENABLED 0
#endif

#if (STARFIRE_CHECK_LEVEL >= 3)
#	define STARFIRE_ASSERT(expr, ...)				SUBSTRATE_HALT_IMPL(expr, STARFIRE_CRITICAL, __VA_ARGS__)
#	define STARFIRE_VALIDATE(expr, ...)				SUBSTRATE_HALT_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE(expr, ...)				SUBSTRATE_ENSURE_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_FAILS(expr, ...)		SUBSTRATE_ENSURE_NOT_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_ALL(expr, ...)			SUBSTRATE_ENSURE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_FAILS_ALL(expr, ...)	SUBSTRATE_ENSURE_NOT_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#else
	// The break is a desk-time tool, so it goes. The message still belongs in a shipping log.
#	define STARFIRE_ASSERT(expr, ...)				SUBSTRATE_LOG_IMPL(expr, STARFIRE_CRITICAL, __VA_ARGS__)
	// The only tier that vanishes. Safe because a VALIDATE site always has a working fallback below it.
#	define STARFIRE_VALIDATE(expr, ...)				(void(0))
#	define STARFIRE_REQUIRE(expr, ...)				SUBSTRATE_ENSURE_NOBREAK_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_FAILS(expr, ...)		SUBSTRATE_ENSURE_NOT_NOBREAK_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_ALL(expr, ...)			SUBSTRATE_ENSURE_NOBREAK_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_REQUIRE_FAILS_ALL(expr, ...)	SUBSTRATE_ENSURE_NOT_NOBREAK_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#endif

// _FAILS takes the same good condition but inverts the return, so it reads
// `if (STARFIRE_REQUIRE_FAILS(cond, ...)) return;` with no leading `!`.
// _ALL drops the once-latch so one pass over a loop names every bad element.

#if(STARFIRE_CHECK_LEVEL >= 1)
#	define STARFIRE_CHECK(expr, ...)				SUBSTRATE_LOG_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#	define STARFIRE_CHECK_ALL(expr, ...)				SUBSTRATE_LOG_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#else
#	define STARFIRE_CHECK(expr, ...)				SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
#	define STARFIRE_CHECK_ALL(expr, ...)				SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
#endif

// Level 1 should be the lowest level, with exception for special builds not yet introduced.
#if(STARFIRE_CHECK_LEVEL >= 1 && STARFIRE_CHECK_SLOW_ENABLED)
#	define STARFIRE_CHECK_SLOW(expr, ...)			SUBSTRATE_LOG_ONCE_IMPL(expr, STARFIRE_ERROR, __VA_ARGS__)
#else
#	define STARFIRE_CHECK_SLOW(expr, ...)			SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, __VA_ARGS__)
#endif