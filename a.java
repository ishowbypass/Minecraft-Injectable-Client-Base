package a;//don't move

public class a {//don't rename

    public static void a() {//don't rename
        try {//leak exception to jni will hide exception info and crash jvm
            new Thread(() -> {//recommend create a new thread to avoid jni waiting for too long

            }).start();
        } catch (Throwable t) {
            t.printStackTrace();
        }
    }

    public static byte[] a(Class<?> capturedClass, byte[] itsData) {//don't rename
        try {//leak exception to jni will hide exception info and crash jvm
            return modifiedData;//modified by yourself
        } catch (Throwable t) {
            t.printStackTrace();
        }
        return itsData;
    }

    public static native int a(Class<?> cls);//don't rename
}