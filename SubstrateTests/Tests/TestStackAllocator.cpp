#include <catch2/catch_test_macros.hpp>

#define SUBSTRATE_ENABLE_DETAILS
#include "Substrate/Allocator.h"

class TestStackAllocator : public Substrate::BaseAllocator
{

};