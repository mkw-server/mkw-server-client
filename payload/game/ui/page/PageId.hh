// See LICENSE-mkw-sp

#pragma once

namespace UI {

enum class PageId {
    None = -0x1,
    Empty = 0x0,

    Max = 0xD3,

    ExtensionsBegin = 0xff,
    ExtensionsEnd,
};
static_assert(
        static_cast<size_t>(PageId::ExtensionsEnd) > static_cast<size_t>(PageId::ExtensionsBegin));

constexpr size_t StandardPageCount() {
    return static_cast<size_t>(PageId::Max);
}

constexpr size_t ExtendedPageCount() {
    return static_cast<size_t>(PageId::ExtensionsEnd) -
            static_cast<size_t>(PageId::ExtensionsBegin);
}

} // namespace UI
