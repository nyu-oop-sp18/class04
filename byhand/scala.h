#pragma once

#include <stdint.h>
#include <string>

// ==========================================================================

// To avoid the "static initialization order fiasco", we use functions
// instead of fields/variables for all pointer values that are statically
// initialized.

// See https://isocpp.org/wiki/faq/ctors#static-init-order

// ==========================================================================

// Forward declarations of data layout and vtables.
// This lets the compiler know that we have definitions forthcoming later in the program.
// See http://www.learncpp.com/cpp-tutorial/17-forward-declarations/
namespace scala {
  struct __Any;
  struct __Any_VT;

  struct __AnyRef;
  struct __AnyRef_VT;

  struct __String;
  struct __String_VT;

  // Definition of types that are equivalent to Scala semantics,
  // i.e., an instance is the address of the object's data layout.
  typedef __Any *Any;
  typedef __AnyRef *AnyRef;
  typedef __String *String;
}

namespace java {
  namespace lang {
    struct __Class;
    struct __Class_VT;

    typedef __Class *Class;
  }
}

// The function returning the canonical null value.
scala::AnyRef null();

namespace scala {
  // ======================================================================

  // The datalayout for scala.Any.
  // This struct is abstract and would never be instantiated.
  struct __Any {
    // A pointer to a vtable which could differ at runtime from the static
    // reference we have to the Any vtable below.
    __Any_VT *__vptr;
  };

  // The vtable layout for scala.Any.
  // Think of this as roughly the 'methods' of scala.Any.
  // This struct is abstract and would never be instantiated.
  struct __Any_VT {
    // The dynamic type, main.cpp will demonstrate this.
    java::lang::Class __is_a;

    // The vtable entries for the methods of Any objects.
    // These members are fields that store function pointers, the syntax:
    // ex:   int32_t     (*sum)          (int32_t, int32_t);
    //       return_type (*field_name)(arg_type_list);
    // See http://www.learncpp.com/cpp-tutorial/78-function-pointers/
    int32_t (*hashCode)(Any);
    bool (*equals)(Any, Any);
    String (*toString)(Any);
  };

  // ======================================================================


  // The data layout for scala.AnyRef (== java.lang.Object).
  struct __AnyRef {
    __AnyRef_VT *__vptr;

    // The constructor
    __AnyRef();

    // Some of the methods implemented by scala.AnyRef.
    static int32_t hashCode(AnyRef);
    static bool equals(AnyRef, Any);
    static String toString(AnyRef);
    static java::lang::Class getClass(AnyRef);

    // The function returning the class object representing scala.AnyRef.
    static java::lang::Class __class();

    // The vtable for scala.AnyRef itself.
    // Moreover, always a reference to the behaviours of scala.AnyRef.
    static __AnyRef_VT __vtable;
  };

  // The vtable layout for scala.String.
  struct __AnyRef_VT {
    // The dynamic type.
    java::lang::Class __is_a;

    // The vtable entries for the methods of AnyRef objects
    int32_t (*hashCode)(AnyRef);
    bool (*equals)(AnyRef, Any);
    String (*toString)(AnyRef);
    java::lang::Class (*getClass)(AnyRef);

    __AnyRef_VT()
        : __is_a(__AnyRef::__class()),
          hashCode(&__AnyRef::hashCode),
          equals(&__AnyRef::equals),
          toString(&__AnyRef::toString),
          getClass(&__AnyRef::getClass) {
    }
  };


  // ======================================================================

  // The data layout for scala.String (== java.lang.String).
  struct __String {
    __String_VT *__vptr;

    // The member that contains the actual string data.
    std::string data;

    // The constructor
    __String(std::string data);

    // Some of the methods implemented by scala.String.
    static int32_t hashCode(String);

    static bool equals(String, AnyRef);

    static String toString(String);

    static int32_t length(String);

    static char charAt(String, int32_t);

    // The function returning the class object representing scala.String.
    static java::lang::Class __class();

    // The vtable for scala.String.
    static __String_VT __vtable;
  };

  // The vtable layout for scala.String.
  struct __String_VT {
    // The dynamic type.
    java::lang::Class __is_a;

    // The vtable entries for the methods of String objects
    int32_t (*hashCode)(String);
    bool (*equals)(String, AnyRef);
    String (*toString)(String);
    java::lang::Class (*getClass)(String);
    int32_t (*length)(String);
    char (*charAt)(String, int32_t);

    __String_VT()
        : __is_a(__String::__class()),
          hashCode(&__String::hashCode),
          equals(&__String::equals),
          toString(&__String::toString),
          getClass((java::lang::Class(*)(String)) &__AnyRef::getClass), // "inheriting" getClass from AnyRef
          length(&__String::length),
          charAt(&__String::charAt) {
    }
  };
}

namespace java {
  namespace lang {

    // ======================================================================

    // Class is a little special in that all other classes will be 'composed' with
    // a Class instance. Its purpose is to encapsulate type information about a runtime 'instance'.
    // See http://docs.oracle.com/javase/8/docs/api/java/lang/Class.html

    // The data layout for java.lang.Class.
    struct __Class {
      __Class_VT *__vptr;
      scala::String name;
      Class parent;

      // The constructor.
      __Class(scala::String name, Class parent);

      // Some of the instance methods of java.lang.Class.
      static scala::String toString(Class);
      static scala::String getName(Class);
      static Class getSuperclass(Class);
      static bool isInstance(Class, scala::AnyRef);

      // The function returning the class object representing java.lang.Class.
      static Class __class();

      // The vtable for java.lang.Class.
      static __Class_VT __vtable;
    };

    // The vtable layout for java.lang.Class.
    struct __Class_VT {
      // The dynamic type.
      Class __is_a;

      // The vtable entries for the methods of Class objects
      int32_t (*hashCode)(Class);
      bool (*equals)(Class, scala::AnyRef);
      scala::String (*toString)(Class);
      Class (*getClass)(Class);
      scala::String (*getName)(Class);
      Class (*getSuperclass)(Class);
      bool (*isInstance)(Class, scala::AnyRef);

      __Class_VT()
          : __is_a(__Class::__class()),
            hashCode((int32_t(*)(Class)) &scala::__AnyRef::hashCode),
            equals((bool (*)(Class, scala::AnyRef)) &scala::__AnyRef::equals),
            toString(&__Class::toString),
            getClass((Class(*)(Class)) & scala::__AnyRef::getClass),
            getName(&__Class::getName),
            getSuperclass(&__Class::getSuperclass),
            isInstance(&__Class::isInstance) {
      }
    };
  }
}

// ==========================================================================

// Function for converting a C string literal to a translated Scala string.
inline scala::String stringLiteral(const char * s) {
  // C++ implicitly converts the C string to a std::string.
  return new scala::__String(s);
}
