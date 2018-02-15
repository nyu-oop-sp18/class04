#include "scala.h"

#include <stdexcept>
#include <sstream>

// The function returning the canonical null value.
scala::AnyRef null() {
  static scala::AnyRef value(0); // init the pointer type to 0, the 'null pointer'
  return value;
}

namespace scala {

  // scala.AnyRef()
  __AnyRef::__AnyRef() : __vptr(&__vtable) {}


  // scala.AnyRef.hashCode()
  int32_t __AnyRef::hashCode(AnyRef __this) {
    return (int32_t) (intptr_t) __this;
  }

  // scala.Any.equals(Any)
  bool __AnyRef::equals(AnyRef __this, Any other) {
    return (Any) __this == other;
  }

  // scala.AnyRef.toString()
  String __AnyRef::toString(AnyRef __this) {
    // Class k = this.getClass();
    java::lang::Class k = __this->__vptr->getClass(__this);

    std::ostringstream sout;
    sout << k->__vptr->getName(k)->data
         << '@' << std::hex << (uintptr_t) __this;
    return new __String(sout.str());
  }

  // scala.AnyRef.getClass()
  java::lang::Class __AnyRef::getClass(AnyRef __this) {
    return __this->__vptr->__is_a;
  }

  // Internal accessor for scala.AnyRef's class.
  java::lang::Class __AnyRef::__class() {
    // static variable k is initialized on first call to __AnyRef::__class()
    // subsequent calls reuse the initial value of k
    static java::lang::Class k =
        new java::lang::__Class(stringLiteral("java.lang.Object"), (java::lang::Class) null());
    return k;
  }

  // The vtable for scala.AnyRef.  Note that this definition
  // invokes the default no-arg constructor for __AnyRef_VT.
  __AnyRef_VT __AnyRef::__vtable;

  // =======================================================================

  // scala.String(<literal>)
  __String::__String(std::string data)
      : __vptr(&__vtable),
        data(data) {
  }

  // scala.String.hashCode()
  int32_t __String::hashCode(String __this) {
    int32_t hash = 0;

    // Use a C++ iterator to access string's characters.
    for (std::string::iterator itr = __this->data.begin();
         itr < __this->data.end();
         itr++) {
      hash = 31 * hash + *itr;
    }

    return hash;
  }

  // scala.String.equals()
  bool __String::equals(String __this, AnyRef o) {
    // Make sure object is a string:
    // if (! o instanceof String) return false;
    java::lang::Class k = __String::__class();
    if (!k->__vptr->isInstance(k, o)) return false;

    // Do the actual comparison.
    String other = (String) o; // Downcast.
    return __this->data.compare(other->data) == 0;
  }

  // scala.String.toString()
  String __String::toString(String __this) {
    return __this;
  }

  // scala.String.length()
  int32_t __String::length(String __this) {
    return __this->data.length();
  }

  // scala.String.charAt()
  char __String::charAt(String __this, int32_t idx) {
    if (0 > idx || (unsigned) idx >= __this->data.length()) {
      throw std::out_of_range("Index out of bounds for string " + __this->data);
    }

    // Use std::string::operator[] to get character without
    // duplicate range check.
    return __this->data[idx];
  }

  // Internal accessor for scala.String's class.
  java::lang::Class __String::__class() {
    static java::lang::Class k =
        new java::lang::__Class(stringLiteral("java.lang.String"), __AnyRef::__class());
    return k;
  }

  // The vtable for scala.String.  Note that this definition
  // invokes the default no-arg constructor for __String_VT.
  __String_VT __String::__vtable;

  // =======================================================================
}

namespace java {
  namespace lang {
    // java.lang.Class(String, Class)
    __Class::__Class(scala::String name, Class parent)
        : __vptr(&__vtable),
          name(name),
          parent(parent) {
    }

    // java.lang.Class.toString()
    scala::String __Class::toString(Class __this) {
      return new scala::__String("class " + __this->name->data);
    }

    // java.lang.Class.getName()
    scala::String __Class::getName(Class __this) {
      return __this->name;
    }

    // java.lang.Class.getSuperclass()
    Class __Class::getSuperclass(Class __this) {
      return __this->parent;
    }

    // java.lang.Class.isInstance(AnyRef)
    bool __Class::isInstance(Class __this, scala::AnyRef o) {
      // isInstance traverses the inheritance hierarchy upwards
      // (until it hits null) to determine whether an object
      // is an instance of a given class
      Class k = o->__vptr->getClass(o);

      do {
        if (__this->__vptr->equals(__this, (scala::AnyRef) k)) return true;
        k = k->__vptr->getSuperclass(k);
      } while ((Class) null() != k);

      return false;
    }

    // Internal accessor for java.lang.Class' class.
    Class __Class::__class() {
      static Class k =
          new __Class(stringLiteral("java.lang.Class"), scala::__AnyRef::__class());
      return k;
    }

    // The vtable for java.lang.Class.  Note that this definition
    // invokes the default no-arg constructor for __Class_VT.
    __Class_VT __Class::__vtable;
  }
}


