#pragma once

#include <limits>
#include <cstddef>

namespace storm {
namespace detail {

class BaseHandle
{
public:
	constexpr BaseHandle () noexcept = default;

	constexpr explicit BaseHandle (size_t index) noexcept
		: index_(index)
	{ }

	[[nodiscard]] constexpr size_t Index () const noexcept
	{ return index_; }

    [[nodiscard]] constexpr bool IsValid () const noexcept
	{ return index_ != InvalidIndex; }

protected:
	static const size_t InvalidIndex = std::numeric_limits<size_t>::max();

	size_t index_ = InvalidIndex;
};

template<typename STREAM>
STREAM& operator << (STREAM& os, const BaseHandle handle)
{
	return (os << handle.Index() );
}

#define DEFINE_HANDLE(Handle) \
class Handle : public ::storm::detail::BaseHandle \
{ \
public: \
	constexpr Handle () noexcept = default; \
	constexpr explicit Handle (size_t index) noexcept: \
		BaseHandle(index) {} \
	[[nodiscard]] constexpr bool operator == (const Handle& rhs) const noexcept  \
	{ return Index() == rhs.Index(); } \
	[[nodiscard]] constexpr bool operator != (const Handle& rhs) const noexcept  \
	{ return Index() != rhs.Index(); } \
	[[nodiscard]] constexpr bool operator < (const Handle& rhs) const noexcept \
	{ return Index() < rhs.Index(); } \
	[[nodiscard]] constexpr bool operator <= (const Handle& rhs) const noexcept \
	{ return Index() <= rhs.Index(); } \
	[[nodiscard]] constexpr bool operator > (const Handle& rhs) const noexcept \
	{ return Index() > rhs.Index(); } \
	[[nodiscard]] constexpr bool operator >= (const Handle& rhs) const noexcept \
	{ return Index() >= rhs.Index(); } \
	constexpr Handle& Invalidate () noexcept \
	{ \
		index_ = InvalidIndex; \
		return *this; \
	} \
};

} // namespace detail
} // namespace storm

