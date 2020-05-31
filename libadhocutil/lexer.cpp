#include "lexer.h"
#include "compileTimeFormatter.h"

namespace AdHoc {
	const Lexer::State Lexer::InitialState = "";

	Lexer::Lexer() = default;

	Lexer::Lexer(Rules r) : rules(std::move(r)) { }

	AdHocFormatter(UnexpectedInputState, "Unexpected input in state (%?) at %?");
	void
	Lexer::extract(const gchar * string, size_t length) const
	{
		ExecuteState es;
		while (es.pos < length) {
			const Rule * selected = nullptr;
			for (const auto & r : rules) {
				const auto & s = std::get<0>(r);
				if (s.find(es.getState()) == s.end()) {
					continue;
				}
				const auto & p = std::get<1>(r);
				if (p->matches(string, length, es.pos)) {
					selected = &r;
					break;
				}
			}
			if (!selected) {
				throw std::runtime_error(UnexpectedInputState::get(es.getState(), string + es.pos));
			}
			es.pat = std::get<1>(*selected);
			const auto & h = std::get<2>(*selected);
			h(&es);
			es.pos += es.pat->matchedLength();
		}
	}

	Lexer::ExecuteState::ExecuteState()
	{
		stateStack.push_back(InitialState);
	}

	void
	Lexer::ExecuteState::setState(const State & s)
	{
		stateStack.back() = s;
	}

	void
	Lexer::ExecuteState::pushState(const State & s)
	{
		stateStack.push_back(s);
	}

	void
	Lexer::ExecuteState::popState()
	{
		stateStack.pop_back();
	}

	const Lexer::State &
	Lexer::ExecuteState::getState() const
	{
		return stateStack.back();
	}

	size_t
	Lexer::ExecuteState::depth() const
	{
		return stateStack.size();
	}

	size_t
	Lexer::ExecuteState::position() const
	{
		return pos;
	}

	Lexer::PatternPtr
	Lexer::ExecuteState::pattern() const
	{
		return pat;
	}
}
