#pragma once

#define __IF0(...)
#define __IF1(...) __VA_ARGS__

#define __IF_ELSE0(A, B) B
#define __IF_ELSE1(A, B) A

#define __IF_ELSEVA0(A, ...) __VA_ARGS__
#define __IF_ELSEVA1(A, ...) A

#define __ELSE_IFVA0(B, ...) B
#define __ELSE_IFVA1(B, ...) __VA_ARGS__

#define PP_IF(C, ...) PP_JOIN(__IF, C)(__VA_ARGS__)

#define PP_IF_ELSE(C, A, B) PP_JOIN(__IF_ELSE, C)(A, B)
#define PP_IF_ELSEVA(C, A, ...) PP_JOIN(__IF_ELSEVA, C)(A __VA_OPT__(,) __VA_ARGS__)
#define PP_ELSE_IFVA(C, B, ...) PP_JOIN(__ELSE_IFVA, C)(B __VA_OPT__(,) __VA_ARGS__)

