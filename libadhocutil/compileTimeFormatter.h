#include <iostream>
#include <string.h>
#include <boost/static_assert.hpp>

namespace AdHoc {
	constexpr int WRAP_AT = 120;

	template <int, char...> struct Buffer { };

	template <const char * const & S, char stop, int start, int offset, char ...>
	struct Upto {
		template<typename stream>
		static auto stuff(stream &, const Buffer<start> & f)
		{
			return f;
		}
	};
	template <const char * const & S, char stop, int start, int offset, char s0, char... sn>
	struct Upto<S, stop, start, offset, s0, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			return Upto<S, stop, start, offset + 1, sn...>::stuff(s, Buffer<len, sm..., s0>());
		}
	};
	template <const char * const & S, char stop, int start, int offset, char... sn>
	struct Upto<S, stop, start, offset, stop, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			s.write(S + start, sizeof...(sm));
			return Buffer<sizeof...(sm), stop, sn...>();
		}
	};
	template <const char * const & S, char stop, int start, int offset, char... sn>
	struct Upto<S, stop, start, offset, 0, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			s.write(S + start, sizeof...(sm));
			return Buffer<sizeof...(sm), 0, sn...>();
		}
	};
	template <const char * const & S, char stop, int start, char s0, char... sn>
	struct Upto<S, stop, start, WRAP_AT, s0, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			s.write(S + start, sizeof...(sm));
			return Buffer<sizeof...(sm), s0, sn...>();
		}
	};
	template <const char * const & S, char stop, int start, char... sn>
	struct Upto<S, stop, start, WRAP_AT, stop, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			s.write(S + start, sizeof...(sm));
			return Buffer<sizeof...(sm), stop, sn...>();
		}
	};
	template <const char * const & S, char stop, int start, char... sn>
	struct Upto<S, stop, start, WRAP_AT, 0, sn...> {
		template<typename stream, int len, char... sm>
		static auto stuff(stream & s, const Buffer<len, sm...> &)
		{
			s.write(S + start, sizeof...(sm));
			return Buffer<sizeof...(sm), 0, sn...>();
		}
	};

	template <const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter {
		template<typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			next(s, Upto<S, '%', start, 0, sn...>::stuff(s, Buffer<0>()), pn...);
		}
		template<typename ... Pn, int len, char... ssn, template <int, char...> class Buffer>
		static void next(stream & s, const Buffer<len, ssn...>&, const Pn & ... pn)
		{
			StreamWriter<S, start + len, stream, ssn...>::write(s, pn...);
		}
	};

	template<const char * const & S, int start, typename stream>
	struct StreamWriter<S, start, stream> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...) { }
	};

	template<const char * const & S, int start, typename stream>
	struct StreamWriter<S, start, stream, 0> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...) { }
	};

	// Default stream writer formatter
	template<const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter<S, start, stream, '%', '?', sn...> {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << p;
			StreamWriter<S, start + 2, stream, sn...>::write(s, pn...);
		}
	};

	// Unknown stream writer formatter
	template<const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter<S, start, stream, '%', sn...> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...)
		{
			BOOST_STATIC_ASSERT_MSG(!&err, "invalid format string/arguments");
		}
		static int err;
	};

	template <bool, char...> struct ParserBuffer { };

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
		static auto append(const ParserBuffer<true, ssn...> & b)
		{
			return join(b, Parser<S, offset + 1 + WRAP_AT, 0, S[offset + 1 + WRAP_AT]>::parse());
		}
		template<char...ssn>
		static auto append(const ParserBuffer<false, ssn...> & b)
		{
			return b;
		}
		template<bool more, char...ssn, char...ssm>
		static auto join(const ParserBuffer<true, ssn...> &, const ParserBuffer<more, ssm...> &)
		{
			return ParserBuffer<more, ssn..., ssm...>();
		}
	};

	template <const char * const & S, bool more, char ... sn>
	struct ParserBase {
		static auto parse()
		{
			return innerparse();
		}
		static auto innerparse()
		{
			return ParserBuffer<more, sn...>();
		}
	};

	template <const char * const & S, int offset, char s0, char ... sn>
	struct Parser<S, offset, WRAP_AT, s0, sn...> : public ParserBase<S, true, sn..., s0> { };

	template <const char * const & S, int offset, char ... sn>
	struct Parser<S, offset, WRAP_AT, 0, sn...> : public ParserBase<S, true, sn..., 0> { };

	template <const char * const & S, int offset, int roffset, char ... sn>
	struct Parser<S, offset, roffset, 0, sn...> : public ParserBase<S, false, sn..., 0>{ };

	template <const char * const & S>
	struct Formatter {
		template<typename stream, typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			run(Parser<S, 0, 0, *S>::parse(), s, pn...);
		}

		template<typename stream, char...ssn, template<bool, char...> class ParserBuffer, typename ... Pn>
		static void run(const ParserBuffer<false, ssn...> &, stream & s, const Pn & ... pn)
		{
			StreamWriter<S, 0, stream, ssn...>::write(s, pn...);
		}
	};
}

