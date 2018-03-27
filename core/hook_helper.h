#ifndef ITE_HOOK_HELPER_H
#define ITE_HOOK_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#define HOOK(original_func, hook_func)         { _ ## original_func = hook_func; }
#define HOOK_RESTORE_TO_DEFAULT(original_func) { _ ## original_func = original_func ## _default; }

#ifdef __cplusplus
}
#endif

#endif