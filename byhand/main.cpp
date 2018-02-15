#include <iostream>

#include "scala.h"

using namespace scala;
using namespace java::lang;
using namespace std;

int main(void) {

  // val s: String = new String("Hello")
  String s = new __String("Hello");

  // val a: Any = s
  Any a = (Any) s;

  // println(a.hashCode)
  cout << a->__vptr->hashCode(a) << endl;

  // val s2: String = o.toString()
  String s2 = a->__vptr->toString(a);

  // println(s2)
  cout << s2->data << endl;

  // println(s.length())
  cout << s->__vptr->length(s) << endl;

  // println(s.hashCode())
  cout << s->__vptr->hashCode(s) << endl;

  return 0;
}
