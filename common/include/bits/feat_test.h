#pragma once

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#ifndef __has_attribute
#define __has_attribute(x) (0)
#endif

#ifndef __has_c_attribute
#define __has_c_attribute(x) (0)
#endif

#if __has_c_attribute(unsequenced)
#define _ATTRIBUTE_UNSEQ [[unsequenced]]
#else
#define _ATTRIBUTE_UNSEQ
#endif

#if __has_c_attribute(reproducible)
#define _ATTRIBUTE_REPRODUCIBLE [[reproducible]]
#else
#define _ATTRIBUTE_REPRODUCIBLE
#endif
