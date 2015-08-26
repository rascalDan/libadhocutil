#ifndef ADHOC_BUFFER_H
#define ADHOC_BUFFER_H

#include "intrusivePtrBase.h"
#include <string>
#include <vector>
#include <stdarg.h>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include "visibility.h"

class DLL_PUBLIC Buffer;

namespace std {
	DLL_PUBLIC std::ostream & operator<<(std::ostream &, const Buffer &);    	
}

class DLL_PUBLIC Buffer : public virtual IntrusivePtrBase {
	public:
		typedef boost::intrusive_ptr<Buffer> Ptr;
		typedef boost::intrusive_ptr<const Buffer> CPtr;

		Buffer();
		Buffer(const char * src);
		Buffer(char * src, bool copy);
		~Buffer();

		Buffer & operator+=(const char * str);
		Buffer & operator+=(const std::string & str);
		Buffer & operator=(const char * str);
		Buffer & operator=(const std::string & str);
		Buffer & operator=(const Buffer & str);
		bool operator!() const;

		operator bool() const;
		operator std::string() const;
		operator const char *() const;

		void writeto(char * buf, size_t bufSize, size_t off) const;
		friend std::ostream & std::operator<<(std::ostream &, const Buffer &);

		Buffer & append(const char * str);
		Buffer & append(char * str, bool copy);
		Buffer & append(const std::string & str);
		Buffer & appendf(const char * fmt, ...) __attribute__((format (printf, 2, 3)));
		Buffer & vappendf(const char * fmt, va_list args);
		template <typename ... Params>
		Buffer & appendbf(const std::string & fmtstr, const Params & ... params)
		{
			return appendbf(*getFormat(fmtstr), params...);
		}
		template <typename Param, typename ... Params>
		Buffer & appendbf(boost::format & fmt, const Param & param, const Params & ... params)
		{
			fmt % param;
			return appendbf(fmt, params...);
		}
		Buffer & appendbf(boost::format & fmt);
		Buffer & clear();

		size_t length() const;
		std::string str() const;

		static boost::shared_ptr<boost::format> getFormat(const std::string & msgfmt);

	private:
		void DLL_PRIVATE flatten() const;

		class DLL_PRIVATE Fragment : public virtual IntrusivePtrBase {
			public:
				typedef boost::intrusive_ptr<Fragment> Ptr;
				typedef boost::intrusive_ptr<const Fragment> CPtr;

				Fragment(const char *, size_t);
				Fragment(const char *);
				Fragment(char *, size_t, bool);
				~Fragment();

				size_t len; // Excluding NULL term
				char * buf;
		};
		typedef std::vector<Fragment::Ptr> Content;
		mutable Content content;
};

#endif

