#ifndef UINT_DATA
#define UINT_DATA

#include <vector>
#include <memory>

struct uint_data {
    uint_data();
    uint_data(unsigned int a);
    uint_data(uint_data const& other);
    uint_data(size_t sz, unsigned int value);
    ~uint_data();
    uint_data& operator=(uint_data const& other);
    size_t size() const;
    void do_unique();
    void set(size_t ind, unsigned int value);
    unsigned int get(size_t ind) const;
    void pop_back();
    void push_back(unsigned int value);
    unsigned int back();
    unsigned int* data();
    unsigned int const* data() const;

    enum {
        EMPTY, SMALL, BIG
    } type;

    size_t dataSize;

    union {
        unsigned int small;
        std::shared_ptr<std::vector<unsigned int>> big;
    };

};
#endif