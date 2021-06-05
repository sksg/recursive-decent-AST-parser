#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

struct string_scanner {
    const char* source = nullptr;
    const char* position = nullptr;
    const char* const end = nullptr;

    static string_scanner from_string(const std::string &str) {
        return {str.data(), str.data(), str.data() + str.length()};
    }

    bool at_end(unsigned offset = 0) {
        assert(position + offset <= end);
        return position + offset >= end;
    }

    char peek(int offset = 0) {
        assert(position + offset < end);
        return position[offset];
    }

    void advance(unsigned offset = 1) {
        assert(position + offset <= end);
        position += offset;
    }

    char next() {
        assert(position < end);
        return (position++)[0];
    }
};

struct token {
    enum ENUM {
        END_OF_INPUT = '\0',
        OPEN_PARENTHESIS = '(', CLOSE_PARENTHESIS = ')', COMMA = ',',
        COLON = ':', SEMI_COLON = ';', NEW_LINE = '\n', DOT = '.',
        PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/',
        BANG = '!', EQUAL = '=', GREATER = '>', LESSER = '<',
        BANG_EQUAL = ('!' + '='), LESSER_EQUAL = ('<' + '='),
        EQUAL_EQUAL = ('=' + '='), GREATER_EQUAL = ('>' + '='),
        INT, FLOAT, TRUE, FALSE, IDENTIFIER,
        BEGIN_INPUT,
        BAD_CHAR
    } kind = BAD_CHAR;

    union {
        ulong length = 1;
        ulong int_value;
        double float_value;
    };
    const char* position = nullptr;

    token(const char* position, ENUM kind) {
        this->position = position;
        this->kind = kind;
    }
    token(const char* position, char match) {
        this->position = position;
        kind = (ENUM)match;
    }
    token(const char* position, ulong value) {
        this->position = position;
        kind = INT;
        int_value = value;
    }
    token(const char* position, double value) {
        this->position = position;
        kind = FLOAT;
        float_value = value;
    }
    token(const char* position, bool value) {
        this->position = position;
        kind = value ? TRUE : FALSE;
    }

    token(const char* position, char match1, char match2) {
        this->position = position;
        kind = ENUM(match1 + match2);
    }
    static token identifier(const char* position, ulong length) {
        auto id = token{position, IDENTIFIER}; 
        id.length = length;
        return id;
    }
    static token bad_char(const char* position) {
        return token(position, BAD_CHAR);
    }
    static token end_of_input(const char* position) {
        return token(position, END_OF_INPUT);
    }
    static token begin_input(const char* position) {
        return token(position, BEGIN_INPUT);
    }
};

std::ostream& operator << (std::ostream& str, const token &tkn) {
    switch (tkn.kind) {
        case token::OPEN_PARENTHESIS:   str << (char)tkn.kind; break;
        case token::CLOSE_PARENTHESIS:  str << (char)tkn.kind; break;
        case token::COMMA:              str << (char)tkn.kind; break;
        case token::COLON:              str << (char)tkn.kind; break;
        case token::SEMI_COLON:         str << (char)tkn.kind; break;
        case token::NEW_LINE:           str << "\\n"; break;
        case token::DOT:                str << (char)tkn.kind; break;
        case token::PLUS:               str << (char)tkn.kind; break;
        case token::MINUS:              str << (char)tkn.kind; break;
        case token::STAR:               str << (char)tkn.kind; break;
        case token::SLASH:              str << (char)tkn.kind; break;
        case token::BANG:               str << (char)tkn.kind; break;
        case token::EQUAL:              str << (char)tkn.kind; break;
        case token::GREATER:            str << (char)tkn.kind; break;
        case token::LESSER:             str << (char)tkn.kind; break;
        case token::BANG_EQUAL:         str << "!="; break;
        case token::EQUAL_EQUAL:        str << "=="; break;
        case token::GREATER_EQUAL:      str << ">="; break;
        case token::LESSER_EQUAL:       str << "<="; break;
        case token::INT:                str << tkn.int_value; break;
        case token::FLOAT:              str << tkn.float_value; break;
        case token::TRUE:               str << "true"; break;
        case token::FALSE:              str << "false"; break;
        case token::IDENTIFIER:         str << std::string_view(tkn.position, tkn.length); break;
        case token::END_OF_INPUT:      str << "'End of input/file'"; break;
    }
    return str;
}

struct string_tokenizer {
    string_scanner scanner;

    static string_tokenizer from_string(const std::string &str) {
        return string_tokenizer{string_scanner::from_string(str)};
    }

    bool consume(char match) {
        if (!scanner.at_end() && scanner.peek() == match) {
            scanner.advance();
            return true;
        }
        return false;
    }

    template <int N>
    bool consume(const char (&match)[N]) {
        int offset = 0;
        while (!scanner.at_end(offset) && offset < (N - 1)) {
            if (scanner.peek(offset) != match[offset])
                return false;
            ++offset;
        };
        return offset == (N - 1);
    }

    ulong consume_int(ulong int_value = 0) {
        while (!scanner.at_end()) {
            int digit = scanner.peek() - '0';
            if (0 <= digit && digit <= 9) {
                int_value *= 10;
                int_value += digit;
            } else
                break;
            scanner.advance();
        }
        return int_value;
    }

    double consume_float(double float_value = 0) {
        double factor = 0.1;
        while (!scanner.at_end()) {
            int digit = scanner.peek() - '0';
            if (0 <= digit && digit <= 9) {
                factor *= 0.1;
                float_value = digit * factor;
            } else
                break;
            scanner.advance();
        }
        return float_value;
    }

    token consume_number(ulong int_value) {
        auto position = scanner.position - 1;
        int_value = consume_int(int_value);

        if (consume('.')) {
            double float_value = consume_float(int_value);

            if (!consume('E') && !consume('e'))
                return token(position, float_value);

            int sign = consume('+') || !consume('-') ? 1 : -1;
            float_value *= pow(10, sign * consume_int(0));
            return token(position, float_value);
        }

        if (!consume('E') && !consume('e'))
            return token(position, int_value);
        
        if (consume('-')) {
            double float_value = int_value;
            float_value *= pow(10, -consume_int(0));
            return token(position, float_value);
        }

        consume('+');
        int_value *= pow(10, consume_int(0));
        return token(position, int_value);
    }

    token consume_identifier_or_bad_char() {
        char prev = scanner.peek(-1);
        if (!(prev == '_' ||
            ('0' <= prev && prev <= '9') ||
            ('a' <= prev && prev <= 'z') ||
            ('A' <= prev && prev <= 'Z')
        )) return token::bad_char(scanner.position - 1);
        
        auto start = scanner.position - 1;
        while (!scanner.at_end()) {
            char next = scanner.peek();
            if (next == '_' ||
                ('0' <= next && next <= '9') ||
                ('a' <= next && next <= 'z') ||
                ('A' <= next && next <= 'Z')
            ) scanner.advance();
            else
                break;
        }
        return token::identifier(start, ulong(scanner.position - start));
    }

    token next() {
        while (!scanner.at_end()) {
            auto next = scanner.next();
            switch (next) {
                // Skip characters
                case ' ': case '\t': case '\r':
                    continue;
                // Single characters
                case '(': case ')': case '+': case '-': case '*': case '/':
                case '.': case ',': case ':': case ';': case '\n':
                    return token(scanner.position - 1, next);
                // Dual characters
                case '!': case '=': case '>': case '<':
                    return consume('=') ? token(scanner.position - 1, next, '=') : token(scanner.position - 1, next);
                // Numbers
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                    return consume_number(next - '0');
                // Multi characters
                case 'f': return consume("alse") ? token(scanner.position - 1, false) : consume_identifier_or_bad_char();
                case 't': return consume("rue")  ? token(scanner.position - 1, true)  : consume_identifier_or_bad_char();
                // Identifiers (or bad characters)
                default: return consume_identifier_or_bad_char();
            }
        }

        return token::end_of_input(scanner.end);
    }
};
