# Selective HAL compile: build only modules listed in HAL_MODULES
# Requires: HAL_ROOT, HAL_MODULES
set(HAL_SRCS ${HAL_ROOT}/Src/stm32f1xx_hal.c)
foreach(m ${HAL_MODULES})
  set(CANDIDATE ${HAL_ROOT}/Src/stm32f1xx_hal_${m}.c)
  if(EXISTS ${CANDIDATE})
    list(APPEND HAL_SRCS ${CANDIDATE})
  else()
    message(WARNING "HAL module '${m}' not found at ${CANDIDATE}")
  endif()
endforeach()
