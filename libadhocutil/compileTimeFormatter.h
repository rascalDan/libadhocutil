#pragma once

#include <array>
#include <boost/preprocessor/control/iif.hpp> // IWYU pragma: keep
#include <boost/preprocessor/variadic/size.hpp> // IWYU pragma: keep
#include <cstddef>
#include <iostream>
#include <optional>
#include <sstream> // IWYU pragma: export
#include <type_traits>
// Mapped for for BOOST_PP_VARIADIC_SIZE, BOOST_PP... in tests
// IWYU pragma: no_include <boost/test/unit_test.hpp>

namespace AdHoc {
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
	template<const auto S>
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

	template<const auto S, auto n, auto start = 0U, auto L = strlen<S>()>
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

	template<const auto S, auto n, auto start = 0U, auto L = strlen<S>()>
	static constexpr decltype(L)
	strchrnul()
	{
		decltype(start) off = start;
		while (off < L && S[off] != n) {
			++off;
		}
		return off;
	}

	template<const auto S, const auto L> class FormatterDetail;

	/// Template used to apply parameters to a stream.
	template<const auto S, auto L, auto pos, typename stream, typename, auto...> struct StreamWriter {
		/// Write parameters to stream.
		template<typename... Pn>
		static void
		write(stream &, const Pn &...)
		{
			static_assert(!L, "invalid format string/arguments");
		}
	};

	/// Helper to simplify implementations of StreamWriter.
	template<const auto S, auto L, auto pos, typename stream> struct StreamWriterBase {
		/// Continue processing parameters.
		template<typename... Pn>
		static inline void
		next(stream & s, const Pn &... pn)
		{
			FormatterDetail<S, L>::template Parser<stream, pos + 1, Pn...>::run(s, pn...);
		}
	};

#define StreamWriterT(...) \
	template<const auto S, auto L, auto pos, typename stream, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', __VA_ARGS__, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__) + pos, stream>

#define StreamWriterTP(P, ...) \
	template<const auto S, auto L, auto pos, typename stream, auto P, auto... sn> \
	struct StreamWriter<S, L, pos, stream, void, '%', __VA_ARGS__, sn...> : \
		public StreamWriterBase<S, L, BOOST_PP_VARIADIC_SIZE(__VA_ARGS__) + pos, stream>

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
	appendStream(stream & s, const char_type * p, std::streamsize n)
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
	template<const auto S, const auto L> class FormatterDetail {
	private:
		using strlen_t = decltype(strlen<S>());
		template<const auto, auto, auto, typename> friend struct StreamWriterBase;

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

	namespace support {
		template<typename CharT, std::size_t N> class basic_fixed_string : public std::array<CharT, N> {
		public:
			// cppcheck-suppress noExplicitConstructor
			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,modernize-avoid-c-arrays,hicpp-explicit-conversions)
			constexpr basic_fixed_string(const CharT (&str)[N + 1])
			{
				for (decltype(N) x = 0; x < N; x++) {
					this->at(x) = str[x];
				}
			}
			// NOLINTNEXTLINE(hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
			constexpr basic_fixed_string(const CharT * str, decltype(N) len)
			{
				for (decltype(N) x = 0; x < len; x++) {
					this->at(x) = str[x];
				}
			}
		};

		template<typename CharT, std::size_t N>
		// NOLINTNEXTLINE(hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
		basic_fixed_string(const CharT (&str)[N])->basic_fixed_string<CharT, N - 1>;
	}

	template<const support::basic_fixed_string Str> class LiteralFormatter : public FormatterDetail<Str, Str.size()> {
	};

	template<const auto & S, decltype(strlen(S)) L = strlen(S)>
	class Formatter :
		public FormatterDetail<support::basic_fixed_string<typename std::decay<decltype(S[0])>::type, L>(S, L), L> {
	};

#define AdHocFormatter(name, str) using name = ::AdHoc::LiteralFormatter<str>

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

	namespace literals {
		template<const support::basic_fixed_string Str> constexpr inline auto operator""_fmt() noexcept
		{
			return AdHoc::FormatterDetail<Str, Str.size()>();
		}
	}
}
