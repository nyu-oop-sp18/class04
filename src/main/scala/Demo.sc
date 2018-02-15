
class A(private val x: Int, val y: Int) {
  
  def m1(): Int = y
  private def m2(): Int = x
}

class B(x1: Int, y1: Int, val z: Int) extends A(x1, y1) {
  
  override def m1(): Int = z
  def m3(): Int = x1 + y + z
}

def f(a: A) = a.m1()

f(new A(1, 2))

val a: A = new B(1, 2, 3)

f(a)