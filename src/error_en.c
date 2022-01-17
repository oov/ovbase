#include "ovbase.h"

NODISCARD error generic_error_message_mapper_en(uint_least32_t const code, struct NATIVE_STR *const message) {
  switch (code) {
  case err_fail:
    return scpy(message, NSTR("failed."));
  case err_unexpected:
    return scpy(message, NSTR("unexpected."));
  case err_invalid_arugment:
    return scpy(message, NSTR("invalid argument."));
  case err_null_pointer:
    return scpy(message, NSTR("null pointer."));
  case err_out_of_memory:
    return scpy(message, NSTR("out of memory."));
  case err_not_sufficient_buffer:
    return scpy(message, NSTR("not sufficient buffer."));
  case err_not_found:
    return scpy(message, NSTR("not found."));
  case err_abort:
    return scpy(message, NSTR("aborted."));
  case err_not_implemented_yet:
    return scpy(message, NSTR("not implemented yet."));
  }
  return scpy(message, NSTR("unknown error code."));
}
