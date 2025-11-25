// gen by Visual Studio
#include "pch.h"

// recommend use Visual Studio

// these all .h in your jdk install path
// move .h to same your dllmain.cpp path
#include "jni.h"

#include "windows.h"
#include "commdlg.h"
#include "TlHelp32.h"

// ishowbypass(手淫) 2025 licensed by GPL3.0
// thanks to TheQmaks, radioegor146, Lefraudeur!!!
// maybe the simplest loader, this loader no need to parse class
// supports all minecraft version
// one-way calls minecraft, if you need events call from game, use Thread.contextClassLoader with reflect
// if you need unloading all class, then crossing ClassLoader.parent is not applicable
// didn't use any hook lib
// supports ClassLoader.getResource()
// entry is: a.a.a()V, also native methods is registered in there, int a(Class) triggers Retransform, byte[] a(Class, byte[]) receive and return modified class data
// UnregisterNatives will auto trigger when class unloaded, class will unload after its ClassLoader is unloaded. ClassLoader will unload the DLL loaded by its own class during unloading
// note that JNIEnv is bound to Java threads and Windows threads, and each JNIEnv has its own context

HMODULE dll; // for unload
PVOID veh;	 // for unload

void WINAPI FreeDLL()
{
	FreeLibrary(dll);
}

void JNICALL callEntryClassInOurClassLoaderAndOurJar(JNIEnv *jni, jobject ourClassLoader)
{
	jclass ClassLoader = jni->FindClass("java/lang/ClassLoader");
	jmethodID loadClass = jni->GetMethodID(ClassLoader, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

	jstring entryClassName = jni->NewStringUTF("a.a");
	jclass entryClass = reinterpret_cast<jclass>(jni->CallObjectMethod(ourClassLoader, loadClass, entryClassName));
	jmethodID entryMethod = jni->GetStaticMethodID(entryClass, "a", "()V");
	jni->CallStaticVoidMethod(entryClass, entryMethod);
}

void JNICALL createOurClassLoaderToLoadOurJar(JNIEnv *jni, jobject parentClassLoader)
{
	// var createdClassLoader = new URLClassLoader(new URL[]{new URL("file", "", jarPath)}, parentClassLoader);

	OPENFILENAMEW ofn = {sizeof(ofn)};
	WCHAR jarPath[MAX_PATH];
	ofn.lpstrFile = jarPath;
	ofn.nMaxFile = MAX_PATH;

	if (GetOpenFileNameW(&ofn))
	{
		jclass URL = jni->FindClass("java/net/URL");
		jmethodID URLInit = jni->GetMethodID(URL, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
		jobject url = jni->NewObject(URL, URLInit, jni->NewStringUTF("file"), jni->NewStringUTF(""), jni->NewString(reinterpret_cast<jchar *>(jarPath), wcslen(jarPath)));
		jobjectArray urlArray = jni->NewObjectArray(1, URL, url);

		jclass URLClassLoader = jni->FindClass("java/net/URLClassLoader");
		jmethodID URLClassLoaderInit = jni->GetMethodID(URLClassLoader, "<init>", "([Ljava/net/URL;Ljava/lang/ClassLoader;)V");

		jobject createdClassLoader = jni->NewObject(URLClassLoader, URLClassLoaderInit, urlArray, parentClassLoader);

		callEntryClassInOurClassLoaderAndOurJar(jni, createdClassLoader);
	}
}

void JNICALL findGameJavaThreadToGetThreadContextClassLoader(JNIEnv *jni)
{
	// createClassLoader(Thread.currentThread().getContextClassLoader());

	jclass Thread = jni->FindClass("java/lang/Thread");
	jmethodID currentThread = jni->GetStaticMethodID(Thread, "currentThread", "()Ljava/lang/Thread;");
	jmethodID getContextClassLoader = jni->GetMethodID(Thread, "getContextClassLoader", "()Ljava/lang/ClassLoader;");

	createOurClassLoaderToLoadOurJar(jni, jni->CallObjectMethod(jni->CallStaticObjectMethod(Thread, currentThread), getContextClassLoader));
}

void WINAPI enableBreakPoint(HANDLE thread)
{
	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	GetThreadContext(thread, &ctx);

	ctx.Dr0 = reinterpret_cast<DWORD64>(GetProcAddress(GetModuleHandleW(L"jvm.dll"), "JVM_CurrentTimeMillis"));
	ctx.Dr7 |= 1;

	SetThreadContext(thread, &ctx);
}

void WINAPI disableBreakPoint(HANDLE thread)
{
	CONTEXT ctx = {};
	ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;

	GetThreadContext(thread, &ctx);

	ctx.Dr0 = 0;
	ctx.Dr7 &= ~1;

	SetThreadContext(thread, &ctx);
}

LONG WINAPI vehThatToBeRegistered(PEXCEPTION_POINTERS ExceptionInfo)
{
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP)
	{
		if (ExceptionInfo->ExceptionRecord->ExceptionAddress == reinterpret_cast<PVOID>(GetProcAddress(GetModuleHandleW(L"jvm.dll"), "JVM_CurrentTimeMillis")))
		{
			// unhook
			ExceptionInfo->ContextRecord->Dr0 = 0;
			ExceptionInfo->ContextRecord->Dr7 &= ~1;

			disableBreakPoint(GetCurrentThread());

			RemoveVectoredExceptionHandler(veh);

			// https://learn.microsoft.com/zh-cn/cpp/build/x64-calling-convention
			// x64 calling convention, JNIEnv is in parameter 1, so its rcx
			findGameJavaThreadToGetThreadContextClassLoader(reinterpret_cast<JNIEnv *>(ExceptionInfo->ContextRecord->Rcx));

			CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(FreeDLL), 0, 0, 0);

			return EXCEPTION_CONTINUE_EXECUTION;
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

// hardware breakpoint is thread sensitive, fortunately JVM threads is correspond to Windows threads
void WINAPI findNativeThreadAndSetBreakPointToHookJNIEnvPtr()
{
	if (HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId()))
	{
		THREADENTRY32 threadInfo = {};
		threadInfo.dwSize = sizeof(THREADENTRY32);

		if (Thread32First(snapshot, &threadInfo))
		{
			do // there nesting not my bad
			{
				if (threadInfo.th32OwnerProcessID == GetCurrentProcessId())
				{
					if (HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION | THREAD_GET_CONTEXT | THREAD_SET_CONTEXT, 0, threadInfo.th32ThreadID))
					{
						if (PWSTR threadDescription; SUCCEEDED(GetThreadDescription(thread, &threadDescription)))
						{
							if (wcscmp(threadDescription, L"Render thread") == 0)
							{
								if (veh = AddVectoredExceptionHandler(true, vehThatToBeRegistered))
								{
									enableBreakPoint(thread);
								}
							}
							LocalFree(threadDescription);
						}
						CloseHandle(thread);
					}
				}
			} while (Thread32Next(snapshot, &threadInfo));
		}
		CloseHandle(snapshot);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		dll = hModule;
		DisableThreadLibraryCalls(hModule);
		if (HANDLE thread = CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(findNativeThreadAndSetBreakPointToHookJNIEnvPtr), 0, 0, 0))
		{
			CloseHandle(thread);
		}
	}
	return TRUE;
}