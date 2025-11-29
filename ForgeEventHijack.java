package client.hook;

import client.event.ForgeEvent;
import net.minecraftforge.eventbus.EventBus;
import net.minecraftforge.eventbus.api.Event;
import org.objectweb.asm.*;

import java.lang.reflect.Constructor;

public class ForgeEventHijack extends HookTemplate {
    @Override
    public Class<?> getTarget() {
        return EventBus.class;
    }

    @Override
    public ClassVisitor createVisitor(ClassWriter writer) {
        return new CV(Opcodes.ASM9, writer);
    }

    public static class CV extends ClassVisitor {
        public CV(int api, ClassVisitor classVisitor) {
            super(api, classVisitor);
        }

        @Override
        public MethodVisitor visitMethod(int access, String name, String descriptor, String signature, String[] exceptions) {
            MethodVisitor mv = super.visitMethod(access, name, descriptor, signature, exceptions);
            if ("post".equals(name)) {
                return new MV(api, mv);
            }
            return mv;
        }
    }

    public static class MV extends MethodVisitor {
        public MV(int api, MethodVisitor methodVisitor) {
            super(api, methodVisitor);
        }

        @Override
        public void visitCode() {
            super.visitCode();

            mv.visitMethodInsn(Opcodes.INVOKESTATIC, Type.getInternalName(Thread.class), "currentThread", Type.getMethodDescriptor(Type.getType(Thread.class)), false);
            mv.visitMethodInsn(Opcodes.INVOKEVIRTUAL, Type.getInternalName(Thread.class), "getContextClassLoader", Type.getMethodDescriptor(Type.getType(ClassLoader.class)), false);
            mv.visitLdcInsn(ForgeEvent.class.getName().replace("/", "."));
            mv.visitMethodInsn(Opcodes.INVOKEVIRTUAL, Type.getInternalName(ClassLoader.class), "loadClass", Type.getMethodDescriptor(Type.getType(Class.class), Type.getType(String.class)), false);

            mv.visitLdcInsn(1);
            mv.visitTypeInsn(Opcodes.ANEWARRAY, Type.getInternalName(Class.class));

            mv.visitInsn(Opcodes.DUP);
            mv.visitLdcInsn(0);
            mv.visitLdcInsn(Type.getType(Event.class));
            mv.visitInsn(Opcodes.AASTORE);

            mv.visitMethodInsn(Opcodes.INVOKEVIRTUAL, Type.getInternalName(Class.class), "getDeclaredConstructor", Type.getMethodDescriptor(Type.getType(Constructor.class), Type.getType(Class[].class)), false);

            mv.visitLdcInsn(1);
            mv.visitTypeInsn(Opcodes.ANEWARRAY, Type.getInternalName(Object.class));

            mv.visitInsn(Opcodes.DUP);
            mv.visitLdcInsn(0);
            mv.visitVarInsn(Opcodes.ALOAD, 1);
            mv.visitInsn(Opcodes.AASTORE);

            mv.visitMethodInsn(Opcodes.INVOKEVIRTUAL, Type.getInternalName(Constructor.class), "newInstance", Type.getMethodDescriptor(Type.getType(Object.class), Type.getType(Object[].class)), false);
            mv.visitInsn(Opcodes.POP);
        }
    }
}