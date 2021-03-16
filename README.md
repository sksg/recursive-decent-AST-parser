# recursive-decent-AST-parser

This is a small attempt to create a parser for the grammar
```
expression → factor ( ( "+" | "-" ) factor)*
factor     → unary ( ( "/" | "*" ) unary)*
unary      → "-" number | number
number     → sign? unsigned exponent?
exponent   → ("E" | "e") sign? digit+
unsigned   → ((digit+ ("." digit*)?) | "." digit+)
sign       → "+" | "-"
digit      → "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
```

# TODO list

Will will be implementing the following features during this project:

- [ ] Basic calculation i.e. addition, subtraction, multiplication, and division.
- [ ] basic logic operation i.e. true/false, comparisions, and (the operation), and or (the operation).

and the list will keep growing as we move along.

