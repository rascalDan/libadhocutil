#ifndef ADHOCUTIL_BUFFER_H
#define ADHOCUTIL_BUFFER_H

#include "c++11Helpers.h"
#include "visibility.h"
#include <boost/format.hpp>
#include <cstdarg>
#include <string>
#include <vector>

namespace AdHoc {
	class DLL_PUBLIC Buffer;
}

namespace std {
	DLL_PUBLIC std::ostream & operator<<(std::ostream &, const AdHoc::Buffer &);
}

namespace AdHoc {

	/// High-speed text buffer for easy creation of programatically created strings.
	class DLL_PUBLIC Buffer {
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
		using Ptr = std::shared_ptr<Buffer>;
		/// Const pointer typedef.
		using CPtr = std::shared_ptr<const Buffer>;

		/** Create an empty buffer */
		Buffer() = default;
		~Buffer() = default;
		/** Create a buffer containing a single element from the given char * */
		Buffer(const char * src, CStringHandling);
		/** Create a buffer containing a single element from the given char * */
		Buffer(char * src, CStringHandling);
		/** Create a buffer containing a single element from the given std::string */
		explicit Buffer(const std::string &);

		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT(Buffer);

		/** Append the given char * (will copy) */
		Buffer & operator+=(const char * str);
		/** Append the given std::string */
		Buffer & operator+=(const std::string & str);
		/** Replace all current content with the given char * (will copy) */
		Buffer & operator=(const char * str);
		/** Replace all current content with the given std::string */
		Buffer & operator=(const std::string & str);
		/** true if the buffer contains no elements (empty) */
		bool operator!() const;
		/** true if the buffer contains no elements (non-empty) */
		explicit operator bool() const;
		/** Converts all elements into a single flattened std::string */
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		operator std::string() const;
		/** Converts all elements into a single flattened char * */
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
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
		Buffer & appendf(const char * fmt, ...) __attribute__((format(printf, 2, 3)));
		/** Append the given printf style format string and va_list to the buffer. */
		Buffer & vappendf(const char * fmt, va_list args);
		/** Append the given boost::format style format string and arguments to the buffer. */
		template<typename... Params>
		Buffer &
		appendbf(const std::string & fmtstr, const Params &... params)
		{
			auto bf = getFormat(fmtstr);
			return appendbf(bf, params...);
		}
		/** Append the given boost::format and arguments to the buffer. */
		template<typename Param, typename... Params>
		Buffer &
		appendbf(boost::format & fmt, const Param & param, const Params &... params)
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
		static boost::format getFormat(const std::string & msgfmt);

	private:
		Buffer & appendbf(boost::format & fmt);
		void DLL_PRIVATE flatten() const;

		class DLL_PRIVATE FragmentBase {
		public:
			FragmentBase() = default;
			virtual ~FragmentBase() = default;
			SPECIAL_MEMBERS_DEFAULT(FragmentBase);

			[[nodiscard]] virtual size_t length() const = 0;
			[[nodiscard]] virtual char operator[](size_t) const = 0;
			[[nodiscard]] virtual const char * c_str() const = 0;
			[[nodiscard]] virtual std::string str() const = 0;
		};

		class DLL_PRIVATE CStringFragment : public FragmentBase {
		public:
			CStringFragment(const char *, CStringHandling);
			CStringFragment(const char *, CStringHandling, size_t);
			CStringFragment(char *, CStringHandling);
			CStringFragment(char *, CStringHandling, size_t);
			SPECIAL_MEMBERS_DELETE(CStringFragment);
			~CStringFragment() override;

			[[nodiscard]] size_t length() const override;
			[[nodiscard]] char operator[](size_t) const override;
			[[nodiscard]] const char * c_str() const override;
			[[nodiscard]] std::string str() const override;

		private:
			const size_t len; // Excluding NULL term
			const char * buf;
			const CStringHandling handling;
		};

		class DLL_PRIVATE StringFragment : public FragmentBase {
		public:
			explicit StringFragment(std::string);

			[[nodiscard]] size_t length() const override;
			[[nodiscard]] char operator[](size_t) const override;
			[[nodiscard]] const char * c_str() const override;
			[[nodiscard]] std::string str() const override;

		private:
			const std::string buf;
		};

		using FragmentPtr = std::shared_ptr<FragmentBase>;
		using Content = std::vector<FragmentPtr>;
		mutable Content content;
	};

}

// libmisc compat macros
#define vstringf(...) AdHoc::Buffer().vappendf(__VA_ARGS__).str()
#define stringf(...) AdHoc::Buffer().appendf(__VA_ARGS__).str()
#define stringbf(...) AdHoc::Buffer().appendbf(__VA_ARGS__).str()

#endif
