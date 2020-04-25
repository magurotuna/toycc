#include "toycc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // print pos whitespaces
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void print_token() {
  for (Token *tok = token; tok; tok = tok->next) {
    char *ident = malloc(tok->len + sizeof(char));
    strncpy(ident, tok->str, tok->len);
    printf("kind: %d\tval: %d\tident: %s\tlen: %d\n", tok->kind, tok->val,
           ident, tok->len);
    free(ident);
  }
}

bool consume(char *op) {
  if ((token->kind != TK_RESERVED && token->kind != TK_RETURN) ||
      strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op)
    error_at(token->str, "Unexpected character: '%c'\n", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str,
             "Number is expected, but got a different kind of token.");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() { return token->kind == TK_EOF; }

bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char **str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = *str;
  tok->len = len;
  cur->next = tok;
  *str += len;
  return tok;
}

bool two_char_token(char *p) {
  return !strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "<=", 2) ||
         !strncmp(p, ">=", 2);
}

bool one_char_token(char *p) {
  return *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
         *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=';
}

void tokenize() {
  char *p = user_input;
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, &p, 6);
      continue;
    }

    if (two_char_token(p)) {
      cur = new_token(TK_RESERVED, cur, &p, 2);
      continue;
    }

    if (one_char_token(p)) {
      cur = new_token(TK_RESERVED, cur, &p, 1);
      continue;
    }

    if ('a' <= *p && *p <= 'z') {
      int len = 1;
      char next = *(p + len);
      while ('a' <= next && next <= 'z') {
        len++;
        next = *(p + len);
      }
      cur = new_token(TK_IDENT, cur, &p, len);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, &p, 0);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "Unable to tokenize!");
  }

  new_token(TK_EOF, cur, &p, 0);
  token = head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt();
  code[i] = NULL;
}

Node *stmt() {
  Node *node;

  if (consume("return")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  expect(';');
  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LTE, node, add());
    else if (consume(">"))
      // treat this as "<" to make it easy to generate assembly
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      // treat this as "<=" to make it easy to generate assembly
      node = new_node(ND_LTE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume("+"))
    return primary();
  else if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), primary());
  else
    return primary();
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(')');
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = locals ? locals->offset + 8 : 0;
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  return new_node_num(expect_number());
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}
