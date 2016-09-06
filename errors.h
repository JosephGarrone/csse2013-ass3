/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Errors declarations
 *
 * NOTE TO TUTOR:
 *
 * I understand that my error stuff is way over worked,
 * but i wanted to get a good understanding of unions and 
 * structs (It was the first part of the assignment I wrote).
 * It's meant to be written extensibly (IE: I may use it in 
 * assignment 4, or at least as a base for a much better 
 * implementation).
 *
 * */

#include <stdbool.h>

#ifndef ERRORS_H
#define ERRORS_H

/*
 * Error code structs/unions/data
 * */

/* Client errors */
typedef enum {
    CLIENT_SUCCESS = 0,
    CLIENT_INVALID_ARGS = 1,
    CLIENT_INVALID_PLAYER_COUNT = 2,
    CLIENT_INVALID_PLAYER_ID = 3,
    CLIENT_PIPE_CLOSED = 4,
    CLIENT_INVALID_MESSAGE = 5
} ClientCode;

typedef struct { 
    ClientCode code;
    const char *message;
} ClientError;

/* Server errors */
typedef enum {
    SERVER_SUCCESS = 0,
    SERVER_INVALID_ARGS = 1,
    SERVER_INVALID_WIN_SCORE = 2,
    SERVER_DENIED_DECK_FILE = 3,
    SERVER_INVALID_DECK_FILE = 4,
    SERVER_CHILD_PROCESS_FAILED = 5,
    SERVER_CHILD_DIED = 6,
    SERVER_CHILD_INVALID_COMM = 7,
    SERVER_CHILD_INVALID_PLAY = 8,
    SERVER_SIGINT_RECEIVED = 9
} ServerCode;

typedef struct {
    ServerCode code;
    const char *message;
    
} ServerError;

/* Enum for type of error */
typedef enum {
    ERROR_SERVER,
    ERROR_CLIENT
} ErrorType;


/* Top level error object*/
typedef struct {
    ErrorType type;
    int code;
    union {
        ServerError server;
        ClientError client;
    } payload;
} Error;

/* Function prototypes */
void log_error(Error *error);
Error *get_error(int code, bool serverSide);
int trigger_error(int code, bool serverSide);

#endif
