#pragma once

#include "c++11Helpers.h"
#include <utility>

namespace AdHoc {
	/// A unique_ptr like construct for non-pointer objects.
	/// Implements RAII.
	template<typename T, typename D> class Handle {
	public:
		/// Constructs a Handle that owns t, to be tidied with d
		Handle(T t, D d) noexcept : inst(std::move(t)), deleter(std::move(d)), owning(true) { }

		/// Constructs a Handle that takes over ownership of h
		Handle(Handle && h) noexcept : inst(std::move(h.inst)), deleter(std::move(h.deleter)), owning(h.owning)
		{
			h.owning = false;
		}

		~Handle()
		{
			if (owning) {
				deleter(inst);
			}
		}

		/// Standard special members
		SPECIAL_MEMBERS_COPY(Handle, delete);

		/// Takes over ownership of h
		Handle &
		operator=(Handle && h) noexcept
		{
			if (owning) {
				deleter(inst);
			}
			inst = std::move(h.inst);
			deleter = std::move(h.deleter);
			owning = h.owning;
			h.owning = false;
			return *this;
		}

		/// Returns a reference to the managed object.
		[[nodiscard]] T &
		get() noexcept
		{
			return inst;
		}
		/// Returns a const reference to the managed object.
		[[nodiscard]] const T &
		get() const noexcept
		{
			return inst;
		}

		/// Returns a pointer to the managed object.
		[[nodiscard]] T *
		operator->() noexcept
		{
			return inst;
		}
		/// Returns a const pointer to the managed object.
		[[nodiscard]] const T *
		operator->() const noexcept
		{
			return inst;
		}

		/// Returns a reference to the managed object.
		[[nodiscard]] T &
		operator*() noexcept
		{
			return inst;
		}
		/// Returns a const reference to the managed object.
		[[nodiscard]] const T &
		operator*() const noexcept
		{
			return inst;
		}

		/// Returns a reference to the managed object.
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		operator T &() noexcept
		{
			return inst;
		}
		/// Returns a const reference to the managed object.
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		operator const T &() const noexcept
		{
			return inst;
		}

	private:
		T inst;
		D deleter;
		bool owning;
	};

	template<typename T, typename D, typename... Args>
	Handle<T, D>
	make_handle(D && d, Args &&... args)
	{
		return {T(std::forward<Args>(args)...), std::forward<D>(d)};
	}
}
