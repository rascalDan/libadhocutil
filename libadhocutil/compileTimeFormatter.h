#ifndef ADHOCUTIL_COMPILE_TIME_FORMATTER_H
#define ADHOCUTIL_COMPILE_TIME_FORMATTER_H

#include <array>
#include <boost/preprocessor/variadic/size.hpp>
#include <cstring>
#include <iostream>
#include <optional>
#include <sstream> // IWYU pragma: export

namespace AdHoc {
#ifdef __cpp_nontype_template_parameter_class
#	define USE_FIXED_STRING
#endif

#ifdef USE_FIXED_STRING
#	define CtfString const auto
#else
#	define CtfString const auto &
#endif
	// Template char utils
	template<typename char_type>
	constexpr bool
	isdigit(const char_type & ch)
	{
		return (ch >= '0' && ch <= '9');
	}

	template<typename char_type>
	constexpr bool
	ispositivedigit(const char_type & ch)
	{
		return (ch >= '1' && ch <= '9');
	}

	// Template string utils
	template<CtfString S>
	static constexpr auto
	strlen()
	{
		auto off = 0U;
		while (S[off]) {
			++off;
		}
		return off;
	}

	template<typename char_type>
	static constexpr auto
	strlen(const char_type * S)
	{
		auto off = 0U;
		while (S[off]) {
			++off;
		}
		return off;
	}

	template<CtfString S, auto n, auto start = 0U, auto L = strlen<S>()>
	static constexpr std::optional<decltype(start)>
	strchr()
	{
		static_assert(start <= L);
		decltype(start) off = start;
		while (off < L && S[off] != n) {
			++off;
		}
		if (off == L) {
			return {};
		}
		return off;
	}

	template<CtfString S, auto n, auto start = 0U, auto L = strlen<S>()>
	static constexpr decltype(L)
	strchrnul()
	{
		decltype(start) off = start;
		while (off < L && S[off] != n) {
			++off;
		}
		return off;
	}

	template<CtfString S, const auto L> class FormatterDetail;

	/// Template used to apply parameters to a stream.
	template<CtfString S, auto L, auto pos, typename stream, typename, auto...> struct StreamWriter {
		/// Write parameters to stream.
		template<typename... Pn>
		static void
		write(stream &, const Pn &...)
		{
			static_assert(!L, "invalid format string/arguments");
		}
	};

	/// Helper to simplify implementations of StreamWriter.
	template<CtfString S, auto L, auto pos, typename stream> struct StreamWriterBase {
		/// Continue processing parameters.
		template<typename... Pn>
		static inline void
		next(stream & s, const Pn &... pn)
		{
			FormatterDetail<S, L>::template Parser<stream, pos + 1, Pn...>::run(s, pn...);
		}
	};

#define StreamWriterT(C...) \
	template<CtfString S, auto L, auto pos, typename stream, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

#define StreamWriterTP(P, C...) \
	template<CtfString S, auto L, auto pos, typename stream, auto P, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', C, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(C) + pos, stream>

	// Default stream writer formatter
	StreamWriterT('?') {
		template<typename P, typename... Pn>
		static inline void
		write(stream & s, const P & p, const Pn &... pn)
		{
			s << p;
			StreamWriter::next(s, pn...);
		}
	};

	// Escaped % stream writer formatter
	StreamWriterT('%') {
		template<typename... Pn>
		static inline void
		write(stream & s, const Pn &... pn)
		{
			s << '%';
			StreamWriter::next(s, pn...);
		}
	};

	template<typename stream, typename char_type>
	static inline void
	appendStream(stream & s, const char_type * p, size_t n)
	{
		s.write(p, n);
	}

	template<typename stream>
	static inline auto
	streamLength(stream & s)
	{
		return s.tellp();
	}

	/**
	 * Compile time string formatter.
	 * @param S the format string.
	 */
	template<CtfString S, const auto L> class FormatterDetail {
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
		template<typename... Pn>
		static inline auto
		get(const Pn &... pn)
		{
			std::basic_stringstream<char_type> s;
			return write(s, pn...).str();
		}
		/**
		 * Get a string containing the result of formatting.
		 * @param pn the format arguments.
		 * @return the formatted string.
		 */
		template<typename... Pn>
		inline auto
		operator()(const Pn &... pn) const
		{
			return get(pn...);
		}

		/**
		 * Write the result of formatting to the given stream.
		 * @param s the stream to write to.
		 * @param pn the format arguments.
		 * @return the stream.
		 */
		template<typename stream, typename... Pn>
		static inline stream &
		write(stream & s, const Pn &... pn)
		{
			return Parser<stream, 0U, Pn...>::run(s, pn...);
		}
		/**
		 * Write the result of formatting to the given stream.
		 * @param s the stream to write to.
		 * @param pn the format arguments.
		 * @return the stream.
		 */
		template<typename stream, typename... Pn>
		inline typename std::enable_if<std::is_base_of_v<std::basic_ostream<char_type>, stream>, stream>::type &
		operator()(stream & s, const Pn &... pn) const
		{
			return write(s, pn...);
		}

	private:
		template<typename stream, auto pos, typename... Pn> struct Parser {
			static inline stream &
			run(stream & s, const Pn &... pn)
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
			template<strlen_t ph, strlen_t off = 0U, auto... Pck>
			static inline void
			packAndWrite(stream & s, const Pn &... pn)
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

#ifdef USE_FIXED_STRING
	// New C++20 implementation
	namespace support {
		template<typename CharT, std::size_t N> class basic_fixed_string : public std::array<CharT, N> {
		public:
			constexpr basic_fixed_string(const CharT (&str)[N + 1])
			{
				for (decltype(N) x = 0; x < N; x++) {
					this->at(x) = str[x];
				}
			}
			constexpr basic_fixed_string(const CharT * str, decltype(N) len)
			{
				for (decltype(N) x = 0; x < len; x++) {
					this->at(x) = str[x];
				}
			}
		};

		template<typename CharT, std::size_t N>
		basic_fixed_string(const CharT (&str)[N]) -> basic_fixed_string<CharT, N - 1>;
	}

	template<const support::basic_fixed_string Str> class LiteralFormatter : public FormatterDetail<Str, Str.size()> {
	};

	template<const auto & S, decltype(strlen(S)) L = strlen(S)>
	class Formatter :
		public FormatterDetail<support::basic_fixed_string<typename std::decay<decltype(S[0])>::type, L>(S, L), L> {
	};

#	define AdHocFormatter(name, str) using name = ::AdHoc::LiteralFormatter<str>

	template<const support::basic_fixed_string Str, typename... Pn>
	inline auto
	scprintf(const Pn &... pn)
	{
		return FormatterDetail<Str, Str.size()>::get(pn...);
	}

	template<const support::basic_fixed_string Str, typename stream, typename... Pn>
	inline auto &
	scprintf(stream & strm, const Pn &... pn)
	{
		return FormatterDetail<Str, Str.size()>::write(strm, pn...);
	}

	template<const support::basic_fixed_string Str, typename... Pn>
	inline auto &
	cprintf(const Pn &... pn)
	{
		return scprintf<Str>(std::cout, pn...);
	}

#else
	// Classic pre-C++20 implementation
#	include "unique.h"
	template<const auto & S, decltype(strlen<S>()) L = strlen<S>()> class Formatter : public FormatterDetail<S, L> {
	};

#	define AdHocFormatterTypedef(name, str, id) \
		inline constexpr auto id = str; \
		using name = ::AdHoc::Formatter<id>
#	define AdHocFormatter(name, str) AdHocFormatterTypedef(name, str, MAKE_UNIQUE(name))

#endif

	namespace literals {
#ifdef USE_FIXED_STRING
		template<const support::basic_fixed_string Str> constexpr inline auto operator""_fmt() noexcept
		{
			return AdHoc::FormatterDetail<Str, Str.size()>();
		}
#else
#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wgnu-string-literal-operator-template"
#	endif
		/// CTF format string holder
		template<typename T, T... t> struct FMT {
			/// CTF format string
			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
			static constexpr char fmtstr[] = {t...};
		};
		template<typename T, T... t> inline auto operator""_fmt() noexcept
		{
			return AdHoc::FormatterDetail<FMT<T, t...>::fmtstr, sizeof...(t)>();
		}
#endif
#ifdef __clang__
#	pragma clang diagnostic pop
#endif
	}
}

#endif
