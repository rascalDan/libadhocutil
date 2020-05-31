#ifndef ADHOCUTIL_REFLECTION_H
#define ADHOCUTIL_REFLECTION_H

#include <boost/lexical_cast.hpp>
#include <functional>
#include <istream>
#include <map>
#include <memory>
#include <string>
#ifndef yyFlexLexer
#	define yyFlexLexer nvpBaseFlexLexer
#	include <FlexLexer.h>
#endif
#include "c++11Helpers.h"
#include "visibility.h"

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
			explicit ValueNotFound(const std::string &);
		};

		using AssignFunc = std::function<void(const std::string &)>;
		using AssignMap = std::map<std::string, AssignFunc>;

		template<typename T> class TargetBase {
		public:
			TargetBase() = default;
			virtual ~TargetBase() = default;
			virtual AssignFunc assign(T *) const = 0;
			SPECIAL_MEMBERS_DEFAULT(TargetBase);
		};

		template<typename T, typename V> class Target : public TargetBase<T> {
		public:
			explicit Target(V T::*t) : target(t) { }

			AssignFunc
			assign(T * t) const override
			{
				return [t, this](const std::string & value) {
					t->*target = boost::lexical_cast<V>(value);
				};
			}

		private:
			V T::*target;
		};
		/// @endcond

#define NvpTarget(T) std::map<std::string, std::shared_ptr<::AdHoc::NvpParse::TargetBase<T>>>
#define NvpValue(c, m) \
	{ \
#		m, std::make_shared < ::AdHoc::NvpParse::Target < c, decltype(c::m)>>(&c::m) \
	}

		/** Parse an input stream into the given object.
		 * @param in The input stream.
		 * @param tm The Target Map for the object.
		 * @param t The target instance to populate.
		 */
		template<typename T>
		static void
		parse(std::istream & in, const NvpTarget(T) & tm, T & t)
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

		int yylex() override;
		void LexerError(const char * msg) override;

		void process(const std::string & value) const;
		std::string name;
		const AssignMap values;
	};

}

#endif
