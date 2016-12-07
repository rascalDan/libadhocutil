#include <iostream>
#include <string.h>
#include <boost/static_assert.hpp>

namespace AdHoc {
	template <char...> struct Buffer { };

	template <char stop, char ...>
	struct Upto {
		template<typename stream>
		static auto stuff(stream &, const Buffer<> & f)
		{
			return f;
		}
	};
	template <char stop, char s0, char... sn>
	struct Upto<stop, s0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<sm...> &)
		{
			return Upto<stop, sn...>::stuff(s, Buffer<sm..., s0>());
		}
	};
	template <char stop, char... sn>
	struct Upto<stop, stop, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<stop, sn...>();
		}
	};
	template <char stop, char... sn>
	struct Upto<stop, 0, sn...> {
		template<typename stream, char... sm>
		static auto stuff(stream & s, const Buffer<sm...> &)
		{
			char buf[] = {sm...};
			s.write(buf, sizeof...(sm));
			return Buffer<0, sn...>();
		}
	};

	template <typename stream, char ... sn>
	struct StreamWriter {
		template<typename ... Pn>
  	static void write(stream & s, const Pn & ... pn)
    {
    	next(s, Upto<'%', sn...>::stuff(s, Buffer<>()), pn...);
    }
		template<typename ... Pn, char... ssn, template <char...> class Buffer>
		static void next(stream & s, const Buffer<ssn...>&, const Pn & ... pn)
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

	template <const char * const & S, char s0 = *S, int offset = 0, char ... sn>
	struct Formatter {
		template<typename stream, typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			Formatter<S, S[offset], offset + 1, sn..., S[offset]>::write(s, pn...);
		}
	};

	template <const char * const & S, int offset, char ... sn>
	struct Formatter<S, 0, offset, sn...> {
		template<typename stream, typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			StreamWriter<stream, sn...>::write(s, pn...);
		}
	};
}

