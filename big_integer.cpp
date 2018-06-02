#include "big_integer.h"

#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <cmath>

#define uint unsigned int
#define ull unsigned long long

uint const BILLION = 1e9;
uint const BASE = UINT32_MAX;

using namespace std;

void big_integer::make_shorten() {
    while (_data.size() > 1 && _data.back() == 0) {
        _data.pop_back();
    }
    if (_data.size() == 1 && _data.get(0) == 0) {
        sign = 0;
    }
}

big_integer::big_integer()
        : sign(0)
        , _data(0)
{}

big_integer::big_integer(big_integer const& other)
        : sign(other.sign)
        , _data(other._data)
{}

big_integer::big_integer(int a)  {
    if (a == INT32_MIN) {
        *this = big_integer((unsigned int)a);
        sign = 1;
    } else {
        sign = (a < 0);
        _data = uint_data((uint)abs(a));
    }
}

big_integer::big_integer(unsigned int a)
        : sign(0)
        , _data(a)
{}

big_integer string_to_big_integer(std::string const& str) {
    big_integer res;
    int start = 0;
    bool flag = false;
    if (str[0] == '-') {
        start++;
        flag = true;
    }
    int i = (str.length() - start) % 9 + start;
    if (i > start) {
        res = stoi(str.substr(start, i - start));
    }
    while (i + 9 <= (int)str.length()) {
        res *= BILLION;
        res += stoi(str.substr(i, 9));
        i += 9;
    }
    return (flag ? -res : res);
}

big_integer::big_integer(std::string const& str)
        : big_integer(string_to_big_integer(str))
{
    make_shorten();
}

/*big_integer::~big_integer() {
}*/

big_integer& big_integer::operator=(big_integer const& other) {
    sign = other.sign;
    _data = other._data;
    return *this;
}

big_integer& big_integer::operator+=(big_integer const& rhs) {
    if (sign != rhs.sign) {
        if (!sign) {
            big_integer copy_rhs(rhs);
            copy_rhs.sign = 0;
            *this -= copy_rhs;
            make_shorten();
            return *this;
        } else {
            sign = 0;
            *this -= rhs;
            sign ^= 1;
            make_shorten();
            return *this;
        }
    }
    //now sign == rhs.sign
    bool cf = 0;
    for (size_t i = 0; i < max(_data.size(), rhs._data.size()); i++) {
        if (i == _data.size()) {
            _data.push_back(0);
        }
        uint res = _data.get(i) + (i < rhs._data.size() ? rhs._data.get(i) : 0) + (uint)cf;
        cf = (res < _data.get(i));
        _data.set(i, res);
    }
    if (cf) {
        _data.push_back(1);
    }
    make_shorten();
    return *this;
}

big_integer& big_integer::operator-=(big_integer const& rhs) {
    if (sign != rhs.sign) {
        if (!sign) {
            big_integer copy_rhs(rhs);
            copy_rhs.sign = 0;
            *this += copy_rhs;
            make_shorten();
            return *this;
        } else {
            sign = 0;
            *this += rhs;
            sign = 1;
            make_shorten();
            return *this;
        }
    }
    //now sign == rhs.sign
    if (sign) {
        big_integer copy_rhs(rhs);
        sign = 0;
        copy_rhs.sign = 0;
        *this -= copy_rhs;
        sign ^= 1;
        make_shorten();
        return *this;
    }
    if (*this < rhs) {
        *this = rhs - *this;
        sign = 1;
        make_shorten();
        return *this;
    }
    //now *this > rhs
    bool cf = 0;
    for (size_t i = 0; i < _data.size(); i++) {
        uint res = _data.get(i) - (i < rhs._data.size() ? rhs._data.get(i) : 0) - (uint)cf;
        cf = (res > _data.get(i));
        _data.set(i, res);
    }
    make_shorten();
    return *this;
}

big_integer& big_integer::operator*=(big_integer const& rhs) {
    uint_data res(_data.size() + rhs._data.size(), 0);
    _data.do_unique();
    unsigned int* this_data = _data.data();
    unsigned int* res_data = res.data();
    unsigned int const* rhs_data = rhs._data.data();
    for (size_t i = 0; i < rhs._data.size(); i++) {
        ull carry = 0;
        for (size_t j = 0; j < _data.size(); j++) {
            ull mul = (ull)rhs_data[i] * (ull)this_data[j];
            mul += carry;
            carry = (mul >> 32);
            mul -= (carry << 32);
            res_data[i + j] = res_data[i + j] + (uint)mul;
            if (res_data[i + j] < mul) {
                carry += 1;
            }
        }
        if (carry > 0) {
            res_data[i + _data.size()] = res_data[i + _data.size()] + (uint)carry;
        }
    }
    _data = res;
    sign ^= rhs.sign;
    make_shorten();
    return *this;
}

void big_integer::my_sub(big_integer& b, int shl) { //_data.size == b._data.size + shl && *this >= 0 && b >= 0
    int n = _data.size() - 1;
    while ((int)b._data.size() + shl < (int)_data.size()) {
        b._data.push_back(0);
    }
    while (n >= shl && _data.get(n) == b._data.get(n - shl)) {
        _data.set(n, 0);
        n--;
    }
    if (n < shl) {
        make_shorten();
        b.make_shorten();
        return;
    }
    if (_data.get(n) > b._data.get(n - shl)) {
        bool cf = 0;
        for (int i = shl; i <= n; i++) {
            uint res = _data.get(i) - b._data.get(i - shl) - (uint)cf;
            cf = (res > _data.get(i));
            _data.set(i, res);
        }
        make_shorten();
    } else {
        bool cf = 0;
        for (int i = 0; i <= n; i++) {
            uint h = (i < shl ? 0 : b._data.get(i - shl));
            uint res = h - _data.get(i) - (uint)cf;
            cf = (res > h);
            _data.set(i, res);
        }
        sign = 1;
        make_shorten();
    }
    b.make_shorten();
}

big_integer& big_integer::operator/=(big_integer const& rhs) {
    if (rhs._data.size() == 1 && rhs._data.get(0) == 0) {
        throw "division by zero";
    }
    big_integer abs_a = *this;
    abs_a.sign = 0;
    big_integer abs_b = rhs;
    abs_b.sign = 0;
    if (abs_b._data.size() > 1) {
        while (abs_b._data.back() < BASE / 2) {
            abs_a <<= 1;
            abs_b <<= 1;
        }
    }
    int m = abs_a._data.size() - abs_b._data.size(), n = abs_b._data.size();
    uint_data res(m + 1, 0);  //res = a / b
    for (int i = m; i >= 0; i--) {
        ull sf_first = (n + i < (int)abs_a._data.size() ? ((ull)abs_a._data.get(n + i) << 32) : 0);
        ull sf_second = (n + i - 1 < (int)abs_a._data.size() ? (ull)abs_a._data.get(n + i - 1) : 0);
        uint w = (uint)((sf_first + sf_second) / (ull)abs_b._data.get(n - 1));
        res.set(i, min(BASE, w));
        if (res.get(i) == 0) {
            continue;
        }
        big_integer h = (res.get(i) * abs_b);
        abs_a.my_sub(h, i);   		      //abs_a -= (h << (i * 32));
        while (abs_a.sign) { //*this < 0
            res.set(i, res.get(i) - 1);
            abs_a.sign = 0;
            abs_a.my_sub(abs_b, i);      //abs_a += (abs_b << (i * 32));
            abs_a.sign ^= 1;
            abs_a.make_shorten();
        }
    }
    _data = res;
    sign ^= rhs.sign;
    make_shorten();
    return *this;
}

big_integer& big_integer::operator%=(big_integer const& rhs) {
    *this -= rhs * (*this / rhs);
    make_shorten();
    return *this;
}

void big_integer::change_form() {
    if (!sign) {
        return;
    }
    for (size_t i = 0; i < _data.size(); i++) {
        _data.set(i, ~_data.get(i));
    }
    (*this) -= 1;
}

template<typename FunT>
void big_integer::apply_binary_fun_to_all(big_integer rhs, FunT fun) {
    change_form();
    rhs.change_form();
    sign = fun(sign, rhs.sign);
    for (size_t i = 0; i < max(_data.size(), rhs._data.size()); i++) {
        if (i == _data.size()) {
            _data.push_back(0);
        }
        uint w = fun(_data.get(i), (i < rhs._data.size() ? rhs._data.get(i) : 0));
        _data.set(i, w);
    }
    change_form();
    make_shorten();
}

big_integer& big_integer::operator&=(big_integer const& rhs) {
    apply_binary_fun_to_all(rhs, [](unsigned int a, unsigned int b) { return a & b; });
    return *this;
}

big_integer& big_integer::operator|=(big_integer const& rhs) {
    apply_binary_fun_to_all(rhs, [](unsigned int a, unsigned int b) { return a | b; });
    return *this;
}

big_integer& big_integer::operator^=(big_integer const& rhs) {
    apply_binary_fun_to_all(rhs, [](unsigned int a, unsigned int b) { return a ^ b; });
    return *this;
}

big_integer& big_integer::operator<<=(int rhs) {
    uint cell = rhs / 32;
    uint shift = rhs % 32;
    uint_data res;
    for (uint i = 0; i < cell; i++) {
        res.push_back(0);
    }
    for (size_t i = 0; i < _data.size(); i++) {
        res.push_back(_data.get(i));
    }
    uint carry = 0;
    for (size_t i = cell; i < res.size(); i++) {
        int new_carry = (shift ? (res.get(i) >> (32 - shift)) : 0);
        res.set(i, (res.get(i) << shift) + carry);
        carry = new_carry;
    }
    res.push_back(carry);
    _data = res;
    make_shorten();
    return *this;
}

big_integer& big_integer::operator>>=(int rhs) {
    (*this).change_form();
    uint cell = rhs / 32;
    uint shift = rhs % 32;
    uint_data res;
    for (size_t i = cell; i < _data.size(); i++) {
        res.push_back(_data.get(i));
    }
    uint trans = 0;
    if (sign) {
        trans = UINT32_MAX << (32 - shift);
    }
    for (int i = (int)res.size() - 1; i >= 0; i--) {
        uint new_trans = (res.get(i) << (32 - shift));
        res.set(i, (res.get(i) >> shift) + trans);
        trans = new_trans;
    }
    _data = res;
    (*this).change_form();
    make_shorten();
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer res(*this);
    res.sign ^= 1;
    res.make_shorten();
    return res;
}

big_integer big_integer::operator~() const {
    big_integer res = (*this);
    res.change_form();
    res.sign ^= 1;
    for (size_t i = 0; i < _data.size(); i++) {
        res._data.set(i, ~res._data.get(i));
    }
    res.change_form();
    res.make_shorten();
    return res;
}

big_integer& big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    ++*this;
    make_shorten();
    return *this;
}

big_integer& big_integer::operator--() {
    *this -= 1;
    make_shorten();
    return *this;
}

big_integer big_integer::operator--(int) {
    --*this;
    make_shorten();
    return *this;
}

big_integer operator+(big_integer a, big_integer const& b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const& b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const& b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const& b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const& b) {
    return a %= b;
}

big_integer operator&(big_integer a, big_integer const& b) {
    return a &= b;
}

big_integer operator|(big_integer a, big_integer const& b) {
    return a |= b;
}

big_integer operator^(big_integer a, big_integer const& b) {
    return a ^= b;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer operator >> (big_integer a, int b) {
    return a >>= b;
}

bool operator==(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign || a._data.size() != b._data.size()) {
        return false;
    }
    for (size_t i = 0; i < a._data.size(); i++) {
        if (a._data.get(i) != b._data.get(i)) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const& a, big_integer const& b) {
    return !(a == b);
}

bool operator<(big_integer const& a, big_integer const& b) {
    if (a.sign != b.sign) {
        return a.sign;
    }
    if (a._data.size() != b._data.size()) {
        return a._data.size() < b._data.size();
    }
    for (int i = (int)a._data.size() - 1; i >= 0; i--) {
        if (a._data.get(i) != b._data.get(i)) {
            return (a._data.get(i) < b._data.get(i)) ^ a.sign;
        }
    }
    return a.sign;
}

bool operator>(big_integer const& a, big_integer const& b) {
    return !(a < b) && (a != b);
}

bool operator<=(big_integer const& a, big_integer const& b) {
    return (a < b) || (a == b);
}

bool operator>=(big_integer const& a, big_integer const& b) {
    return !(a < b);
}

std::string to_string(big_integer const& a) {
    if (a == 0) {
        return "0";
    }
    string res = "";
    big_integer abs_a = (a.sign ? -a : a);
    while (abs_a._data.size() > 1 || abs_a._data.get(0) != 0) {
        while (res.length() % 9 != 0) {
            res.append("0");
        }
        big_integer mod = abs_a % BILLION;
        abs_a /= BILLION;
        string s = to_string(mod._data.get(0));
        reverse(s.begin(), s.end());
        res.append(s);
    }
    if (a.sign) {
        res.append("-");
    }
    reverse(res.begin(), res.end());
    return res;
}

std::ostream& operator<<(std::ostream& s, big_integer const& a) {
    return s << to_string(a);
}