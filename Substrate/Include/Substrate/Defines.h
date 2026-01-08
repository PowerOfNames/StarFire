#pragma once

#if defined(SUBSTRATE_ENABLE_DETAILS)
#define SUBSTRATE_DETAILS_ENABLED true
#else
#define SUBSTRATE_DETAILS_ENABLED false
#endif


#define WIN32_LEAN_AND_MEAN
#ifdef HANDLE
#undef HANDLE
#endif