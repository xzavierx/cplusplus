# 函数参数规则
如果你只实现`void foo(X&)`;而没有实现`void foo(X&&)`,行为如同C++98：foo()可因lvalue但不能因rvalue调用
如果你实现`void foo(const X&);`, 没有实现`void foo(X&&)`, 行为如同C++98：foo()可因lvalue也可因rvalue被调用
如果你实现`void foo(X&&)`但没有实现`void foo(X&)`,foo()可因rvalue调用，但不能lvalue调用它。

# 返回值规则
你不需要也不应该move()返回值。
```cpp
X foo() {
  X x;
  return x;
}
```
x优先选择move构造，如果没有move构造，则选择copy构造函数。否则编译错误。