#include <stdbool.h>

typedef enum {
  TK_RESERVED, // symbol
  TK_IDENT, // identifier
  TK_NUM,      // integer
  TK_EOF,      // end of file
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  struct Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_ASSIGN, // =
  ND_LVAR, // local variable
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LTE, // <=
  ND_GT,  // >
  ND_GTE, // >=
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
};

Token *token;
char *user_input;
Node *code[100];

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char **str, int len);
bool two_char_token(char *p);
bool one_char_token(char *p);
void tokenize();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);
void gen_lval(Node *node);
