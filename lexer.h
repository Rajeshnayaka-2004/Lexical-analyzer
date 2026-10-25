
#ifndef LEXER_H
#define LEXER_H

#define MAX_TOKEN_LENGTH 256

typedef enum
{
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    OPERATOR,
    DELIMITER,
    CHARACTER,
    STRING_LITERAL,
    UNKNOWN
} TokenType;

void scan_source(const char *source);

const char *token_type_name(TokenType type);

#endif
