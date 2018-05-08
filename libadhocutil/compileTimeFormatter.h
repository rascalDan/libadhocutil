#include <sstream>
#include <string.h>
#include <boost/preprocessor/variadic/size.hpp>
#include "unique.h"

namespace AdHoc {
	/// @cond
	constexpr int WRAP_AT = 120;

	namespace FormatterImpl {
		template <bool, char...> struct ParserBuffer { };
		template <int, char...> struct Buffer { };
	}

	template <const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter {
		template<typename ... Pn>
		static void write(stream & s, const Pn & ... pn);

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

	template<const char * const & S, int start, int len, typename stream, char ... sn>
	struct StreamWriterBase {
		template<typename ... Pn>
		static void next(stream & s, const Pn & ... pn)
		{
			StreamWriter<S, start + len, stream, sn...>::write(s, pn...);
		}
	};

#define StreamWriterT(C...) \
	template<const char * const & S, int start, typename stream, char ... sn> \
	struct StreamWriter<S, start, stream, '%', C, sn...> : \
		public StreamWriterBase<S, start, BOOST_PP_VARIADIC_SIZE(C) + 1, stream, sn...>

#define StreamWriterTP(P, C...) \
	template<const char * const & S, int start, typename stream, char P, char ... sn> \
	struct StreamWriter<S, start, stream, '%', C, sn...> : \
		public StreamWriterBase<S, start, BOOST_PP_VARIADIC_SIZE(C) + 1, stream, sn...>

	// Default stream writer formatter
	StreamWriterT('?') {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << p;
			StreamWriter::next(s, pn...);
		}
	};

	// Escaped % stream writer formatter
	StreamWriterT('%') {
		template<typename ... Pn>
		static void write(stream & s, const Pn & ... pn)
		{
			s << '%';
			StreamWriter::next(s, pn...);
		}
	};

	// Unknown stream writer formatter
	template<const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter<S, start, stream, '%', sn...> {
		template<typename ... Pn>
		static void write(stream &, const Pn & ...)
		{
			static_assert(!&err, "invalid format string/arguments");
		}
		static int err;
	};

	/**
	 * Compile time string formatter.
	 * @param S the format string.
	 */
	template <const char * const & S>
	class Formatter {
		private:
			template<const char * const &, int, typename, char ...> friend struct StreamWriter;

			template <int start, int offset, char ...>
			struct Upto {
				template<typename stream>
				static auto scan(stream &, const FormatterImpl::Buffer<start> & f)
				{
					return f;
				}
			};
			template <int start, int offset, char s0, char... sn>
			struct Upto<start, offset, s0, sn...> {
				template<typename stream, int len, char... sm>
				static auto scan(stream & s, const FormatterImpl::Buffer<len, sm...> &)
				{
					return Upto<start, offset + 1, sn...>::scan(s, FormatterImpl::Buffer<len, sm..., s0>());
				}
			};
			template <int start, char... sn>
			struct UptoWrite {
				template<typename stream, int len, char... sm>
				static auto scan(stream & s, const FormatterImpl::Buffer<len, sm...> &)
				{
					s.write(S + start, sizeof...(sm));
					return FormatterImpl::Buffer<sizeof...(sm), sn...>();
				}
			};
			template <int start, int offset, char... sn>
			struct Upto<start, offset, '%', sn...> : public UptoWrite<start, '%', sn...> { };
			template <int start, int offset, char... sn>
			struct Upto<start, offset, 0, sn...> : public UptoWrite<start, 0, sn...> { };
			template <int start, char s0, char... sn>
			struct Upto<start, WRAP_AT, s0, sn...> : public UptoWrite<start, s0, sn...> { };
			template <int start, char... sn>
			struct Upto<start, WRAP_AT, '%', sn...> : public UptoWrite<start, '%', sn...> { };
			template <int start, char... sn>
			struct Upto<start, WRAP_AT, 0, sn...> : public UptoWrite<start, 0, sn...> { };

			template <int offset, int roffset, char s0, char ... sn>
			struct Parser {
				static auto parse()
				{
					return append(innerparse());
				}
				static auto innerparse()
				{
					return Parser<offset + 1, roffset + 1, S[offset + 1], sn..., s0>::innerparse();
				}
				template<char...ssn>
				static auto append(const FormatterImpl::ParserBuffer<true, ssn...> & b)
				{
					return join(b, Parser<offset + 1 + WRAP_AT, 0, S[offset + 1 + WRAP_AT]>::parse());
				}
				template<char...ssn>
				static auto append(const FormatterImpl::ParserBuffer<false, ssn...> & b)
				{
					return b;
				}
				template<bool more, char...ssn, char...ssm>
				static auto join(const FormatterImpl::ParserBuffer<true, ssn...> &, const FormatterImpl::ParserBuffer<more, ssm...> &)
				{
					return FormatterImpl::ParserBuffer<more, ssn..., ssm...>();
				}
			};

			template <bool more, char ... sn>
			struct ParserBase {
				static auto parse()
				{
					return innerparse();
				}
				static auto innerparse()
				{
					return FormatterImpl::ParserBuffer<more, sn...>();
				}
			};

			template <int offset, char s0, char ... sn>
			struct Parser<offset, WRAP_AT, s0, sn...> : public ParserBase<true, sn..., s0> { };

			template <int offset, char ... sn>
			struct Parser<offset, WRAP_AT, 0, sn...> : public ParserBase<true, sn..., 0> { };

			template <int offset, int roffset, char ... sn>
			struct Parser<offset, roffset, 0, sn...> : public ParserBase<false, sn..., 0>{ };

		public:
			/**
			 * Get a string containing the result of formatting.
			 * @param pn the format arguments.
			 * @return the formatted string.
			 */
			template<typename ... Pn>
			static std::string get(const Pn & ... pn)
			{
				std::stringstream s;
				return run(Parser<0, 0, *S>::parse(), s, pn...).str();
			}

			/**
			 * Write the result of formatting to the given stream.
			 * @param s the stream to write to.
			 * @param pn the format arguments.
			 * @return the stream.
			 */
			template<typename stream, typename ... Pn>
			static stream & write(stream & s, const Pn & ... pn)
			{
				return run(Parser<0, 0, *S>::parse(), s, pn...);
			}

		private:
			template<typename stream, char...ssn, template<bool, char...> class ParserBuffer, typename ... Pn>
			static stream & run(const ParserBuffer<false, ssn...> &, stream & s, const Pn & ... pn)
			{
				StreamWriter<S, 0, stream, ssn...>::write(s, pn...);
				return s;
			}
	};

	/// @cond
	template <const char * const & S, int start, typename stream, char ... sn>
	template<typename ... Pn>
	void StreamWriter<S, start, stream, sn...>::write(stream & s, const Pn & ... pn)
	{
		next(s, Formatter<S>::template Upto<start, 0, sn...>::scan(s, FormatterImpl::Buffer<0>()), pn...);
	}
	/// @endcond
}

#define AdHocFormatterTypedef(name, str, id) \
	extern constexpr auto id = str; \
	typedef ::AdHoc::Formatter<id> name
#define AdHocFormatter(name, str) \
	AdHocFormatterTypedef(name, str, MAKE_UNIQUE(name))

