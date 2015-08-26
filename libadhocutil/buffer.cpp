#include "buffer.h"
#include <string.h>
#include <stdio.h>

Buffer::Fragment::Fragment(const char * b) :
	len(strlen(b))
{
	buf = (char*)malloc(len + 1);
	memcpy(buf, b, len + 1);
	*(buf + len) = '\0';
}

Buffer::Fragment::Fragment(const char * b, size_t l) :
	len(l)
{
	buf = (char*)malloc(l + 1);
	memcpy(buf, b, l + 1);
	*(buf + len) = '\0';
}

Buffer::Fragment::Fragment(char * b, size_t l, bool copy) :
	len(l)
{
	if (copy) {
		buf = (char*)malloc(l + 1);
		memcpy(buf, b, l + 1);
	}
	else {
		buf = b;
	}
	*(buf + len) = '\0';
}

Buffer::Fragment::~Fragment()
{
	free(buf);
}

Buffer::Buffer()
{
}

Buffer::Buffer(const char * src)
{
	content.push_back(new Fragment(src));
}

Buffer::Buffer(char * src, bool copy)
{
	content.push_back(new Fragment(src, strlen(src), copy));
}

Buffer::~Buffer()
{
}

Buffer &
Buffer::append(const char * str)
{
	if (str && *str) {
		content.push_back(new Fragment(str));
	}
	return *this;
}

Buffer &
Buffer::append(char * str, bool copy)
{
	if (str && *str) {
		content.push_back(new Fragment(str, strlen(str), copy));
	}
	return *this;
}

Buffer &
Buffer::append(const std::string & str)
{
	if (!str.empty()) {
		content.push_back(new Fragment(str.c_str(), str.length()));
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
		content.push_back(new Fragment(frag, len, false));
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
	while (f != content.end() && (*f)->len < off) {
		off -= (*f)->len;
		++f;
	}
	while (f != content.end() && bufSize) {
		for (size_t c = 0; bufSize && c < (*f)->len; bufSize--) {
			*buf++ = (*f)->buf[c++];
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
		for (const Fragment::Ptr & f : content) {
			res.append(f->buf, f->len);
		}
		return res;
	}
	else if (content.size() == 1) {
		return std::string(content.front()->buf, content.front()->len);
	}
	return std::string();
}

Buffer::operator const char *() const
{
	if (content.empty()) {
		return "";
	}
	flatten();
	return content.front()->buf;
}

void
Buffer::flatten() const
{
	if (content.size() > 1) {
		auto len = length();
		auto buf = (char*)malloc(len + 1);
		auto f = new Fragment(buf, len, false);
		for (const Fragment::Ptr & f : content) {
			memcpy(buf, f->buf, f->len);
			buf += f->len;
		}
		*buf = '\0';
		content.clear();
		content.push_back(f);
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
		len += c->len;
	}
	return len;
}

Buffer &
Buffer::operator=(const char * str)
{
	content.resize(1);
	content[0] = new Fragment(str);
	return *this;
}

Buffer &
Buffer::operator=(const std::string & str)
{
	content.resize(1);
	content[0] = new Fragment(str.c_str(), str.length());
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
		os.write(f->buf, f->len);
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

