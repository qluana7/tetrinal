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

std::optional<std::vector<tetromino>> tetromino::gen(const std::string& __s) {
    std::vector<tetromino> __r;
    std::string::const_iterator __it = __s.begin();

    while (__it != __s.end()) {
        if (*__it == ' ' || *__it == ',') {
            ++__it;
            continue;
        }
        if (*__it == '[' || *__it == '*') {
            std::vector<tetromino> __set;
                
            if (*__it == '[') {
                bool __exclude = false;
                ++__it;
                    
                if (*__it == '^') {
                    __exclude = true;
                    ++__it;
                }

                while (true) {
                    if (__it == __s.end())
                        return std::nullopt;

                    if (*__it == ']') break;

                    if (*__it == ' ' || *__it == ',') {
                        ++__it;
                        continue;
                    }

                    auto __t = tetromino::from_char(*__it);

                    if (!__t.has_value())
                        return std::nullopt;

                    __set.push_back(*__t);

                    ++__it;
                }

                ++__it;

                if (__set.empty())
                    return std::nullopt;

                if (__exclude) {
                    std::vector<tetromino> __all = {
                        tetromino::I, tetromino::J, tetromino::L,
                        tetromino::O, tetromino::S, tetromino::T,
                        tetromino::Z
                    };

                    for (const auto& __t : __set) {
                        auto __it = std::find(__all.begin(), __all.end(), __t);
                        if (__it != __all.end()) {
                            __all.erase(__it);
                        }
                    }

                    if (__all.empty())
                        return std::nullopt;

                    __set = __all;
                }
            } else {
                ++__it;

                __set = {
                    tetromino::I, tetromino::J, tetromino::L,
                    tetromino::O, tetromino::S, tetromino::T,
                    tetromino::Z
                };
            }

            auto __mt = std::mt19937{std::random_device{}()};

            if (__it != __s.end() && *__it == 'p') {
                ++__it;

                if (__it == __s.end() || !std::isdigit(*__it))
                    return std::nullopt;

                u32 __count = 0;
                while (__it != __s.end() && std::isdigit(*__it)) {
                    __count = __count * 10 + (*__it - '0');
                    ++__it;
                }

                if (__count == 0 || __count > __set.size())
                    return std::nullopt;

                std::vector<tetromino> __chosen;
                std::sample(__set.begin(), __set.end(),
                            std::back_inserter(__chosen),
                            __count, __mt);

                __r.insert(__r.end(), __chosen.begin(), __chosen.end());
            } else if (*__it == '!') {
                ++__it;

                std::vector<tetromino> __shuffled = __set;
                std::shuffle(__shuffled.begin(), __shuffled.end(),
                             std::mt19937{std::random_device{}()});
                __r.insert(__r.end(), __shuffled.begin(), __shuffled.end());
            } else {
                auto __mt = std::mt19937{std::random_device{}()};

                auto __t = __set[
                    std::uniform_int_distribution<std::size_t>(0, __set.size() - 1)(__mt)
                    ];
                __r.push_back(__t);
            }
        } else if (std::isalpha(*__it)) {
            auto __t = tetromino::from_char(*__it);
            if (!__t.has_value())
                return std::nullopt;
            __r.push_back(*__t);
            ++__it;
        } else {
            return std::nullopt; // Invalid character
        }
    }

    if (__r.empty())
        return std::nullopt;

    return __r;
}