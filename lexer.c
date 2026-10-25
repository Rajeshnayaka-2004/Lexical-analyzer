
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

static const char *keywords[] =
{
    "auto", "break", "case", "char", "const",
    "continue", "default", "do", "double", "else",
    "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short",
    "signed", "sizeof", "static", "struct", "switch",
    "typedef", "union", "unsigned", "void", "volatile",
    "while", "_Bool"
};

static const char *operators[] =
{
    "<<=", ">>=", "...",
    "++", "--", "->", "==", "!=", "<=", ">=",
    "&&", "||", "<<", ">>",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "+", "-", "*", "/", "%", "=", "<", ">", "!",
    "&", "|", "^", "~", "?", ":", "#"
};

const char *token_type_name(TokenType type)
{
    switch (type)
    {
        case KEYWORD:        return "KEYWORD";
        case IDENTIFIER:     return "IDENTIFIER";
        case CONSTANT:       return "CONSTANT";
        case OPERATOR:       return "OPERATOR";
        case DELIMITER:      return "DELIMITER";
        case CHARACTER:      return "CHARACTER";
        case STRING_LITERAL: return "STRING_LITERAL";
        default:             return "UNKNOWN";
    }
}

static void emit_token(const char *text, TokenType type)
{
    printf("%-15s : %s\n", token_type_name(type), text);
}

static int is_keyword(const char *word)
{
    size_t i;
    size_t count = sizeof(keywords) / sizeof(keywords[0]);

    for (i = 0; i < count; i++)
    {
        if (strcmp(word, keywords[i]) == 0)
            return 1;
    }

    return 0;
}

static int operator_length(const char *source)
{
    size_t i;
    size_t count = sizeof(operators) / sizeof(operators[0]);

    for (i = 0; i < count; i++)
    {
        size_t len = strlen(operators[i]);

        if (strncmp(source, operators[i], len) == 0)
            return (int)len;
    }

    return 0;
}

static int is_identifier_start(char c)
{
    return isalpha((unsigned char)c) || c == '_';
}

static int is_identifier_part(char c)
{
    return isalnum((unsigned char)c) || c == '_';
}

static int is_number_start(const char *s, int i)
{
    return isdigit((unsigned char)s[i]) ||
           (s[i] == '.' &&
            isdigit((unsigned char)s[i + 1]));
}

void scan_source(const char *source)
{
    int i = 0;

    while (source[i] != '\0')
    {
        char token[MAX_TOKEN_LENGTH];
        int j = 0;
        int len;

        /* Skip whitespace */
        if (isspace((unsigned char)source[i]))
        {
            i++;
            continue;
        }

        /* Skip single-line comments */
        if (source[i] == '/' && source[i + 1] == '/')
        {
            i += 2;

            while (source[i] != '\0' && source[i] != '\n')
                i++;

            continue;
        }

        /* Skip multi-line comments */
        if (source[i] == '/' && source[i + 1] == '*')
        {
            i += 2;

            while (source[i] != '\0' &&
                   !(source[i] == '*' && source[i + 1] == '/'))
            {
                i++;
            }

            if (source[i] == '\0')
            {
                emit_token("Unterminated comment", UNKNOWN);
                return;
            }

            i += 2;
            continue;
        }

        /* Read string literals */
        if (source[i] == '"')
        {
            int closed = 0;

            token[j++] = source[i++];

            while (source[i] != '\0' && source[i] != '\n')
            {
                if (source[i] == '\\' && source[i + 1] != '\0')
                {
                    if (j < MAX_TOKEN_LENGTH - 2)
                        token[j++] = source[i];

                    i++;
                }

                if (j < MAX_TOKEN_LENGTH - 2)
                    token[j++] = source[i];

                if (source[i] == '"')
                {
                    closed = 1;
                    i++;
                    break;
                }

                i++;
            }

            token[j] = '\0';

            if (closed)
                emit_token(token, STRING_LITERAL);
            else
                emit_token(token, UNKNOWN);

            continue;
        }

        /* Read character literals */
        if (source[i] == '\'')
        {
            int closed = 0;

            token[j++] = source[i++];

            while (source[i] != '\0' && source[i] != '\n')
            {
                if (source[i] == '\\' && source[i + 1] != '\0')
                {
                    if (j < MAX_TOKEN_LENGTH - 2)
                        token[j++] = source[i];

                    i++;
                }

                if (j < MAX_TOKEN_LENGTH - 2)
                    token[j++] = source[i];

                if (source[i] == '\'')
                {
                    closed = 1;
                    i++;
                    break;
                }

                i++;
            }

            token[j] = '\0';

            if (closed)
                emit_token(token, CHARACTER);
            else
                emit_token(token, UNKNOWN);

            continue;
        }

        /* Read identifiers and keywords */
        if (is_identifier_start(source[i]))
        {
            while (is_identifier_part(source[i]))
            {
                if (j < MAX_TOKEN_LENGTH - 1)
                    token[j++] = source[i];

                i++;
            }

            token[j] = '\0';

            if (is_keyword(token))
                emit_token(token, KEYWORD);
            else
                emit_token(token, IDENTIFIER);

            continue;
        }

        /* Read integer and simple decimal constants */
        if (is_number_start(source, i))
        {
            int dot_count = 0;
            int invalid = 0;

            while (isdigit((unsigned char)source[i]) ||
                   source[i] == '.')
            {
                if (source[i] == '.')
                {
                    dot_count++;

                    if (dot_count > 1)
                        invalid = 1;
                }

                if (j < MAX_TOKEN_LENGTH - 1)
                    token[j++] = source[i];

                i++;
            }

            /* Detect values such as 123abc */
            if (is_identifier_start(source[i]))
            {
                invalid = 1;

                while (is_identifier_part(source[i]))
                {
                    if (j < MAX_TOKEN_LENGTH - 1)
                        token[j++] = source[i];

                    i++;
                }
            }

            token[j] = '\0';

            if (invalid)
                emit_token(token, UNKNOWN);
            else
                emit_token(token, CONSTANT);

            continue;
        }

        /* Recognise operators */
        len = operator_length(&source[i]);

        if (len > 0)
        {
            int k;

            for (k = 0; k < len; k++)
                token[k] = source[i + k];

            token[len] = '\0';
            i += len;

            emit_token(token, OPERATOR);
            continue;
        }

        /* Recognise delimiters */
        if (strchr("(){}[];,.", source[i]) != NULL)
        {
            token[0] = source[i++];
            token[1] = '\0';

            emit_token(token, DELIMITER);
            continue;
        }

        /* Handle unknown characters */
        token[0] = source[i++];
        token[1] = '\0';

        emit_token(token, UNKNOWN);
    }
}
