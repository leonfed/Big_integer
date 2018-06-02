#include "uint_data.h"
#include <cassert>

uint_data::uint_data()
        : type(EMPTY)
        , dataSize(0)
        , small(0)
{}

uint_data::uint_data(unsigned int a)
        : type(SMALL)
        , dataSize(1)
        , small(a)
{}

uint_data::uint_data(uint_data const& other) {
    if (other.type == BIG) {
        new(&big) std::shared_ptr<std::vector<unsigned int>>;
        big = other.big;
    } else {
        small = other.small;
    }
    type = other.type;
    dataSize = other.dataSize;
}

uint_data::uint_data(size_t sz, unsigned int value) {
    if (sz == 1) {
        type = SMALL;
        small = value;
        dataSize = 1;
    } else {
        type = BIG;
        new(&big) std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(sz, value));
        dataSize = sz;
    }
}

uint_data::~uint_data() {
    if (type == BIG) {
        big.~shared_ptr();
    }
}

uint_data& uint_data::operator=(uint_data const& other) {
    if (other.type == BIG) {
        if (type == SMALL) {
            new(&big) std::shared_ptr<std::vector<unsigned int>>;
        }
        big = other.big;
    } else {
        if (type == BIG) {
            big.~shared_ptr();
        }
        small = other.small;
    }
    type = other.type;
    dataSize = other.dataSize;
    return *this;
}

size_t uint_data::size() const {
    return dataSize;
}

void uint_data::do_unique() {
    if (type != BIG) {
        return;
    }
    if (!big.unique()) {
        big = std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(*big));
    }
}

unsigned int uint_data::get(size_t ind) const {
    assert(type != EMPTY);
    if (type == BIG) {
        return (*big)[ind];
    } else {
        assert(ind == 0);
        return small;
    }
}

void uint_data::set(size_t ind, unsigned int value) {
    assert(type != EMPTY);
    if (type == SMALL) {
        assert(ind == 0);
        small = value;
    } else {
        if (!big.unique()) {
            big = std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(*big));
        }
        (*big)[ind] = value;
    }
}

void uint_data::pop_back() {
    assert(type != EMPTY);
    if (type == BIG && big->size() == 2) {
        unsigned int value = (*big)[0];
        big.~shared_ptr();
        small = value;
        type = SMALL;
    } else if (type == BIG) {
        if (!big.unique()) {
            big = std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(*big));
        }
        big->pop_back();
    } else if (type == SMALL) {
        type = EMPTY;
    }
    dataSize--;
}

void uint_data::push_back(unsigned int value) {
    if (type == EMPTY) {
        small = value;
        type = SMALL;
        dataSize = 1;
        return;
    }
    if (type == SMALL) {
        int s = small;
        new(&big) std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(1, s));
        type = BIG;
    } else if (!big.unique()) {
        big = std::shared_ptr<std::vector<unsigned int>>(new std::vector<unsigned int>(*big));
    }
    big->push_back(value);
    dataSize++;
}

unsigned int uint_data::back() {
    return get(dataSize - 1);
}

unsigned int* uint_data::data() {
    if (type == EMPTY) {
        return nullptr;
    } else if (type == SMALL) {
        return &small;
    } else {
        return big.get()->data();
    }
}

unsigned int const* uint_data::data() const {
    if (type == EMPTY) {
        return nullptr;
    } else if (type == SMALL) {
        return &small;
    } else {
        return big.get()->data();
    }
}
