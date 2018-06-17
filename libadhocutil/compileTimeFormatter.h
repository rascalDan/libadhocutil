#ifndef ADHOCUTIL_COMPILE_TIME_FORMATTER_H
#define ADHOCUTIL_COMPILE_TIME_FORMATTER_H

#include <sstream>
#include <string.h>
#include <boost/preprocessor/variadic/size.hpp>
#include "unique.h"

namespace AdHoc {
	// Template string utils
	template<const auto & S>
	static constexpr auto strlen()
	{
		auto off = 0;
		while (S[off]) { ++off; }
		return off;
	}

	template<const auto & S, auto n, auto start = 0, auto L = strlen<S>()>
	static constexpr auto strchr()
	{
		static_assert(start <= L);
		decltype(start) off = start;
		while (off < L && S[off] != n) { ++off; }
		if (off == L) return -1;
		return off;
	}

	template<const auto & S, auto n, auto start = 0, auto L = strlen<S>()>
	static constexpr decltype(L) strchrnul()
	{
		decltype(start) off = start;
		while (off < L && S[off] != n) { ++off; }
		return off;
	}

	template <const auto &, int> class Formatter;

	template<const auto & S, auto L, auto pos, typename stream, auto ...>
	struct StreamWriter {
	};

	template<const auto & S, auto L, auto pos, typename stream>
	struct StreamWriterBase {
		template<typename ... Pn>
		static inline void next(stream & s, const Pn & ... pn)
		{
			Formatter<S, L>::template Parser<stream, pos + 1, Pn...>::run(s, pn...);
		}
	};

#define StreamWriterT(C...) \
	template<const auto & S, auto L, auto pos, typename stream, auto ... sn> \
	struct StreamWriter<S, L, pos, stream, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

#define StreamWriterTP(P, C...) \
	template<const auto & S, auto L, auto pos, typename stream, auto P, auto ... sn> \
	struct StreamWriter<S, L, pos, stream, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

	// Default stream writer formatter
	StreamWriterT('?') {
		template<typename P, typename ... Pn>
		static inline void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << p;
			StreamWriter::next(s, pn...);
		}
	};

	// Escaped % stream writer formatter
	StreamWriterT('%') {
		template<typename ... Pn>
		static inline void write(stream & s, const Pn & ... pn)
		{
			s << '%';
			StreamWriter::next(s, pn...);
		}
	};

	// Unknown stream writer formatter
	template<const auto & S, auto L, auto pos, typename stream, auto ... sn>
	struct StreamWriter<S, L, pos, stream, '%', sn...> {
		template<typename ... Pn>
		static void write(stream & s, const Pn & ...)
		{
			static_assert(!&s, "invalid format string/arguments");
		}
	};

	/**
	 * Compile time string formatter.
	 * @param S the format string.
	 */
	template <const auto & S, int L = strlen<S>()>
	class Formatter {
		private:
			template<const auto &, auto, auto, typename> friend struct StreamWriterBase;

		public:
			/**
			 * Get a string containing the result of formatting.
			 * @param pn the format arguments.
			 * @return the formatted string.
			 */
			template<typename ... Pn>
			static inline auto get(const Pn & ... pn)
			{
				std::basic_stringstream<typename std::decay<decltype(*S)>::type> s;
				return write(s, pn...).str();
			}

			/**
			 * Write the result of formatting to the given stream.
			 * @param s the stream to write to.
			 * @param pn the format arguments.
			 * @return the stream.
			 */
			template<typename stream, typename ... Pn>
			static inline stream & write(stream & s, const Pn & ... pn)
			{
				return Parser<stream, 0, Pn...>::run(s, pn...);
			}

		private:
			template<typename stream, auto pos, typename ... Pn>
			struct Parser {
				static inline stream &
				run(stream & s, const Pn & ... pn)
				{
					if (pos != L) {
						constexpr auto ph = strchrnul<S, '%', pos, L>();
						if constexpr (ph != pos) {
							s.write((S + pos), ph - pos);
						}
						if constexpr (ph != L) {
							packAndWrite<ph>(s, pn...);
						}
					}
					return s;
				}
				template<auto ph, auto off = 0, auto ... Pck>
				static inline void packAndWrite(stream & s, const Pn & ... pn)
				{
					if constexpr (ph + off == L || sizeof...(Pck) == 32) {
						StreamWriter<S, L, ph, stream, Pck...>::write(s, pn...);
					}
					else if constexpr (ph + off < L) {
						packAndWrite<ph, off + 1, Pck..., S[ph + off]>(s, pn...);
					}
				}
			};
	};
}

#define AdHocFormatterTypedef(name, str, id) \
	inline constexpr auto id = str; \
	typedef ::AdHoc::Formatter<id> name
#define AdHocFormatter(name, str) \
	AdHocFormatterTypedef(name, str, MAKE_UNIQUE(name))

#endif

