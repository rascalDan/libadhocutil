#ifndef ADHOCUTIL_BUFFER_H
#define ADHOCUTIL_BUFFER_H

#include "intrusivePtrBase.h"
#include <string>
#include <vector>
#include <stdarg.h>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>
#include "visibility.h"

namespace AdHoc {
	class DLL_PUBLIC Buffer;
}

namespace std {
	DLL_PUBLIC std::ostream & operator<<(std::ostream &, const AdHoc::Buffer &);
}

namespace AdHoc {

/// High-speed text buffer for easy creation of programatically created strings.
class DLL_PUBLIC Buffer : public virtual IntrusivePtrBase {
	public:
		/** How should Buffer handle char * arguments? */
		enum CStringHandling {
			/// Use the memory passed verbatim.
			Use,
			/// Copy the memory passed to it for it's own use.
			Copy,
			/// Use the memory passed verbatim and free it when finished.
			Free
		};

		/// Pointer typedef.
		typedef boost::intrusive_ptr<Buffer> Ptr;
		/// Const pointer typedef.
		typedef boost::intrusive_ptr<const Buffer> CPtr;

		/** Create an empty buffer */
		Buffer();
		/** Create a buffer containing a single element from the given char * */
		Buffer(const char * src, CStringHandling);
		/** Create a buffer containing a single element from the given char * */
		Buffer(char * src, CStringHandling);
		/** Create a buffer containing a single element from the given std::string */
		Buffer(const std::string &);
		~Buffer();

		/** Append the given char * (will copy) */
		Buffer & operator+=(const char * str);
		/** Append the given std::string */
		Buffer & operator+=(const std::string & str);
		/** Replace all current content with the given char * (will copy) */
		Buffer & operator=(const char * str);
		/** Replace all current content with the given std::string */
		Buffer & operator=(const std::string & str);
		/** Replace all current content with the given Buffer's elements. Shallow copy as all elements are immutable. */
		Buffer & operator=(const Buffer & str);
		/** true if the buffer contains no elements (empty) */
		bool operator!() const;
		/** true if the buffer contains no elements (non-empty) */
		operator bool() const;
		/** Converts all elements into a single flattened std::string */
		operator std::string() const;
		/** Converts all elements into a single flattened char * */
		operator const char *() const;

		/**
		 * Writes all elements in turn to the given buffer space, including a null terminator.
		 * @param buf Address of buffer to write into.
		 * @param bufSize Maximum number of bytes to write.
		 * @param off Effective starting position to copy from.
		 */
		void writeto(char * buf, size_t bufSize, size_t off) const;
		/** Write the Buffer to a std::ostream */
		friend std::ostream & std::operator<<(std::ostream &, const Buffer &);

		/** Append the given char * to the end of the buffer. */
		Buffer & append(const char * str, CStringHandling h);
		/** Append the given char * to the end of the buffer. */
		Buffer & append(char * str, CStringHandling h);
		/** Append the given std::string to the end of the buffer. */
		Buffer & append(const std::string & str);
		/** Append the given printf style format string and arguments to the buffer. */
		Buffer & appendf(const char * fmt, ...) __attribute__((format (printf, 2, 3)));
		/** Append the given printf style format string and va_list to the buffer. */
		Buffer & vappendf(const char * fmt, va_list args);
		/** Append the given boost::format style format string and arguments to the buffer. */
		template <typename ... Params>
		Buffer & appendbf(const std::string & fmtstr, const Params & ... params)
		{
			return appendbf(*getFormat(fmtstr), params...);
		}
		/** Append the given boost::format and arguments to the buffer. */
		template <typename Param, typename ... Params>
		Buffer & appendbf(boost::format & fmt, const Param & param, const Params & ... params)
		{
			fmt % param;
			return appendbf(fmt, params...);
		}
		/** Clear the Buffer of all content. */
		Buffer & clear();
		/** Get the total size of all elements. */
		size_t length() const;
		/** Test if the Buffer is empty. */
		bool empty() const;
		/** Flattern the Buffer and return it as a std::string. */
		std::string str() const;

		/** Helper function to centralize the construction of boost::format instances. */
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

}

// libmisc compat macros
#define vstringf(...) AdHoc::Buffer().vappendf(__VA_ARGS__).str()
#define stringf(...) AdHoc::Buffer().appendf(__VA_ARGS__).str()
#define stringbf(...) AdHoc::Buffer().appendbf(__VA_ARGS__).str()

#endif

