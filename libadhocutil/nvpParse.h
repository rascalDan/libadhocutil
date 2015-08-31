#ifndef ADHOCUTIL_REFLECTION_H
#define ADHOCUTIL_REFLECTION_H

#include <string.h>
#include <string>
#include <map>
#include <istream>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#ifndef yyFlexLexer
#define yyFlexLexer nvpBaseFlexLexer
#include <FlexLexer.h>
#endif
#include <visibility.h>

class NvpParse : public yyFlexLexer {
	public:
		class ValueNotFound : public std::runtime_error {
			public:
				ValueNotFound(const std::string &);
		};

		typedef boost::function<void(const std::string &)> AssignFunc;
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
					return [t,this](const auto & value) {
						t->*target = boost::lexical_cast<V>(value);
					};
				}

			private:
				V T::*target;
		};

#define NvpTarget(T) std::map<std::string, boost::shared_ptr<NvpParse::TargetBase<T>>>
#define NvpValue(c, m) { #m, boost::shared_ptr<NvpParse::Target<c, decltype(c::m)>>(new NvpParse::Target<c, decltype(c::m)>(&c::m)) }

		template <typename T>
		static void parse(std::istream & in, const NvpTarget(T) & tm, T & t)
		{
			NvpParse::AssignMap am;
			for (const auto & v : tm) {
				am[v.first] = v.second->assign(&t);
			}
			return parse(in, am);
		}

		DLL_PUBLIC static void parse(std::istream & in, const AssignMap & m);

	private:
		NvpParse(std::istream & in, const AssignMap &);
		~NvpParse() = default;

		int yylex() override;
		void LexerError(const char * msg) override;

		void process(const std::string & value) const;
		std::string name;
		const AssignMap values;
};

#endif

