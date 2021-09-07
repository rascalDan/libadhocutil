#ifndef ADHOCUTIL_COMPILE_TIME_FORMATTER_PRINTF_H
#define ADHOCUTIL_COMPILE_TIME_FORMATTER_PRINTF_H

#include "../compileTimeFormatter.h"
#include <boost/assert.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <iomanip>
#include <type_traits>

namespace AdHoc {
#define BASICCONV(PARAMTYPE, OP, ...) \
	StreamWriterT(__VA_ARGS__) { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const PARAMTYPE & p, const Pn &... pn) \
		{ \
			OP; \
			s.copyfmt(std::ios(nullptr)); \
			StreamWriter::next(s, pn...); \
		} \
	}

	// Integers (d, i, o, u, x, X)
#define INTCONV(BASE, OP, CONV) \
	BASICCONV(BASE, OP, CONV); \
	BASICCONV(short BASE, OP, 'h', CONV); \
	BASICCONV(long BASE, OP, 'l', CONV); \
	BASICCONV(long long BASE, OP, 'l', 'l', CONV);
	INTCONV(int, s << std::dec << p, 'i');
	INTCONV(int, s << std::dec << p, 'd');
	INTCONV(unsigned int, s << std::oct << p, 'o');
	INTCONV(unsigned int, s << std::dec << p, 'u');
	INTCONV(unsigned int, s << std::nouppercase << std::hex << p, 'x');
	INTCONV(unsigned int, s << std::uppercase << std::hex << p, 'X');
#undef INTCONV

	BASICCONV(intmax_t, s << std::dec << p, 'j', 'd');
	BASICCONV(uintmax_t, s << std::dec << p, 'j', 'u');
	BASICCONV(ssize_t, s << std::dec << p, 'z', 'd');
	BASICCONV(size_t, s << std::dec << p, 'z', 'u');
	BASICCONV(short int, s << std::dec << p, 'h', 'h', 'i'); // char
	BASICCONV(short int, s << std::dec << p, 'h', 'h', 'd'); // char
	BASICCONV(unsigned char, s << std::dec << p, 'h', 'h', 'u');
	BASICCONV(unsigned char, s << std::oct << p, 'h', 'h', 'o');
	BASICCONV(unsigned char, s << std::nouppercase << std::hex << p, 'h', 'h', 'x');
	BASICCONV(unsigned char, s << std::uppercase << std::hex << p, 'h', 'h', 'X');

	// Floating point (a, A, e, E, f, F, g, G)
#define FPCONV(BASE, OP, CONV) \
	BASICCONV(BASE, OP, CONV); \
	BASICCONV(long BASE, OP, 'L', CONV);
	FPCONV(double, s << std::nouppercase << std::hexfloat << p, 'a');
	FPCONV(double, s << std::uppercase << std::hexfloat << p, 'A');
	FPCONV(double, s << std::nouppercase << std::scientific << p, 'e');
	FPCONV(double, s << std::uppercase << std::scientific << p, 'E');
	FPCONV(double, s << std::nouppercase << std::fixed << p, 'f');
	FPCONV(double, s << std::uppercase << std::fixed << p, 'F');
	FPCONV(double, s << std::nouppercase << std::defaultfloat << p, 'g');
	FPCONV(double, s << std::uppercase << std::defaultfloat << p, 'G');
#undef FPCONV

	BASICCONV(std::string_view, s << p, 's');
	BASICCONV(std::wstring_view, s << p, 'l', 's');
	BASICCONV(char, s << p, 'c');
	BASICCONV(wchar_t, s << p, 'l', 'c');
#undef BASICCONV
	StreamWriterT('p') {
		template<typename Obj, typename... Pn>
		static inline void
		write(stream & s, Obj * const ptr, const Pn &... pn)
		{
			s << std::showbase << std::hex << reinterpret_cast<long unsigned int>(ptr);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
		template<typename Ptr, typename... Pn>
		static inline void
		write(stream & s, const Ptr & ptr, const Pn &... pn)
		{
			write(s, ptr.get(), pn...);
		}
	};

	StreamWriterT('m') {
		template<typename... Pn>
		static inline void
		write(stream & s, const Pn &... pn)
		{
			s << strerror(errno);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};
	StreamWriterT('n') {
		template<typename... Pn>
		static inline void
		write(stream & s, int * n, const Pn &... pn)
		{
			BOOST_ASSERT_MSG(n, "%n conversion requires non-null parameter");
			*n = streamLength(s);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};

	////
	// Width/precision embedded in format string
	template<auto... chs>
	constexpr auto
	decdigits()
	{
		static_assert((isdigit(chs) && ... && true));
		int n = 0;
		(
				[&n](auto ch) {
					n = (n * 10) + (ch - '0');
				}(chs),
				...);
		return n;
	}
#define AUTON(z, n, data) \
	BOOST_PP_COMMA_IF(n) \
	auto BOOST_PP_CAT(data, n)
#define NS(z, n, data) \
	BOOST_PP_COMMA_IF(n) \
	BOOST_PP_CAT(data, n)
#define ISDIGIT(z, n, data) &&isdigit(BOOST_PP_CAT(data, BOOST_PP_ADD(n, 1)))
#define FMTWIDTH(unused, d, data) \
	template<CtfString S, auto L, auto pos, typename stream, BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), AUTON, n), auto nn, \
			auto... sn> \
	struct StreamWriter<S, L, pos, stream, \
			typename std::enable_if<ispositivedigit(n0) BOOST_PP_REPEAT(d, ISDIGIT, n) && !isdigit(nn)>::type, '%', \
			BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n), nn, sn...> { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			constexpr auto p = decdigits<BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n)>(); \
			s << std::setw(p); \
			StreamWriter<S, L, pos + BOOST_PP_ADD(d, 1), stream, void, '%', nn, sn...>::write(s, pn...); \
		} \
	};
	BOOST_PP_REPEAT(6, FMTWIDTH, void);
#define FMTPRECISION(unused, d, data) \
	template<CtfString S, auto L, auto pos, typename stream, BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), AUTON, n), auto nn, \
			auto... sn> \
	struct StreamWriter<S, L, pos, stream, \
			typename std::enable_if<isdigit(n0) BOOST_PP_REPEAT(d, ISDIGIT, n) && !isdigit(nn)>::type, '%', '.', \
			BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n), nn, sn...> { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			constexpr auto p = decdigits<BOOST_PP_REPEAT(BOOST_PP_ADD(d, 1), NS, n)>(); \
			s << std::setprecision(p); \
			StreamWriter<S, L, pos + BOOST_PP_ADD(d, 2), stream, void, '%', nn, sn...>::write(s, pn...); \
		} \
	};
	BOOST_PP_REPEAT(6, FMTPRECISION, void);
#undef AUTON
#undef NS
#undef ISDIGIT
#undef FMTWIDTH

	StreamWriterT('.', '*') {
		template<typename... Pn>
		static inline void
		write(stream & s, int l, const Pn &... pn)
		{
			s << std::setw(l);
			StreamWriter<S, L, pos + 2, stream, void, '%', sn...>::write(s, pn...);
		}
	};
	StreamWriterT('.', '*', 's') {
		template<typename... Pn>
		static inline void
		write(stream & s, int l, const std::string_view & p, const Pn &... pn)
		{
			s << p.substr(0, l);
			s.copyfmt(std::ios(nullptr));
			StreamWriter::next(s, pn...);
		}
	};

		// Flags
#define FLAGCONV(OP, ...) \
	StreamWriterT(__VA_ARGS__) { \
		template<typename... Pn> \
		static inline void \
		write(stream & s, const Pn &... pn) \
		{ \
			OP; \
			StreamWriter<S, L, pos + 1, stream, void, '%', sn...>::write(s, pn...); \
		} \
	};
	FLAGCONV(s << std::showbase, '#');
	FLAGCONV(s << std::setfill('0'), '0');
	FLAGCONV(s << std::left, '-');
	FLAGCONV(s << std::showpos, '+');
	FLAGCONV(s << std::setfill(' '), ' ');
#undef FLAGCONV
}

#endif
