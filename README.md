# recursive-decent-AST-parser

This is a small attempt to create a parser for the grammar
```
// Grammar root:
statement → (declaration | assignment | expression) (";" | "\n")

// Top statements:
declaration → identifier ":" ("=" expression | identifier ("=" expression)?)
assignment  → identifier "=" expression
expression  → comparison

// Expressions:
comparison → sum ( ( "!=" | "==" | "<" | ">" | "<=" | ">=" ) sum)*
sum        → product ( ( "+" | "-" ) product)*
product    → unary ( ( "/" | "*" ) unary)*
unary      → ("!" | "+" | "-") literal | literal

// Primitives:
literal    → number | "true" | "false" | "(" expression ")" | identifier
number     → ("+" | "-")? (((digits ("." digits?)?) | "." digits)) (("E" | "e") ("+" | "-")? digits)?
identifier → token % {"for", "struct", "while", "if", "else"}

// Atoms:
digits        → ["0"-"9"]+
token        → ("_" | ["a"-"z"] | ["A"-"Z"]) ("_" | ["a"-"z"] | ["A"-"Z"] | ["0"-"9"])*
```

# TODO list

Will will be implementing the following features during this project:

- [x] Basic calculation i.e. addition, subtraction, multiplication, and division.
- [x] basic logic operation i.e. true/false, comparisions, and (the operation), and or (the operation).
- [x] variable declaration, assignment, and use

and the list will keep growing as we move along.

