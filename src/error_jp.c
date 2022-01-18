#include "ovbase.h"

NODISCARD error generic_error_message_mapper_jp(int const code, struct NATIVE_STR *const message) {
  switch (code) {
  case err_fail:
    return scpy(message, NSTR("処理に失敗しました。"));
  case err_unexpected:
    return scpy(message, NSTR("予期しないエラーです。"));
  case err_invalid_arugment:
    return scpy(message, NSTR("引数が間違っています。"));
  case err_null_pointer:
    return scpy(message, NSTR("ポインターが割り当てられていません。"));
  case err_out_of_memory:
    return scpy(message, NSTR("メモリーが確保できません。"));
  case err_not_sufficient_buffer:
    return scpy(message, NSTR("バッファが小さすぎます。"));
  case err_not_found:
    return scpy(message, NSTR("対象が見つかりませんでした。"));
  case err_abort:
    return scpy(message, NSTR("中断されました。"));
  case err_not_implemented_yet:
    return scpy(message, NSTR("実装されていません。"));
  }
  return scpy(message, NSTR("未知のエラーコードです。"));
}
