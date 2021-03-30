#pragma once
#include <iostream>
#include <string>
#include <string_view>

struct syntax {
    enum ENUM {
        FAILED,
        DECLARATION,
        // binaries
        ADDITION, SUBTRACTION, MULTIPLICATION, DIVITION,
        LESS, GREATER, LESS_EQUAL, GREATER_EQUAL,
        NOT_EQUAL, EQUAL,
        ASSIGNMENT,
        // unaries
        PLUS, MINUS, NOT,
        // literals
        IDENTIFIER, INT, FLOAT, BOOL
    } kind = FAILED;

    struct declaration_t;
    struct binary_t;
    struct unary_t;
    struct identifier_t;

    union {
        struct declaration_t *declaration;
        struct binary_t *binary;
        struct unary_t *unary;
        struct identifier_t *id;
        long   int_value;
        double float_value;
        bool   bool_value;
    };

    syntax& var();
    syntax& type();
    syntax& value();
    syntax& left();
    syntax& right();
    syntax& inner();

    const syntax& var() const;
    const syntax& type() const;
    const syntax& value() const;
    const syntax& left() const;
    const syntax& right() const;
    const syntax& inner() const;

    syntax(syntax&& other);
    syntax& operator=(syntax&& other);
    syntax(ENUM kind, syntax&& inner);
    syntax(ENUM kind, syntax&& left, syntax&& right);
    syntax(syntax&& var, syntax&& type, syntax&& value);
    syntax(const char* position, int length);
    syntax(long value);
    syntax(double value);
    syntax(bool value);
    syntax();  // same as fail()
    static syntax fail();

    bool failed();

    ~syntax();
};

struct syntax::unary_t {syntax inner;};
struct syntax::binary_t {syntax left, right;};
struct syntax::declaration_t {syntax var, type, value;};
struct syntax::identifier_t {const char* name; int length;};

const syntax& syntax::inner() const { return unary->inner; }
const syntax& syntax::left()  const { return binary->left; }
const syntax& syntax::right() const { return binary->right; }
const syntax& syntax::var()   const { return declaration->var; }
const syntax& syntax::type()  const { return declaration->type; }
const syntax& syntax::value() const { return declaration->value; }

syntax& syntax::inner() { return unary->inner; }
syntax& syntax::left()  { return binary->left; }
syntax& syntax::right() { return binary->right; }
syntax& syntax::var()   { return declaration->var; }
syntax& syntax::type()  { return declaration->type; }
syntax& syntax::value() { return declaration->value; }

syntax::syntax(syntax&& other) {
    switch (other.kind) {
        case DECLARATION:
            declaration = other.declaration; break;
        case ADDITION:
        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVITION:
        case NOT_EQUAL:
        case EQUAL:
        case LESS:
        case GREATER:
        case LESS_EQUAL:
        case GREATER_EQUAL:
        case ASSIGNMENT:
            binary = other.binary; break;
        case PLUS:
        case MINUS:
        case NOT:
            unary = other.unary; break;
        case IDENTIFIER:
            id = other.id; break;
        case INT:
            int_value = other.int_value; break;
        case FLOAT:
            float_value = other.float_value; break;
        case BOOL:
            bool_value = other.bool_value; break;
    };
    kind = other.kind;
    other.kind = FAILED;
}

syntax& syntax::operator=(syntax&& other) {
    switch (other.kind) {
        case DECLARATION:
            declaration = other.declaration; break;
        case ADDITION:
        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVITION:
        case NOT_EQUAL:
        case EQUAL:
        case LESS:
        case GREATER:
        case LESS_EQUAL:
        case GREATER_EQUAL:
        case ASSIGNMENT:
            binary = other.binary; break;
        case PLUS:
        case MINUS:
        case NOT:
            unary = other.unary; break;
        case IDENTIFIER:
            id = other.id; break;
        case INT:
            int_value = other.int_value; break;
        case FLOAT:
            float_value = other.float_value; break;
        case BOOL:
            bool_value = other.bool_value; break;
    };
    kind = other.kind;
    other.kind = FAILED;
    return *this;
}

syntax::syntax(ENUM kind, syntax&& inner) {
    this->kind = kind;
    unary = new unary_t;
    std::swap(unary->inner, inner);
}

syntax::syntax(ENUM kind, syntax&& left, syntax&& right) {
    this->kind = kind;
    binary = new binary_t;
    std::swap(binary->left, left);
    std::swap(binary->right, right);
}

syntax::syntax(syntax&& var, syntax&& type, syntax&& value) {
    kind = DECLARATION;
    declaration = new declaration_t;
    std::swap(declaration->var, var);
    std::swap(declaration->type, type);
    std::swap(declaration->value, value);
}

syntax::syntax(const char* position, int length) {
    kind = IDENTIFIER;
    id = new identifier_t{position, length};
}

syntax::syntax(long value) {
    kind = INT;
    int_value = value;
}

syntax::syntax(double value) {
    kind = FLOAT;
    float_value = value;
}

syntax::syntax(bool value) {
    kind = BOOL;
    bool_value = value;
}

syntax::syntax() {}

syntax syntax::fail() {
    auto fail = syntax();
    fail.kind = syntax::FAILED;
    return fail;
}

bool syntax::failed() {
    return kind == syntax::FAILED;
}


syntax::~syntax() {
    switch (kind) {
        case DECLARATION:
            delete declaration; break;
        case ADDITION:
        case SUBTRACTION:
        case MULTIPLICATION:
        case DIVITION:
        case NOT_EQUAL:
        case EQUAL:
        case LESS:
        case GREATER:
        case LESS_EQUAL:
        case GREATER_EQUAL:
        case ASSIGNMENT:
            delete binary; break;
        case PLUS:
        case MINUS:
        case NOT:
            delete unary; break;
        case IDENTIFIER:
            delete id; break;
    };
    kind = FAILED;
}

std::ostream& operator<<(std::ostream &str, const syntax& ast) {
    switch (ast.kind) {
        case syntax::DECLARATION:
            str << "(" << ast.var() << ":" << ast.type() << "=" << ast.value() << ")"; break;
        case syntax::ADDITION:
            str << "(" << ast.left() << " + " << ast.right() << ")"; break;
        case syntax::SUBTRACTION:
            str << "(" << ast.left() << " - " << ast.right() << ")"; break;
        case syntax::MULTIPLICATION:
            str << "(" << ast.left() << " * " << ast.right() << ")"; break;
        case syntax::DIVITION:
            str << "(" << ast.left() << " / " << ast.right() << ")"; break;
        case syntax::NOT_EQUAL:
            str << "(" << ast.left() << " != " << ast.right() << ")"; break;
        case syntax::EQUAL:
            str << "(" << ast.left() << " == " << ast.right() << ")"; break;
        case syntax::LESS:
            str << "(" << ast.left() << " < " << ast.right() << ")"; break;
        case syntax::GREATER:
            str << "(" << ast.left() << " > " << ast.right() << ")"; break;
        case syntax::LESS_EQUAL:
            str << "(" << ast.left() << " <= " << ast.right() << ")"; break;
        case syntax::GREATER_EQUAL:
            str << "(" << ast.left() << " >= " << ast.right() << ")"; break;
        case syntax::ASSIGNMENT:
            str << "(" << ast.left() << " = " << ast.right() << ")"; break;
        case syntax::PLUS:
            str << "(+ " << ast.inner() << ")"; break;
        case syntax::MINUS:
            str << "(- " << ast.inner() << ")"; break;
        case syntax::NOT:
            str << "(! " << ast.inner() << ")"; break;
        case syntax::IDENTIFIER:
            str << "'" << std::string_view(ast.id->name, ast.id->length) << "'id"; break;
        case syntax::INT:
            str << ast.int_value << "i"; break;
        case syntax::FLOAT:
            str << ast.float_value << "f"; break;
        case syntax::BOOL:
            str << (ast.bool_value ? "true" : "false"); break;
        case syntax::FAILED:
            str << "failed"; break;
    }
    return str;
}
