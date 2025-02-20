#include <iostream>
#include <type_traits>
#include <string>

struct base_mapper {
};

// 主模板
template <size_t Index>
struct argument_mapper : base_mapper {
  template<
    typename Arg,
    typename ...Args,
    typename = std::enable_if_t<(sizeof...(Args) >= Index)>>
  static constexpr decltype(auto) call(Arg &&arg, Args &&...args) {
    return argument_mapper<Index - 1>::call(
      std::forward<Args>(args)...);
  }

  template<
    typename ...Args,
    typename = std::enable_if_t<(sizeof...(Args) > Index)>>
  constexpr auto operator()(Args &&...args) const {
    return call(std::forward<Args>(args)...);
  }
};
// 特化模板，递归的终止条件，用于提取索引为0的参数
template<> 
struct argument_mapper<0> : base_mapper {
  template<
    typename Arg,
    typename ...Args>
  static constexpr decltype(auto) call(Arg &&arg, Args &&...args) {
    return std::forward<Arg>(arg);
  }

  template<
    typename Arg,
    typename ...Args>
  constexpr auto operator()(Arg &&arg, Args &&...args) const {
    return std::forward<Arg>(arg);
  }
};

int main() {
  // 提取第一个参数
  argument_mapper<0> _1;  
  std::cout << _1(1) << std::endl;

  // 提取第二个参数
  argument_mapper<1> _2;  
  std::cout << _2(1, "helloworld") << std::endl;

  // 提取第三个参数
  float a = 100.0;
  argument_mapper<2> _3;  
  std::cout << _3(1, "helloworld", std::ref(a)) << std::endl;
}
