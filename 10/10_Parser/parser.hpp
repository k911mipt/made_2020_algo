#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <cassert>
#include <string>
#include "./lexer.hpp"

namespace AST {

    struct Node {
        virtual ~Node() {}
    };

    template <typename T = AST::Node>
    inline void delete_nodes(std::vector<T*>& nodes) {
        for (auto node : nodes) {
            delete node;
        }
    }

    struct ValueNode : Node {};
    struct ControlNode : Node {};

    struct BinOp : ValueNode {
        BinOp(ValueNode* _left, Lexeme _operand, ValueNode* _right) : var(_left), operand(_operand), right(_right) {}
        ~BinOp() {
            delete var;
            delete right;
        }
        ValueNode* var;
        Lexeme operand;
        ValueNode* right;
    };

    struct Num : ValueNode {
        Num(Lexeme _lex) : lex(_lex) {}
        Lexeme lex;
    };

    struct UnaryOp : ValueNode {
        UnaryOp(Lexeme _operand, ValueNode* _expr) : operand(_operand), expr(_expr) {}
        ~UnaryOp() {
            delete expr;
        }
        Lexeme operand;
        ValueNode* expr;
    };

    struct Var : ValueNode {
        Var(Lexeme _id) : id(_id) {}
        Lexeme id;
        //int value; // TODO : remove this field
    };

    struct Type : Node {
        Type(Lexeme& _lexeme) : type(_lexeme.type) {}
        Token type;
    };

    struct VarDecl : Node {
        VarDecl(Var* _var, Type* _type) : var(_var), type(_type) {}
        ~VarDecl() {
            delete var;
            delete type;
        }
        Var* var;
        Type* type;
    };

    struct Compound : Node {
        explicit Compound(std::vector<Node*>&& _children) : children(_children) {}
        ~Compound() {
            delete_nodes(children);
        }
        std::vector<Node*> children;
    };

    struct Block : Node {
        explicit Block(std::vector<VarDecl*>& _decl, Compound* _comp) : declarations(std::move(_decl)), compound_statement(_comp) {}
        ~Block() {
            delete_nodes(declarations);
            delete compound_statement;
        }
        std::vector<VarDecl*> declarations;
        Compound* compound_statement;
    };

    struct Program : Node {
        Program(std::string _name, Block* _block) : name(_name), block(_block) {}
        ~Program() {
            delete block;
        }
        std::string name;
        Block* block;
    };

    struct Assign : Node {
        Assign(Var* _var, Lexeme _operand, ValueNode* _expr) noexcept : var(_var), operand(_operand), expr(_expr) {}
        ~Assign() {
            delete var;
            delete expr;
        }
        Var* var;
        Lexeme operand;
        ValueNode* expr;
    };

    struct NoOp : Node {};
}


class ParserException : public std::exception {
public:
    explicit ParserException(size_t _line, size_t _column, std::string _msg = "Invalid syntax") noexcept : std::exception(), line(_line), column(_column) {
        // TODO add "expected %a, got %b part"
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


class Parser {
public:
    Parser(Lexer& _lexer) : lexer(_lexer), current_lexeme(lexer.get_next_token()) {}
    auto parse() {
        auto node = program();
        if (current_lexeme.type != Token::EOP) {
            error();
        }
        return node;
    }
private:
    void error() {
        throw ParserException(lexer.get_line(), lexer.get_col());
    }

    void eat(Token type) {
        if (current_lexeme.type == type) {
            current_lexeme = lexer.get_next_token();
        } else {
            error();
        }
    }

    AST::Block* block() {
        auto declaration_nodes = declarations();
        try {
            auto compound_statement_node = compound_statement();
            return new AST::Block(declaration_nodes, compound_statement_node);
        }
        catch (...) {
            delete_nodes(declaration_nodes);
            throw;
        }
    }

    std::vector<AST::VarDecl*> declarations() {
        std::vector<AST::VarDecl*> result;
        try {
            if (current_lexeme.type == Token::VAR) {
                eat(current_lexeme.type);
                while (current_lexeme.type == Token::ID) {
                    auto var_dec_pack = variable_declaration();
                    result.insert(result.end(), var_dec_pack.begin(), var_dec_pack.end());
                    eat(Token::SEMI);
                }
            }
            return result;
        }
        catch (...) {
            delete_nodes(result);
            throw;
        }
    }

    std::vector<AST::VarDecl*> variable_declaration() {
        std::vector<AST::Var*> var_nodes({ new AST::Var(current_lexeme) });
        try {
            eat(Token::ID);
            while (current_lexeme.type == Token::COMMA) {
                eat(current_lexeme.type);
                var_nodes.push_back(new AST::Var(current_lexeme));
                eat(Token::ID);
            }
            eat(Token::COLON);
            std::vector<AST::VarDecl*> var_declarations;
            auto type_node = type_spec();
            for (auto var_node : var_nodes) {
                var_declarations.push_back(new AST::VarDecl(var_node, new AST::Type(*type_node)));
            }
            delete type_node;
            return var_declarations;
        }
        catch (...) {
            delete_nodes(var_nodes);
            throw;
        }
    }

    AST::Type* type_spec() {
        auto lexeme = current_lexeme;
        eat(lexeme.type);
        return new AST::Type(lexeme);
    }

    AST::Program* program() {
        eat(Token::PROGRAM);
        auto var_node = variable();
        auto prog_name = var_node->id.str;
        try {
            eat(Token::SEMI);
            auto program_node = new AST::Program(prog_name, block());
            eat(Token::DOT);
            return program_node;
            try {
                eat(Token::DOT);
                return program_node;
            }
            catch (...) {
                delete program_node;
                throw;
            }
        }
        catch (...) {
            delete var_node;
            throw;
        }
    }

    AST::Compound* compound_statement() {
        eat(Token::BEGIN);
        auto result = new AST::Compound(statement_list());
        try {
            eat(Token::END);
            return result;
        }
        catch (...) {
            delete result;
            throw;
        }
    }

    std::vector<AST::Node*> statement_list() {
        auto node = statement();
        std::vector<AST::Node*> result = { node };
        try {
            while (current_lexeme.type == Token::SEMI) {
                eat(Token::SEMI);
                result.push_back(statement());
            }
            if (current_lexeme.type == Token::ID) {
                error();
            }
            return result;
        }
        catch (...) {
            delete_nodes(result);
            throw;
        }
    }

    AST::Node* statement() {
        if (current_lexeme.type == Token::BEGIN) {
            return compound_statement();
        } else if (current_lexeme.type == Token::ID) {
            return assignment_statement();
        } else {
            return empty();
        }
    }

    AST::Assign* assignment_statement() {
        auto var = variable();
        try {
            auto lexeme = current_lexeme;
            eat(Token::ASSIGN);
            auto right = expr();
            return new AST::Assign(var, lexeme, right);
        }
        catch (...) {
            delete var;
            throw;
        }
    }

    AST::Var* variable() {
        auto node = new AST::Var(current_lexeme);
        try {
            eat(Token::ID);
            return node;
        }
        catch (...) {
            delete node;
            throw;
        }
    }

    AST::NoOp* empty() {
        return new AST::NoOp();
    }

    AST::ValueNode* factor() {
        const auto lexeme = current_lexeme;
        const auto type = lexeme.type;
        switch (type) {
        case Token::PLUS: eat(type); return new AST::UnaryOp(lexeme, factor());
        case Token::MINUS: eat(type); return new AST::UnaryOp(lexeme, factor());
        case Token::INTEGER_CONST:
        case Token::REAL_CONST: eat(type); return new AST::Num(lexeme);
        case Token::LPAREN: {
            eat(type);
            auto node = expr();
            try {
                eat(Token::RPAREN);
                return node;
            }
            catch (...) {
                delete node;
                throw;
            }
        }
        default: return variable();
        }
    }

    AST::ValueNode* term() {
        auto node = factor();
        try {
            while (current_lexeme.type == Token::MUL || current_lexeme.type == Token::INTEGER_DIV || current_lexeme.type == Token::FLOAT_DIV) {
                const auto lexeme = current_lexeme;
                eat(lexeme.type);
                node = new AST::BinOp(node, lexeme, factor());
            }
            return node;
        }
        catch (...) {
            delete node;
            throw;
        }
    }

    AST::ValueNode* expr() {
        auto node = term();
        try {
            while (current_lexeme.type == Token::PLUS || current_lexeme.type == Token::MINUS) {
                const auto lexeme = current_lexeme;
                eat(lexeme.type);
                node = new AST::BinOp(node, lexeme, term());
            }
            return node;
        }
        catch (...) {
            delete node;
            throw;
        }
    }

private:
    Lexer& lexer;
    Lexeme current_lexeme;
};

#endif  // !PARSER_HPP
