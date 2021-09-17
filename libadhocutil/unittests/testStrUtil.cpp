#include <strUtil.h>

constexpr auto
ltrim(std::string_view s, const auto c)
{
	return AdHoc::ltrim(s, c);
}
constexpr auto
rtrim(std::string_view s, const auto c)
{
	return AdHoc::rtrim(s, c);
}
constexpr auto
trim(std::string_view s, const auto c)
{
	return AdHoc::trim(s, c);
}

static_assert(ltrim("foobar", 'r') == "foobar");
static_assert(rtrim("foobar", 'r') == "fooba");
static_assert(ltrim("foobar", 'f') == "oobar");
static_assert(rtrim("foobar", 'f') == "foobar");

static_assert(ltrim("foobar", "r") == "foobar");
static_assert(rtrim("foobar", "r") == "fooba");
static_assert(ltrim("foobar", "f") == "oobar");
static_assert(rtrim("foobar", "f") == "foobar");

static_assert(ltrim("foobar", "fo") == "bar");
static_assert(ltrim("foobar", "of") == "bar");
static_assert(rtrim("foobar", "fo") == "foobar");
static_assert(rtrim("foobar", "of") == "foobar");

static_assert(ltrim("foobar", "ar") == "foobar");
static_assert(ltrim("foobar", "ra") == "foobar");
static_assert(rtrim("foobar", "ar") == "foob");
static_assert(rtrim("foobar", "ra") == "foob");

static_assert(trim("roobar", 'r') == "ooba");
static_assert(trim("foobar", 'r') == "fooba");
static_assert(trim("foobar", 'f') == "oobar");
static_assert(trim("foobar", "f") == "oobar");
static_assert(trim("foobar", "r") == "fooba");
static_assert(trim("foobar", "fr") == "ooba");
static_assert(trim("foobar", "ofr") == "ba");
