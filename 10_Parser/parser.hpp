#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <cassert>
#include "./lexer.hpp"

namespace AST {

    struct Node {
        std::string type;
        virtual void delete_children() noexcept {}
    };

    void delete_node(Node* node) noexcept {
        node->delete_children();
        delete node;
    }

    struct BinOp : Node {
        BinOp(Node* _left, Lexeme _operand, Node* _right) : var(_left), operand(_operand), right(_right) {}
        Node* var;
        Lexeme operand;
        Node* right;
        virtual void delete_children() noexcept override {
            delete_node(var);
            delete_node(right);
        }
    };

    struct Num : Node {
        Num(Lexeme _num) : value(_num.number) {}
        int value;
    };

    struct UnaryOp : Node {
        UnaryOp(Lexeme _operand, Node* _expr) : operand(_operand), expr(_expr) {}
        Lexeme operand;
        Node* expr;
        void delete_children() noexcept override {
            delete_node(expr);
        }
    };

    struct Compound : Node {
        std::vector<Node*> children;
        void delete_children() noexcept override {
            for (auto node : children) {
                delete_node(node);
            }
        }
    };

    struct Var : Node {
        Var(Lexeme _id) : id(_id) {}
        Lexeme id;
        int value;
    };

    struct Assign : Node {
        Assign(Var* _var, Lexeme _operand, Node* _expr) : var(_var), operand(_operand), expr(_expr) {}
        Var* var;
        Lexeme operand;
        Node* expr;
        void delete_children() noexcept override {
            delete_node(var);
            delete_node(expr);
        }
    };

    struct NoOp : Node {};
}



class Parser {
public:
    Parser(Lexer& _lexer) : lexer(_lexer), current_lexeme(lexer.get_next_token()) {}
    auto parse() {
        auto node = program();
        if (current_lexeme.type != EOP) {
            error();
        }
        return node;
    }
private:
    void error() {
        throw std::domain_error("Invalid syntax");
    }

    void eat(Token type) {
        if (current_lexeme.type == type) {
            current_lexeme = lexer.get_next_token();
        } else {
            error();
        }
    }

    AST::Node* program() {
        auto node = compound_statement();
        eat(DOT);
        return node;
    }

    AST::Node* compound_statement() {
        eat(BEGIN);
        auto nodes = statement_list();
        eat(END);

        auto root = new AST::Compound();
        root->children = std::move(nodes);
        return root;
    }

    std::vector<AST::Node*> statement_list() {
        auto node = statement();
        std::vector<AST::Node*> results = { node };
        while (current_lexeme.type == SEMI) {
            eat(SEMI);
            results.push_back(statement());
        }
        if (current_lexeme.type == ID) {
            error();
        }
        return results;
    }

    AST::Node* statement() {
        if (current_lexeme.type == BEGIN) {
            return compound_statement();
        } else if (current_lexeme.type == ID) {
            return assignment_statement();
        } else {
            return empty();
        }
    }

    AST::Node* assignment_statement() {
        auto var = variable();
        auto lexeme = current_lexeme;
        eat(ASSIGN);
        auto right = expr();
        return new AST::Assign(var, lexeme, right);
    }

    AST::Var* variable() {
        auto node = new AST::Var(current_lexeme);
        eat(ID);
        return node;
    }

    AST::Node* empty() {
        return new AST::NoOp();
    }

    AST::Node* factor() {
        const auto lexeme = current_lexeme;
        const auto type = lexeme.type;
        switch (type) {
        case PLUS: eat(type); return new AST::UnaryOp(lexeme, factor());
        case MINUS: eat(type); return new AST::UnaryOp(lexeme, factor());
        case INTEGER: eat(type); return new AST::Num(lexeme);
        case LPAREN: {
            eat(type);
            AST::Node* node = expr();
            eat(RPAREN);
            return node;
        }
        default: return variable();
        }
    }

    AST::Node* term() {
        auto node = factor();
        while (current_lexeme.type == MUL || current_lexeme.type == DIV) {
            const Lexeme lexeme = current_lexeme;
            eat(lexeme.type);
            node = new AST::BinOp(node, lexeme, factor());
        }
        return node;
    }

    AST::Node* expr() {
        auto node = term();
        while (current_lexeme.type == PLUS || current_lexeme.type == MINUS) {
            const Lexeme lexeme = current_lexeme;
            eat(lexeme.type);
            node = new AST::BinOp(node, lexeme, term());
        }
        return node;
    }

private:
    Lexer& lexer;
    Lexeme current_lexeme;
};

#endif  // !PARSER_HPP