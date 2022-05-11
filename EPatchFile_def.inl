#if NUM_PARAMS > 10
#error EPatchFile: too many params
#endif

#define TOKENPASTE(p, n, s, ss) p ## n ## s ## ss
#define TOKENPASTE2(p, n, s, ss) TOKENPASTE(p, n, s, ss)

#define FUNCTION_NAME(name) TOKENPASTE2(name, NUM_PARAMS, SUFFIX_CONVENTION, )

static uintptr_t CALLING_CONVENTION FUNCTION_NAME(Log)(

#if NUM_PARAMS == 0
	void
#endif

#if NUM_PARAMS >= 1
	uintptr_t arg0
#endif

#if NUM_PARAMS >= 2
	, uintptr_t arg1
#endif

#if NUM_PARAMS >= 3
	, uintptr_t arg2
#endif

#if NUM_PARAMS >= 4
	, uintptr_t arg3
#endif

#if NUM_PARAMS >= 5
	, uintptr_t arg4
#endif

#if NUM_PARAMS >= 6
	, uintptr_t arg5
#endif

#if NUM_PARAMS >= 7
	, uintptr_t arg6
#endif

#if NUM_PARAMS >= 8
	, uintptr_t arg7
#endif

#if NUM_PARAMS >= 9
	, uintptr_t arg8
#endif

#if NUM_PARAMS >= 10
	, uintptr_t arg9
#endif

);
