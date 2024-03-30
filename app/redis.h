#include <stdbool.h>

enum redis_command_type 
{ 
    SIMPLE, 
    AGGREGATE, 
    BULK 
};

enum redis_command_subtype 
{
	SIMPLE_STRING,
	SIMPLE_ERRORS,
	INTEGERS,
	BULK_STRING,
	ARRAYS,
	NULLS,
	BOOLEANS,
	DOUBLES,
	BIG_NUMBERS,
	BULK_ERRORS,
	VERBATIM_STRINGS,
	MAPS,
	SETS,
	PUSHES
};

typedef struct 
{
	enum redis_command_type type;
	enum redis_command_subtype subtype;
	char *string_command;
	char *original_command;
	bool bool_command;
	struct redis_command *child_command;
	struct redis_command *next_command;
    int length;
} redis_command;

/**
 * Get the command object from command_string
 */ 
redis_command* get_redis_command(char *command_string);

/**
 * Free the allocated memory for redis command
 * recursively
 */
void free_redis_command(redis_command *command);

void build_redis_response(char *response, char* res);