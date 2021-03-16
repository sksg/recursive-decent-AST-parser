#pragma once
#include <memory>
#include <iostream>
#include <string>
#include <cmath>
#include <cassert>

namespace ast {
    struct expression {
        enum kind_t {
            unknown,
            // binaries
            addition, subtraction, multiplication, divition,
            // unaries
            negation,
            // literals
            literal_int, literal_float
        } kind = unknown;

        union {
            long int_value;
            double float_value;
            expression* inner;
            struct {
                expression* left, *right;
            };
        };

        static expression* new_binary(kind_t kind, expression* left, expression* right) {
            auto binary = new expression;
            binary->kind = kind;
            binary->left = left;
            binary->right = right;
            return binary;
        }

        static expression* new_unary(kind_t kind, expression* inner) {
            auto unary = new expression;
            unary->kind = kind;
            unary->inner = inner;
            return unary;
        }

        static expression* new_literal(long value) {
            auto literal = new expression;
            literal->kind = literal_int;
            literal->int_value = value;
            return literal;
        }

        static expression* new_literal(double value) {
            auto literal = new expression;
            literal->kind = literal_float;
            literal->float_value = value;
            return literal;
        }

        ~expression() {
            switch (kind) {
                case addition:
                case subtraction:
                case multiplication:
                case divition:
                    delete left;
                    delete right;
                    break;
                case negation:
                    delete inner;
                    break;
                default:
                    break;
            }
        }
    };
}

std::ostream& operator<<(std::ostream &str, ast::expression* expr) {
    switch (expr->kind) {
        case ast::expression::addition:
            str << "(+ " << expr->left << " " << expr->right << ")"; break;
        case ast::expression::subtraction:
            str << "(- " << expr->left << " " << expr->right << ")"; break;
        case ast::expression::multiplication:
            str << "(* " << expr->left << " " << expr->right << ")"; break;
        case ast::expression::divition:
            str << "(/ " << expr->left << " " << expr->right << ")"; break;
        case ast::expression::negation:
            str << "(- " << expr->inner << ")"; break;
        case ast::expression::literal_int:
            str << expr->int_value << "i"; break;
        case ast::expression::literal_float:
            str << expr->float_value << "f"; break;
        default:
            break;
    }
    return str;
}

struct parser {
    const char* position = nullptr;
    const char* const end = nullptr;

    static parser from_string(const std::string &str) {
        return {str.data(), str.data() + str.length()};
    }

    bool not_at_end() {
        assert(position <= end);
        return position < end;
    }

    bool consume_match(char token) {
        assert(position < end);
        if (position[0] == token) {
            position++;
            return true;
        }
        return false;
    }

    long digits;
    double decimal_factor;

    bool consume_match_digits() {
        assert(position < end);
        digits = 0;
        decimal_factor = 1;
        bool match = false;
        while(position < end) {
            switch (position[0]) {
                case '0': digits *= 10; break;
                case '1': digits =  digits * 10 + 1; break;
                case '2': digits =  digits * 10 + 2; break;
                case '3': digits =  digits * 10 + 3; break;
                case '4': digits =  digits * 10 + 4; break;
                case '5': digits =  digits * 10 + 5; break;
                case '6': digits =  digits * 10 + 6; break;
                case '7': digits =  digits * 10 + 7; break;
                case '8': digits =  digits * 10 + 8; break;
                case '9': digits =  digits * 10 + 9; break;
                default: return match;
            }
            match = true;
            position++;
            decimal_factor *= 10;
        }

        return match;
    }

    ast::expression* expression() {
        auto expr = factor();

        while (not_at_end()) {
            if (consume_match('+'))
                expr = ast::expression::new_binary(
                    ast::expression::addition,
                    expr, factor()
                );
            else if (consume_match('-'))
                expr = ast::expression::new_binary(
                    ast::expression::subtraction,
                    expr, factor()
                );
            else
                break;
        }

        return expr;
    }

    ast::expression* factor() {
        auto expr = unary();

        while (not_at_end()) {
            if (consume_match('/'))
                expr = ast::expression::new_binary(
                    ast::expression::divition,
                    expr, factor()
                );
            else if (consume_match('*'))
                expr = ast::expression::new_binary(
                    ast::expression::multiplication,
                    expr, factor()
                );
            else
                break;
        }

        return expr;
    }

    ast::expression* unary() {
        if (not_at_end() && consume_match('-'))
            return ast::expression::new_unary(
                ast::expression::negation,
                number()
            );
        
        return number();
    }

    ast::expression* number() {
        int sign = 1;
        if (consume_match('+')) {}
        else if (consume_match('-'))
            sign = -1;

        bool is_float = false;
        long integral_part = 0;
        double decimal_part = 0;
        double exponent = 1;
        if (not_at_end() && consume_match_digits()) {
            integral_part = sign * digits;
            if (not_at_end() && consume_match('.')) {
                is_float = true;
                if (not_at_end() && consume_match_digits())
                    decimal_part = digits / decimal_factor;
            }
        } else if (
            not_at_end() && consume_match('.') &&
            not_at_end() && consume_match_digits()
        ) {
            is_float = true;
            decimal_part = digits / decimal_factor;
        } else
            throw std::runtime_error("Missing number");

        if (not_at_end() && (consume_match('E') || consume_match('e'))) {
            if (not_at_end() && consume_match('-')) {
                is_float = true;
                consume_match_digits();
                exponent = pow(10, -digits);
            } else if (not_at_end()) {
                consume_match('+');
                consume_match_digits();
                exponent = pow(10, digits);
            } else
                throw std::runtime_error("Ill formed exponent");
        }

        if (is_float)
            return ast::expression::new_literal((integral_part + decimal_part) * exponent);
        else
            return ast::expression::new_literal(integral_part * long(exponent));
    }
};