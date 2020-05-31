#pragma once
#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include <cassert>
#include "./parser.hpp"

static const auto& TYPE_BINOP = typeid(AST::BinOp);
static const auto& TYPE_NUM = typeid(AST::Num);
static const auto& TYPE_UNARYOP = typeid(AST::UnaryOp);
static const auto& TYPE_COMPOUND = typeid(AST::Compound);
static const auto& TYPE_ASSIGN = typeid(AST::Assign);
static const auto& TYPE_VAR = typeid(AST::Var);
static const auto& TYPE_NOOP = typeid(AST::NoOp);
static const auto& TYPE_PROGRAM = typeid(AST::Program);
static const auto& TYPE_BLOCK = typeid(AST::Block);
static const auto& TYPE_VARDECL = typeid(AST::VarDecl);
static const auto& TYPE_TYPE = typeid(AST::Type);

class Interpreter {
public:
    Interpreter(Parser _parser) : parser(_parser) {}
    void interprete() {
        const auto tree = parser.parse();
        if (tree == nullptr) {
            return;
        }
        visit(tree);
        delete tree;
    }
private:
#pragma region VoidNodes

    void visit_Compound(AST::Compound* node) {
        for (auto child : node->children) {
            visit(child);
        }
    }

    void visit_Assign(AST::Assign* node) {
        auto var_name = node->var->id.str;
        scope[var_name] = visit_ValueNode(node->expr);
    }

    void visit_Program(AST::Program* node) {
        visit(node->block);
    }

    void visit_Block(AST::Block* node) {
        for (auto decl : node->declarations) {
            visit(decl);
        }
        visit(node->compound_statement);
    }

    void visit_VarDecl(AST::VarDecl* node) {}

    void visit_Type(AST::Type* node) {}

    void visit_NoOp(AST::NoOp* node) {}

    void visit(AST::Node* node) {
        const std::type_info& node_type = typeid(*node);
        if (node_type == TYPE_COMPOUND) {
            visit_Compound(static_cast<AST::Compound*>(node));
        } else if (node_type == TYPE_ASSIGN) {
            visit_Assign(static_cast<AST::Assign*>(node));
        } else if (node_type == TYPE_NOOP) {
            visit_NoOp(static_cast<AST::NoOp*>(node));
        } else if (node_type == TYPE_PROGRAM) {
            visit_Program(static_cast<AST::Program*>(node));
        } else if (node_type == TYPE_BLOCK) {
            visit_Block(static_cast<AST::Block*>(node));
        } else if (node_type == TYPE_VARDECL) {
            visit_VarDecl(static_cast<AST::VarDecl*>(node));
        } else if (node_type == TYPE_TYPE) {
            visit_Type(static_cast<AST::Type*>(node));
        } else {
            assert(false);
        }
    }
#pragma endregion VoidNodes

#pragma region ValueNodes
    double visit_BinOp(AST::BinOp* node) {
        auto lhs = visit_ValueNode(node->var);
        auto rhs = visit_ValueNode(node->right);
        // TODO overflow errors
        switch (node->operand.type) {
        case Token::PLUS: return lhs + rhs;
        case Token::MINUS: return lhs - rhs;
        case Token::MUL: return lhs * rhs;
        case Token::INTEGER_DIV: return static_cast<int>(lhs) / static_cast<int>(rhs);
        case Token::FLOAT_DIV: return lhs / rhs;
        default: assert(false);
        }
        return 0;
    }

    double visit_Num(AST::Num* node) {
        return node->lex.f_num;
    }

    double visit_UnaryOp(AST::UnaryOp* node) {
        auto rhs = visit_ValueNode(node->expr);
        switch (node->operand.type) {
        case Token::PLUS: return +rhs;
        case Token::MINUS: return -rhs;
        default: assert(false);
        }
        return 0;
    }

    double visit_Var(AST::Var* node) {
        auto var_name = node->id.str;
        return scope.at(var_name); // TODO not found exception
    }

    double visit_ValueNode(AST::ValueNode* node) {
        const std::type_info& node_type = typeid(*node);
        if (node_type == TYPE_BINOP) {
            return visit_BinOp(static_cast<AST::BinOp*>(node));
        } else if (node_type == TYPE_NUM) {
            return visit_Num(static_cast<AST::Num*>(node));
        } else if (node_type == TYPE_UNARYOP) {
            return visit_UnaryOp(static_cast<AST::UnaryOp*>(node));
        } else if (node_type == TYPE_VAR) {
            return visit_Var(static_cast<AST::Var*>(node));
        } else {
            assert(false); 
            return 0;
        }
    }
#pragma endregion ValueNodes

public:
    std::unordered_map<std::string, double> scope;
private:
    Parser& parser;
};


#endif  // !INTERPRETER_HPP