#pragma once

#if defined TRACY_ENABLE
#include "Tracy/Tracy.hpp"

#define PROFILE_COLOR												0xAD5FC2		

#define PROFILE_FUNCTION											ZoneScoped
#define PROFILE_SCOPE(name)											ZoneScopedNC(name, PROFILE_COLOR)
#define PROFILE_THREAD_NAME(name, id)								tracy::SetThreadNameWithHint(name, id)
#define PROFILE_ERROR_SCOPE(name)									ZoneScopedNC(name, 0xFF0000)
#define PROFILE_FRAME_MARK											FrameMark

#define PROFILE_PLOT(name, val)										TracyPlot(name, val)
#define PROFILE_PLOT_CONFIG_NUMBER(name)							TracyPlotConfig(name, tracy::PlotFormatType::Number, 0, false, PROFILE_COLOR)
#define PROFILE_PLOT_CONFIG_MEMORY(name)							TracyPlotConfig(name, tracy::PlotFormatType::Memory, 0, false, PROFILE_COLOR)
#define PROFILE_PLOT_CONFIG_PERCENTAGE(name)						TracyPlotConfig(name, tracy::PlotFormatType::Percentage, 0, true, PROFILE_COLOR)

#define PROFILE_MEMORY_ALLOC(ptr, size, name)						TracyAllocN(ptr, size, name)
#define PROFILE_MEMORY_FREE(ptr, name)								TracyFreeN(ptr, name)

#define PROFILE_MUTEX_LOCK_DECL(mutexType, lockVar)					TracyLockable(mutexType, lockVar)
#define PROFILE_MUTEX_LOCK_GUARD(guardType, mutexType, lockVar)		guardType<LockableBase(mutexType)> lock(lockVar)

#else

#define PROFILE_FUNCTION											do {} while(false)
#define PROFILE_SCOPE(name)											do {} while(false)
#define PROFILE_THREAD_NAME(name, id)								do {} while(false)
#define PROFILE_ERROR_SCOPE(name)									do {} while(false)
#define PROFILE_FRAME_MARK											do {} while(false)

#define PROFILE_PLOT(name, val)										do {} while(false)
#define PROFILE_PLOT_CONFIG_NUMBER(name)							do {} while(false)
#define PROFILE_PLOT_CONFIG_MEMORY(name)							do {} while(false)
#define PROFILE_PLOT_CONFIG_PERCENTAGE(name)						do {} while(false)

#define PROFILE_MEMORY_ALLOC(ptr, size, name)						do {} while(false)
#define PROFILE_MEMORY_FREE(ptr, name)								do {} while(false)

#define PROFILE_MUTEX_LOCK(mutexType, lockVar)						mutexType lockVar
#define PROFILE_MUTEX_LOCK_GUARD(guardType, mutexType, lockVar)		guardType<mutexType> lock(lockVar)

#endif //TRACY_ENABLE_PROFILING
