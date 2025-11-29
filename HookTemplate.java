package client.hook;

import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;

public abstract class HookTemplate {

    public byte[] oldData;
    public byte[] newData;

    public byte[] edit(byte[] in) {
        this.oldData = in;
        ClassReader reader = new ClassReader(in);
        ClassWriter writer = new ClassWriter(reader, ClassWriter.COMPUTE_MAXS | ClassWriter.COMPUTE_FRAMES);
        reader.accept(createVisitor(writer), ClassReader.EXPAND_FRAMES);
        this.newData = writer.toByteArray();
        return this.newData;
    }

    public abstract Class<?> getTarget();

    public abstract ClassVisitor createVisitor(ClassWriter writer);
}