#ifndef ADHOCUTIL_LEXER_H
#define ADHOCUTIL_LEXER_H

#include <vector>
#include <glibmm/ustring.h>
#include <set>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>
#include "visibility.h"

namespace AdHoc {
	class DLL_PUBLIC Lexer {
		public:
			class Pattern {
				public:
					virtual ~Pattern() = default;

					virtual bool matches(const gchar *, size_t, size_t) const = 0;
					virtual size_t matchedLength() const = 0;
					virtual boost::optional<Glib::ustring> match(int) const = 0;
			};
			typedef boost::shared_ptr<Pattern> PatternPtr;

			typedef std::string State;
			typedef std::set<State> States;

			class ExecuteState {
				public:
					ExecuteState();

					void pushState(const State &);
					void popState();
					void setState(const State &);
					const State & getState() const;
					size_t depth() const;

					size_t position;
					PatternPtr pattern;

				private:
					std::vector<State> stateStack;
			};

			typedef boost::function<void(ExecuteState *)> Handler;
			typedef boost::tuple<States, PatternPtr, Handler> Rule;
			typedef std::vector<Rule> Rules;

			static const State InitialState;
			Lexer();
			Lexer(const Rules &);

			Rules rules;

			void extract(const gchar * string, size_t length) const;
	};
}

#endif

