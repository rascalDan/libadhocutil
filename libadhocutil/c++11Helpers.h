#ifndef ADHOCUTIL_CPP11_HELPERS
#define ADHOCUTIL_CPP11_HELPERS

#define SPECIAL_MEMBERS_CONS(T, type) \
	T(const T &) = type; \
	T(T &&) noexcept = type
#define SPECIAL_MEMBERS_ASSIGN(T, type) \
	T & operator=(const T &) = type; \
	T & operator=(T &&) noexcept = type
#define SPECIAL_MEMBERS_MOVE(T, type) \
	T(T &&) noexcept = type; \
	T & operator=(T &&) noexcept = type
#define SPECIAL_MEMBERS_MOVE_RO(T) \
	T(const T &) = delete; \
	T(T &&) noexcept = default; \
	SPECIAL_MEMBERS_ASSIGN(T, delete)
#define SPECIAL_MEMBERS_COPY(T, type) \
	T(const T &) = type; \
	T & operator=(const T &) = type
#define SPECIAL_MEMBERS_COPY_RO(T) \
	SPECIAL_MEMBERS_CONS(T, default); \
	SPECIAL_MEMBERS_ASSIGN(T, delete)
#define SPECIAL_MEMBERS_DEFAULT(T) \
	SPECIAL_MEMBERS_COPY(T, default); \
	SPECIAL_MEMBERS_MOVE(T, default)
#define SPECIAL_MEMBERS_DELETE(T) \
	SPECIAL_MEMBERS_COPY(T, delete); \
	SPECIAL_MEMBERS_MOVE(T, delete)
#define SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(T) \
	SPECIAL_MEMBERS_COPY(T, delete); \
	SPECIAL_MEMBERS_MOVE(T, default)

#endif

