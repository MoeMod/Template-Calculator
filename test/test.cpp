#include <iostream>
#include <string>
#include <sstream>

namespace tc
{

// 前置声明
template<class T1, class T2>
struct mul;

// 变量和常量
struct base_variable{};
struct base_const{};
struct x : base_variable{};
template<int n>
struct c : base_const
{
	static const int value = n;
};
template<int a, int b>
struct fraction : base_const
{

};
// 函数
template<class T>
struct sin{};
template<class T>
struct cos{};
template<class T>
struct tan{};
// 负号
template<class T>
struct neg{};

// 加法
template<class T1, class T2>
struct add{};
// 乘法
template<class T1, class T2>
struct mul{};
template<class T1, class T2>
struct div{};
/*
// 二元运算符顺序
template<template<class...> class...Args>
struct order_list{};
template<class List, class T>
struct order_list_find;
template<template<template<class...> class...Args> class List, template<class...> class T, template<class...> class...Args>
struct order_list_find<List<T, Args...>, T>
{
	static const int value = 0;
};
template<template<template<class...> class...Args> class List, template<class...> class T, template<class...> class...Args, template<class...> class First>
struct order_list_find<List<First, Args...>, T>
{
	static const int value = order_list_find<List<Args...>, T>::value + 1;
};
using binary_order = order_list<mul, add>;
template<template<class...> class F, template<class...> class G>
struct binary_order_before
{
	static const bool value = order_list_find<binary_order, F>::value < order_list_find<binary_order, G>::value;
};
*/
// 常量测试
/*template<class T>
struct is_const
{
	static const bool value = false;
};
template<int n>
struct is_const<c<n>>
{
	static const bool value = true;
};*/
template<class T>
using is_const = std::is_base_of<base_const, T>;
// 函数名测试
template<template<class...> class Target, class T>
struct is_function
{
	static const bool value = false;
};
template<template<class...> class Target, class...Args>
struct is_function<Target, Target<Args...>>
{
	static const bool value = true;
};

// 最大公约数
template<int a, int b>
struct gcd
{
	static const int value = gcd<b, a%b>::value;
};
template<int a>
struct gcd<a, 0>
{
	static const int value = a;
};

// 表达式化简（Simplify Expression）
template<class T, class Enable = void>
struct se
{
	using type = T;
};
template<class T>
struct can_simplify
{
	static const bool value = !std::is_same<T, typename se<T>::type>::value;
};
template<template<class> class UnaryFuntion, class T>
struct se<UnaryFuntion<T>, std::enable_if_t<(can_simplify<T>::value)>>
{
	using type = typename se<UnaryFuntion<typename se<T>::type>>::type;
};
template<template<class, class> class BinaryFuntion, class T1, class T2>
struct se<BinaryFuntion<T1, T2>, std::enable_if_t<(can_simplify<T1>::value || can_simplify<T2>::value)>>
{
	using type = typename se<BinaryFuntion<typename se<T1>::type, typename se<T2>::type>>::type;
};
// f(x) + c = c + f(x) 常数提前, f(x)!=c
template<class T1, int n>
struct se<add<T1, c<n>>, std::enable_if_t<!is_const<T1>::value && !can_simplify<T1>::value>>
{
	using type = typename se<add<c<n>, typename se<T1>::type>>::type;
};
// 0 + f(x) = f(x), f(x)!=c
template<class T2>
struct se<add<c<0>, T2>, std::enable_if_t<!is_const<T2>::value && !can_simplify<T2>::value>>
{
	using type = typename se<T2>::type;
};
// f(x) + 0 = 0, f(x)!=c
template<class T1>
struct se<add<T1, c<0>>, std::enable_if_t<!is_const<T1>::value && !can_simplify<T1>::value>>
{
	using type = typename se<T1>::type;
};
template<int a, int b>
struct se<add<c<a>, c<b>>>
{
	using type = c<a + b>;
};
// f(x)+f(x)=2*f(x)
template<class T>
struct se<add<T, T>, std::enable_if_t<!can_simplify<T>::value && !is_const<T>::value>>
{
	using type = typename se<mul<c<2>, typename se<T>::type>>::type;
};
// f(x)+(g(x)+h(x))=(f(x)+g(x))+h(x) 加法结合律，往左边合并
template<class F, class G, class H>
struct se<add<F, add<G, H>>, std::enable_if_t<!can_simplify<F>::value && !can_simplify<add<G, H>>::value && !is_function<add, F>::value>>
{
	using type = typename se<add<add<F, G>, H>>::type;
};
template<int n>
struct se < c<n>, std::enable_if_t<(n < 0)>>
{
	using type = neg<c<-n>>;
};
template<int n>
struct se < neg<c<n>>, std::enable_if_t < (n < 0) >>
{
	using type = c<-n>;
};
// -(-x) = x 一元去括号
template<class T>
struct se<neg<neg<T>>>
{
	using type = typename se<T>::type;
};
// -(a+b) = (-a)+(-b) 二元去括号
template<class T1, class T2>
struct se<neg<add<T1, T2>>, std::enable_if_t<!(can_simplify<T1>::value || can_simplify<T2>::value)>>
{
	using type = typename add<typename se<neg<T1>>::type, typename se<neg<T2>>::type>::type;
};
// f(x) * c = c * f(x) 常数提前, f(x)!=c
template<class T1, int n>
struct se<mul<T1, c<n>>, std::enable_if_t<!is_const<T1>::value && !is_function<add, T1>::value>>
{
	using type = typename se<mul<c<n>, typename se<T1>::type>>::type;
};
// f(x)*(g(x)*h(x))=(f(x)*g(x))*h(x) 乘法结合律，往左边合并
template<class F, class G, class H>
struct se<mul<F, mul<G, H>>, std::enable_if_t<!can_simplify<F>::value && !can_simplify<mul<G, H>>::value && !is_function<mul, F>::value>>
{
	using type = typename se<mul<mul<F, G>, H>>::type;
};
// 0*f(x)=0, f(x)!=c
template<class T2>
struct se<mul<c<0>, T2>, std::enable_if_t<!is_const<T2>::value>>
{
	using type = c<0>;
};
// 1*f(x)=f(x), f(x)!=c
template<class T2>
struct se<mul<c<1>, T2>, std::enable_if_t<!is_const<T2>::value>>
{
	using type = typename se<T2>::type;
};
template<int a, int b>
struct se<mul<c<a>, c<b>>>
{
	using type = typename c<a * b>;
};
// f(x)*(g(x)+h(x)) = f(x)*g(x)+f(x)*h(x) 左乘法分配律
template<class F, class G, class H>
struct se<mul<F, add<G, H>>, std::enable_if_t<!can_simplify<F>::value && !can_simplify<add<G, H>>::value >>
{
	using type = typename se<add<typename se<mul<F, G>>::type, typename se<mul<F, H>>::type>>::type;
};
// (g(x)+h(x))*f(x) = f(x)*g(x)+f(x)*h(x) 右乘法分配律
template<class F, class G, class H>
struct se<mul<add<G, H>, F>, std::enable_if_t<!is_function<add, F>::value && !is_function<mul, F>::value && !can_simplify<F>::value && !can_simplify<add<G, H>>::value >>
{
	using type = typename se<add<typename se<mul<F, G>>::type, typename se<mul<F, H>>::type>>::type;
};
// c1*f(x) + c2*f(x) = (c1+c2)*f(x) 合并同类项，当c1与c2可加
template<class T1, class T2, class F>
struct se<add<mul<T1, F>, mul<T2, F>>, std::enable_if_t<can_simplify<add<T1, T2>>::value && !std::is_same<T1, T2>::value>>
{
	using type = typename se<
		mul<typename se<add<T1, T2>>::type, typename se<F>::type>
	>::type;
};
// (f(x) + g(x)) + h(x) 逐个合并同类项
template<class F, class G, class H> // F与H可合并
struct se<add<add<F, G>, H>, std::enable_if_t<can_simplify<add<F, H>>::value && !can_simplify<add<F, G>>::value && !can_simplify<H>::value && !is_const<H>::value>>
{
	using type = typename se<
		add<typename se<add<F, H>>::type, typename se<G>::type>
	>::type;
};
template<class F, class G, class H> // G与H可合并
struct se<add<add<F, G>, H>, std::enable_if_t<can_simplify<add<G, H>>::value && !can_simplify<add<F, H>>::value && !can_simplify<add<F, G>>::value && !can_simplify<H>::value && !is_const<H>::value>>
{
	using type = typename se<
		add<typename se<add<G, H>>::type, typename se<F>::type>
	>::type;
};

// 求导数
template<class T>
struct der
{
	using type = der<T>;
};
// C' = 0
template<int n>
struct der<c<n>>
{
	using type = c<0>;
};
// x' = 1
template<>
struct der<x>
{
	using type = c<1>;
};
// (f(x)+g(x))' = f'(x) + g'(x)
template<class T1, class T2>
struct der<add<T1, T2>>
{
	using type = add< der<T1>, der<T2> >;
};
// [f(x)g(x)]'=f'(x)g(x) + f(x)g'(x)
template<class T1, class T2>
struct der<mul<T1, T2>>
{
	using dT1dx = der<T1>;
	using dT2dx = der<T2>;
	using type = add< 
		mul<dT1dx, T2>, 
		mul<T1, dT2dx> 
	>;
};
// sin'(f(x))= cos(f(x)) * f'(x)
template<class T>
struct der<sin<T>>
{
	using type = mul< cos<T>, der<T> >;
};


std::string to_str(x)
{
	return "x";
}
template<int n>
std::string to_str(c<n>)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}
template<class T1, class T2>
std::string to_str(mul<T1, T2>)
{
	return std::string("(") + to_str(T1()) + std::string("*") + to_str(T2()) + std::string(")");
}
template<class T>
std::string to_str(neg<T>)
{
	return std::string("-(") + to_str(T()) + std::string(")");
}
template<class T1, class T2>
auto to_str(add<T1, T2>) -> std::enable_if_t<(is_function<add, T1>::value && is_function<add, T2>::value), std::string>
{
	return to_str(T1()) + std::string("+") + to_str(T2());
}
template<class T1, class T2>
auto to_str(add<T1, T2>) -> std::enable_if_t<!(is_function<add, T1>::value && is_function<add, T2>::value), std::string>
{
	return std::string("(") + to_str(T1()) + std::string("+") + to_str(T2()) + std::string(")");
}

}
using namespace tc;
int main()
{
	using y = mul< add<mul<x, c<3>>, c<5>>, add<c<4>, mul<x, c<2>>>>;
	using z = mul< mul< y, y>, mul< mul< y, y>, y>>;
	std::cout << typeid(z).name() << std::endl;
	std::cout << typeid(se<z>::type).name() << std::endl;

	std::cout << to_str(z()) << std::endl;
	std::cout << to_str(se<z>::type()) << std::endl;
}