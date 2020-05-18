#pragma once
#ifndef LEXER_HPP
#define LEXER_HPP


#include <istream> 
#include <string>
#include <sstream>
#include <unordered_map>
#include <limits>
#include <cctype>

const int INTEGER_MAX = std::numeric_limits<int>::max();

enum class Token {
    PROGRAM,
    VAR,
    BEGIN, // start of block
    END, // end of block
    ASSIGN, // :=


    INTEGER, // integer type
    REAL, // float type

    ID, // identifier
    INTEGER_CONST,
    REAL_CONST,

    PLUS,
    MINUS,
    MUL,
    DIV,
    INTEGER_DIV, // integer division
    FLOAT_DIV, // float division

    LPAREN, // (
    RPAREN, // )

    SEMI, // ;
    COLON, // :
    DOT, // .
    COMMA, // ,

    EOP, // end of parse
};
const char NONE_CHAR = 0;

struct Lexeme {
public:
    //Lexeme(Token _type, int _number) : type(_type), int_num(_number) {}
    Lexeme(Token _type, double _number) : type(_type), f_num(_number) {}
    Lexeme(Token _type, std::string _str) : type(_type), str(_str) {}

    Token type;

    // TODO waste of memory here with extra variables.. think about optimizing it later
    //int int_num = 0;
    double f_num = 0;
    std::string str = "";
};

const std::unordered_map<std::string, Lexeme> RESERVED_KEYWORDS({
    { "PROGRAM", Lexeme(Token::PROGRAM, "PROGRAM") },
    { "VAR", Lexeme(Token::VAR, "VAR") },
    { "DIV", Lexeme(Token::INTEGER_DIV, "DIV") },
    { "INTEGER", Lexeme(Token::INTEGER, "INTEGER") },
    { "REAL", Lexeme(Token::REAL, "REAL") },
    { "BEGIN", Lexeme(Token::BEGIN, "BEGIN") },
    { "END", Lexeme(Token::END, "END") },
});

class LexerException : public std::exception {
public:
    explicit LexerException(size_t _line, size_t _column, std::string _msg = "Invalid character") noexcept : std::exception(), line(_line), column(_column) {
        std::stringstream ss;
        ss << _msg << "\nLine: " << line << " Column: " << column;
        msg = ss.str();
    }
    const char* what() const noexcept override {
        return msg.c_str();
    }
protected:
    std::string msg;
private:
    size_t line;
    size_t column;
};


class LexerOverflowException : public LexerException {
public:
    explicit LexerOverflowException(size_t _line, size_t _column) noexcept : LexerException(_line, _column, "Number exceeds INT_MAX") { }
};


class Lexer {
public:
    explicit Lexer(std::istream& _stream) : stream(_stream) {
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
            if (cursor == '{') {
                skip_comment();
                continue;
            }
            if (isalpha(cursor)) {
                return get_id();
            }
            if (isdigit(cursor)) {
                return get_number();
            }
            if (cursor == ':') {
                advance();
                if (cursor == '=') {
                    advance(); return Lexeme(Token::ASSIGN, ":=");
                }
                return Lexeme(Token::COLON, ":");
            }
            switch (cursor) {
            case ':': {
                advance();
                if (cursor == '=') {
                    advance(); return Lexeme(Token::ASSIGN, ":=");
                }
                return Lexeme(Token::COLON, ":");
            }
            case '/': advance(); return Lexeme(Token::FLOAT_DIV, "/");
            case ',': advance(); return Lexeme(Token::COMMA, ",");
            case ';': advance(); return Lexeme(Token::SEMI, ";");
            case '.': advance(); return Lexeme(Token::DOT, ".");
            case '+': advance(); return Lexeme(Token::PLUS, "+");
            case '-': advance(); return Lexeme(Token::MINUS, "-");
            case '*': advance(); return Lexeme(Token::MUL, "*");
            case '(': advance(); return Lexeme(Token::LPAREN, "(");
            case ')': advance(); return Lexeme(Token::RPAREN, ")");
            default: error();
            }
        }
        return Lexeme(Token::EOP, 0);
    }
    auto get_line() const { return line; }
    auto get_col() const { return col; }
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
    void skip_comment() { 
        while (cursor != '}') advance(); 
        advance();
    }

    int cur_digit() { return cursor - '0'; }

    Lexeme get_number() {
        std::stringstream ss;
        while (isdigit(cursor)) {
            ss << cur_digit();
            advance();
        }
        if (cursor != '.') {
            return Lexeme(Token::INTEGER_CONST, std::stoi(ss.str()));
        } else {
            ss << cursor;
            advance();
            while (isdigit(cursor)) {
                ss << cur_digit();
                advance();
            }
            return Lexeme(Token::REAL_CONST, std::stof(ss.str()));
        }
    }
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
        std::stringstream ss;
        while (cursor != NONE_CHAR && isalnum(cursor)) {
            ss << static_cast<char>(std::toupper(static_cast<unsigned char>(cursor)));
            advance();
        }
        auto id = ss.str();
        return RESERVED_KEYWORDS.find(id) == RESERVED_KEYWORDS.end()
            ? Lexeme(Token::ID, id)
            : RESERVED_KEYWORDS.at(id);
    }
    void error() const {
        throw LexerException(line, col);
    }

private:
    const int INT_MAX_DIV_10 = INTEGER_MAX / 10;
    const int INT_MAX_LAST_DIGIT = INTEGER_MAX % 10;
    std::istream& stream;
    std::string str;
    size_t line = 0;
    size_t col = 0;
    char cursor = NONE_CHAR;
};

#endif  // !LEXER_HPP