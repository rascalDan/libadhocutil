#ifndef ADHOCUTIL_COMPILE_TIME_FORMATTER_H
#define ADHOCUTIL_COMPILE_TIME_FORMATTER_H

#include <sstream>
#include <cstring>
#include <array>
#include <boost/preprocessor/variadic/size.hpp>
#include "unique.h"

namespace AdHoc {
#define CtfString const auto &
	// Template char utils
	template<typename char_type>
	constexpr bool isdigit(const char_type & ch)
	{
		return (ch >= '0' && ch <= '9');
	}

	template<typename char_type>
	constexpr bool ispositivedigit(const char_type & ch)
	{
		return (ch >= '1' && ch <= '9');
	}

	// Template string utils
	template<CtfString S>
	static constexpr auto strlen()
	{
		auto off = 0;
		while (S[off]) { ++off; }
		return off;
	}

	template<CtfString S, auto n, auto start = 0, auto L = strlen<S>()>
	static constexpr auto strchr()
	{
		static_assert(start <= L);
		decltype(start) off = start;
		while (off < L && S[off] != n) { ++off; }
		if (off == L) {
			return -1;
		}
		return off;
	}

	template<CtfString S, auto n, auto start = 0, auto L = strlen<S>()>
	static constexpr decltype(L) strchrnul()
	{
		decltype(start) off = start;
		while (off < L && S[off] != n) { ++off; }
		return off;
	}

	template<CtfString S, decltype(strlen<S>())> class Formatter;

	/// Template used to apply parameters to a stream.
	template<CtfString S, auto L, auto pos, typename stream, typename, auto ...>
	struct StreamWriter {
		/// Write parameters to stream.
		template<typename ... Pn>
		static void write(stream &, const Pn & ...)
		{
			static_assert(!L, "invalid format string/arguments");
		}
	};

	/// Helper to simplify implementations of StreamWriter.
	template<CtfString S, auto L, auto pos, typename stream>
	struct StreamWriterBase {
		/// Continue processing parameters.
		template<typename ... Pn>
		static inline void next(stream & s, const Pn & ... pn)
		{
			Formatter<S, L>::template Parser<stream, pos + 1, Pn...>::run(s, pn...);
		}
	};

#define StreamWriterT(C...) \
	template<CtfString S, auto L, auto pos, typename stream, auto ... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

#define StreamWriterTP(P, C...) \
	template<CtfString S, auto L, auto pos, typename stream, auto P, auto ... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
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

	template<typename stream, typename char_type>
	static inline void appendStream(stream & s, const char_type * p, size_t n)
	{
		s.write(p, n);
	}

	template<typename stream>
	static inline auto streamLength(stream & s)
	{
		return s.tellp();
	}

	/**
	 * Compile time string formatter.
	 * @param S the format string.
	 */
	template <CtfString S, decltype(strlen<S>()) L = strlen<S>()>
	class Formatter {
		private:
			using strlen_t = decltype(strlen<S>());
			template<CtfString, auto, auto, typename> friend struct StreamWriterBase;

		public:
			/// The derived charater type of the format string.
			using char_type = typename std::decay<decltype(S[0])>::type;
			/**
			 * Get a string containing the result of formatting.
			 * @param pn the format arguments.
			 * @return the formatted string.
			 */
			template<typename ... Pn>
			static inline auto get(const Pn & ... pn)
			{
				std::basic_stringstream<char_type> s;
				return write(s, pn...).str();
			}
			/**
			 * Get a string containing the result of formatting.
			 * @param pn the format arguments.
			 * @return the formatted string.
			 */
			template<typename ... Pn>
			inline auto operator()(const Pn & ... pn) const
			{
				return get(pn...);
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
			/**
			 * Write the result of formatting to the given stream.
			 * @param s the stream to write to.
			 * @param pn the format arguments.
			 * @return the stream.
			 */
			template<typename stream, typename ... Pn>
			inline typename std::enable_if<(bool)&stream::write, stream>::type &
			operator()(stream & s, const Pn & ... pn) const
			{
				return write(s, pn...);
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
							appendStream(s, &S[pos], ph - pos);
						}
						if constexpr (ph != L) {
							packAndWrite<ph>(s, pn...);
						}
					}
					return s;
				}
				template<strlen_t ph, strlen_t off = 0, auto ... Pck>
				static inline void packAndWrite(stream & s, const Pn & ... pn)
				{
					if constexpr (ph + off == L || sizeof...(Pck) == 32) {
						StreamWriter<S, L, ph, stream, void, Pck...>::write(s, pn...);
					}
					else if constexpr (ph + off < L) {
						packAndWrite<ph, off + 1, Pck..., S[ph + off]>(s, pn...);
					}
				}
			};
	};

	namespace literals {
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif
		/// CTF format string holder
		template<typename T, T ... t> struct FMT
		{
			/// CTF format string
			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
			static constexpr char __FMT[] = {t...};
		};
		template<typename T, T ... t> inline auto operator""_fmt() noexcept
		{
			return AdHoc::Formatter<FMT<T, t...>::__FMT, sizeof...(t)>();
		}
#ifdef __clang__
#pragma clang diagnostic pop
#endif
	}
}

#define AdHocFormatterTypedef(name, str, id) \
    inline constexpr auto id = str; \
    using name = ::AdHoc::Formatter<id>
#define AdHocFormatter(name, str) \
    AdHocFormatterTypedef(name, str, MAKE_UNIQUE(name))

#endif

