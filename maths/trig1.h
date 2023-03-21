#ifndef _TRIG1_H_
#define _TRIG1_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdint.h>

/// @brief Integer trigonometric sine fucntion
///⏎
/// here angle is:
///   -32768 for -2*PI
///   0 for 0
///   +32768 (which is really 0) for 2*PI
///
/// the return value is
///   -32768 for -1.0
///   0 for 0.0
///   +32768 for 1.0
///
/// @param angle expressed as int16_t
/// @returns sin() in range -32767 to +32767
int64_t sin1(int16_t angle);

/// @brief Integer trigonometric cosine fucntion
///⏎
/// here angle is:
///   -32768 for -2*PI
///   0 for 0
///   +32768 (which is really 0) for 2*PI
///
/// the return value is
///   -32767 for -1.0
///   0 for 0.0
///   +32767 for 1.0
///
/// @param angle expressed as int16_t
/// @returns sin() in range -32767 to +32767
int64_t cos1(int16_t angle);

#ifdef  __cplusplus
}
#endif
#endif
