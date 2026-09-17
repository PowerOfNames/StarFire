#include <catch2/catch_test_macros.hpp>

#include "Substrate/CheckMacros.h"

#include <string>


// ---------------------------------------------------------------------------
// Test seams
//
// CheckMacros.h is pure preprocessor, so the mechanism can only be observed
// through the two things it calls out to: the log macro it is handed, and
// SUBSTRATE_DEBUG_BREAK(). Both are rebound to counters below.
//
// The real SUBSTRATE_DEBUG_BREAK() is __debugbreak(), which aborts the test
// runner when no debugger is attached - that alone would make every failing
// HALT and ENSURE path untestable. The *_IMPL macros only *reference*
// SUBSTRATE_DEBUG_BREAK inside their replacement lists, and a replacement list
// is rescanned at the point of use, so redefining it here rebinds it for every
// expansion in this file without touching the Substrate header.
// ---------------------------------------------------------------------------

namespace {

	int g_ExpressionEvaluations = 0;
	int g_ArgumentEvaluations = 0;
	int g_BreakCount = 0;

	struct TestLog
	{
		static inline int Count = 0;
		static inline std::string LastMessage;
		static inline size_t LastArgumentCount = 0;

		template<typename... TArgs>
		static void Record(const char* message, const TArgs&... args)
		{
			++Count;
			LastMessage = message;
			LastArgumentCount = sizeof...(TArgs);
			((void)args, ...);
		}
	};

	void ResetProbes()
	{
		g_ExpressionEvaluations = 0;
		g_ArgumentEvaluations = 0;
		g_BreakCount = 0;
		TestLog::Count = 0;
		TestLog::LastMessage.clear();
		TestLog::LastArgumentCount = 0;
	}

	// Returns its argument, and records that the guarded expression was evaluated.
	bool Expression(bool result)
	{
		++g_ExpressionEvaluations;
		return result;
	}

	// Returns its argument, and records that a log argument was evaluated.
	int Argument(int value)
	{
		++g_ArgumentEvaluations;
		return value;
	}
}

#define TEST_LOG(...) TestLog::Record(__VA_ARGS__)

#undef SUBSTRATE_DEBUG_BREAK
#define SUBSTRATE_DEBUG_BREAK() (++g_BreakCount)


namespace {

	// Every macro expansion below is a distinct call site owning a distinct
	// once-latch, and a latch lives for the whole process. Each site is
	// therefore exercised by exactly one TEST_CASE, so the tests stay
	// independent of the order Catch2 runs them in.

	void LogOnceSite(bool condition)
	{
		SUBSTRATE_LOG_ONCE_IMPL(Expression(condition), TEST_LOG, "log once {} {}", Argument(1), Argument(2));
	}

	void LogOnceSiteA() { SUBSTRATE_LOG_ONCE_IMPL(false, TEST_LOG, "site A"); }
	void LogOnceSiteB() { SUBSTRATE_LOG_ONCE_IMPL(false, TEST_LOG, "site B"); }

	template<int TSite>
	void LogOnceTemplateSite()
	{
		SUBSTRATE_LOG_ONCE_IMPL(false, TEST_LOG, "template site");
	}

	void HaltSite(bool condition)
	{
		SUBSTRATE_HALT_IMPL(Expression(condition), TEST_LOG, "halt {}", Argument(7));
	}

	bool EnsureSite(bool condition)
	{
		return SUBSTRATE_ENSURE_ONCE_IMPL(Expression(condition), TEST_LOG, "ensure {}", Argument(9));
	}

	void DisabledStatementSite()
	{
		SUBSTRATE_DISABLED_STATEMENT_IMPL(Expression(false), "disabled statement {}", Argument(3));
	}

	bool DisabledExpressionSite()
	{
		return SUBSTRATE_DISABLED_EXPRESSION_IMPL(Expression(false), "disabled expression {}", Argument(4));
	}

	// The statement forms must survive an unbraced if/else - that is the whole
	// point of the do/while(false) wrapper. This mainly has to compile.
	void UnbracedIfSite(bool condition)
	{
		if (condition)
			SUBSTRATE_LOG_ONCE_IMPL(false, TEST_LOG, "unbraced then");
		else
			SUBSTRATE_DISABLED_STATEMENT_IMPL(false, "unbraced else");
	}


	// --- Added with the no-latch / inverted forms -------------------------

	void LogSite(bool condition)
	{
		SUBSTRATE_LOG_IMPL(Expression(condition), TEST_LOG, "log {}", Argument(11));
	}

	bool EnsureNoBreakSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOBREAK_ONCE_IMPL(Expression(condition), TEST_LOG, "ensure nobreak {}", Argument(12));
	}

	bool EnsureNotSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOT_ONCE_IMPL(Expression(condition), TEST_LOG, "ensure not {}", Argument(13));
	}

	bool EnsureNotNoBreakSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOT_NOBREAK_ONCE_IMPL(Expression(condition), TEST_LOG, "ensure not nobreak {}", Argument(14));
	}

	bool EnsureAllSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOBREAK_IMPL(Expression(condition), TEST_LOG, "ensure all {}", Argument(15));
	}

	bool EnsureNotAllSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOT_NOBREAK_IMPL(Expression(condition), TEST_LOG, "ensure not all {}", Argument(16));
	}


	// --- Added for the Debug forms of _VALIDATE and the _ALL tiers ---------

	void HaltOnceSite(bool condition)
	{
		SUBSTRATE_HALT_ONCE_IMPL(Expression(condition), TEST_LOG, "halt once {}", Argument(17));
	}

	bool EnsureBreakAllSite(bool condition)
	{
		return SUBSTRATE_ENSURE_IMPL(Expression(condition), TEST_LOG, "ensure break all {}", Argument(18));
	}

	bool EnsureNotBreakAllSite(bool condition)
	{
		return SUBSTRATE_ENSURE_NOT_IMPL(Expression(condition), TEST_LOG, "ensure not break all {}", Argument(19));
	}
}


TEST_CASE("CheckMacros LogOnce", "[CheckMacros][LogOnce]")
{
	ResetProbes();

	// A passing check costs one expression evaluation and nothing else.
	LogOnceSite(true);
	LogOnceSite(true);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_ArgumentEvaluations == 0);
	REQUIRE(g_BreakCount == 0);
	REQUIRE(g_ExpressionEvaluations == 2);

	// The first failure logs.
	LogOnceSite(false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_ExpressionEvaluations == 3);
	REQUIRE(g_ArgumentEvaluations == 2);
	REQUIRE(g_BreakCount == 0);

	// The message and both arguments arrive as separate arguments. This is the
	// interesting half: they travel through SUBSTRATE_EXPAND and an extra macro
	// layer, which is exactly where MSVC traditional preprocessing collapses
	// __VA_ARGS__ into a single token.
	REQUIRE(TestLog::LastMessage == "log once {} {}");
	REQUIRE(TestLog::LastArgumentCount == 2);

	// Every later failure re-evaluates the expression but never logs again, so
	// the log arguments are not evaluated either.
	LogOnceSite(false);
	LogOnceSite(false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_ExpressionEvaluations == 5);
	REQUIRE(g_ArgumentEvaluations == 2);
}

TEST_CASE("CheckMacros LogOnce latches per call site", "[CheckMacros][LogOnce]")
{
	ResetProbes();

	// Two expansions are two latches, not one shared latch.
	LogOnceSiteA();
	LogOnceSiteB();
	REQUIRE(TestLog::Count == 2);

	LogOnceSiteA();
	LogOnceSiteB();
	REQUIRE(TestLog::Count == 2);
}

TEST_CASE("CheckMacros LogOnce latches per template instantiation", "[CheckMacros][LogOnce]")
{
	ResetProbes();

	// A single expansion inside a template still yields one latch *per
	// instantiation*, so "once" means once per instantiated function, not once
	// per line of source. Worth pinning: the allocators are templates.
	LogOnceTemplateSite<1>();
	LogOnceTemplateSite<1>();
	REQUIRE(TestLog::Count == 1);

	LogOnceTemplateSite<2>();
	REQUIRE(TestLog::Count == 2);
}

TEST_CASE("CheckMacros Halt", "[CheckMacros][Halt]")
{
	ResetProbes();

	HaltSite(true);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);
	REQUIRE(g_ArgumentEvaluations == 0);
	REQUIRE(g_ExpressionEvaluations == 1);

	// Halt has no latch by design: it logs and breaks on *every* failure.
	HaltSite(false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(TestLog::LastMessage == "halt {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	HaltSite(false);
	HaltSite(false);
	REQUIRE(TestLog::Count == 3);
	REQUIRE(g_BreakCount == 3);
	REQUIRE(g_ArgumentEvaluations == 3);
	REQUIRE(g_ExpressionEvaluations == 4);
}

TEST_CASE("CheckMacros Ensure", "[CheckMacros][Ensure]")
{
	ResetProbes();

	// Passing: returns true, evaluates the expression exactly once, stays silent.
	REQUIRE(EnsureSite(true) == true);
	REQUIRE(g_ExpressionEvaluations == 1);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);
	REQUIRE(g_ArgumentEvaluations == 0);

	// First failure: returns false, logs and breaks once.
	REQUIRE(EnsureSite(false) == false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(TestLog::LastMessage == "ensure {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	// Later failures keep returning false - only the reporting is latched.
	REQUIRE(EnsureSite(false) == false);
	REQUIRE(EnsureSite(false) == false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(g_ArgumentEvaluations == 1);
	REQUIRE(g_ExpressionEvaluations == 4);

	// And a later success still returns true.
	REQUIRE(EnsureSite(true) == true);
	REQUIRE(TestLog::Count == 1);
}

TEST_CASE("CheckMacros disabled statement form", "[CheckMacros][Disabled]")
{
	ResetProbes();

	DisabledStatementSite();
	DisabledStatementSite();

	// Neither the guarded expression nor the log arguments may be evaluated
	// when a tier is compiled out - both sit inside sizeof().
	REQUIRE(g_ExpressionEvaluations == 0);
	REQUIRE(g_ArgumentEvaluations == 0);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);
}

TEST_CASE("CheckMacros disabled expression form", "[CheckMacros][Disabled]")
{
	ResetProbes();

	// A compiled-out ENSURE must report success, so an
	// `if (!ENSURE(x)) return;` call site keeps running the happy path
	// instead of bailing out.
	REQUIRE(DisabledExpressionSite() == true);
	REQUIRE(DisabledExpressionSite() == true);

	REQUIRE(g_ExpressionEvaluations == 0);
	REQUIRE(g_ArgumentEvaluations == 0);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);
}

TEST_CASE("CheckMacros statement forms are single statements", "[CheckMacros][Disabled]")
{
	ResetProbes();

	// Compiling this file at all proves the do/while(false) wrapper holds; the
	// calls just confirm the enabled branch still reaches the logger.
	UnbracedIfSite(true);
	REQUIRE(TestLog::Count == 1);

	UnbracedIfSite(false);
	REQUIRE(TestLog::Count == 1);
}


TEST_CASE("CheckMacros Log has no latch", "[CheckMacros][Log]")
{
	ResetProbes();

	LogSite(true);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_ExpressionEvaluations == 1);

	// Unlike LogOnce, every failure reports. This is what the _ALL statement
	// variants are built on: a loop must be able to name every bad element.
	LogSite(false);
	LogSite(false);
	LogSite(false);
	REQUIRE(TestLog::Count == 3);
	REQUIRE(g_ArgumentEvaluations == 3);
	REQUIRE(TestLog::LastMessage == "log {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	// It never breaks, in any configuration.
	REQUIRE(g_BreakCount == 0);
}

TEST_CASE("CheckMacros Ensure without break", "[CheckMacros][Ensure]")
{
	ResetProbes();

	REQUIRE(EnsureNoBreakSite(true) == true);
	REQUIRE(TestLog::Count == 0);

	// Reports once and returns false every time - but never breaks. This is the
	// form the expression tier degrades to below Debug, so the early-out at the
	// call site still happens in a shipping build.
	REQUIRE(EnsureNoBreakSite(false) == false);
	REQUIRE(EnsureNoBreakSite(false) == false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 0);
	REQUIRE(EnsureNoBreakSite(true) == true);
}

TEST_CASE("CheckMacros Ensure inverted", "[CheckMacros][Ensure]")
{
	ResetProbes();

	// Inverted return, same good condition: a holding condition yields false, so
	// the call site reads `if (REQUIRE_FAILS(cond)) return;` with no leading !.
	REQUIRE(EnsureNotSite(true) == false);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);

	REQUIRE(EnsureNotSite(false) == true);
	REQUIRE(EnsureNotSite(false) == true);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(TestLog::LastMessage == "ensure not {}");
}

TEST_CASE("CheckMacros Ensure inverted without break", "[CheckMacros][Ensure]")
{
	ResetProbes();

	REQUIRE(EnsureNotNoBreakSite(true) == false);
	REQUIRE(EnsureNotNoBreakSite(false) == true);
	REQUIRE(EnsureNotNoBreakSite(false) == true);
	REQUIRE(TestLog::Count == 1);

	// The whole point of this form. A stray SUBSTRATE_DEBUG_BREAK() here would
	// abort a Release or Profiling run on the first failure, with no debugger
	// attached to catch it. This assertion is the regression pin for that.
	REQUIRE(g_BreakCount == 0);
}

TEST_CASE("CheckMacros Ensure all has no latch", "[CheckMacros][Ensure][All]")
{
	ResetProbes();

	REQUIRE(EnsureAllSite(true) == true);
	REQUIRE(TestLog::Count == 0);

	// Reports every failure so a loop can name every bad element. This is the
	// below-Debug form of _REQUIRE_ALL, so it never breaks; the Debug form, which
	// breaks on every failure, is pinned in "Ensure all breaks on every failure".
	REQUIRE(EnsureAllSite(false) == false);
	REQUIRE(EnsureAllSite(false) == false);
	REQUIRE(EnsureAllSite(false) == false);
	REQUIRE(TestLog::Count == 3);
	REQUIRE(g_ArgumentEvaluations == 3);
	REQUIRE(g_BreakCount == 0);

	REQUIRE(EnsureAllSite(true) == true);
	REQUIRE(TestLog::Count == 3);
}

TEST_CASE("CheckMacros Ensure all inverted", "[CheckMacros][Ensure][All]")
{
	ResetProbes();

	REQUIRE(EnsureNotAllSite(true) == false);
	REQUIRE(EnsureNotAllSite(false) == true);
	REQUIRE(EnsureNotAllSite(false) == true);
	REQUIRE(TestLog::Count == 2);
	REQUIRE(g_BreakCount == 0);
}


TEST_CASE("CheckMacros Halt once", "[CheckMacros][Halt]")
{
	ResetProbes();

	HaltOnceSite(true);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);
	REQUIRE(g_ArgumentEvaluations == 0);

	// The Debug form of _VALIDATE: the first failure logs and breaks...
	HaltOnceSite(false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(TestLog::LastMessage == "halt once {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	// ...and every later failure is silent: no log, no break, and the log
	// arguments are not evaluated again.
	HaltOnceSite(false);
	HaltOnceSite(false);
	REQUIRE(TestLog::Count == 1);
	REQUIRE(g_BreakCount == 1);
	REQUIRE(g_ArgumentEvaluations == 1);
	REQUIRE(g_ExpressionEvaluations == 4);
}

TEST_CASE("CheckMacros Ensure all breaks on every failure", "[CheckMacros][Ensure][All]")
{
	ResetProbes();

	REQUIRE(EnsureBreakAllSite(true) == true);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);

	// The Debug form of _REQUIRE_ALL: no latch, and a break on every failure, so
	// continuing in the debugger stops again at the next bad element.
	REQUIRE(EnsureBreakAllSite(false) == false);
	REQUIRE(EnsureBreakAllSite(false) == false);
	REQUIRE(EnsureBreakAllSite(false) == false);
	REQUIRE(TestLog::Count == 3);
	REQUIRE(g_BreakCount == 3);
	REQUIRE(g_ArgumentEvaluations == 3);
	REQUIRE(TestLog::LastMessage == "ensure break all {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	REQUIRE(EnsureBreakAllSite(true) == true);
	REQUIRE(TestLog::Count == 3);
	REQUIRE(g_BreakCount == 3);
	REQUIRE(g_ExpressionEvaluations == 5);
}

TEST_CASE("CheckMacros Ensure all inverted breaks on every failure", "[CheckMacros][Ensure][All]")
{
	ResetProbes();

	// The Debug form of _REQUIRE_FAILS_ALL: same good condition, inverted return.
	REQUIRE(EnsureNotBreakAllSite(true) == false);
	REQUIRE(TestLog::Count == 0);
	REQUIRE(g_BreakCount == 0);

	REQUIRE(EnsureNotBreakAllSite(false) == true);
	REQUIRE(EnsureNotBreakAllSite(false) == true);
	REQUIRE(TestLog::Count == 2);
	REQUIRE(g_BreakCount == 2);
	REQUIRE(g_ArgumentEvaluations == 2);
	REQUIRE(TestLog::LastMessage == "ensure not break all {}");
	REQUIRE(TestLog::LastArgumentCount == 1);

	REQUIRE(EnsureNotBreakAllSite(true) == false);
	REQUIRE(TestLog::Count == 2);
	REQUIRE(g_BreakCount == 2);
}
