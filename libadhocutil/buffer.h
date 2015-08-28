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
		enum CStringHandling { Use, Copy, Free };

		typedef boost::intrusive_ptr<Buffer> Ptr;
		typedef boost::intrusive_ptr<const Buffer> CPtr;

		Buffer();
		Buffer(const char * src, CStringHandling);
		Buffer(char * src, CStringHandling);
		Buffer(const std::string &);
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

		Buffer & append(const char * str, CStringHandling h);
		Buffer & append(char * str, CStringHandling h);
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
		Buffer & clear();

		size_t length() const;
		std::string str() const;

		static boost::shared_ptr<boost::format> getFormat(const std::string & msgfmt);

	private:
		Buffer & appendbf(boost::format & fmt);
		void DLL_PRIVATE flatten() const;

		class DLL_PRIVATE FragmentBase : public virtual IntrusivePtrBase {
			public:
				virtual ~FragmentBase() = 0;

				virtual size_t length() const = 0;
				virtual char operator[](size_t) const = 0;
				virtual const char * c_str() const = 0;
				virtual std::string str() const = 0;
		};

		class DLL_PRIVATE CStringFragment : public FragmentBase {
			public:
				CStringFragment(const char *, CStringHandling);
				CStringFragment(const char *, CStringHandling, size_t);
				CStringFragment(char *, CStringHandling);
				CStringFragment(char *, CStringHandling, size_t);
				~CStringFragment();

				size_t length() const;
				char operator[](size_t) const;
				const char * c_str() const;
				std::string str() const;

			private:
				const size_t len; // Excluding NULL term
				const char * buf;
				const CStringHandling handling;
		};

		class DLL_PRIVATE StringFragment : public FragmentBase {
			public:
				StringFragment(const std::string &);

				size_t length() const;
				char operator[](size_t) const;
				const char * c_str() const;
				std::string str() const;

			private:
				const std::string buf;
		};

		typedef boost::intrusive_ptr<FragmentBase> FragmentPtr;
		typedef std::vector<FragmentPtr> Content;
		mutable Content content;
};
Buffer::FragmentBase::~FragmentBase() = default;

// libmisc compat macros
#define vstringf Buffer().vappendf
#define stringf Buffer().appendf
#define stringbf Buffer().appendbf

#endif

