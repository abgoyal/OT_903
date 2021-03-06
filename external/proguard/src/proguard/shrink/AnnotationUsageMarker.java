
package proguard.shrink;

import proguard.classfile.*;
import proguard.classfile.attribute.Attribute;
import proguard.classfile.attribute.annotation.*;
import proguard.classfile.attribute.annotation.visitor.*;
import proguard.classfile.attribute.visitor.AttributeVisitor;
import proguard.classfile.constant.*;
import proguard.classfile.constant.visitor.ConstantVisitor;
import proguard.classfile.util.SimplifiedVisitor;
import proguard.classfile.visitor.*;

public class AnnotationUsageMarker
extends      SimplifiedVisitor
implements   AttributeVisitor,
             AnnotationVisitor,
             ElementValueVisitor,
             ConstantVisitor,
             ClassVisitor,
             MemberVisitor
{
    private final UsageMarker usageMarker;

    // Fields acting as a return parameters for several methods.
    private boolean attributeUsed;
    private boolean annotationUsed;
    private boolean elementValueUsed;
    private boolean classUsed;
    private boolean methodUsed;


    /**
     * Creates a new AnnotationUsageMarker.
     * @param usageMarker the usage marker that is used to mark the classes
     *                    and class members.
     */
    public AnnotationUsageMarker(UsageMarker usageMarker)
    {
        this.usageMarker = usageMarker;
    }


    // Implementations for AttributeVisitor.

    public void visitAnyAttribute(Clazz clazz, Attribute attribute) {}


    public void visitAnyAnnotationsAttribute(Clazz clazz, AnnotationsAttribute annotationsAttribute)
    {
        // Mark the necessary annotation information.
        attributeUsed = false;
        annotationsAttribute.annotationsAccept(clazz, this);

        if (attributeUsed)
        {
            // We got a positive used flag, so some annotation is being used.
            // Mark this attribute as being used as well.
            usageMarker.markAsUsed(annotationsAttribute);

            markConstant(clazz, annotationsAttribute.u2attributeNameIndex);
        }
    }


    public void visitAnyParameterAnnotationsAttribute(Clazz clazz, Method method, ParameterAnnotationsAttribute parameterAnnotationsAttribute)
    {
        // Mark the necessary annotation information.
        attributeUsed = false;
        parameterAnnotationsAttribute.annotationsAccept(clazz, method, this);

        if (attributeUsed)
        {
            // We got a positive used flag, so some annotation is being used.
            // Mark this attribute as being used as well.
            usageMarker.markAsUsed(parameterAnnotationsAttribute);

            markConstant(clazz, parameterAnnotationsAttribute.u2attributeNameIndex);
        }
    }


    public void visitAnnotationDefaultAttribute(Clazz clazz, Method method, AnnotationDefaultAttribute annotationDefaultAttribute)
    {
        // Mark the necessary annotation information in any annotation elements.
        annotationDefaultAttribute.defaultValueAccept(clazz, this);

        // Always mark annotation defaults.
        usageMarker.markAsUsed(annotationDefaultAttribute);

        markConstant(clazz, annotationDefaultAttribute.u2attributeNameIndex);
    }


    // Implementations for AnnotationVisitor.

    public void visitAnnotation(Clazz clazz, Annotation annotation)
    {
        if (isReferencedClassUsed(annotation))
        {
            // Mark the annotation as being used.
            usageMarker.markAsUsed(annotation);

            markConstant(clazz, annotation.u2typeIndex);

            // Mark the necessary element values.
            annotation.elementValuesAccept(clazz, this);

            // The return values.
            annotationUsed = true;
            attributeUsed  = true;
        }
    }


    // Implementations for ElementValueVisitor.

    public void visitConstantElementValue(Clazz clazz, Annotation annotation, ConstantElementValue constantElementValue)
    {
        if (isReferencedMethodUsed(constantElementValue))
        {
            // Mark the element value as being used.
            usageMarker.markAsUsed(constantElementValue);

            markConstant(clazz, constantElementValue.u2elementNameIndex);
            markConstant(clazz, constantElementValue.u2constantValueIndex);

            // The return value.
            elementValueUsed = true;
        }
    }


    public void visitEnumConstantElementValue(Clazz clazz, Annotation annotation, EnumConstantElementValue enumConstantElementValue)
    {
        if (isReferencedMethodUsed(enumConstantElementValue))
        {
            // Check the referenced classes.
            classUsed = true;
            enumConstantElementValue.referencedClassesAccept(usageMarker);

            if (classUsed)
            {
                // Mark the element value as being used.
                usageMarker.markAsUsed(enumConstantElementValue);

                markConstant(clazz, enumConstantElementValue.u2elementNameIndex);
                markConstant(clazz, enumConstantElementValue.u2typeNameIndex);
                markConstant(clazz, enumConstantElementValue.u2constantNameIndex);

                // The return value.
                elementValueUsed = true;
            }
        }
    }


    public void visitClassElementValue(Clazz clazz, Annotation annotation, ClassElementValue classElementValue)
    {
        if (isReferencedMethodUsed(classElementValue))
        {
            // Check the referenced classes.
            classUsed = true;
            classElementValue.referencedClassesAccept(usageMarker);

            if (classUsed)
            {
                // Mark the element value as being used.
                usageMarker.markAsUsed(classElementValue);

                markConstant(clazz, classElementValue.u2elementNameIndex);
                markConstant(clazz, classElementValue.u2classInfoIndex);

                // The return value.
                elementValueUsed = true;
            }
        }
    }


    public void visitAnnotationElementValue(Clazz clazz, Annotation annotation, AnnotationElementValue annotationElementValue)
    {
        if (isReferencedMethodUsed(annotationElementValue))
        {
            boolean oldAnnotationUsed = annotationUsed;

            // Check and mark the contained annotation.
            annotationUsed = false;
            annotationElementValue.annotationAccept(clazz, this);

            if (annotationUsed)
            {
                // Mark the element value as being used.
                usageMarker.markAsUsed(annotationElementValue);

                markConstant(clazz, annotationElementValue.u2elementNameIndex);

                // The return value.
                elementValueUsed = true;
            }

            annotationUsed = oldAnnotationUsed;
        }
    }


    public void visitArrayElementValue(Clazz clazz, Annotation annotation, ArrayElementValue arrayElementValue)
    {
        if (isReferencedMethodUsed(arrayElementValue))
        {
            boolean oldelementValueUsed = elementValueUsed;

            // Check and mark the contained element values.
            elementValueUsed = false;
            arrayElementValue.elementValuesAccept(clazz, annotation, this);

            if (elementValueUsed)
            {
                // Mark the element value as being used.
                usageMarker.markAsUsed(arrayElementValue);

                markConstant(clazz, arrayElementValue.u2elementNameIndex);

                // The return value.
                //elementValueUsed = true;
            }
            else
            {
                elementValueUsed = oldelementValueUsed;
            }
        }
    }


    // Implementations for ConstantVisitor.

    public void visitAnyConstant(Clazz clazz, Constant constant)
    {
        usageMarker.markAsUsed(constant);
    }


    public void visitClassConstant(Clazz clazz, ClassConstant classConstant)
    {
        classUsed = usageMarker.isUsed(classConstant);

        // Is the class constant marked as being used?
        if (!classUsed)
        {
            // Check the referenced class.
            classUsed = true;
            classConstant.referencedClassAccept(this);

            // Is the referenced class marked as being used?
            if (classUsed)
            {
                // Mark the class constant and its Utf8 constant.
                usageMarker.markAsUsed(classConstant);

                markConstant(clazz, classConstant.u2nameIndex);
            }
        }
    }


    // Implementations for ClassVisitor.

    public void visitProgramClass(ProgramClass programClass)
    {
        classUsed = usageMarker.isUsed(programClass);
    }


    public void visitLibraryClass(LibraryClass libraryClass)
    {
        classUsed = true;
    }


    // Implementations for MemberVisitor.

    public void visitProgramMethod(ProgramClass programClass, ProgramMethod programMethod)
    {
        methodUsed = usageMarker.isUsed(programMethod);
    }


    public void visitLibraryMethod(LibraryClass LibraryClass, LibraryMethod libraryMethod)
    {
        classUsed = true;
    }


    // Small utility methods.

    /**
     * Returns whether the annotation class has been marked as being used.
     */
    private boolean isReferencedClassUsed(Annotation annotation)
    {
        // Check if the referenced class is being used.
        classUsed = true;
        annotation.referencedClassAccept(this);

        return classUsed;
    }


    /**
     * Returns whether the annotation method has been marked as being used.
     */
    private boolean isReferencedMethodUsed(ElementValue elementValue)
    {
        // Check if the referenced method is being used.
        methodUsed = true;
        elementValue.referencedMethodAccept(this);

        return methodUsed;
    }


    /**
     * Marks the specified constant pool entry.
     */
    private void markConstant(Clazz clazz, int index)
    {
        if (index > 0)
        {
            clazz.constantPoolEntryAccept(index, this);
        }
    }
}
