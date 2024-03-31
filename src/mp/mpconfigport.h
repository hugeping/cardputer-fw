/* This file is part of the MicroPython project, http://micropython.org/
 * The MIT License (MIT)
 * Copyright (c) 2022-2023 Damien P. George
 */

// Include common MicroPython embed configuration.
#include "port/mpconfigport_common.h"
//#include "esp_system.h"
//#include "freertos/FreeRTOS.h"
extern void mp_write(const char *str, int len);

#define MP_PLAT_PRINT_STRN(str, len) mp_write(str, len)

// Use the minimal starting configuration (disables all optional features).
#define MICROPY_CONFIG_ROM_LEVEL                (MICROPY_CONFIG_ROM_LEVEL_MINIMUM)

// MicroPython configuration.
#define MICROPY_ENABLE_COMPILER                 (1)
#define MICROPY_ENABLE_GC                       (1)
#define MICROPY_PY_GC                           (1)

// object representation and NLR handling
#define MICROPY_OBJ_REPR                    (MICROPY_OBJ_REPR_A)
#define MICROPY_NLR_SETJMP                  (1)
#define MICROPY_GCREGS_SETJMP               (1)
#define MICROPY_HELPER_REPL                 (1)
#define MICROPY_PY_RANDOM		    (1)
#define MICROPY_PY_RANDOM_EXTRA_FUNCS       (1)
#define MICROPY_FLOAT_IMPL  MICROPY_FLOAT_IMPL_DOUBLE
#define MICROPY_PY_MATH			    (1)
#define MICROPY_LONGINT_IMPL MICROPY_LONGINT_IMPL_LONGLONG
