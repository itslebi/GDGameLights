//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: custom type traits helpers
//======================================================================================================================

#ifndef CPPUTILS_TYPETRAITS_INCLUDED
#define CPPUTILS_TYPETRAITS_INCLUDED


#include "Essential.hpp"

#include <type_traits>
#include <array>    // data, size, begin, end
#include <tuple>    // get_nth_type


// C++ named type categories in short:
//  * fundamental types     - void, nullptr, ints, floats, chars, bool
//  * scalar types          - ints, chars, enums, pointers
//  * aggregate types       - arrays, plain structs with no base classes, constructors and virtual methods
//  * trivial types         - scalar, trivially constructible (constructor does nothing) and copyable classes, aggregates of the previous
//  * standard layout types - scalar, classes without virtual methods or base classes, aggregates of the previous
//  * POD types             - trivial and standard layout


/// a syntax simplification for constraining template types
#define REQUIRES( ... ) typename std::enable_if< __VA_ARGS__, int >::type = 0


namespace fut {


template< typename T >
struct remove_cvref
{
    using type = typename std::remove_cv< typename std::remove_reference<T>::type >::type;
};

template< typename ... >
using void_t = void;


} // namespace fut


namespace own {


template< size_t typeIdx, typename ... Types >
struct get_nth_type
{
	using type = typename std::tuple_element< typeIdx, std::tuple< Types ... > >::type;
};

#if __cplusplus >= 201703L
template< typename T, typename ... Args >
struct is_any_of
{
	static constexpr bool value = (std::is_same< T, Args >::value || ...);
};
#endif

// const, volatile, reference

/// Compares bare value types without any const, volatile modifiers.
template< typename T1, typename T2 >
struct is_same_except_cv
	: std::is_same< typename std::remove_cv<T1>::type, typename std::remove_cv<T2>::type > {};

/// Compares value types without added reference.
template< typename T1, typename T2 >
struct is_same_except_ref
	: std::is_same< typename std::remove_reference<T1>::type, typename std::remove_reference<T2>::type > {};

/// Compares bare value types without any const, volatile modifiers and without reference.
template< typename T1, typename T2 >
struct is_same_except_cvref
	: std::is_same< typename fut::remove_cvref<T1>::type, typename fut::remove_cvref<T2>::type > {};

// type categories

/// This determines types that are convertible to integer and serializable using selected endianity rules.
template< typename T >
struct is_int_or_enum
{
	static constexpr bool value = std::is_integral<T>::value || std::is_enum<T>::value;
};

/// Determines whether a type can be interpreted and serialized as a byte (char, unsigned char, uint8_t, std::byte, ...)
template< typename T >
struct is_byte_alike
{
	static constexpr bool value = is_int_or_enum<T>::value && sizeof(T) == 1;
};

template< typename T >
struct is_character
{
	static constexpr bool value =
		std::is_same< T, char >::value
	 #if __cplusplus >= 202002L
		|| std::is_same< T, char, char8_t >::value
	 #endif
		|| std::is_same< T, char16_t >::value
		|| std::is_same< T, char32_t >::value
		|| std::is_same< T, wchar_t >::value;
};

// conditional types

/// For integer it returns the integer, and for enum it returns its underlying_type.
// https://stackoverflow.com/questions/56972288/metaprogramming-construct-that-returns-underlying-type-for-an-enum-and-integer-f
template< typename T >
struct int_type
{
	using type = typename std::conditional<
		/*if*/ std::is_enum<T>::value,
		/*then*/ std::underlying_type<T>,
		/*else*/ std::enable_if< std::is_integral<T>::value, T >
	>::type::type;
};

template< typename T1, typename T2 >
struct bigger_type
{
	using type = typename std::conditional<
		/*if*/ (sizeof(T1) >= sizeof(T2)),
		/*then*/ T1,
		/*else*/ T2
	>::type;
};

template< typename SrcT, typename DstT >
struct corresponding_constness
{
	using type = typename std::conditional<
		/*if*/ std::is_const< SrcT >::value,
		/*then*/ std::add_const< DstT >,
		/*else*/ std::remove_const< DstT >
	>::type::type;
};

// ranges and their properties

// https://stackoverflow.com/questions/13830158/check-if-a-variable-type-is-iterable
/// Whether a data structure is a range that can be iterated over.
template< typename T >
struct is_range
{
 private:

	template< typename T_, typename = void >
	struct _is_range : std::false_type {};

	template< typename T_ >
	struct _is_range< T_,
		fut::void_t< decltype( std::begin( std::declval< T_ & >() ) ), decltype( std::end( std::declval< T_ & >() ) ) >
	> : std::true_type {};

 public:

	static constexpr bool value = _is_range<T>::value;
};

/// Whether a container or range has its data stored contiguously in the memory.
template< typename T >
struct has_contiguous_data
{
 private:

	template< typename T_, typename = void >
	struct _has_contiguous_data : std::false_type {};

	template< typename T_ >
	struct _has_contiguous_data< T_,
		fut::void_t< decltype( fut::data( std::declval< T_ & >() ) ), decltype( fut::size( std::declval< T_ & >() ) ) >
	> : std::true_type {};

 public:

	static constexpr bool value = _has_contiguous_data<T>::value;
};

/// Whether a container can be resized (vector, string, ...)
template< typename T >
struct is_resizable
{
 private:

	template< typename T_, typename = void >
	struct _is_resizable : std::false_type {};

	template< typename T_ >
	struct _is_resizable< T_,
		fut::void_t< decltype( std::declval< T_ & >().resize( std::declval< size_t >() ) ) >
	> : std::true_type {};

 public:

	static constexpr bool value = _is_resizable<T>::value;
};

template< typename T >
struct is_contiguous_range
{
	static constexpr bool value = is_range<T>::value && has_contiguous_data<T>::value;
};
template< typename T >
struct is_resizable_range
{
	static constexpr bool value = is_range<T>::value && is_resizable<T>::value;
};

template< typename T >
struct range_element
{
	using type = typename std::remove_reference< decltype( *std::begin( std::declval< T & >() ) ) >::type;
};
template< typename T >
struct range_value
{
	using type = typename fut::remove_cvref< decltype( *std::begin( std::declval< T & >() ) ) >::type;
};

template< typename T, typename E >
struct is_range_of
{
 private:

	template< typename T_, typename E_, REQUIRES( is_range<T_>::value ) >
	static constexpr bool _is_range_of()
	{
		return std::is_same< typename range_element<T_>::type, E_ >::value;
	}

	template< typename T_, typename E_, REQUIRES( !is_range<T_>::value ) >
	static constexpr bool _is_range_of()
	{
		return false;
	}

 public:

	static constexpr bool value = _is_range_of< T, E >();
};

template< typename T >
struct is_trivial_range
{
 private:

	template< typename T_, REQUIRES( is_range<T_>::value ) >
	static constexpr bool _is_trivial_range()
	{
		return std::is_trivial< typename range_element<T_>::type >::value;
	}

	template< typename T_, REQUIRES( !is_range<T_>::value ) >
	static constexpr bool _is_trivial_range()
	{
		return false;
	}

 public:

	static constexpr bool value = _is_trivial_range<T>();
};

template< typename T >
struct is_range_of_byte_alikes
{
 private:

	template< typename T_, REQUIRES( is_range<T_>::value ) >
	static constexpr bool _is_range_of_byte_alikes()
	{
		return is_byte_alike< typename range_element<T_>::type >::value;
	}

	template< typename T_, REQUIRES( !is_range<T_>::value ) >
	static constexpr bool _is_range_of_byte_alikes()
	{
		return false;
	}

 public:

	static constexpr bool value = _is_range_of_byte_alikes<T>();
};

template< typename T >
struct is_c_array
{
 private:

	template< typename T_ >
	struct _is_c_array : std::false_type {};

	template< typename T_, size_t N >
	struct _is_c_array< T_ [N] > : std::true_type {};

	template< typename T_, size_t N >
	struct _is_c_array< T_ (&)[N] > : std::true_type {};

 public:

	static constexpr bool value = _is_c_array<T>::value;
};

template< typename T, typename E >
struct is_c_array_of
{
 private:

	template< typename T_, typename E_, REQUIRES( is_c_array<T_>::value ) >
	static constexpr bool _is_c_array_of()
	{
		return std::is_same< typename range_value<T_>::type, E_ >::value;
	}

	template< typename T_, typename E_, REQUIRES( !is_c_array<T_>::value ) >
	static constexpr bool _is_c_array_of()
	{
		return false;
	}

 public:

	static constexpr bool value = _is_c_array_of< T, E >();
};

template< typename T >
struct is_std_array
{
 private:

	template< typename T_ >
	struct _is_std_array : std::false_type {};

	template< typename ElemType, size_t size_ >
	struct _is_std_array< std::array< ElemType, size_ > > : std::true_type {};

 public:

	static constexpr bool value = _is_std_array< typename std::remove_cv<T>::type >::value;
};


#if __cplusplus >= 202002L
// https://stackoverflow.com/questions/44012938/how-to-tell-if-template-type-is-an-instance-of-a-template-class
namespace impl
{
	template< typename, template< typename ... > typename >
	struct is_template_instance_impl : public std::false_type {};

	template< template< typename ... > typename Template, typename ... TemplateArgs >
	struct is_template_instance_impl< Template< TemplateArgs ... >, Template > : public std::true_type {};
}
template< typename Type, template< typename ... > typename Template >
using is_template_instance = impl::is_template_instance_impl< typename std::decay< Type >::type, Template >;
#endif


} // namespace own


#endif // CPPUTILS_TYPETRAITS_INCLUDED
