#pragma once


#define __PP_FOREACH_10(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) __PP_FOREACH_9(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9) APPLY(v10)
#define  __PP_FOREACH_9(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9) __PP_FOREACH_8(APPLY, v1, v2, v3, v4, v5, v6, v7, v8) APPLY(v9)
#define  __PP_FOREACH_8(APPLY, v1, v2, v3, v4, v5, v6, v7, v8) __PP_FOREACH_7(APPLY, v1, v2, v3, v4, v5, v6, v7) APPLY(v8)
#define  __PP_FOREACH_7(APPLY, v1, v2, v3, v4, v5, v6, v7) __PP_FOREACH_6(APPLY, v1, v2, v3, v4, v5, v6) APPLY(v7)
#define  __PP_FOREACH_6(APPLY, v1, v2, v3, v4, v5, v6) __PP_FOREACH_5(APPLY, v1, v2, v3, v4, v5) APPLY(v6)
#define  __PP_FOREACH_5(APPLY, v1, v2, v3, v4, v5) __PP_FOREACH_4(APPLY, v1, v2, v3, v4) APPLY(v5)
#define  __PP_FOREACH_4(APPLY, v1, v2, v3, v4) __PP_FOREACH_3(APPLY, v1, v2, v3) APPLY(v4)
#define  __PP_FOREACH_3(APPLY, v1, v2, v3) __PP_FOREACH_2(APPLY, v1, v2) APPLY(v3)
#define  __PP_FOREACH_2(APPLY, v1, v2) APPLY(v1) APPLY(v2)
#define  __PP_FOREACH_1(APPLY, v1) APPLY(v1)


#define PP_FOREACH(APPLY, ...) PP_EXPAND(PP_JOIN(__PP_FOREACH_, PP_NARG(__VA_ARGS__))(APPLY, __VA_ARGS__))

#define __PP_FOREACH2_10(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) __PP_FOREACH2_9(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9) APPLY(v10)
#define  __PP_FOREACH2_9(APPLY, v1, v2, v3, v4, v5, v6, v7, v8, v9) __PP_FOREACH2_8(APPLY, v1, v2, v3, v4, v5, v6, v7, v8) APPLY(v9)
#define  __PP_FOREACH2_8(APPLY, v1, v2, v3, v4, v5, v6, v7, v8) __PP_FOREACH2_7(APPLY, v1, v2, v3, v4, v5, v6, v7) APPLY(v8)
#define  __PP_FOREACH2_7(APPLY, v1, v2, v3, v4, v5, v6, v7) __PP_FOREACH2_6(APPLY, v1, v2, v3, v4, v5, v6) APPLY(v7)
#define  __PP_FOREACH2_6(APPLY, v1, v2, v3, v4, v5, v6) __PP_FOREACH2_5(APPLY, v1, v2, v3, v4, v5) APPLY(v6)
#define  __PP_FOREACH2_5(APPLY, v1, v2, v3, v4, v5) __PP_FOREACH2_4(APPLY, v1, v2, v3, v4) APPLY(v5)
#define  __PP_FOREACH2_4(APPLY, v1, v2, v3, v4) __PP_FOREACH2_3(APPLY, v1, v2, v3) APPLY(v4)
#define  __PP_FOREACH2_3(APPLY, v1, v2, v3) __PP_FOREACH2_2(APPLY, v1, v2) APPLY(v3)
#define  __PP_FOREACH2_2(APPLY, v1, v2) APPLY(v1) APPLY(v2)
#define  __PP_FOREACH2_1(APPLY, v1) APPLY(v1)


#define PP_FOREACH2(APPLY, ...) PP_EXPAND(PP_JOIN(__PP_FOREACH2_, PP_NARG(__VA_ARGS__))(APPLY, __VA_ARGS__))



#define  __PP_FOREACH_PAIRS_8(APPLY, a1, b1, a2, b2, a3, b3, a4, b4)    APPLY(a1, b1) APPLY(a1, b2) APPLY(a1, b3) APPLY(a1, b4)\
                                                                        APPLY(a2, b1) APPLY(a2, b2) APPLY(a2, b3) APPLY(a2, b4)\
                                                                        APPLY(a3, b1) APPLY(a3, b2) APPLY(a3, b3) APPLY(a3, b4)\
                                                                        APPLY(a4, b1) APPLY(a4, b2) APPLY(a4, b3) APPLY(a4, b4)

#define  __PP_FOREACH_PAIRS_6(APPLY, a1, b1, a2, b2, a3, b3)    APPLY(a1, b1) APPLY(a1, b2) APPLY(a1, b3)\
                                                                APPLY(a2, b1) APPLY(a2, b2) APPLY(a2, b3)\
                                                                APPLY(a3, b1) APPLY(a3, b2) APPLY(a3, b3)

#define  __PP_FOREACH_PAIRS_4(APPLY, a1, b1, a2, b2) APPLY(a1, b1) APPLY(a1, b2)\
                                                     APPLY(a2, b1) APPLY(a2, b2)

#define  __PP_FOREACH_PAIRS_2(APPLY, a1, b1) APPLY(a1, b1)

#define PP_FOREACH_PAIRS(APPLY, ...) PP_EXPAND(PP_JOIN(__PP_FOREACH_PAIRS_, PP_NARG(__VA_ARGS__))(APPLY, __VA_ARGS__))

