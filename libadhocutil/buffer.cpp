#include "buffer.h"
#include <string.h>
#include <stdio.h>

Buffer::FragmentBase::~FragmentBase() = default;

//
// CString Fragment
//

Buffer::CStringFragment::CStringFragment(const char * b, CStringHandling h) :
	len(strlen(b)),
	buf(b),
	handling(h)
{
}

Buffer::CStringFragment::CStringFragment(const char * b, CStringHandling h, size_t l) :
	len(l),
	buf(b),
	handling(h)
{
}

Buffer::CStringFragment::CStringFragment(char * b, CStringHandling h) :
	len(strlen(b)),
	buf(b),
	handling(h)
{
}

Buffer::CStringFragment::CStringFragment(char * b, CStringHandling h, size_t l) :
	len(l),
	buf(b),
	handling(h)
{
}

Buffer::CStringFragment::~CStringFragment()
{
	if (handling != Use) { // Copy or Free
		free(const_cast<char *>(buf));
	}
}

char
Buffer::CStringFragment::operator[](size_t x) const
{
	return buf[x];
}

size_t
Buffer::CStringFragment::length() const
{
	return len;
}

const char *
Buffer::CStringFragment::c_str() const
{
	return buf;
}

std::string
Buffer::CStringFragment::str() const
{
	return buf;
}

//
// Std::String Fragment
//

Buffer::StringFragment::StringFragment(const std::string & str) :
	buf(str)
{
}

size_t
Buffer::StringFragment::length() const
{
	return buf.length();
}

const char *
Buffer::StringFragment::c_str() const
{
	return buf.c_str();
}

std::string
Buffer::StringFragment::str() const
{
	return buf;
}

char
Buffer::StringFragment::operator[](size_t x) const
{
	return buf[x];
}

//
// Buffer :)
//

Buffer::Buffer()
{
}

Buffer::Buffer(const char * src, CStringHandling h)
{
	append(src, h);
}

Buffer::Buffer(char * src, CStringHandling h)
{
	append(src, h);
}

Buffer::Buffer(const std::string & str)
{
	append(str);
}

Buffer::~Buffer()
{
}

Buffer &
Buffer::append(const char * str, CStringHandling h)
{
	if (str && *str) {
		if (h == Copy) {
			content.push_back(new StringFragment(str));
		}
		else {
			content.push_back(new CStringFragment(str, h));
		}
	}
	return *this;
}

Buffer &
Buffer::append(char * str, CStringHandling h)
{
	if (str && *str) {
		if (h == Copy) {
			content.push_back(new StringFragment(str));
		}
		else {
			content.push_back(new CStringFragment(str, h));
		}
	}
	return *this;
}

Buffer &
Buffer::append(const std::string & str)
{
	if (!str.empty()) {
		content.push_back(new StringFragment(str));
	}
	return *this;
}

Buffer &
Buffer::appendf(const char * fmt, ...)
{
	va_list v;
	va_start(v, fmt);
	vappendf(fmt, v);
	va_end(v);
	return *this;
}

Buffer &
Buffer::vappendf(const char * fmt, va_list args)
{
	char * frag;
	size_t len = vasprintf(&frag, fmt, args);
	if (len > 0) {
		content.push_back(new CStringFragment(frag, Free, len));
	}
	else {
		free(frag);
	}
	return *this;
}

Buffer &
Buffer::appendbf(boost::format & fmt)
{
	append(fmt.str());
	return *this;
}

Buffer &
Buffer::clear()
{
	content.clear();
	return *this;
}

boost::shared_ptr<boost::format>
Buffer::getFormat(const std::string & msgfmt)
{
	return boost::shared_ptr<boost::format>(new boost::format(msgfmt));
}

void
Buffer::writeto(char * buf, size_t bufSize, size_t off) const
{
	Content::const_iterator f = content.begin();
	while (f != content.end() && (*f)->length() < off) {
		off -= (*f)->length();
		++f;
	}
	while (f != content.end() && bufSize) {
		for (size_t c = 0; bufSize && c < (*f)->length(); bufSize--) {
			*buf++ = (**f)[c++];
		}
		++f;
		off = 0;
	}
	*buf = '\0';
}

Buffer::operator std::string() const
{
	if (content.size() > 1) {
		std::string res;
		res.reserve(length());
		for (const auto & f : content) {
			res.append(f->str());
		}
		return res;
	}
	else if (content.size() == 1) {
		return std::string(content.front()->str());
	}
	return std::string();
}

Buffer::operator const char *() const
{
	if (content.empty()) {
		return "";
	}
	flatten();
	return content.front()->c_str();
}

void
Buffer::flatten() const
{
	if (content.size() > 1) {
		auto f = new StringFragment(str());
		content.resize(1);
		content.front() = f;
	}
}

std::string
Buffer::str() const
{
	return *this;
}

size_t
Buffer::length() const
{
	size_t len = 0;
	for (const Content::value_type & c : content) {
		len += c->length();
	}
	return len;
}

Buffer &
Buffer::operator=(const char * str)
{
	content.resize(1);
	content.front() = new StringFragment(str);
	return *this;
}

Buffer &
Buffer::operator=(const std::string & str)
{
	content.resize(1);
	content.front() = new StringFragment(str);
	return *this;
}

Buffer &
Buffer::operator=(const Buffer & buf)
{
	content = buf.content;
	return *this;
}

Buffer::operator bool() const
{
	return !content.empty();
}

bool
Buffer::operator!() const
{
	return content.empty();
}

std::ostream &
std::operator<<(std::ostream & os, const Buffer & b)
{
	for (const auto & f : b.content) {
		os.write(f->c_str(), f->length());
	}
	return os;
}

Buffer &
Buffer::operator+=(const char * str)
{
	return append(str);
}

Buffer &
Buffer::operator+=(const std::string & str)
{
	return append(str);
}

