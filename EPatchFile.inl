#if NUM_PARAMS > 10
#error EPatchFile: too many params
#endif

#define TOKENPASTE(p, n, s, ss) p ## n ## s ## ss
#define TOKENPASTE2(p, n, s, ss) TOKENPASTE(p, n, s, ss)

#undef TYPE_NAME
#define TYPE_NAME TOKENPASTE2(FUNC_, NUM_PARAMS, PARAMS, SUFFIX_CONVENTION_UPPER) 

typedef uintptr_t (* TYPE_NAME)(

#if NUM_PARAMS == 0
	void
#endif

#if NUM_PARAMS >= 1
	uintptr_t
#endif

#if NUM_PARAMS >= 2
	, uintptr_t
#endif

#if NUM_PARAMS >= 3
	, uintptr_t
#endif

#if NUM_PARAMS >= 4
	, uintptr_t
#endif

#if NUM_PARAMS >= 5
	, uintptr_t
#endif

#if NUM_PARAMS >= 6
	, uintptr_t
#endif

#if NUM_PARAMS >= 7
	, uintptr_t
#endif

#if NUM_PARAMS >= 8
	, uintptr_t
#endif

#if NUM_PARAMS >= 9
	, uintptr_t
#endif

#if NUM_PARAMS >= 10
	, uintptr_t
#endif

);

#define FUNCTION_NAME(name) TOKENPASTE2(name, NUM_PARAMS, SUFFIX_CONVENTION, )

uintptr_t CALLING_CONVENTION FUNCTION_NAME(EPatch::Log)(

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

)
{
	EPatch *patch;
    TYPE_NAME func;
	std::string buf;

    {
        MutexLocker lock(&log_mutex);

        patch = FindLogPatch((void *)FUNCTION_NAME(EPatch::Log));
        if (!patch)
        {
            DPRINTF("%s: FATAL cannot find patch.\n", FUNCNAME);
            exit(-1);
        }

        if (patch->log_before)
        {
            Utils::Sprintf(buf, true, "\n%s called.\n", patch->function.c_str());

            if (patch->log_ra)
            {
                Utils::Sprintf(buf, true, "Return address is %p. Relative: 0x%x\n", BRA(0), REL_ADDR32_2(BRA(0), patch));
				PrintStackTrace(8, buf);
            }
			
#if NUM_PARAMS >= 1			
			patch->LogParam(arg0, 0, buf);
#endif

#if NUM_PARAMS >= 2
            patch->LogParam(arg1, 1, buf);
#endif

#if NUM_PARAMS >= 3
            patch->LogParam(arg2, 2, buf);
#endif

#if NUM_PARAMS >= 4
            patch->LogParam(arg3, 3, buf);
#endif

#if NUM_PARAMS >= 5
            patch->LogParam(arg4, 4, buf);
#endif

#if NUM_PARAMS >= 6
            patch->LogParam(arg5, 5, buf);
#endif

#if NUM_PARAMS >= 7
            patch->LogParam(arg6, 6, buf);
#endif

#if NUM_PARAMS >= 8
            patch->LogParam(arg7, 7, buf);
#endif

#if NUM_PARAMS >= 9
            patch->LogParam(arg8, 8, buf);
#endif

#if NUM_PARAMS >= 10
            patch->LogParam(arg9, 9, buf);
#endif
        }

        func = (TYPE_NAME)patch->log_original_func;
    }

    uintptr_t ret = func(
	
#if NUM_PARAMS >= 1
	arg0
#endif

#if NUM_PARAMS >= 2
	, arg1
#endif

#if NUM_PARAMS >= 3
	, arg2
#endif

#if NUM_PARAMS >= 4
	, arg3
#endif

#if NUM_PARAMS >= 5
	, arg4
#endif

#if NUM_PARAMS >= 6
	, arg5
#endif

#if NUM_PARAMS >= 7
	, arg6
#endif

#if NUM_PARAMS >= 8
	, arg7
#endif

#if NUM_PARAMS >= 9
	, arg8
#endif

#if NUM_PARAMS >= 10
	, arg9
#endif

);

    {
        MutexLocker lock(&log_mutex);

        if (!patch->log_before)
        {
            Utils::Sprintf(buf, true, "\n%s was called and has returned.\n", patch->function.c_str());

            if (patch->log_ra)
            {
                Utils::Sprintf(buf, true, "Return address is %p. Relative: 0x%x\n", BRA(0), REL_ADDR32_2(BRA(0), patch));
            }
			
#if NUM_PARAMS >= 1			
			patch->LogParam(arg0, 0, buf);
#endif

#if NUM_PARAMS >= 2
            patch->LogParam(arg1, 1, buf);
#endif

#if NUM_PARAMS >= 3
            patch->LogParam(arg2, 2, buf);
#endif

#if NUM_PARAMS >= 4
            patch->LogParam(arg3, 3, buf);
#endif

#if NUM_PARAMS >= 5
            patch->LogParam(arg4, 4, buf);
#endif

#if NUM_PARAMS >= 6
            patch->LogParam(arg5, 5, buf);
#endif

#if NUM_PARAMS >= 7
            patch->LogParam(arg6, 6, buf);
#endif

#if NUM_PARAMS >= 8
            patch->LogParam(arg7, 7, buf);
#endif

#if NUM_PARAMS >= 9
            patch->LogParam(arg8, 8, buf);
#endif

#if NUM_PARAMS >= 10
            patch->LogParam(arg9, 9, buf);
#endif
        }
		
		patch->LogParam(ret, -1, buf);
        DPRINTF("%s\n", buf.c_str());
    }
	
    return ret;
}