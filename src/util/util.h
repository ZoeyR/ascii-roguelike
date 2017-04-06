#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <iostream>
typedef struct {} Unit;
Unit unit();

template <typename T, typename E>
class Result {
    union {
        T ok;
        E err;
    };
    enum {
        OK,
        ERR,
    } tag;
    public:
        Result(T ok) {
            this->ok = ok;
            tag = Result::OK;
        }
        Result(E err) {
            this->err = err;
            tag = Result::ERR;
        }
        bool is_ok() {
            return tag == Result::OK;
        }
        bool is_err() {
            return tag == Result::ERR;
        }
        T unwrap() {
            if (is_ok()) {
                return ok;
            }
            exit(1);
        }
        T expect(const std::string& message) {
            if (is_ok())  {
                return ok;
            }
            std::cout << message;
            exit(1);
        }
};

#define relative_array(distance, row, col, height, width, postfix) \
    int right##postfix = (col >= width - distance) ? col : col + distance; \
    int left##postfix = (col < 0 + distance) ? 0 : col - distance; \
    int top##postfix = (row < 0 + distance) ? 0 : row - distance; \
    int bottom##postfix = (row >= height - distance) ? row : row + distance \
    
typedef enum  {
    TOO_LARGE,
    INVALID_STRING
} IntParseError;

Result<int, IntParseError> parse_int(char* str);

int better_rand(int limit);

class Dice {
    public:
        int base;
        int num;
        int sides;
        Dice();
        int roll();
        bool parse_str(std::string& str);
        void print();
};

#endif