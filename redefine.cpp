// gen by Visual Studio
#include "pch.h"

// recommend use Visual Studio

// these all .h in your jdk install path
// move .h to same your dllmain.cpp path
#include "jni.h"
#include "jvmti.h"

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

jvmtiEnv *jvmti; // ensure this is unique
jclass patcher;	 // temp store LocalRef

// https://docs.oracle.com/javase/8/docs/platform/jvmti/jvmti.html#ClassFileLoadHook
// by Lefraudeur
void JNICALL ClassFileLoadHookProcessor(jvmtiEnv *jvmti, JNIEnv *jni,
										jclass captured_class, jobject its_own_classloader, const char *its_name, jobject its_protection_domain,
										jint its_data_len, const unsigned char *its_data, jint *replace_with_len, unsigned char **replace_with)
{
	if (!patcher)
		return;

	if (!captured_class)
		return; // dont delete!

	jbyteArray old = jni->NewByteArray(its_data_len);
	jni->SetByteArrayRegion(old, 0, its_data_len, reinterpret_cast<const jbyte *>(its_data));

	jbyteArray result = reinterpret_cast<jbyteArray>(jni->CallStaticObjectMethod(patcher, jni->GetStaticMethodID(patcher, "a", "(Ljava/lang/Class;[B)[B"), captured_class, old));

	unsigned char *new_patched;
	jsize new_size = jni->GetArrayLength(result);

	jvmti->Allocate(new_size, &new_patched); // Oracle allowed
	jni->GetByteArrayRegion(result, 0, new_size, reinterpret_cast<jbyte *>(new_patched));

	*replace_with = new_patched;
	*replace_with_len = new_size;

	patcher = 0;
}

void JNICALL enableCallback(jvmtiEnv *jvmti)
{
	jvmtiCapabilities capa{};
	jvmtiEventCallbacks callbacks{};
	jvmti->GetPotentialCapabilities(&capa);
	jvmti->AddCapabilities(&capa);
	callbacks.ClassFileLoadHook = ClassFileLoadHookProcessor;
	jvmti->SetEventCallbacks(&callbacks, sizeof(jvmtiEventCallbacks));
	jvmti->SetEventNotificationMode(JVMTI_ENABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, 0);
}

void JNICALL disableCallback(jvmtiEnv *jvmti)
{
	jvmtiCapabilities capa{};
	jvmtiEventCallbacks callbacks{};
	jvmti->GetPotentialCapabilities(&capa);
	jvmti->RelinquishCapabilities(&capa);
	jvmti->SetEventCallbacks(&callbacks, sizeof(jvmtiEventCallbacks));
	jvmti->SetEventNotificationMode(JVMTI_DISABLE, JVMTI_EVENT_CLASS_FILE_LOAD_HOOK, 0);
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
	disableCallback(jvmti);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	vm->GetEnv(reinterpret_cast<void **>(&jvmti), JVMTI_VERSION_1_2);
	enableCallback(jvmti);
	return JNI_VERSION_1_8;
}

extern "C"
{
	JNIEXPORT jint JNICALL Java_a_a_a(JNIEnv *jni, jclass caller, jclass target)
	{
		patcher = caller;
		return jvmti->RetransformClasses(1, &target);
	}
}