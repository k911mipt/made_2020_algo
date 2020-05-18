#pragma once
#ifndef LEXER_HPP
#define LEXER_HPP


#include <istream> 
#include <string>
#include <sstream>
#include <unordered_map>
#include <limits>

const int INTEGER_MAX = std::numeric_limits<int>::max();

enum Token {
    BEGIN, // start of block
    END, // end of block
    ASSIGN, // :=


    INTEGER, // integer number

    ID, // identifier

    PLUS,
    MINUS,
    MUL,
    DIV,

    LPAREN, // (
    RPAREN, // )

    SEMI, // ;
    DOT, // .

    EOP, // end of parse
};
const char NONE_CHAR = 0;

struct Lexeme {
public:
    Lexeme(Token _type, int _number) : type(_type), number(_number) {}
    Lexeme(Token _type, std::string _str) : type(_type), str(_str) {}

    Token type;
    int number = 0;
    std::string str = "";
};

const std::unordered_map<std::string, Lexeme> RESERVED({
    { "BEGIN", Lexeme(Token::BEGIN, "BEGIN") },
    { "END", Lexeme(Token::END, "END") }
});

class LexerException : public std::exception {
public:
    explicit LexerException(size_t _line, size_t _column) noexcept : line(_line), column(_column), std::exception() {}
    const char* what() const noexcept override {
        return "Invalid character"; 
    }
private:
    size_t line;
    size_t column;
};


class LexerOverflowException : public LexerException {
public:
    using LexerException::LexerException;
    const char* what() const noexcept override {
        return "Number exceeds INT_MAX";
    }
};


class Lexer {
public:
    explicit Lexer(std::istream& _stream) : stream(_stream) {
        //stream.
        do {
            ++line;
        } while (getline(stream, str) && str.size() == 0);
        cursor = str[0];
    }

    Lexeme get_next_token() {
        while (cursor != NONE_CHAR) {
            if (isspace(cursor)) {
                skip_spaces();
                continue;
            }
            if (isalpha(cursor)) {
                return get_id();
            }
            if (isdigit(cursor)) {
                return Lexeme(Token::INTEGER, get_integer());
            }
            if (cursor == ':' && peek() == '=') {
                advance(); advance(); return Lexeme(Token::ASSIGN, ":=");
            }
            switch (cursor) {
            case ';': advance(); return Lexeme(Token::SEMI, ";");
            case '.': advance(); return Lexeme(Token::DOT, ".");
            case '+': advance(); return Lexeme(Token::PLUS, "+");
            case '-': advance(); return Lexeme(Token::MINUS, "-");
            case '*': advance(); return Lexeme(Token::MUL, "*");
            case '/': advance(); return Lexeme(Token::DIV, "/");
            case '(': advance(); return Lexeme(Token::LPAREN, "(");
            case ')': advance(); return Lexeme(Token::RPAREN, ")");
            default: error();
            }
        }
        return Lexeme(EOP, 0);
    }

private:
    void advance() {
        ++col;
        if (col == str.size()) {
            col = 0;
            if (stream.eof()) {
                str = "";
            } else do {
                ++line;
                //getline(stream, str);
                
            } while (getline(stream, str) && str.size() == 0);
        }
        cursor = str[col];
    }

    char peek() {
        auto peek_col = col + 1;
        if (peek_col == str.size()) {
            return NONE_CHAR;
        } else {
            return str[peek_col];
        }
    }

    void skip_spaces() { while (isspace(cursor)) advance(); }
    int cur_digit() { return cursor - '0';  }
    int get_integer() {
        // TODO add support of INT_MIN, which is -INT_MAX - 1
        int result = 0;
        while (isdigit(cursor)) {
            int digit = cur_digit();
            if (INT_MAX_DIV_10 < result || (INT_MAX_DIV_10 == result && INT_MAX_LAST_DIGIT < digit)) {
                throw LexerOverflowException(line, col);
            }
            result = result * 10 + digit;
            advance();
        }
        if (isalpha(cursor)) {
            error();
        }
        return result;
    }
    Lexeme get_id() {
        /* TODO fix multiline id problem, currently
           ```
             some
             id
           ```
           interprets as a single "someid", because of skipping '\n' char and it shouldn't: correct will be "some" and "id"
           But for now lets take it as a feature :)
        */
        // TODO optimize this after fixing multiline to not use "heavy" stringstream. remember first position instead and then just create a string from dedicated buffer
        auto start_pos = col;
        std::stringstream ss;
        while (cursor != NONE_CHAR && isalnum(cursor)) {
            ss << cursor;
            advance();
        }
        auto id = ss.str();
        return RESERVED.find(id) == RESERVED.end()
            ? Lexeme(Token::ID, id)
            : RESERVED.at(id);
    }
    void error() const {
        throw LexerException(line, col);
    }

private:
    const int INT_MAX_DIV_10 = INTEGER_MAX / 10;
    const int INT_MAX_LAST_DIGIT = INTEGER_MAX - INT_MAX_DIV_10;
    std::istream& stream;
    std::string str;
    size_t line = 0;
    size_t col = 0;
    char cursor = NONE_CHAR;
};

#endif  // !LEXER_HPP