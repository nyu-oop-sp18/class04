# Class 4

## Inheritance and Subtype Polymorphism

Class inheritance is a fundamental concept in almost all
object-oriented programming languages. It describes the ability to
have an object or class 'specialize' another one, inheriting parent
data and behavior. The subclass (i.e. the inheriting class) defines a
*subtype* of the type of its superclass (i.e. the parent class it
inherits from). Here, we think of the type of the class as the
collection of its members and their signatures (i.e. the fields of the
class with their types as well as its methods with their parameter and
return types).

The type of a subclass can extend the type of its superclass by adding
new members. Since the only way to interact with an object is by
accessing or calling its members, any operation that can be performed
with an object of the superclass can also be performed with an object
of the subclass. This leads to the notion of *subtype polymorphism*:
objects that belong to the subtype can be used whenever an object of
the supertype is expected. That is, one can think of the objects of
the subtype as forming a subset of the objects of the supertype. For
example, consider the following code snippet:

```scala
class A(val x: Int)
class B(x0: Int, val y: Int) extends A(x0)

def f(a: A): Int = a.x

f(new B(1, 2))
```

Class `B` extends class `A`, thus forming a subtype relationship
between the types of the two classes. In Scala, this subtype
relationship is expressed by the notation `B <: A`.

Since `B` is a subtype of `A`, it is OK to call the function `f`,
which expects an `A` with a `B` object instead. In particular, the
access to the field `x` of `a` in the body of `f` can be safely
executed on `B` instances because class `B` inherits all members of
class `A`, including field `x`.

## Static vs. Dynamic Types and Dynamic Dispatch

A superclass `A` is open for extension, i.e., it allows behavior to be
extended without modifying `A`'s source code by adding and overriding
methods in the subclasses of `A`. To understand the semantics of calls
to overridden methods, we have to understand the difference between
*static* and *dynamic* types.

The static type of an expression in a program is the type that the
compiler infers for that expression at compile-time. On the other
hand, the dynamic type of an expression is the actual type of the
value obtained when the expression is evaluated at run-time. For
instance, consider the following code snippet:

```scala
class A(val x: Int) {
  def m(): Int = x
}
class B(x0: Int, val y: Int) extends A(x0) {
  override def m(): Int = x + y
}

def f(a: A) = a.m()

val a: A = new B(1, 2)
a.m()
```

The static type of `a` in the last line is `A`. The compiler infers
this type from the type annotation in the declaration of `a` on the
previous line. On the other hand, the dynamic type of `a` on the last
line is `B` since when `a` is evaluated at run-time, it refers to the
`B` instance created on the previous line. 

The behavior of a call to an overridden method such as `m` on the last
line is determined by the dynamic type of the receiver expression of
the method call. The call goes to the most recent implementation of
the method in the subtype hierarchy, starting from the dynamic type of
the receiver. Thus, in the example, the call `a.m()` on the last line
goes to `B.m` and not `A.m`. The last line therefore evaluates to `3`
and not `1`. This semantics of method calls is referred to as *dynamic
dispatch*. Methods that are dynamically dispatched are also called
*virtual methods*. In Scala, all public and protected methods of
classes are virtual by default whereas private methods are non-virtual.

Note that a receiver expression can have more than one dynamic
type. For instance, if we call the function `f` with an `A` instance,
the dynamic type of `a` in `f` for this call will be `A` and the call
`a.m()` in the body of `f` will go to `A.m`. On the other hand, if we
call `f` with a `B` instance, then the dynamic type of `a` in `f` for
this call will be `B` and the call to `a.m()` in the body of `f` will
go to `B.m`.

## Implementing Subtype Polymorphism and Dynamic Dispatch by Hand

In the following, we will implement subtype polymorphism and dynamic
dispatch by hand, leveraging techniques similar to what compilers for
OOP languages do when they compile OOP code to executable machine code
or byte code.

The goal of this exercise is two-fold:

1. You will obtain a better understanding of what happens when an OOP
   program is executed and what is the performance overhead associated
   with using certain OOP features.
   
1. You will learn how to simulate OOP techniques in languages that do
   not support object-oriented programming directly.

In our exercise, we will manually translate several classes from the
Scala class hierarchy into another programming language in such a way
that the obtained program does not rely on OOP features, yet behaves
the same way as the original program. We will use C++ as the target of
our translation. Of course, C++ is a full-fledged object-oriented
language in its own right. So in our exercise we will restrict
ourselves to a subset of C++ that does not use OOP features such as
class inheritance explicitly. Essentially, we will be using the C
subset of C++. We use C++ instead of C out of convenience. In
particular, we can use C++ namespaces to simulate the package
structure of a Scala program without too much effort.

Before we go into the details of the translation, we first have to
discuss two important concepts:

* Object data layout in memory

* Virtual method tables (aka *vtables*)

To implement virtual method dispatch efficiently we need to think
about the data layout of objects in memory. Towards that end, we will
first understand inheritance and virtual methods by looking at the
data layout of objects and vtables. Then we will review C++ code that
implements `scala.AnyRef`, `scala.String` and `java.lang.Class` so
that you can see what these classes look like when they are compiled
to low-level machine code or byte code to be executed in the JVM.

### Object Data Layout in Memory

In the following, we will answer these questions:

* For any given object, how is the data organized in memory?

* How does the run-time find a particular data member of an instance?

* How do these things work when dealing with inheritance hierarchies?

* In particular, how is dynamic dispatch realized?

To get started, consider the following simple Scala classes:

```scala
class A(val x: Int, val y: Int)
class B(x1: Int, y1: Int, val z: Int) extends A(x1, y1)
```

The data members of a class are laid out contiguously in memory for
each instance:

```
     A Instance:
   0┌─────────────┐
    │ value of x  │
   4├─────────────┤> members of A
    │ value of y  │
    └─────────────┘

     B Instance:
   0┌─────────────┐
    │ value of x  │
   4├─────────────┤> members of A
    │ value of y  │
   8├═════════════┤
    │ value of z  │  additional members of B
    └─────────────┘
```

Note that:

* Each data member can be accessed via a fixed offset from the base
  address of the data layout. The offset is determined by the number
  of bytes needed to represent a value of the type of that data member
  (e.g. 4 bytes for `Int` values and 8 bytes for any type derived from
  `AnyRef` assuming a 64-bit architecture).

* Subclass objects have the same memory layout as superclass objects
  with additional space for the subclass members that succeeds the space
  for the superclass members.

* Objects of type `B` can be polymorphically operated on as if they
  were objects of type `A`, since the offsets of the subclass members
  are the same. E.g.  the expression `a.y` where `a` has static type
  `A` would translate to:
  
  1. Take the address stored in the reference `a`, which points to the
     base of an object data layout.
  
  2. Add to it the offset of field `y` in the data layout of `A`,
     which is 4.
     
  3. Dereference the resulting address to retrieve the value of field
     `y` in the object referred to by `a`.

  These steps also work if the dynamic type of `a` is `B` since the
  relative offset of the entry for `y` in the `B` data layout is the
  same as in the `A` data layout.

**Question:** If we have polymorphic data structures of variable
sizes, how should we pass the data? **Answer:** by reference. Hence in
Scala (and Java), all objects are passed by reference.

Note that because the data layouts of subclass instances are
compatible with the data layout of superclass instances, there is no
need for the run-time to check the dynamic type of instances in order
to access data members.

Now let's add some methods to `A` and `B`:
  
  ```scala
    class A(val x: Int, val y: Int) { 
      def m1() = { ... }
      def m2() = { ... }
    }
    class B(x1: Int, y1: Int, val z: Int) extends A(x1, y1) {
      override def m2() = { /* overriding A.m2 */ ... }
      def m3() = { ... }
    }
  ```
  
So should we take the same approach for methods as for data? That is,
for each method declared in a class, we could add an entry to the data
layout that stores a pointer to the implementation of that
method. When we override a method in a subclass, we simply change the
pointer at the appropriate entry in the subclass data layout to point
to the new implementation. This would give us the following
memory representation of an A and a B instance at run-time.

```
      A Instance:
    0┌─────────────┐
     │ value of x  │
    4├─────────────┤
     │ value of y  │
    8├─────────────┤                    ┌──────────────┐
     │ ptr. to m1  │───────────────────>│impl. of A.m1 │
   16├─────────────┤                    └──────────────┘
     │ ptr. to m2  │────────┐  ┌──────────────┐     ^
     └─────────────┘        └─>│impl. of A.m2 │     │
                               └──────────────┘     │
      B Instance:                                   │ 
    0┌─────────────┐                                │
     │ value of x  │                                │
    4├─────────────┤                                │
     │ value of y  │                                │
    8├─────────────┤                                │
     │ ptr. to m1  │────────────────────────────────┘ 
   16├─────────────┤           ┌─────────────┐
     │ ptr. to m2  │──────────>│impl. of B.m2│
   32├═════════════┤           └─────────────┘
     │ value of z  │
   36├─────────────┤           ┌─────────────┐
     │ ptr. to m3  │──────────>│impl. of B.m3│
     └─────────────┘           └─────────────┘
```

A call `a.m1()` would compile to

1. Take the base address of the data layout pointed to by `a`.

1. Add to the base address the relative offset of the pointer to `m1`
   in the data layout for `A`, which is 8.
   
1. Dereference to the resulting address to retrieve the pointer to the
   correct implementation of `m1`.
   
1. Call the method found at the retrieved address.

Again, this compiled code would work polymorphically for both `A` and
`B` instances and implement the dynamic dispatch correctly.

However, if we used this approach, then for every method added in a subclass
the size of each instance would grow by the size of one pointer. Consequently

* object creation would be slower, and

* memory consumption would be higher.

## Virtual Method Tables (vtables)

We can avoid wasting space for each method in each object instance by
adding an extra level of indirection. When a class defines a virtual
method, the compiler adds a hidden member variable to the data layout
of that class.  This hidden member variable is called the *virtual
pointer* (aka *vpointer*).

The vpointer points to the *virtual method table* (aka *vtable*). Each
class has its own vtable which is shared by all instances of that
class. The vtable is an array of pointers to functions that implement
the virtual methods of the class.

As with the object data layout, the vtable of a subclass has the same
layout as the vtable of its superclass, with additional entries for
the subclass' virtual methods appended to it. Thus, the offsets of the
pointers to the shared methods are the same in both vtables.

The vtable of a subclass is created by copying the entries from the
vtable of the superclass and changing the pointers of overridden
methods to point to the new implementations. At instance creation (at
runtime) the vpointer of the instance will be set to point to the
right vtable of the instance's class.

For our example, we would get the following memory representation at run-time

```
      A Instance:                A vtable:
    0┌─────────────┐        ┌> 0┌────────────┐                    ┌─────────────┐
     │ vptr        │────────┘   │ ptr. to m1 │───────────────────>│impl. of A.m1│
    8├─────────────┤           8├────────────┤                    └─────────────┘
     │ value of x  │            │ ptr. to m2 │────────┐  ┌─────────────┐   ^
   12├─────────────┤            └────────────┘        └─>│impl. of A.m2│   │
     │ value of y  │                                     └─────────────┘   │
     └─────────────┘                                                       │
                                                                           │
      B Instance:                B vtable:                                 │
    0┌─────────────┐        ┌> 0┌────────────┐                             │
     │ vptr        │────────┘   │ ptr. to m1 │─────────────────────────────┘ 
    8├─────────────┤           8├────────────┤           ┌─────────────┐
     │ value of x  │            │ ptr. to m2 │──────────>│impl. of B.m2│
   12├─────────────┤          16├════════════┤           └─────────────┘ 
     │ value of y  │            │ ptr. to m3 │────────┐  ┌─────────────┐
   16├═════════════┤            └────────────┘        └─>│impl. of B.m3│
     │ value of z  │                                     └─────────────┘
     └─────────────┘
```

Note that at run-time, the vpointer of all `A` instances will point to
the same vtable data structure, and similarly for `B`.

When a call to a virtual method is executed on an instance, the
runtime looks up the vtable of the instance's dynamic type via the
vpointer, and then looks up the method's implementation for that type
via the corresponding pointer in the vtable. The two pointer lookups
realize the *dynamic dispatch* of the method call.

Virtual methods thus add some runtime overhead:

* The data layout of each object grows by the size of one pointer (to
  store the vpointer).

* Each virtual method call involves a constant overhead of two pointer
  lookups compared to a regular function call (first, to retrieve the
  address of the right vtable, second to retrieve the address of the
  correct implementation to which the call should be dispatched).

* Due to the additional indirection of calling methods via pointers,
  the compiler also has fewer opportunities for applying static code
  optimizations such a inlining function calls. Though this is
  mitigated by just-in-time optimization techniques in modern JVM
  implementations.

### C++ Implementation of Inheritance

For our manual translation of Scala inheritance to C++ without
inheritance, we follow these guidelines: 

* For virtual methods we have a per class vtable containing pointers
  to the method implementations for that class.

* Every object has a vpointer pointing to the one vtable of its class.

* New methods added by a subclass extend the vtable, adding new
  pointers at the end.

* Overriden methods go into an existing slot.

* E.g. `AnyRef` and `String` have their own vtables, and `String`'s
  vtable is an overridden clone of `AnyRef`'s vtable.

* When we translate a Scala class that extends a superclass, we
  clone the superclass' vtable and add slots for any new
  methods that are *not private* to the vtable.

* Private methods don't go into the vtable because they are not
  virtual (i.e., can't be overridden and are therefore not dynamically
  dispatched).


We start by declaring types for our Scala classes with forward
declarations. `typedef` declarations are used to define the Scala type
names as pointers to `struct`s representing data layouts. In this way
the semantics of the object types are the same as in Scala.
  
  ```c++
    // Forward declarations of data layout and vtables.
    struct __Any;
    struct __Any_VT;

    struct __AnyRef;
    struct __AnyRef_VT;

    struct __String;
    struct __String_VT;

    struct __Class;
    struct __Class_VT;

    // Definition of type names, which are equivalent to Scala semantics,
    // i.e., an instance is the address of the object's data layout.
    typedef __Any* Any;
    typedef __AnyRef* AnyRef;
    typedef __Class* Class;
    typedef __String* String;
  ```

Note that a `struct` in C++ is just like a C++ `class` except that the
default visibility of members is `public` rather than `private`.

In our implementation we will ignore memory management. Scala provides
automatic memory management via garbage collection whereas C++ uses
semi-automatic memory management via destructors. Without accounting
for this difference in the semantics of the two languages, the
programs resulting from our translation scheme will leak
memory. That's OK for this exercise. A more faithful translation would
either have to implement a garbage collector, or use C++-style
automatic memory management based on smart pointers.

### Implementing `scala.AnyRef`

* Data layout of `scala.AnyRef` in C++

  ```c++
    struct __AnyRef {
      // The vpointer
      __AnyRef_VT* __vptr;
      // The constructor.
      __AnyRef();

      // The methods implemented by scala.AnyRef.
      static int32_t hashCode(AnyRef);
      static bool equals(AnyRef, Any);
      static Class getClass(AnyRef);
      static String toString(AnyRef);

      // The function returning the class object representing
      // scala.AnyRef.
      static Class __class();

      // The vtable for scala.AnyRef.
      static __AnyRef_VT __vtable;
    };
  ```
* `__AnyRef` has 
  
  * a `__vptr` field which is the vpointer to the virtual method table,
  * a static `__vtable` field which is the one vtable for all `AnyRef` instances,
  * static methods `hashCode`, `equals`, `getClass`, and `toString`
    for the implementations of the inbuilt methods of `AnyRef`
  * and a `__class` method which returns the class object representing
    the type of `AnyRef`.

* Note that the static methods such as `equals` that implement the
  built-in methods of type `AnyRef` all take an `AnyRef` as first
  argument. This first argument will be a pointer to the object on
  which the method is called (i.e., the implicit `this` parameter of
  an instance method in Scala).

* Further note that `Int` in Scala has 32 bits, but in C++ the size of
  type `int` depends on the architecture. So we specify `int32_t` for
  `hashCode`'s return type.

* We make `__class` a static method as opposed to a
  static field to avoid the possibility that it is initialized
  after other static variables that depend on it during initialization. 

* Vtable layout of `scala.AnyRef`:
  
  ```c++
    struct __AnyRef_VT {
      Class __is_a;
      int32_t (*hashCode)(AnyRef);
      bool (*equals)(AnyRef, Any);
      Class (*getClass)(AnyRef);
      String (*toString)(AnyRef);

      __Any_VT() 
        : __is_a(__AnyRef::__class()),
          hashCode(&__AnyRef::hashCode),
          equals(&__AnyRef::equals),
          getClass(&__AnyRef::getClass),
          toString(&__AnyRef::toString) {}
    };
  ```

* `__AnyRef_VT` has 
  * an `__is_a` property which points to the `Class` object for `AnyRef`, 
  * and function pointers to the methods of `scala.AnyRef`.
  
* For example, `int32_t (*hashCode)(AnyRef);` declares a pointer field
  `hashCode` to a function that takes an `AnyRef` as argument and
  returns an `int32_t`.

* The "no-argument" constructor `__AnyRef_VT()` stores the addresses
  of `__AnyRef`'s `hashCode`, `equals`, `getClass` and
  `toString` implementations in the appropriate fields of the vtable.

  * Notice how we use `&` to get the address of the functions.

### Implementing `scala.String`

* The data layout of `scala.String` in C++ 
  
  ```c++
    struct __String {
      // The vpointer (just like in __AnyRef)
      __String_VT* __vptr;
      // The field holding the actual data for representing this Scala string
      std::string data;

      // The constructor;
      __String(std::string data);

      // The methods implemented by scala.String.
      static int32_t hashCode(String);
      static bool equals(String, Any);
      static String toString(String);
      static int32_t length(String);
      static char charAt(String, int32_t);

      // The function returning the class object representing
      // scala.String.
      static Class __class();

      // The vtable for scala.String.
      static __String_VT __vtable;
    };
  ```

* `__String` is a clone of `__AnyRef` except that it adds a field
  `data` to represent the actual string (we use the C++ `std::string`
  type for this purpose), and also provides implementations for a
  `length` and a `charAt` method.

* **Important**: the fields that `String` inherits from its superclass
  `AnyRef` are declared first in `__String` and they appear **in the
  same order** as they appear in the superclasses data layout. New
  fields are declared after the inherited ones.

* The static methods `hashCode`, `equals`, and `toString` are
  redeclared, but now with `String` as the first argument for the
  implicit `this` parameter. These methods are the implementations
  that will override the corresponding method implementations of
  `AnyRef` in `String`'s vtable.
  
* There is no new implementation for `getClass` since `String` does not
  override the `getClass` method of `AnyRef`.

* The vtable for `scala.String` looks like this

  ```c++
   // The vtable layout for scala.String.
   struct __String_VT {
     Class __is_a;
     int32_t (*hashCode)(String);
     bool (*equals)(String, Any);
     Class (*getClass)(String);
     String (*toString)(String);
     int32_t (*length)(String);
     char (*charAt)(String, int32_t);

     __String_VT()
       : __is_a(__String::__class()),
         hashCode(&__String::hashCode),
         equals(&__String::equals),
         getClass( (Class(*)(String)) &__AnyRef::getClass),
         toString(&__String::toString),
         length(&__String::length),
         charAt(&__String::charAt) {
     }
   };
  ```

* Again, similar to `__AnyRef_VT` with additional slots for the new
  methods.
  
* **Important**: the pointer for the methods that `String` inherits
  from its superclass `AnyRef` are declared first in `__String_VT` and
  they appear **in the same order** as they appear in
  `__AnyRef_VT`. The pointers for the new methods `length` and
  `charAt` are declared after the inherited ones.
  
* Each vtable entry is initialized with function pointers pointing to
  the most recent version of the corresponding method, e.g., in
  `__String_VT`
  
  * `getClass` is initialized to point to `__AnyRef::getClass` since
    `String` does not override `getClass`.
    
  * all other methods are overriden in `String` or newly added in
    `String`, so we use the new implementations of these methods.
  
* The type of the first parameter for `__Any`’s `getClass` and
  `__String_V`’s `getClass` differ (the implicit `this`), so we need a
  cast to make the C++ compiler happy (since we don't use C++
  inheritance to express that `__String` is a subtype of `__AnyRef`,
  the compiler does not know that a `String` can be interpreted as an
  `AnyRef`).
  
### Implementing `java.lang.Class`

* We also need to define `java.lang.Class` because every object needs
  a class object, which is static and shared by all instances of the
  class.
  
* The `Class` objects are used to keep track of the dynamic types of
  objects at run-time. One would need this information to implement
  Scala's reflection mechanisms such as dynamic type checks for
  up-casts. We also use the `Class` objects to implement the generic
  default implementation of the `toString` method of `AnyRef`.

* Here is the data layout of `Class`.

  ```c++
    struct __Class {
      __Class_VT* __vptr;
      String name;
      Class parent;

      // The constructor.
      __Class(String name, Class parent);

      // The instance methods of java.lang.Class.
      static String toString(Class);
      static String getName(Class);
      static Class getSuperclass(Class);
      static bool isInstance(Class, AnyRef);

      // The function returning the class object representing
      // java.lang.Class.
      static Class __class();

      // The vtable for java.lang.Class.
      static __Class_VT __vtable;
    };
  ```

* `__Class` has a `name` field to denote the name of the class as well
  as a `parent` field to reference the parent class. The latter is
  used to implement the `getSuperclass` method, which returns a
  reference to an object's superclass.

* The vtable layout for `java.lang.Class`
  ```c++
    struct __Class_VT {
      Class __is_a;
      int32_t (*hashCode)(Class);
      bool (*equals)(Class, Any);
      Class (*getClass)(Class);
      String (*toString)(Class);
      String (*getName)(Class);
      Class (*getSuperclass)(Class);
      bool (*isInstance)(Class, AnyRef);

      __Class_VT()
        : __is_a(__Class::__class()),
          hashCode((int32_t(*)(Class))&__AnyRef::hashCode),
          equals((bool(*)(Class,Any))&__AnyRef::equals),
          getClass((Class(*)(Class))&__AnyRef::getClass),
          toString(&__Class::toString),
          getName(&__Class::getName),
          getSuperclass(&__Class::getSuperclass),
          isInstance(&__Class::isInstance) {}
    };
  ```

### Auxiliary Functions

* We need some auxiliary global functions

  * a function that returns the canonical `null` value

  * a function to convert a C string literal to a `scala::String`
    (This is needed so that C++ does not implicitly convert C strings to
    `std::string`)

```c++

scala::AnyRef null();

inline scala::String literal(const char * s) {
  // C++ implicitly converts the C string to a std::string.
  return new scala::__String(s);
}
```


### The method implementations in C++

```c++
// scala.AnyRef()
__AnyRef::__AnyRef() : __vptr(&__vtable) {}

// scala.AnyRef.hashCode()
int32_t __AnyRef::hashCode(Any __this) {
  return (int32_t)(intptr_t)__this;
}

// scala.AnyRef.equals(Any)
bool __AnyRef::equals(AnyRef __this, Any other) {
  return (Any) __this == other;
}

// scala.AnyRef.getClass()
Class __AnyRef::getClass(AnyRef __this) {
  return __this->__vptr->__is_a;
}

// scala.AnyRef.toString()
String __AnyRef::toString(AnyRef __this) {
  // Class k = this.getClass();
  Class k = __this->__vptr->getClass(__this);

  std::ostringstream sout;
  sout << k->__vptr->getName(k)->data
       << '@' << std::hex << (uintptr_t)__this;
  return new __String(sout.str());
}

// Internal accessor for scala.AnyRef's class.
Class __AnyRef::__class() {
  static Class k =
    new __Class(__rt::literal("java.lang.Object"), (Class) null());
  return k;
}

// The vtable for scala.AnyRef.  Note that this definition
// invokes the default no-arg constructor for __AnyRef_VT.
__AnyRef_VT __AnyRef::__vtable;
```

A few notes:

* `hashCode` and the other methods take `this` as an implicit
  parameter. Since `this` is a reserved keyword in C++, we use
  `__this` as a reference to the instance receiving the method call.

* For `hashCode` we use the address of the object stored in the
  `__this` parameter as the unique hash code of the instance. However,
  we cannot cast `__this` directly to `int32_t` because that would not
  work on 64-bit architectures. So we cast first to `intptr_t` and
  then to `int32_t`.

* See the rest of the code in `scala.cpp` and `main.cpp`

* The implementation of `__Class` is important because without it we
  would not be able to track the dynamic type of objects (which is
  needed for reflection). `Class` is what links objects in the
  inheritance hierarchy.

* `isInstance` traverses the inheritance hierarchy upwards (until it
  hits `null`) to determine whether an object is an instance of a
  given class.
  
  ```c++
    // scala.Class.isInstance(AnyRef)
    bool __Class::isInstance(Class __this, AnyRef o) {
      Class k = o->__vptr->getClass(o);

      do {
        if (__this->__vptr->equals(__this, (Any) k)) return true;
        k = k->__vptr->getSuperclass(k);
      } while ((Class) null() != k);

      return false;
    }
  ```

### Translating Simple Scala expressions to C++

* Variable declarations are translated almost literally. For example, the
  Scala declaration

  ```scala
  var s: String;
  ```
  
  will look like this in C++
  
  ```c++
  String s;
  ```
  
  Recall that the C++ type `String` is just a short-hand for the type
  `__String*`, i.e., a pointer to the data layout of a `String`
  instance.
  
* Object creation is also translated almost literally. For example,
  the Scala statement
  
  ```scala
  s = new String("hello");
  ```
  
  is translated to
  
  ```c++
  s = new __String("hello");
  ```
  
  Note that we call the constructor `__String` for the data layout of
  our `String` instances. In C++, the `new` operator returns a pointer
  to an instance of the type specified by the constructor following `new`.
  
* In Scala, the compiler will map string literals such as `"banana"`
  automatically to values of type `String`. The C++ compiler does not
  know about the Scala `String` type. So when we translate string
  literals that appear outside of `String` constructor calls, we have
  to manually wrap them in `String` objects. We can do this with the
  function `stringLiteral`.
  
  For example, the Scala statement
  
  ```scala
  s = "banana";
  ```
  
  is translated to
  
  ```c++
  s = stringLiteral("banana");
  ```
  
* Method calls are translated by looking up the vtable via the
  vpointer `__vptr` and then calling the appropriate method via the
  right function pointer. This implements dynamic dispatch. The first
  argument of the method call is always the receiver of the method
  call (i.e., the pointer to the instance from which we started the
  `__vptr` lookup). 
  
  For example, the Scala expression
  
  ```scala
  s.charAt(3)
  ```
  
  will be translated to
  
  ```c++
  s->__vptr->charAt(s, 3)
  ```
  
* When we translate down casts, we have to make the cast explicit
  because the C++ compiler does not know how the different Scala types
  relate. For example, the Scala declaration
  
  ```scala
  val a: Any = s;
  ```
  
  is translated to
  
  ```c++
  Any a = (Any) s;
  ```
    
For additional examples, see [main.cpp](https://github.com/nyu-oop-sp18/class04/blob/master/byhand/main.cpp)
  
To compile and run the C++ code, you need to install the build tool
`cmake` for building C/C++ projects. After installation, open a
terminal in the `byhand` directory and execute
  
```bash
  cmake .
  make
  ./scala-sim
```
  
It is instructive to play with the vtable and data layout declarations
and see how changes affect the execution of the main program. For
example, if we swap the order of the declarations of the function
pointers for `hashCode` and `length` in `__String_VT` like this:

```C++
   // The vtable layout for scala.String.
   struct __String_VT {
     Class __is_a;
     int32_t (*length)(String); // used to be hashCode
     bool (*equals)(String, Any);
     Class (*getClass)(String);
     String (*toString)(String);
     int32_t (*hashCode)(String); // used to be length
     char (*charAt)(String, int32_t);

     __String_VT()
       : __is_a(__String::__class()),
         hashCode(&__String::hashCode),
         equals(&__String::equals),
         getClass( (Class(*)(String)) &__AnyRef::getClass),
         toString(&__String::toString),
         length(&__String::length),
         charAt(&__String::charAt) {
     }
   };
```

then the call

```c++
a->__vptr->hashCode(a)
```

on line 18 of main.cpp will go to `__String::length` instead of
`__String::hash_code`. This is because the offset for the function
pointer lookup in the above call is calculated based on the static
type of `a` which is `Any`. Thus, the compiler assumes that
`a->__vptr` points to a `__Any_VT` and in this struct type, the field `hashCode` is
the first entry after field `__is_a`, whereas in `__String_VT`, this entry
now stores the pointer for `length`.
