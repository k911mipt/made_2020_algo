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

class Interpreter {
public:
    Interpreter(Parser _parser) : parser(_parser) {}
    int interprete() {
        const auto tree = parser.parse();
        if (tree == nullptr) {
            return 0;
        }
        auto result = visit(tree);
        AST::delete_node(tree);
        return result;
    }
private:
    int visit_BinOp(AST::BinOp* node) {
        auto lhs = visit(node->var);
        auto rhs = visit(node->right);
        switch (node->operand.type) {
        case PLUS: return lhs + rhs;
        case MINUS: return lhs - rhs;
        case MUL: return lhs * rhs;
        case DIV: return lhs / rhs;
        default: assert(false);
        }
        return 0;
    }

    int visit_Num(AST::Num* node) {
        return node->value;
    }

    int visit_UnaryOp(AST::UnaryOp* node) {
        auto type = node->operand.type;
        auto rhs = visit(node->expr);
        switch (node->operand.type) {
        case PLUS: return +rhs;
        case MINUS: return -rhs;
        default: assert(false);
        }
        return 0;
    }

    int visit_Compound(AST::Compound* node) {
        for (auto child : node->children) {
            visit(child);
        }
        return 0; // TODO check here? why not void?
    }

    int visit_Assign(AST::Assign* node) {
        auto var_name = node->var->id.str;
        scope[var_name] = visit(node->expr);
        return 0; // TODO check here? why not void?
    }

    int visit_Var(AST::Var* node) {
        auto var_name = node->id.str;
        return scope.at(var_name); // TODO not found exception
    }

    int visit_NoOp(AST::NoOp* node) {
        return 0; // TODO check here? why not void?
    }

    int visit(AST::Node* node) {
        const std::type_info& node_type = typeid(*node);
        if (node_type == TYPE_BINOP) {
            return visit_BinOp(static_cast<AST::BinOp*>(node));
        } else if (node_type == TYPE_NUM) {
            return visit_Num(static_cast<AST::Num*>(node));
        } else if (node_type == TYPE_UNARYOP) {
            return visit_UnaryOp(static_cast<AST::UnaryOp*>(node));
        } else if (node_type == TYPE_COMPOUND) {
            return visit_Compound(static_cast<AST::Compound*>(node));
        } else if (node_type == TYPE_ASSIGN) {
            return visit_Assign(static_cast<AST::Assign*>(node));
        } else if (node_type == TYPE_VAR) {
            return visit_Var(static_cast<AST::Var*>(node));
        } else if (node_type == TYPE_NOOP) {
            return visit_NoOp(static_cast<AST::NoOp*>(node));
        } 
        assert(false);
        return 0;
    }
public:
    std::unordered_map<std::string, int> scope;
private:
    Parser& parser;
};


#endif  // !INTERPRETER_HPP