#include "bitmap.h"
#include <stdexcept>
#include <algorithm>

static inline std::size_t __inl_unit_bytes_size(std::size_t bits_size) {
    return bits_size / 8 + (bits_size % 8 == 0 ? 0 : 1);
}

static inline std::size_t __inl_outside_nth_unit(std::size_t pos) {
    return pos / 8;
}

static inline std::uint8_t __inl_mask_bit(std::size_t pos) {
    return 1 << (pos % 8);
}

rwg_http::bitmap::bitmap(std::size_t bits_size)
    : _bits(new std::uint8_t[__inl_unit_bytes_size(bits_size)])
    , _bits_size(bits_size) {
    
    std::size_t units_size = __inl_unit_bytes_size(bits_size);
    std::fill(this->_bits.get(), this->_bits.get() + units_size, 0x00);
}

rwg_http::bitmap::~bitmap() {
}

rwg_http::bit rwg_http::bitmap::operator[](const std::size_t pos) const {
    if (pos >= this->_bits_size) {
        throw std::out_of_range("rwg_http::bitmap operator[]: out of range");
    }

    return rwg_http::bit(this->_bits[__inl_outside_nth_unit(pos)], __inl_mask_bit(pos));
}

std::size_t rwg_http::bitmap::size() const {
    return this->_bits_size;
}

std::size_t rwg_http::bitmap::units_size() const {
    return __inl_unit_bytes_size(this->_bits_size);
}

void rwg_http::bitmap::fill(const std::size_t start_pos, const std::size_t end_pos, const bool bit) {
    if (start_pos > end_pos) {
        throw std::out_of_range("rwg_http::bitmap fill: start_pos great than end_pos");
    }
    if (start_pos >= this->_bits_size) {
        throw std::out_of_range("rwg_http::bitmap fill: start_pos out of range");
    }
    if (end_pos > this->_bits_size) {
        throw std::out_of_range("rwg_http::bitmap fill: end_pos out of range");
    }

    if (start_pos == end_pos) {
        return;
    }

    std::size_t startunit = __inl_outside_nth_unit(start_pos);
    std::uint8_t startunit_mask = static_cast<std::uint8_t>(0xFF - (__inl_mask_bit(start_pos) - 1));
    std::size_t endunit = __inl_outside_nth_unit(end_pos);
    std::uint8_t endunit_mask = static_cast<std::uint8_t>(__inl_mask_bit(end_pos) - 1);

    if (bit) {
        if (startunit == endunit) {
            std::uint8_t mask = startunit_mask & endunit_mask;
            this->_bits[startunit] |= mask;
            return;
        }
        this->_bits[startunit] |= startunit_mask;
        this->_bits[endunit] |= endunit_mask;
        for (auto i = startunit + 1; i < endunit; i++) {
            this->_bits[i] |= 0xFF;
        }
    }
    else {
        if (startunit == endunit) {
            std::uint8_t mask = startunit_mask & endunit_mask;
            this->_bits[startunit] &= ~mask;
            return;
        }
        this->_bits[startunit] &= ~startunit_mask;
        this->_bits[endunit] &= ~endunit_mask;
        for (auto i = startunit + 1; i < endunit; i++) {
            this->_bits[i] &= 0x00;
        }
    }
}

bool rwg_http::bitmap::ensure(const std::size_t start_pos, const std::size_t end_pos, const bool bit) const {
    if (start_pos > end_pos) {
        throw std::out_of_range("rwg_http::bitmap fill: start_pos great than end_pos");
    }
    if (start_pos >= this->_bits_size) {
        throw std::out_of_range("rwg_http::bitmap fill: start_pos out of range");
    }
    if (end_pos > this->_bits_size) {
        throw std::out_of_range("rwg_http::bitmap fill: end_pos out of range");
    }

    if (start_pos == end_pos) {
        return true;
    }

    std::size_t startunit = __inl_outside_nth_unit(start_pos);
    std::uint8_t startunit_mask = static_cast<std::uint8_t>(0xFF - (__inl_mask_bit(start_pos) - 1));
    std::size_t endunit = __inl_outside_nth_unit(end_pos);
    std::uint8_t endunit_mask = static_cast<std::uint8_t>(__inl_mask_bit(end_pos) - 1);

    if (bit) {
        if (startunit == endunit) {
            std::uint8_t mask = startunit_mask & endunit_mask;
            return (this->_bits[startunit] & mask) == mask;
        }

        if ((this->_bits[startunit] & startunit_mask) != startunit_mask) {
            return false;
        }
        if ((this->_bits[endunit] & endunit_mask) != endunit_mask) {
            return false;
        }
        for (auto i = startunit + 1; i < endunit; i++) {
            if (this->_bits[i] != 0xFF) {
                return false;
            }
        }
    }
    else {
        if (startunit == endunit) {
            std::uint8_t mask = startunit_mask & endunit_mask;
            return (this->_bits[startunit] & mask) == 0x00;
        }

        if ((this->_bits[startunit] & startunit_mask) != 0x00) {
            return false;
        }
        if ((this->_bits[endunit] & endunit_mask) != 0x00) {
            return false;
        }
        for (auto i = startunit + 1; i < endunit; i++) {
            if (this->_bits[i] != 0x00) {
                return false;
            }
        }
    }

    return true;
}

rwg_http::bit::bit(std::uint8_t& ref_byte, std::uint8_t mask_byte)
    : _ref_byte(ref_byte)
    , _mask_byte(mask_byte) {}

rwg_http::bit::operator bool() const {
    return (this->_ref_byte & this->_mask_byte) != 0;
}

rwg_http::bit& rwg_http::bit::operator=(const bool&& bit) {
    if (bit) {
        this->_ref_byte |= this->_mask_byte;
    }
    else {
        this->_ref_byte &= ~this->_mask_byte;
    }
    return *this;
}
