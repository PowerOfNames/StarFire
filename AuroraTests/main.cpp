#define CATCH_CONFIG_RUNNER

#include <catch2/catch_session.hpp>

int main(int argc, char* argv[]) {
	// No Aurora log callback is installed: Log::Message no-ops when the
	// callback is null, so AURORA_WARN inside the functions under test is
	// silent rather than crashing.

	int result = Catch::Session().run(argc, argv);

	return result;
}
