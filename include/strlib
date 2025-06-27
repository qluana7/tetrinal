#pragma once

#include <string>
#include <numeric>
#include <vector>
#include <sstream>
#include <functional>
#include <type_traits>

#include <cstddef>

class strlib {
private:
    template <typename ForwardIterator>
    using StringTemplate = typename std::enable_if_t<std::is_same<typename std::iterator_traits<ForwardIterator>::value_type, std::string>::value>;

public:
    template <typename Predicate>
    inline static std::size_t find_not_if(const std::string& src, Predicate pred) {
        std::string::const_iterator b = src.cbegin(), e = src.cend();
        std::size_t pos = 0;

        for (; b != e && pred(*b); b++, pos++);

        return pos;
    }

    template <typename Predicate>
    inline static std::size_t rfind_not_if(const std::string& src, Predicate pred) {
        std::string::const_reverse_iterator b = src.crbegin(), e = src.crend();
        std::size_t pos = src.size();

        for (; b != e && pred(*b); b++, pos--);

        return pos;
    }

    template <typename ForwardIterator, typename = StringTemplate<ForwardIterator>>
    inline static std::string join(ForwardIterator b, ForwardIterator e, const std::string& delim = "") {
        if (b == e) return "";

        std::string output = *b;

        for (b++; b != e; b++)
            output += delim + *b;
        
        return output;
    }

    template <typename ForwardIterator>
    inline static std::string join(ForwardIterator b, ForwardIterator e, std::function<std::string (const typename std::iterator_traits<ForwardIterator>::value_type&)> conv, const std::string& delim = "") {
        if (b == e) return "";

        std::string output = conv(*b);

        for (b++; b != e; b++)
            output += delim + conv(*b);

        return output;
    }

    template <typename ForwardIterator, typename = StringTemplate<ForwardIterator>>
    [[ deprecated ]]
    inline static size_t split(const std::string& src, ForwardIterator dest, char delim)
    { size_t c = 0; std::stringstream ss(src); for (std::string s; std::getline(ss, s, delim); c++, dest++) *dest = s; return c; }

    [[ deprecated ]]
    inline static void split(const std::string& src, std::vector<std::string>& dest, char delim)
    { std::stringstream ss(src); for (std::string s; std::getline(ss, s, delim); dest.push_back(s)); }

    template <typename _OutT, typename _UnaryOperation>
    [[ deprecated ]]
    inline static std::vector<_OutT> split_map(const std::string& __src, _UnaryOperation __unary_op, char __delim) {
        std::stringstream ss(__src);
        std::vector<_OutT> v;
        std::string s;

        while (std::getline(ss, s, __delim))
            v.push_back(__unary_op(s));

        return v;
    }

    template <typename _Container>
    inline static _Container split(const std::string& __src, char __delim) {
        std::stringstream ss(__src);
        _Container ct; auto iter = std::back_inserter(ct);
        std::string s;

        while (getline(ss, s, __delim))
            iter = s;

        return ct;
    }

    template <typename _Container, typename _UnaryOp>
    inline static _Container split_map(const std::string& __src, char __delim, _UnaryOp __unary_op) {
        std::stringstream ss(__src);
        _Container ct; auto iter = std::back_inserter(ct);
        std::string s;

        while (std::getline(ss, s, __delim))
            iter = __unary_op(s);

        return ct;
    }

    template <typename _UnaryFunc>
    inline static void split_foreach(const std::string& __src, char __delim, _UnaryFunc __unary_func) {
        std::stringstream ss(__src);
        std::string s;

        while (std::getline(ss, s, __delim))
            __unary_func(s);
    }

    // Remove character if predict return false.
    template <typename _UnaryPred>
    inline static std::string strrmv(const std::string& __src, _UnaryPred __unary_pred) {
        std::string r; r.resize(__src.size());

        std::size_t i = 0;
        for (auto c : __src) {
            if (!__unary_pred(c)) continue;

            r[i] = c; i++;
        }

        r.resize(i);

        return r;
    }

    inline static bool empty_or_space(const std::string& src)
    { return std::all_of(src.begin(), src.end(), (int (*)(int))std::isspace); }

    inline static std::string ltrim(const std::string& src)
    { return src.substr(find_not_if(src, (int (*)(int))std::isspace)); }

    inline static std::string rtrime(const std::string& src)
    { return src.substr(0, rfind_not_if(src, (int (*)(int))std::isspace)); }

    inline static std::string trim(const std::string& src)
    { return ltrim(rtrime(src)); }

    inline static std::string pad_left(const std::string& src, int cnt, char c)
    { return std::string(std::max<int>(0, cnt - src.size()), c) + src; }

    inline static std::string pad_right(const std::string& src, int cnt, char c)
    { return src + std::string(std::max<int>(0, cnt - src.size()), c); }

    inline static bool startswith(const std::string& src, const std::string& pat)
    { return std::equal(pat.begin(), pat.end(), src.begin()); }

    inline static bool endswith(const std::string& src, const std::string& pat)
    { return std::equal(pat.rbegin(), pat.rend(), src.rbegin()); }

    inline static std::string toupper(const std::string& src)
    { std::string ret(src); for (auto& c : ret) c = std::toupper(c); return ret; }

    inline static std::string tolower(const std::string& src)
    { std::string ret(src); for (auto& c : ret) c = std::tolower(c); return ret; }

    inline static bool try_parse(int& dest, const std::string& src, int base = 10) {
        try {
            size_t idx;
            dest = std::stoi(src, &idx, base);
            return idx == src.size();
        } catch (...) { return false; }
    }

    inline static bool is_digit(const std::string& src)
    { return std::all_of(src.begin(), src.end(), (int (*)(int))std::isdigit); }

    inline static bool is_alpha(const std::string& src)
    { return std::all_of(src.begin(), src.end(), (int (*)(int))std::isalpha); }

    inline static bool is_alnum(const std::string& src)
    { return std::all_of(src.begin(), src.end(), (int (*)(int))std::isalnum); }
};
