#ifndef ADHOCUTIL_REFLECTION_H
#define ADHOCUTIL_REFLECTION_H

#include <string.h>
#include <string>
#include <map>
#include <istream>
#include <functional>
#include <memory>
#include <boost/lexical_cast.hpp>
#ifndef yyFlexLexer
#define yyFlexLexer nvpBaseFlexLexer
#include <FlexLexer.h>
#endif
#include <visibility.h>

namespace AdHoc {

/// Name=Value parser.
/**
 * Parses an input stream of the format Name=Value;Name2=Value2;... into a predefined object
 * structure.
 */
class NvpParse : public yyFlexLexer {
	public:
		/// @cond
		/// Thrown in the event of the input referring to a member that doesn't exist.
		class ValueNotFound : public std::runtime_error {
			public:
				ValueNotFound(const std::string &);
		};

		typedef std::function<void(const std::string &)> AssignFunc;
		typedef std::map<std::string, AssignFunc> AssignMap;

		template <typename T>
		class TargetBase {
			public:
				virtual AssignFunc assign(T *) const = 0;
		};

		template <typename T, typename V>
		class Target : public TargetBase<T> {
			public:
				Target(V T::*t) :
					target(t)
				{
				}

				AssignFunc assign(T * t) const override
				{
					return [t,this](const std::string & value) {
						t->*target = boost::lexical_cast<V>(value);
					};
				}

			private:
				V T::*target;
		};
		/// @endcond

#define NvpTarget(T) std::map<std::string, std::shared_ptr<::AdHoc::NvpParse::TargetBase<T>>>
#define NvpValue(c, m) { #m, std::shared_ptr<::AdHoc::NvpParse::Target<c, decltype(c::m)>>(new ::AdHoc::NvpParse::Target<c, decltype(c::m)>(&c::m)) }

		/** Parse an input stream into the given object.
		 * @param in The input stream.
		 * @param tm The Target Map for the object.
		 * @param t The target instance to populate.
		 */
		template <typename T>
		static void parse(std::istream & in, const NvpTarget(T) & tm, T & t)
		{
			NvpParse::AssignMap am;
			for (const auto & v : tm) {
				am[v.first] = v.second->assign(&t);
			}
			return parse(in, am);
		}

		/** Don't use this function directly, instead use:
		 * @code {.cpp} void parse(std::istream & in, const NvpTarget(T) & tm, T & t) @endcode
		 */
		DLL_PUBLIC static void parse(std::istream & in, const AssignMap & m);

	private:
		NvpParse(std::istream & in, const AssignMap &);
		~NvpParse();

		int yylex() override;
		void LexerError(const char * msg) override;

		void process(const std::string & value) const;
		std::string name;
		const AssignMap values;
};

}

#endif

