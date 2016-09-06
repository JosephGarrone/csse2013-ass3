/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Constants
 *
 */

/* Used for STRTOL */
#define BASE_TEN 10

/* Argument limits */
#define DECK_SIZE 52
#define MIN_PLAYER_COUNT 2
#define MAX_PLAYER_COUNT 4
#define MIN_WIN_SCORE 0

/* ASCII character limits */
#define ASCII_UPPER_START 'A'
#define ASCII_UPPER_END 'Z'
#define NUM_START '0'
#define NUM_END '9'

/* Pipe constants */
#define SOCKET_READ 0
#define SOCKET_WRITE 1
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define EXEC_END_ARGS NULL
#define DEV_NULL "/dev/null"

/* Default buffer size to use */
#define BUFFER_SIZE 80

/* Miscellanous characters used in files and other IO */
#define SUCCESSFUL_START '-'
#define COMMENT_MARKER '#'
#define DECK_MARKER '.'
#define CARD_MARKER ','
#define NUM_SUITS 4

/* Type, Suit, Nul */
#define CARD_STRING_LEN 3

/* Dot, Nul */
#define DECK_MARKER_LEN 1

/* Type, Suit, Nul */
#define MAX_CLIENT_MESSAGE 3

/* Type, Suit */
#define CARD_LENGTH 2

#define NUL_TERM '\0'

/* Only card that won't be dealt in a 3 player game */
#define TWO_DIAMONDS "2D"

/* lowest club */
#define TWO_CLUBS "2C"

/* Misc Strings */
#define WINNERS "Winner(s):"

/* Max number of chars in int32 */
#define INT32_LEN 10
