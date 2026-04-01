#pragma once
#include <unordered_map>
#include <string>
#include <string_view>

namespace StarFire::Containers {

	struct TransparentHash {
		using is_transparent = void; // marks this as transparent
		size_t operator()(std::string_view sv) const noexcept {
			return std::hash<std::string_view>{}(sv);
		}
		size_t operator()(const std::string& s) const noexcept {
			return std::hash<std::string>{}(s);
		}
	};

	struct TransparentEqual {
		using is_transparent = void; // marks this as transparent
		bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
		bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
			return lhs == rhs;
		}
		bool operator()(std::string_view lhs, const std::string& rhs) const noexcept {
			return lhs == rhs;
		}
		bool operator()(const std::string& lhs, std::string_view rhs) const noexcept {
			return lhs == rhs;
		}
	};

	template<typename Value>
	using StringKeyMap = std::unordered_map<std::string, Value, TransparentHash, TransparentEqual>;
}