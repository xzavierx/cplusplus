#include <iostream>
#include <type_traits>
#include <string>

struct base_mapper {
};

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
  argument_mapper<0> _1;  
  std::cout << _1(1) << std::endl;

  argument_mapper<1> _2;  
  std::cout << _2(1, "helloworld") << std::endl;

  float a = 100.0;
  argument_mapper<2> _3;  
  std::cout << _3(1, "helloworld", std::ref(a)) << std::endl;
}
