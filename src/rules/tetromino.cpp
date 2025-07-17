#include "rules/tetromino.hpp"

const tetromino tetromino::I = {
    mino_type::I,
    { {0, 0, 0, 0},
      {1, 1, 1, 1},
      {0, 0, 0, 0},
      {0, 0, 0, 0} }
};

const tetromino tetromino::J = {
    mino_type::J,
    { {1, 0, 0},
      {1, 1, 1},
      {0, 0, 0} }
};

const tetromino tetromino::L = {
    mino_type::L,
    { {0, 0, 1},
      {1, 1, 1},
      {0, 0, 0} }
};

const tetromino tetromino::O = {
    mino_type::O,
    { {1, 1},
      {1, 1} }
};

const tetromino tetromino::S = {
    mino_type::S,
    { {0, 1, 1},
      {1, 1, 0},
      {0, 0, 0} }
};

const tetromino tetromino::T = {
    mino_type::T,
    { {0, 1, 0},
      {1, 1, 1},
      {0, 0, 0} }
};

const tetromino tetromino::Z = {
    mino_type::Z,
    { {1, 1, 0},
      {0, 1, 1},
      {0, 0, 0} }
};

const tetromino tetromino::INVALID = { mino_type::INVALID, { } };

struct single_t { char t; };
struct set_t {
    bool exclude = false;
    u32 count = 0;
    std::vector<tetromino> set;
};

struct seqinfo {
    seqinfo() = default;
    seqinfo(const std::string& __s);

    using expr = std::variant<single_t, set_t>;

    std::vector<expr> expressions;
    bool is_valid = true;

    std::optional<std::vector<tetromino>> generate(std::mt19937& __r) const;
    bool match(const std::vector<tetromino>& __v) const;
};

seqinfo::seqinfo(const std::string& __s) {
    std::string::const_iterator __it = __s.begin();

    while (__it != __s.end()) {
        if (*__it == ' ' || *__it == ',') {
            ++__it;
            continue;
        }
        if (*__it == '[' || *__it == '*') {
            set_t __set;
                
            if (*__it == '[') {
                ++__it;
                    
                if (*__it == '^') {
                    __set.exclude = true;
                    ++__it;
                }

                while (true) {
                    if (__it == __s.end()) {
                        is_valid = false;
                        return;
                    }

                    if (*__it == ']') break;

                    if (*__it == ' ' || *__it == ',') {
                        ++__it;
                        continue;
                    }

                    auto __t = tetromino::from_char(*__it);

                    if (!__t.has_value()) {
                        is_valid = false;
                        return;
                    }

                    __set.set.push_back(*__t);

                    ++__it;
                }

                ++__it;

                if (__set.set.empty()) {
                    is_valid = false;
                    return;
                }
            } else {
                ++__it;

                __set.set = {
                    tetromino::I, tetromino::J, tetromino::L,
                    tetromino::O, tetromino::S, tetromino::T,
                    tetromino::Z
                };
            }

            if (__it != __s.end() && *__it == 'p') {
                ++__it;

                if (__it == __s.end() || !std::isdigit(*__it)) {
                    is_valid = false;
                    return;
                }

                u32 __count = 0;
                while (__it != __s.end() && std::isdigit(*__it)) {
                    __count = __count * 10 + (*__it - '0');
                    ++__it;
                }

                if (__count == 0 || __count > __set.set.size()) {
                    is_valid = false;
                    return;
                }
                
                __set.count = __count;
            } else if (*__it == '!') {
                ++__it;
                
                __set.count = __set.set.size();
            } else __set.count = 1;

            expressions.push_back(__set);
        } else if (std::isalpha(*__it)) {
            single_t __single;

            if (tetromino::from_char(*__it) == std::nullopt) {
                is_valid = false;
                return;
            }

            __single.t = std::toupper(*__it);

            expressions.push_back(__single);
            ++__it;
        } else {
            is_valid = false;
            return;
        }
    }

    if (expressions.empty()) {
        is_valid = false;
        return;
    }
}

std::optional<std::vector<tetromino>> seqinfo::generate(std::mt19937& __r) const {
    std::vector<tetromino> __result;

    for (const auto& __expr : expressions) {
        bool __k = std::visit([&](const auto& __e) {
            using T = std::decay_t<decltype(__e)>;
            if constexpr (std::is_same_v<T, single_t>) {
                auto __t = tetromino::from_char(__e.t);

                if (__t.has_value()) __result.push_back(*__t);
                else return false;
            } else if constexpr (std::is_same_v<T, set_t>) {
                if (__e.set.empty()) return false;

                std::vector<tetromino> __v;

                if (__e.exclude) {
                    std::vector<tetromino> __all = {
                        tetromino::I, tetromino::J, tetromino::L,
                        tetromino::O, tetromino::S, tetromino::T,
                        tetromino::Z
                    };

                    for (const auto& __t : __e.set) {
                        auto __it = std::find(__all.begin(), __all.end(), __t);
                        if (__it != __all.end()) {
                            __all.erase(__it);
                        }
                    }

                    if (__all.empty()) return false;

                    std::sample(__all.begin(), __all.end(),
                                std::back_inserter(__v),
                                std::min(__e.count, static_cast<u32>(__all.size())),
                                __r);
                } else {
                    std::sample(__e.set.begin(), __e.set.end(),
                                std::back_inserter(__v),
                                std::min(__e.count, static_cast<u32>(__e.set.size())),
                                __r);
                }

                std::shuffle(__v.begin(), __v.end(), __r);
                __result.insert(__result.end(), __v.begin(), __v.end());
            }

            return true;
        }, __expr);

        if (!__k) return std::nullopt;
    }

    if (__result.empty()) return std::nullopt;
    return __result;
}

bool seqinfo::match(const std::vector<tetromino>& __v) const {
    auto __iter = __v.begin();

    for (const auto& __expr : expressions) {
        bool __k = std::visit([&](const auto& __e) {
            using T = std::decay_t<decltype(__e)>;
            if constexpr (std::is_same_v<T, single_t>) {
                if (__iter == __v.end()) return false;
                if (*__iter != tetromino::from_char(__e.t).value()) return false;
                ++__iter;
            } else if constexpr (std::is_same_v<T, set_t>) {
                u32 __cnt = std::distance(__iter, __v.end());

                if (__e.count > __cnt) return false;
                bool __chk[7] = { false, };

                for (u32 i = 0; i < __e.count; ++i, ++__iter) {
                    bool __f = std::find(__e.set.begin(), __e.set.end(), *__iter) != __e.set.end();
                    __f ^= __e.exclude;

                    if (!__f) return false;

                    if (__chk[static_cast<u32>(__iter->type())]) return false;
                    __chk[static_cast<u32>(__iter->type())] = true;
                }
            }

            return true;
        }, __expr);

        if (!__k) return false;
    }

    if (__iter != __v.end()) return false;

    return true;
}

std::optional<std::vector<tetromino>> tetromino::gen(const std::string& __s, std::mt19937& __r) {
    seqinfo __seq(__s);

    if (!__seq.is_valid) return std::nullopt;

    return __seq.generate(__r);
}

bool tetromino::sequence_match(const std::vector<tetromino>& __v, const std::string& __s) {
    seqinfo __seq(__s);

    if (!__seq.is_valid) return false;

    return __seq.match(__v);
}