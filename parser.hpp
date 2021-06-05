#pragma once
#include "syntax.hpp"
#include "token.hpp"

#include <memory>
#include <cmath>
#include <cassert>

struct parser {
    string_tokenizer tokenizer;
    token previous_token, current_token, next_token;

    struct failure {
        const char* title, *message, *after_message;
        token previous_token, bad_token;
    };

    failure fail(const char* title, const char* message, const char* after_message = nullptr) {
        return failure{title, message, after_message, previous_token, current_token};
    }

    static parser from_string(const std::string &str) {
        auto tokenizer = string_tokenizer::from_string(str);
        auto first = tokenizer.next();
        auto second = tokenizer.next();
        return {tokenizer, token::begin_input(str.data()), first, second};
    }

    bool at_end() {
        return current_token.kind == token::END_OF_INPUT;
    }

    template <typename T>
    bool match(T kind) {
        return current_token.kind == (token::ENUM)kind;
    }

    bool match(const char kind[3]) {
        return (
            current_token.kind == (token::ENUM)(kind[0] + kind[1])
        );
    }

    template <typename T1, typename T2>
    bool match(T1 current_kind, T2 next_kind) {
        return (
            current_token.kind == (token::ENUM)current_kind &&
            next_token.kind == (token::ENUM)next_kind
        );
    }

    void advance() {
        // std::cout << "Consumed token: " << current_token << std::endl;
        previous_token = current_token;
        current_token = next_token;
        next_token = tokenizer.next();
    }

    template <typename T>
    bool consume(T kind) {
        if (match(kind)) {
            advance();
            return true;
        }
        return false;
    }

    syntax number() {
        if (match(token::INT)) {
            auto num = syntax((long)current_token.int_value);
            advance();
            return num;
        }
        if (match(token::FLOAT)) {
            auto num = syntax(current_token.float_value);
            advance();
            return num;
        }
        return syntax::fail();
    }

    syntax identifier() {
        auto id = syntax(current_token.position, current_token.length);
        advance();
        return id;
    }

    syntax literal() {
        if (consume('(')) {
            auto expr = expression();
            if (consume(')'))
                return expr;
            else
                throw fail("Unbalanced parenthesis!", "Expected a closing parenthesis ')'.");
        }
        if (consume("true"))
            return syntax(true);
        if (consume("false"))
            return syntax(false);

        auto expr = match(token::IDENTIFIER) ? identifier() : number();

        if (expr.failed())
            throw fail(
                "Missing value!",
                "Expected a literal value after ", /*previous_token*/
                " e.g. group, identifier, number, or boolean."
            );

        return expr;
    }

    syntax unary() {
        syntax::ENUM unary_kind = syntax::NONE;

        if (consume('+')) {
            unary_kind = syntax::PLUS;
        } else if (consume('-')) {
            unary_kind = syntax::MINUS;
        } else if (consume('!')) {
            unary_kind = syntax::NOT;
        } else
            return literal();

        auto inner = literal();

        if (
            unary_kind == syntax::PLUS && (
                inner.kind == syntax::INT || inner.kind == syntax::FLOAT
            )
        ) return inner; // NO-OP

        if (unary_kind == syntax::MINUS && inner.kind == syntax::INT)
            inner.int_value = -inner.int_value;
        if (unary_kind == syntax::MINUS && inner.kind == syntax::FLOAT)
            inner.float_value = -inner.float_value;

        return syntax(unary_kind, std::move(inner));
    }

    syntax product() {
        auto expr = unary();

        while (!at_end()) {
            if (consume('/')) {
                if (match('+') || match('-') || match('!'))
                    throw fail(
                        "Invalid syntax!",
                        "Unary operators must be surrounded by '(' and ')' when "
                        "used on the right of a binary expression."
                    );
                expr = syntax(syntax::DIVITION, std::move(expr), literal());
            } else if (consume('*')) {
                if (match('+') || match('-') || match('!'))
                    throw fail(
                        "Invalid syntax!",
                        "Unary operators must be surrounded by '(' and ')' when "
                        "used on the right of a binary expression."
                    );
                expr = syntax(syntax::MULTIPLICATION, std::move(expr), literal());
            } else
                break;
        }

        return expr;
    }

    syntax sum() {
        auto expr = product();

        while (!at_end()) {
            if (consume('+')) {
                if (match('+') || match('-') || match('!'))
                    throw fail(
                        "Invalid syntax!",
                        "Unary operators must be surrounded by '(' and ')' when "
                        "used on the right of a binary expression."
                    );
                expr = syntax(syntax::ADDITION, std::move(expr), product());
            } else if (consume('-')) {
                if (match('+') || match('-') || match('!'))
                    throw fail(
                        "Invalid syntax!",
                        "Unary operators must be surrounded by '(' and ')' when "
                        "used on the right of a binary expression."
                    );
                expr = syntax(syntax::SUBTRACTION, std::move(expr), product());
            } else
                break;
        }

        return expr;
    }

    syntax comparison() {
        auto expr = sum();

        while (!at_end()) {
            if (consume("<="))
                expr = syntax(syntax::LESS_EQUAL, std::move(expr), sum());
            else if (consume(">="))
                expr = syntax(syntax::GREATER_EQUAL, std::move(expr), sum());
            else if (consume("!="))
                expr = syntax(syntax::NOT_EQUAL, std::move(expr), sum());
            else if (consume("=="))
                expr = syntax(syntax::EQUAL, std::move(expr), sum());
            else if (consume('<'))
                expr = syntax(syntax::LESS, std::move(expr), sum());
            else if (consume('>'))
                expr = syntax(syntax::GREATER, std::move(expr), sum());
            else
                break;
        }

        return expr;
    }

    syntax expression() {
        return comparison();
    }

    syntax assignment() {
        auto var = identifier();
        assert(consume('='));  // We already checked this in statement()!!
        auto expr = expression();

        return syntax(syntax::ASSIGNMENT, std::move(var), std::move(expr));
    }

    syntax declaration() {
        auto var = identifier();
        assert(consume(':'));  // We already checked this in statement()!!
        auto type = match(token::IDENTIFIER) ? identifier() : syntax::none();
        auto expr = consume('=') ? expression() : syntax::none();

        if (type.is_none() && expr.is_none())
            throw fail(
                "Malformed variable declaration!",
                "You must declare a variable with either a type or an expression."
            );

        return syntax(std::move(var), std::move(type), std::move(expr));
    }

    syntax statement() {
        syntax stmt;

        if (match(token::IDENTIFIER, ':'))
            stmt = declaration();
        else if (match(token::IDENTIFIER, '='))
            stmt = assignment();
        else 
            stmt = expression();
        
        if (!at_end() && !consume('\n') && !consume(';'))
            throw fail(
                "Missing end of statement!",
                "You must cannot be followed by anything other than a newline or a semicolon ';'"
            );

        return stmt;
    }

    syntax parse() {
        try {
            return statement();
        } catch(const failure& f) {
            std::cerr << "\033[1;31m";
            std::cerr << f.title << '\n';
            std::cerr << "The use of '" << f.bad_token << "' is not supported here:\n";
            std::cerr << "\033[0m";
            std::cerr << tokenizer.scanner.source << '\n';
            std::cerr << "\033[1;32m";
            int position = f.bad_token.position - tokenizer.scanner.source;
            for (int i = 0; i < position; ++i)
                std::cerr << ' ';
            for (int i = 0; i < f.bad_token.length; ++i)
                std::cerr << "↑";
            std::cerr << '\n';
            std::cerr << "\033[1;31m";
            if (f.after_message)
                std::cerr << f.message << "'" << f.previous_token << "'" << f.after_message << '\n';
            else
                std::cerr << f.message << '\n';
            std::cerr << "\033[0m";
        }

        return syntax::none();
    }
};
