/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Constants declarations
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "errors.h"

/*
 * Internal function to fetch the server side error strings
 *
 * Takes a valid ServerCode and returns the associated error message.
 *
 * */
const char *get_server_message(ServerCode code) {
    switch(code) {
        case 0:
            return "";
        case 1:
            return "Usage: clubhub deckfile winscore prog1 prog2 [prog3 "\
                    "[prog4]]\n";
        case 2:
            return "Invalid score\n";
        case 3:
            return "Unable to access deckfile\n";
        case 4:
            return "Error reading deck\n";
        case 5:
            return "Unable to start subprocess\n";
        case 6:
            return "Player quit\n";
        case 7:
            return "Invalid message received from player\n";
        case 8:
            return "Invalid play by player\n";
        case 9:
            return "SIGINT caught\n";
        default:
            return "Unknown error code\n";
    }
    return "Unknown error code\n";
}

/*
 * Internal function to fetch the client side error strings
 *
 * Takes a valid ClientCode and returns the associated error message
 *
 * */
const char *get_client_message(ClientCode code) {
    switch(code) {
        case 0:
            return "";
        case 1:
            return "Usage: player number_of_players myid\n";
        case 2:
            return "Invalid player count\n";
        case 3:
            return "Invalid player ID\n";
        case 4:
            return "Unexpected loss of hub\n";
        case 5:
            return "Bad message from hub\n";
        default:
            return "Unknown error code\n";
    }
    return "Unknown error code\n";
}

/* 
 * Internal function to print server errors
 *
 * */
void log_error_server(ServerError error) {
    if (strlen(error.message) == 0) {
        return;
    }

    fprintf(stderr, "%s", error.message);
}

/* 
 * Internal function to print client errors
 *
 * */
void log_error_client(ClientError error) {
    if (strlen(error.message) == 0) {
        return;
    }

    fprintf(stderr, "%s", error.message);
}

/* 
 * Function to print a given error
 *
 * */
void log_error(Error *error) {
    if (error->type == ERROR_SERVER) {
        log_error_server(error->payload.server);
    } else {
        log_error_client(error->payload.client);
    }
}

/*
 * Function to log an error code and message
 * (mainly used for initial program error checking)
 *
 * Returns the code that triggered the warning
 *
 * */
int trigger_error(int code, bool serverSide) {
    Error *error = get_error(code, serverSide);

    log_error(error);

    free(error);

    return code;
}

/*i
 * Function to fetch the error details for a given error code
 *
 * Returns the details for an error matching a valid code and platform
 *
 * */
Error *get_error(int code, bool serverSide) {
    Error *error = (Error*) malloc(sizeof(Error));

    if (serverSide) {
        error->type = ERROR_SERVER;
        error->payload.server.code = code;
        error->payload.server.message = get_server_message((ServerCode) code);
    } else {
        error->type = ERROR_CLIENT;
        error->payload.client.code = code;
        error->payload.client.message = get_client_message((ClientCode) code);
    }

    error->code = code;

    return error;
}











