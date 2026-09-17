#pragma once

namespace Substrate::CheckHelpers {
	template<typename... Args>
	constexpr bool Unused(const Args&...) noexcept { return true; }
}

#define SUBSTRATE_EXPAND(x) x
#define SUBSTRATE_DEBUG_BREAK() __debugbreak()


#define SUBSTRATE_LOG_IMPL(expr, logMacro, ...)							\
	do {																	\
		if (!(expr)) [[unlikely]] {											\
			SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));						\
		}																	\
	} while (false)


#define SUBSTRATE_LOG_ONCE_IMPL(expr, logMacro, ...)						\
	do {																	\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateLogged = false;							\
			if (!(s_SubstrateLogged)) {										\
				s_SubstrateLogged = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
			}																\
		}																	\
	} while (false)	


#define SUBSTRATE_HALT_IMPL(expr, logMacro, ...)							\
	do {																	\
		if (!(expr)) [[unlikely]] {											\
			SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));						\
			SUBSTRATE_DEBUG_BREAK();										\
		}																	\
	} while (false)


#define SUBSTRATE_HALT_ONCE_IMPL(expr, logMacro, ...)						\
	do {																	\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateHalted = false;							\
			if(!s_SubstrateHalted) {										\
				s_SubstrateHalted = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
				SUBSTRATE_DEBUG_BREAK();									\
			}																\
		}																	\
	} while (false)

#define SUBSTRATE_ENSURE_IMPL(expr, logMacro, ...)							\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));						\
			SUBSTRATE_DEBUG_BREAK();										\
			return false;													\
		}																	\
		return true;														\
	}())

#define SUBSTRATE_ENSURE_ONCE_IMPL(expr, logMacro, ...)						\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateEnsured = false;							\
			if (!s_SubstrateEnsured) {										\
				s_SubstrateEnsured = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
				SUBSTRATE_DEBUG_BREAK();									\
			}																\
			return false;													\
		}																	\
		return true;														\
	}())

#define SUBSTRATE_ENSURE_NOT_IMPL(expr, logMacro, ...)						\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
				SUBSTRATE_DEBUG_BREAK();									\
			return true;													\
		}																	\
		return false;														\
	}())

#define SUBSTRATE_ENSURE_NOT_ONCE_IMPL(expr, logMacro, ...)					\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateEnsured = false;							\
			if (!s_SubstrateEnsured) {										\
				s_SubstrateEnsured = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
				SUBSTRATE_DEBUG_BREAK();									\
			}																\
			return true;													\
		}																	\
		return false;														\
	}())

#define SUBSTRATE_ENSURE_NOBREAK_IMPL(expr, logMacro, ...)					\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));						\
			return false;													\
		}																	\
		return true;														\
	}())

#define SUBSTRATE_ENSURE_NOBREAK_ONCE_IMPL(expr, logMacro, ...)				\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateEnsured = false;							\
			if (!s_SubstrateEnsured) {										\
				s_SubstrateEnsured = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
			}																\
			return false;													\
		}																	\
		return true;														\
	}())

#define SUBSTRATE_ENSURE_NOT_NOBREAK_IMPL(expr, logMacro, ...)				\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));						\
			return true;													\
		}																	\
		return false;														\
	}())

#define SUBSTRATE_ENSURE_NOT_NOBREAK_ONCE_IMPL(expr, logMacro, ...)			\
	([&] () -> bool {														\
		if (!(expr)) [[unlikely]] {											\
			static bool s_SubstrateEnsured = false;							\
			if (!s_SubstrateEnsured) {										\
				s_SubstrateEnsured = true;									\
				SUBSTRATE_EXPAND(logMacro(__VA_ARGS__));					\
			}																\
			return true;													\
		}																	\
		return false;														\
	}())


#define SUBSTRATE_DISABLED_STATEMENT_IMPL(expr, ...)						\
	do {																	\
		(void)sizeof(static_cast<bool>(expr));								\
		(void)sizeof(Substrate::CheckHelpers::Unused(__VA_ARGS__));			\
	} while (false)

#define SUBSTRATE_DISABLED_EXPRESSION_IMPL(expr, ...)						\
	((void)sizeof(static_cast<bool>(expr)),									\
	 (void)sizeof(Substrate::CheckHelpers::Unused(__VA_ARGS__)), true)
		