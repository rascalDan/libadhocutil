#include <iostream>
#include <string.h>
#include <boost/static_assert.hpp>

namespace AdHoc {
	constexpr int WRAP_AT = 120;

	template <bool, char...> struct Buffer { };

	template <char stop, int offset, char ...>
	struct Upto {
		template<typename stream>
		static auto stuff(stream &, const Buffer<false> & f)
		{
			return f;
		}
	};
	template <char stop, int offset, char s0, char... sn>
	struct Upto<stop, offset, s0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			return Upto<stop, offset + 1, sn...>::stuff(s, Buffer<false, sm..., s0>());
		}
	};
	template <char stop, int offset, char... sn>
	struct Upto<stop, offset, stop, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<false, stop, sn...>();
		}
	};
	template <char stop, int offset, char... sn>
	struct Upto<stop, offset, 0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<false, 0, sn...>();
		}
	};
	template <char stop, char s0, char... sn>
	struct Upto<stop, WRAP_AT, s0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<false, s0, sn...>();
		}
	};
	template <char stop, char... sn>
	struct Upto<stop, WRAP_AT, stop, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<false, stop, sn...>();
		}
	};
	template <char stop, char... sn>
	struct Upto<stop, WRAP_AT, 0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<false, sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<false, 0, sn...>();
		}
	};

	template <typename stream, char ... sn>
	struct StreamWriter {
		template<typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			next(s, Upto<'%', 0, sn...>::stuff(s, Buffer<false>()), pn...);
		}
		template<typename ... Pn, char... ssn, template <bool, char...> class Buffer>
		static void next(stream & s, const Buffer<false, ssn...>&, const Pn & ... pn)
		{
			StreamWriter<stream, ssn...>::write(s, pn...);
		}
	};

	template<typename stream>
	struct StreamWriter<stream> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...) { }
	};

	template<typename stream>
	struct StreamWriter<stream, 0> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...) { }
	};

	// Default stream writer formatter
	template<typename stream, char ... sn>
	struct StreamWriter<stream, '%', '?', sn...> {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << p;
			StreamWriter<stream, sn...>::write(s, pn...);
		}
	};

	// Unknown stream writer formatter
	template<typename stream, char ... sn>
	struct StreamWriter<stream, '%', sn...> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...)
		{
			BOOST_STATIC_ASSERT_MSG(!&err, "invalid format string/arguments");
		}
		static int err;
	};

	template <const char * const & S, int offset, int roffset, char s0, char ... sn>
	struct Parser {
		static auto parse()
		{
			return append(innerparse());
		}
		static auto innerparse()
		{
			return Parser<S, offset + 1, roffset + 1, S[offset + 1], sn..., s0>::innerparse();
		}
		template<char...ssn>
		static auto append(const Buffer<true, ssn...> & b)
		{
			return join(b, Parser<S, offset + 1 + WRAP_AT, 0, S[offset + 1 + WRAP_AT]>::parse());
		}
		template<char...ssn>
		static auto append(const Buffer<false, ssn...> & b)
		{
			return b;
		}
		template<bool more, char...ssn, char...ssm>
		static auto join(const Buffer<true, ssn...> &, const Buffer<more, ssm...> &)
		{
			return Buffer<more, ssn..., ssm...>();
		}
	};

	template <const char * const & S, int offset, char s0, char ... sn>
	struct Parser<S, offset, WRAP_AT, s0, sn...> {
		static auto parse()
		{
			return Buffer<true, sn..., s0>();
		}
		static auto innerparse()
		{
			return Buffer<true, sn..., s0>();
		}
	};

	template <const char * const & S, int offset, char ... sn>
	struct Parser<S, offset, WRAP_AT, 0, sn...> {
		static auto parse()
		{
			return Buffer<false, sn...>();
		}
		static auto innerparse()
		{
			return Buffer<false, sn...>();
		}
	};

	template <const char * const & S, int offset, int roffset, char ... sn>
	struct Parser<S, offset, roffset, 0, sn...> {
		static auto parse()
		{
			return Buffer<false, sn..., 0>();
		}
		static auto innerparse()
		{
			return Buffer<false, sn..., 0>();
		}
	};

	template <const char * const & S>
	struct Formatter {
		template<typename stream, typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			run(Parser<S, 0, 0, *S>::parse(), s, pn...);
		}

		template<typename stream, char...ssn, template<bool, char...> class Buffer, typename ... Pn>
		static void run(const Buffer<false, ssn...> &, stream & s, const Pn & ... pn)
		{
			StreamWriter<stream, ssn...>::write(s, pn...);
		}
	};
}

