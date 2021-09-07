%option batch
%option c++
%option noyywrap
%option 8bit
%option stack
%option yyclass="AdHoc::NvpParse"
%option prefix="nvpBase"

%{
#include "nvpParse.h"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#if __clang__
#pragma GCC diagnostic ignored "-Wnull-conversion"
#endif
%}

element [a-zA-Z][a-zA-Z0-9_-]*
identifier {element}("."{element})*
eq "="
value [^ ][^;]*
semi ";"

%x VALUE
%x EQUAL
%x SEMI

%%

<INITIAL>{identifier} {
	name = YYText();
	BEGIN(EQUAL);
}

<EQUAL>{eq} {
	BEGIN(VALUE);
}

<VALUE>{value} {
	process(YYText());
	BEGIN(SEMI);
}

<SEMI>{semi} {
	BEGIN(INITIAL);
}

<*>[ \t\r\n\f] {
}

<*>. {
	throw std::runtime_error(std::string("Lex error at: ") + YYText());
}

%%
#include "safeMapFind.h"

namespace AdHoc {

NvpParse::ValueNotFound::ValueNotFound(const std::string & vn) :
	std::runtime_error("Value not found: " + vn)
{
}

NvpParse::NvpParse(std::istream & in, const AssignMap & v) :
	yyFlexLexer(&in),
	values(v)
{
}

void
NvpParse::process(const std::string & value) const
{
	safeMapLookup<ValueNotFound>(values, name)(value);
}

void
NvpParse::LexerError(const char * msg)
{
	throw std::runtime_error(msg);
}

void
NvpParse::parse(std::istream & in, const AssignMap & m)
{
	NvpParse p(in, m);
	p.yylex();
}

}

