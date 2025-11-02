//======================================================================================================================
// Project: CppUtils
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: unit tests for TypeTraits.hpp
//======================================================================================================================

#include "TypeTraits.hpp"

#include "Span.hpp"

#include <array>
#include <vector>
#include <string>
#include <list>


namespace own {

static_assert( std::is_same< typename get_nth_type< 0, int, char, float >::type, int >::value, "" );
static_assert( std::is_same< typename get_nth_type< 1, int, char, float >::type, char >::value, "" );
static_assert( std::is_same< typename get_nth_type< 2, int, char, float >::type, float >::value, "" );

// is_same_except...

static_assert( is_same_except_cv< char, const char >::value, "" );
static_assert( is_same_except_cv< char, volatile char >::value, "" );
static_assert( !is_same_except_cv< char, char & >::value, "" );
static_assert( !is_same_except_cv< char, const char & >::value, "" );

static_assert( is_same_except_ref< char, char & >::value, "" );
static_assert( !is_same_except_ref< char, const char >::value, "" );
static_assert( !is_same_except_ref< char, const char & >::value, "" );

static_assert( is_same_except_cvref< char, char & >::value, "" );
static_assert( is_same_except_cvref< char, const char >::value, "" );
static_assert( is_same_except_cvref< char, const char & >::value, "" );

// type categories

enum Enum : int { Value };

static_assert( is_int_or_enum< int >::value, "" );
static_assert( is_int_or_enum< Enum >::value, "" );
static_assert( is_int_or_enum< uint8_t >::value, "" );
static_assert( !is_int_or_enum< float >::value, "" );

static_assert( is_byte_alike< uint8_t >::value, "" );
static_assert( is_byte_alike< char >::value, "" );
static_assert( !is_byte_alike< uint16_t >::value, "" );
static_assert( !is_byte_alike< char16_t >::value, "" );

static_assert( is_character< char >::value, "" );
static_assert( is_character< wchar_t >::value, "" );
static_assert( is_character< char16_t >::value, "" );

// conditional types

static_assert( std::is_same< int_type< int >::type, int >::value, "" );
static_assert( std::is_same< int_type< Enum >::type, int >::value, "" );

static_assert( std::is_same< bigger_type< uint16_t, uint32_t >::type, uint32_t >::value, "" );
static_assert( std::is_same< bigger_type< uint64_t, uint32_t >::type, uint64_t >::value, "" );
static_assert( std::is_same< bigger_type< char, wchar_t >::type, wchar_t >::value, "" );
static_assert( std::is_same< bigger_type< float, double >::type, double >::value, "" );

static_assert( std::is_same< corresponding_constness< const char, int >::type, const int >::value, "" );
static_assert( !std::is_same< corresponding_constness< const char, int >::type, int >::value, "" );
static_assert( std::is_same< corresponding_constness< char, const int >::type, int >::value, "" );
static_assert( !std::is_same< corresponding_constness< char, const int >::type, const int >::value, "" );

// range traits

using SingleElement = int;
static_assert( !is_range< SingleElement >::value, "" );
//static_assert( !has_contiguous_data< SingleElement >::value, "" );
//static_assert( !is_resizable< SingleElement >::value, "" );

using CharCArr = char [4];
static_assert( is_range< CharCArr >::value, "" );
static_assert( is_range_of< CharCArr, char >::value, "" );
static_assert( !is_range_of< CharCArr, uint8_t >::value, "" );
static_assert( is_contiguous_range< CharCArr >::value, "" );
static_assert( !is_resizable< CharCArr >::value, "" );
static_assert( is_trivial_range< CharCArr >::value, "" );

using CharArr = std::array< char, 4 >;
static_assert( is_range< CharArr >::value, "" );
static_assert( is_range_of< CharArr, char >::value, "" );
static_assert( !is_range_of< CharArr, uint8_t >::value, "" );
static_assert( is_contiguous_range< CharArr >::value, "" );
static_assert( !is_resizable< CharArr >::value, "" );
static_assert( is_trivial_range< CharArr >::value, "" );

using CharVec = std::vector< char >;
static_assert( is_range< CharVec >::value, "" );
static_assert( is_range_of< CharVec, char >::value, "" );
static_assert( !is_range_of< CharVec, uint8_t >::value, "" );
static_assert( is_contiguous_range< CharVec >::value, "" );
static_assert( is_resizable< CharVec >::value, "" );
static_assert( is_trivial_range< CharVec >::value, "" );

using CharList = std::list< char >;
static_assert( is_range< CharList >::value, "" );
static_assert( is_range_of< CharList, char >::value, "" );
static_assert( !is_range_of< CharList, uint8_t >::value, "" );
static_assert( !is_contiguous_range< CharList >::value, "" );
static_assert( is_resizable< CharList >::value, "" );
static_assert( is_trivial_range< CharList >::value, "" );

using StringVec = std::vector< std::string >;
static_assert( is_range< StringVec >::value, "" );
static_assert( is_range_of< StringVec, std::string >::value, "" );
static_assert( !is_range_of< StringVec, uint8_t >::value, "" );
static_assert( is_contiguous_range< StringVec >::value, "" );
static_assert( is_resizable< StringVec >::value, "" );
static_assert( !is_trivial_range< StringVec >::value, "" );

static_assert( is_c_array< CharCArr >::value, "" );
static_assert( is_c_array_of< CharCArr, char >::value, "" );
static_assert( is_std_array< CharArr >::value, "" );

} // namespace own
