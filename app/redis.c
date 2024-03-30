#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "redis.h"

/**
 * Based on the command get the command type.
 * 
 * This function is useful for commands and sub-commands
 */
enum redis_command_type get_redis_command_type(char command_string);

/**
 * Based on the first character get the command subtype.
 * 
 * This function is useful for commands and sub-commands.
 */
enum redis_command_subtype get_redis_command_subtype(char command_string);

char* get_next_command_section(char *command_string);

char* get_next_section(char *command_string);

/**
 * Get the command object from command_string
 * 
 * Parse the command string. Check if first byte
 * to get the type. Redis protocol is a string based
 * protocol.
 * (See https://redis.io/docs/reference/protocol-spec/)
 */
redis_command* get_redis_command(char *command_string)
{
	char *raw_token;

	redis_command *command = malloc(sizeof(redis_command));

    command->type = get_redis_command_type(command_string[0]);
    command->subtype = get_redis_command_subtype(command_string[0]);

    // if the type is SIMPLE parse the command
    // else get the length and make a recursive call
    if (command->type == SIMPLE)
    {
        // TODO: Parse the remaining command
    }
    else if (command->type == AGGREGATE)
    {
        
        char t, *next_section;
        sscanf(command_string, "%c%d\r\n", &t, &command->length);
        next_section = get_next_command_section(command_string);
        command->child_command = (struct redis_command *) get_redis_command(next_section);

        redis_command *subcommand = (redis_command *) command->child_command;

        for (int j = 1; j < command->length; j++)
        {
            next_section = get_next_command_section(next_section);
            subcommand->next_command = (struct redis_command *) get_redis_command(next_section);
            subcommand = (redis_command *) subcommand->next_command;
        }
               
    }
    else if (command->type == BULK)
    {
        char t;
        sscanf(command_string, "%c%d\r\n", &t, &command->length);

        command->string_command = malloc(sizeof(char) * (command->length + 1));
        command->original_command = malloc(sizeof(char) * (command->length + 1));

        char *next_command = get_next_section(command_string);

        for (int j = 0; j < command->length; j++)
        {
            command->original_command[j] = next_command[j];
            command->string_command[j] = toupper(next_command[j]);
        }

        command->string_command[command->length] = '\0';
    }
	
	return command;
}

/**
 * Check the first character of the command
 * Categorize it's type based on redis protocol. 
 * (See https://redis.io/docs/reference/protocol-spec/#resp-protocol-description)
 */
enum redis_command_type get_redis_command_type(char command_char)
{
    switch (command_char)
	{
	case '+':
	case '-':
	case ':':
	case '_':
	case '#':
	case ',':
	case '(':
		return SIMPLE;

	case '*':
	case '=':
	case '%':
	case '~':
	case '>':
		return AGGREGATE;

    case '$':
    case '!':
        return BULK;

	default:
		// Unrecognized command, End the program with
		// Error. 
		printf("Unrecognized command type");
		exit(1);
	}
}

/**
 * Check the first character of the command.
 * Categorize it's type based on the redis protocol.
 */
enum redis_command_subtype get_redis_command_subtype(char command_char)
{
    switch(command_char)
    {
    case '+':
        return SIMPLE_STRING;
    case '-':
        return SIMPLE_ERRORS;
    case ':':
        return INTEGERS;
    case '_':
        return NULLS;
    case '#':
        return BOOLEANS;
    case ',':
        return DOUBLES;
    case '(':
        return BIG_NUMBERS;
    case '$':
        return BULK_STRING;
    case '*':
        return ARRAYS;
    case '!':
        return BULK_ERRORS;
    case '=':
        return VERBATIM_STRINGS;
    case '%':
        return MAPS;
    case '~':
        return SETS;
    case '>':
        return PUSHES;
    default:
        // Unrecognized command, End the program with 
        // Error.
        printf("Unrecognized command subtype");
        exit(1);
    }
}

void free_redis_command(redis_command *command)
{
    // TODO: implement free_redis_command
}

char* get_next_section(char *command_string)
{
    int i = 0;
    while ((command_string[i] != '\r' && command_string[i + 1] != '\n'))
        i++;

    i += 2;

    return &command_string[i];
}

char* get_next_command_section(char *command_string)
{
    enum redis_command_type type = get_redis_command_type(command_string[0]);

    if (type == BULK)
    {
        char *next = get_next_section(command_string);
        return get_next_section(next);
    }
    else
    {
        return get_next_section(command_string);
    }
}
    
    
void build_redis_response(char *response, char* res)
{
    printf("$%ld\r\n%s\r\n", strlen(res), res);
    sprintf(response, "$%ld\r\n%s\r\n\0", strlen(res), res);
}