/*
 * FatComponents.h -- fat component and mixed mode support macros
 *
 * Copyright � 1995 by Chris Sears.  All rights reserved.
 *
 * See Tech Note QT 05 - Component Manager version 3.0 for more details.
 * For less details, see the Component Manager chapter in IM: More Macintosh Toolbox.
 *
 * � FAT_EQ handles a syntax conflict between THINK_C and MetroWerks
 *
 * 		pascal ComponentResult	TestFatProcedure(ComponentInstance self, long arg)
 *			FAT_EQ ComponentCallNow(kTestFatProcedureSelect, 4);
 *
 * � FAT_uppCallComponentProcInfo is the ProcInfo for CallUniversalProc() used in FAT_GLUE_*
 * It probably won't be used anywhere outside of this file, but it really should be
 * in Components.h.  CallComponentUPP is an extern from InterfaceLib.
 * The arguments in GlueParams are stacked backwards as per QT05.
 *
 * � The FAT_GLUE_* macros create the necessary component glue routines.
 *
 *		FAT_GLUE_1(TestFatProcedure,
 *			kTestFatProcedureSelect, ComponentInstance,
 *			long, arg)
 *
 * � FAT_ROUTINE allows compile time choice between resource descriptors and procedure names.
 *
 *		TrackControl(control_handle, event->where,
 *			FAT_ROUTINE((ControlActionUPP) &scrollbar_action_rd,
 *				(ControlActionUPP) ScrollbarActionProc));
 *
 * � FAT_DESCRIPTOR similarly is useful for rd/proc fields in tables/structures.
 *
 *		static AEOA_Handler aeoa_handlers[] = {
 *			{ typeWildCard, typeWildCard, FAT_DESCRIPTOR(uppOSLAccessorProcInfo, NotHandled) },
 *		};
 *
 * � FAT_RD_ALLOC is just a convenience macro.
 *
 *		#ifdef powerc
 *			FAT_RD_ALLOC(dispatcher_rd, fat_pi_Dispatcher_ProcInfo, main);
 *		#endif
 *
 * � The FAT_VOID_* and FAT_VALUE_* macros simplify creating procinfo enums.
 *
 *		#ifdef	powerc
 *		enum {
 *			FAT_VOID_4(importProcInfo, kPascalStackBased,
 *				short, AcquireRecord *, long *, short *),
 *		};
 *		#endif
 *
 * � The FAT_CALL_* macros macros simplify callback vectors for FAT calls
 * from the component back to the application.
 *
 *		FAT_CALL_4(import_upp, importProcInfo, acquireSelectorAbout,
 *			NIL, &psdata, &ie_result);
 */

#include <Components.h>

#ifdef	THINK_C
#define FAT_EQ		=
#else
#define FAT_EQ
#endif

#ifdef	powerc
enum {
	FAT_uppCallComponentProcInfo = kPascalStackBased
		| RESULT_SIZE(kFourByteCode)
		| STACK_ROUTINE_PARAMETER(1, kFourByteCode)
};

#define FAT_ROUTINE(rd, routine)				rd
#define FAT_DESCRIPTOR(proc_info, routine)		BUILD_ROUTINE_DESCRIPTOR(proc_info, routine)
#define FAT_RD_ALLOC(var, proc_info, routine)												\
	RoutineDescriptor var = BUILD_ROUTINE_DESCRIPTOR(proc_info, routine)

#define FAT_VOID_0(name, stack)																\
	name = stack																			\
		| kNoByteCode
#define FAT_VOID_1(name, stack, a1)															\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))
#define FAT_VOID_2(name, stack, a1, a2)														\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))
#define FAT_VOID_3(name, stack, a1, a2, a3)													\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))
#define FAT_VOID_4(name, stack, a1, a2, a3, a4)												\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))
#define FAT_VOID_5(name, stack, a1, a2, a3, a4, a5)											\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))
#define FAT_VOID_6(name, stack, a1, a2, a3, a4, a5, a6)										\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))
#define FAT_VOID_7(name, stack, a1, a2, a3, a4, a5, a6, a7)									\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))
#define FAT_VOID_8(name, stack, a1, a2, a3, a4, a5, a6, a7, a8)								\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))
#define FAT_VOID_9(name, stack, a1, a2, a3, a4, a5, a6, a7, a8, a9)							\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))									\
		| STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(a9)))
#define FAT_VOID_10(name, stack, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)					\
	name = stack																			\
		| kNoByteCode																		\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))									\
		| STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(a9)))									\
		| STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(a10)))

#define FAT_VALUE_0(name, stack, ret)														\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))
#define FAT_VALUE_1(name, stack, ret, a1)													\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))
#define FAT_VALUE_2(name, stack, ret, a1, a2)												\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))
#define FAT_VALUE_3(name, stack, ret, a1, a2, a3)											\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))
#define FAT_VALUE_4(name, stack, ret, a1, a2, a3, a4)										\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))
#define FAT_VALUE_5(name, stack, ret, a1, a2, a3, a4, a5)									\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))
#define FAT_VALUE_6(name, stack, ret, a1, a2, a3, a4, a5, a6)								\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))
#define FAT_VALUE_7(name, stack, ret, a1, a2, a3, a4, a5, a6, a7)							\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))
#define FAT_VALUE_8(name, stack, ret, a1, a2, a3, a4, a5, a6, a7, a8)						\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))
#define FAT_VALUE_9(name, stack, ret, a1, a2, a3, a4, a5, a6, a7, a8, a9)					\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))									\
		| STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(a9)))
#define FAT_VALUE_10(name, stack, ret, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10)				\
	name = stack																			\
		| RESULT_SIZE(SIZE_CODE(sizeof(ret)))												\
		| STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(a1)))									\
		| STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(a2)))									\
		| STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(a3)))									\
		| STACK_ROUTINE_PARAMETER(4, SIZE_CODE(sizeof(a4)))									\
		| STACK_ROUTINE_PARAMETER(5, SIZE_CODE(sizeof(a5)))									\
		| STACK_ROUTINE_PARAMETER(6, SIZE_CODE(sizeof(a6)))									\
		| STACK_ROUTINE_PARAMETER(7, SIZE_CODE(sizeof(a7)))									\
		| STACK_ROUTINE_PARAMETER(8, SIZE_CODE(sizeof(a8)))									\
		| STACK_ROUTINE_PARAMETER(9, SIZE_CODE(sizeof(a9)))									\
		| STACK_ROUTINE_PARAMETER(10, SIZE_CODE(sizeof(a10)))

#define FAT_CALL_0(func, proc_info)															\
		CallUniversalProc((UniversalProcPtr) func, proc_info)
#define FAT_CALL_1(func, proc_info, arg1)													\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1)
#define FAT_CALL_2(func, proc_info, arg1, arg2)												\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2)
#define FAT_CALL_3(func, proc_info, arg1, arg2, arg3)										\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3)
#define FAT_CALL_4(func, proc_info, arg1, arg2, arg3, arg4)									\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4)
#define FAT_CALL_5(func, proc_info, arg1, arg2, arg3, arg4, arg5)							\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5)
#define FAT_CALL_6(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6)						\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6)
#define FAT_CALL_7(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7)				\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define FAT_CALL_8(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)			\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#define FAT_CALL_9(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)	\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
#define FAT_CALL_10(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)	\
		CallUniversalProc((UniversalProcPtr) func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)

#define FAT_GLUE_0(proc_name, selector, self_type)											\
	pascal ComponentResult																	\
	proc_name(																				\
		self_type self)																		\
	{																						\
		typedef struct {																	\
			unsigned char		flags,														\
								param_size;													\
			short				what;														\
			ComponentInstance	ci;															\
		} GluePB;																			\
		GluePB glue_pb;																		\
		glue_pb.flags = 0;																	\
		glue_pb.param_size = 0;																\
		glue_pb.what = selector;															\
		glue_pb.ci = (ComponentInstance) self;												\
		return CallUniversalProc(CallComponentUPP,											\
			FAT_uppCallComponentProcInfo, &glue_pb);										\
	}

#define FAT_GLUE_1(proc_name, selector, self_type, t1, n1)									\
	pascal ComponentResult																	\
	proc_name(																				\
		self_type self,																		\
		t1 n1) 																				\
	{																						\
		typedef struct {																	\
			t1 n1;																			\
		} GlueParams;																		\
		typedef struct {																	\
			unsigned char		flags,														\
								param_size;													\
			short				what;														\
			GlueParams			params;														\
			ComponentInstance	ci;															\
		} GluePB;																			\
		GluePB glue_pb;																		\
		glue_pb.flags = 0;																	\
		glue_pb.param_size = sizeof(GlueParams);											\
		glue_pb.what = selector;															\
		glue_pb.params.n1 = n1;																\
		glue_pb.ci = (ComponentInstance) self;												\
		return CallUniversalProc(CallComponentUPP,											\
			FAT_uppCallComponentProcInfo, &glue_pb);										\
	}

#define FAT_GLUE_2(proc_name, selector, self_type, t1, n1, t2, n2)							\
	pascal ComponentResult																	\
	proc_name(																				\
		self_type self,																		\
		t1 n1,																				\
		t2 n2) 																				\
	{																						\
		typedef struct {																	\
			t2 n2;				/* backwards - see TN QT05 p. 8 */							\
			t1 n1;																			\
		} GlueParams;																		\
		typedef struct {																	\
			unsigned char		flags,														\
								param_size;													\
			short				what;														\
			GlueParams			params;														\
			ComponentInstance	ci;															\
		} GluePB;																			\
		GluePB glue_pb;																		\
		glue_pb.flags = 0;																	\
		glue_pb.param_size = sizeof(GlueParams);											\
		glue_pb.what = selector;															\
		glue_pb.params.n1 = n1;																\
		glue_pb.params.n2 = n2;																\
		glue_pb.ci = (ComponentInstance) self;												\
		return CallUniversalProc(CallComponentUPP,											\
			FAT_uppCallComponentProcInfo, &glue_pb);										\
	}

#define FAT_GLUE_3(proc_name, selector, self_type, t1, n1, t2, n2, t3, n3)					\
	pascal ComponentResult																	\
	proc_name(																				\
		self_type self,																		\
		t1 n1,																				\
		t2 n2,																				\
		t3 n3)																				\
	{																						\
		typedef struct {																	\
			t3 n3;																			\
			t2 n2;																			\
			t1 n1;																			\
		} GlueParams;																		\
		typedef struct {																	\
			unsigned char		flags,														\
								param_size;													\
			short				what;														\
			GlueParams			params;														\
			ComponentInstance	ci;															\
		} GluePB;																			\
		GluePB glue_pb;																		\
		glue_pb.flags = 0;																	\
		glue_pb.param_size = sizeof(GlueParams);											\
		glue_pb.what = selector;															\
		glue_pb.params.n1 = n1;																\
		glue_pb.params.n2 = n2;																\
		glue_pb.params.n3 = n3;																\
		glue_pb.ci = (ComponentInstance) self;												\
		return CallUniversalProc(CallComponentUPP,											\
			FAT_uppCallComponentProcInfo, &glue_pb);										\
	}
#else

#define FAT_ROUTINE(rd, routine)				routine
#define FAT_DESCRIPTOR(proc_info, routine)		routine

#define FAT_CALL_0(func, proc_info)															\
		(*(func))()
#define FAT_CALL_1(func, proc_info, arg1)													\
		(*(func))(arg1)
#define FAT_CALL_2(func, proc_info, arg1, arg2)												\
		(*(func))(arg1, arg2)
#define FAT_CALL_3(func, proc_info, arg1, arg2, arg3)										\
		(*(func))(arg1, arg2, arg3)
#define FAT_CALL_4(func, proc_info, arg1, arg2, arg3, arg4)									\
		(*(func))(arg1, arg2, arg3, arg4)
#define FAT_CALL_5(func, proc_info, arg1, arg2, arg3, arg4, arg5)							\
		(*(func))(arg1, arg2, arg3, arg4, arg5)
#define FAT_CALL_6(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6)						\
		(*(func))(arg1, arg2, arg3, arg4, arg5, arg6)
#define FAT_CALL_7(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7)				\
		(*(func))(arg1, arg2, arg3, arg4, arg5, arg6, arg7)
#define FAT_CALL_8(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)			\
		(*(func))(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
#define FAT_CALL_9(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)	\
		(*(func))(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9)
#define FAT_CALL_10(func, proc_info, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)	\
		(*(func))(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
#endif