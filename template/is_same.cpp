#include <iostream>
#include <type_traits>
int main() {
  std::cout << std::is_same<char,char>::value << std::endl;
  std::cout << std::is_same<char, char>()() << std::endl;
}
