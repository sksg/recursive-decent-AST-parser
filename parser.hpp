#pragma once
#include "ast.hpp"

#include <memory>
#include <cmath>
#include <cassert> 

struct source_scanner {
    const char* position = nullptr;
    const char* const end = nullptr;

    static source_scanner from_string(const std::string &str) {
        return {str.data(), str.data() + str.length()};
    }

    bool at_end(unsigned offset = 0) {
        assert(position + offset <= end);
        return position + offset >= end;
    }

    char peek(unsigned offset = 0) {
        assert(position + offset < end);
        return position[offset];
    }

    void advance(unsigned offset = 1) {
        assert(position + offset <= end);
        position += offset;
    }
};


struct token {
    const char* position = nullptr;
    long length = 0;
};

struct parser {
    source_scanner scanner;

    static parser from_string(const std::string &str) {
        return {source_scanner::from_string(str)};
    }

    bool match(char token) {
        return !scanner.at_end() && scanner.peek() == token;
    }

    void skip_whitespace() {
        while(match(' ') || match('\t') || match('\r') || match('\n'))
            scanner.advance();
    }

    enum consume_whitespace_flag {
        consume_whitespace, keep_whitespace
    };

    bool consume(char token, consume_whitespace_flag flag=consume_whitespace) {
        if (match(token)) {
            scanner.advance();
            if (flag == consume_whitespace) skip_whitespace();
            return true;
        }
        return false;
    }

    template <int N>
    bool consume(const char (&token)[N], consume_whitespace_flag flag=consume_whitespace) {
        unsigned offset = 0;
        while (!scanner.at_end(offset) && (offset < N - 1)) {
            if (scanner.peek(offset) != token[offset])
                return false;
            ++offset;
        }
        if (offset != (N - 1))
            return false;

        scanner.advance(offset);
        if (flag == consume_whitespace) skip_whitespace();
        return true;
    }

    bool consume_end_statement() {
        while(match(' ') || match('\t') || match('\r'))
            scanner.advance();
        return consume('\n') || consume(';');
    }

    long consume_integer() {
        if (scanner.at_end())
            return -1;

        int digit = scanner.peek() - '0';
        if (digit < 0 || 9 < digit)
            return -1;
        scanner.advance();

        long number = digit;
        while (!scanner.at_end()) {
            digit = scanner.peek() - '0';
            if (digit < 0 || 9 < digit)
                break;
            number = number * 10 + digit;
            scanner.advance();
        }
        return number;
    }

    double consume_decimals() {
        if (scanner.at_end() || !consume('.'))
            return -1;

        int digit = scanner.peek() - '0';
        if (digit < 0 || 9 < digit)
            return 0;
        scanner.advance();

        long factor = 10; 
        double number = double(digit) / factor;
        while (!scanner.at_end()) {
            digit = scanner.peek() - '0';
            if (digit < 0 || 9 < digit)
                break;
            factor *= 10;
            number += double(digit) / factor;
            scanner.advance();
        }
        return number;
    }

    token consume_token() {
        if (scanner.at_end())
            return {};

        auto current = scanner.peek();
        if (
            current != '_' && (
                current < 'a' || 'z' < current
            ) && (
                current < 'A' || 'Z' < current
            )
        ) return {};

        token token{scanner.position, 1};
        scanner.advance();

        while(!scanner.at_end()) {
            current = scanner.peek();
            if (
                current != '_' && (
                    current < 'a' || 'z' < current
                ) && (
                    current < 'A' || 'Z' < current
                ) && (
                    current < '0' || '9' < current
                )
            ) break;
            token.length++;
            scanner.advance();
        }

        skip_whitespace();
        return token;
    }

    syntax number() {
        auto integral_part = consume_integer();
        auto decimal_part = consume_decimals();
        
        if (integral_part < 0 && decimal_part < 0)
            return syntax::fail();


        double exponent_multiplier = 1;
        if (consume('E', keep_whitespace) || consume('e', keep_whitespace)) {
            int sign = consume('+', keep_whitespace) || !consume('-', keep_whitespace) ? 1 : -1;
            auto exponent_part = consume_integer();
            exponent_multiplier = pow(10, sign * exponent_part);
        }

        if (decimal_part < 0 && exponent_multiplier >= 1)
            return syntax(integral_part * (long)exponent_multiplier);
        else if (integral_part < 0 && decimal_part < 0)
            return syntax(exponent_multiplier);
        else if (integral_part < 0)
            return syntax(decimal_part * exponent_multiplier);
        else if (decimal_part < 0)
            return syntax(integral_part * exponent_multiplier);
        else
            return syntax((integral_part + decimal_part) * exponent_multiplier);
    }

    syntax identifier() {
        auto token = consume_token();
        if (token.position != nullptr)
            return syntax(token.position, token.length);
        else
            return syntax::fail();
    }

    syntax literal() {
        if (consume('(')) {
            auto expr = expression();
            if (consume(')'))
                return expr;
            else
                throw std::runtime_error("Unbalanced parenthesis!");
        }
        if (consume("true"))
            return syntax(true);
        if (consume("false"))
            return syntax(false);

        auto expr = identifier();
        if (expr.failed())
            return number();
        return expr;
    }

    syntax unary() {
        if (consume('+')) {
            auto inner = literal();
            if (inner.failed())
                throw std::runtime_error("Missing literal value!");
            else
                return syntax(syntax::PLUS, std::move(inner));
        } if (consume('-')) {
            auto inner = literal();
            if (inner.failed())
                throw std::runtime_error("Missing literal value!");
            else
                return syntax(syntax::MINUS, std::move(inner));
        } if (consume('!')) {
            auto inner = literal();
            if (inner.failed())
                throw std::runtime_error("Missing literal value!");
            else
                return syntax(syntax::NOT, std::move(inner));
        }

        return literal();
    }

    syntax product() {
        auto expr = unary();

        while (!scanner.at_end()) {
            if (consume('/'))
                expr = syntax(syntax::DIVITION, std::move(expr), product());
            else if (consume('*'))
                expr = syntax(syntax::MULTIPLICATION, std::move(expr), product());
            else
                break;
        }

        return expr;
    }

    syntax sum() {
        auto expr = product();

        while (!scanner.at_end()) {
            if (consume('+'))
                expr = syntax(syntax::ADDITION, std::move(expr), product());
            else if (consume('-'))
                expr = syntax(syntax::SUBTRACTION, std::move(expr), product());
            else
                break;
        }

        return expr;
    }

    syntax comparison() {
        auto expr = sum();

        while (!scanner.at_end()) {
            if (consume('<'))
                expr = syntax(syntax::LESS, std::move(expr), sum());
            else if (consume('>'))
                expr = syntax(syntax::GREATER, std::move(expr), sum());
            else if (consume("<="))
                expr = syntax(syntax::LESS_EQUAL, std::move(expr), sum());
            else if (consume(">="))
                expr = syntax(syntax::GREATER_EQUAL, std::move(expr), sum());
            else if (consume("!="))
                expr = syntax(syntax::NOT_EQUAL, std::move(expr), sum());
            else if (consume("=="))
                expr = syntax(syntax::EQUAL, std::move(expr), sum());
            else
                break;
        }

        return expr;
    }

    syntax expression() {
        return comparison();
    }

    syntax assignment() {
        auto pos = scanner.position;
        auto var = identifier();
        if (var.failed() || !consume('=')) {
            scanner.position = pos;
            return syntax::fail();
        }
        auto expr = expression();

        return syntax(syntax::ASSIGNMENT, std::move(var), std::move(expr));
    }

    syntax declaration() {
        auto pos = scanner.position;
        auto var = identifier();
        if (var.failed() || !consume(':')) {
            scanner.position = pos;
            return syntax::fail();
        }
        auto type = identifier();
        if (type.failed())
            type = syntax::none();
        auto expr = consume('=') ? expression() : syntax::fail();

        return syntax(std::move(var), std::move(type), std::move(expr));
    }

    syntax statement() {
        skip_whitespace();

        auto stmt = declaration();
        if (stmt.failed())
            stmt = assignment();
        if (stmt.failed())
            stmt = expression();
        if (!stmt.failed())
            consume_end_statement();
        return stmt;
    }
};