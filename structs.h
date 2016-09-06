/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Structs
 *
 * */

/* Maybe I like my C with a little ++... */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef STRUCTS_H
#define STRUCTS_H

#define NEW_ROUND "newround "
#define NEW_TRICK "newtrick"
#define TRICK_OVER "trickover"
#define YOUR_TURN "yourturn"
#define PLAYED "played "
#define SCORES "scores "
#define END "end"

/* Valid card face values */
typedef enum {
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE    
} CardFace;

/* Valid card suits */
typedef enum {
    HEARTS,
    DIAMONDS,
    CLUBS,
    SPADES
} CardSuit;

/* A card within the game */
typedef struct Card Card;
struct Card {
    CardFace face;
    CardSuit suit;

    void (*read)(Card *this, char *data);
    void (*initialise)(Card *this);
    char *(*getString)(Card *this);
    bool (*equals)(Card *this, char *cmp);
    int (*compare)(Card *this, Card *cmp, bool *same);
    void (*becomeNext)(Card *this);
};

/* A hand that holds a list of cards */
typedef struct Hand Hand;
struct Hand {
    Card **cards;
    int cardsLeft;

    void (*initialise)(Hand *this, int cards);
    bool (*contains)(Hand *this, Card *card);
    char *(*getString)(Hand *this);
};

/* A deck that holds a list of cards */
typedef struct Deck Deck;
struct Deck {
    Card **cards;
    int numCards;

    void (*initialise)(Deck *this);
    void (*print)(Deck *this);
};

/* Types of messages that can be recieved from the client */
typedef enum {
    MSG_CLIENT_STARTED,
    MSG_CLIENT_PLAY
} MsgType;

/* A message that is received from the players */
typedef struct Message Message;
struct Message {
    MsgType type;
    Card *card;

    char *raw;

    bool valid;
    bool clientDied;

    void (*initialise)(Message *this);
};

/* A player within the game */
typedef struct Player Player;
struct Player { 
    FILE *in;
    FILE *out;   
    pid_t pid;

    int id;
    bool started;
    bool running;
    char label;

    int score;
    int maxCards;

    Hand *hand;
    Card *lastPlayed;
    bool isLeading;

    Hand *played; 

    void (*initialise)(Player *this, int id, char label, int cardsLeft);
    Message *(*read)(Player *this);
    void (*write)(Player *this, const char *format, ...);
    bool (*setupComms)(Player *this, int in, int out);
};

/* The hub that runs the game */
typedef struct {
    char *deckFile;
    Deck **decks;
    Deck *currentDeck;
    int deckCount;
    int playingDeck;

    int winScore;
    int trickPoints;
    bool allStarted;

    CardSuit currentSuit;
    
    int numPlayers;
    int currentPlayer;
    Player **players;
} Hub;

/* Types of commands that the clubber can receive */
typedef enum {
    CMD_NEW_ROUND,
    CMD_NEW_TRICK,
    CMD_TRICK_OVER,
    CMD_YOUR_TURN,
    CMD_PLAYED_CARD,
    CMD_LIST_SCORES,
    CMD_END
} CmdType;

/* A command that the clubber can receive */
typedef struct Command Command;
struct Command {
    CmdType type;
    char *data;
    bool valid;

    void (*initialise)(Command *this);
    bool (*getType)(Command *this);
    void (*validate)(Command *this);
};

/* A player in the game, on the client side */
typedef struct Clubber Clubber;
struct Clubber {
    int numPlayers;
    int cardsLeft;
    char name;

    CardSuit suitLed;
    bool last;
    int plays;
    Card *lastPlayed;
    Card *nextClub;

    char *score;

    Hand *clubsPlayed;
    Hand *hand;
    Hand *clubs;
    Hand *spades;
    Hand *diamonds;
    Hand *hearts;

    void (*initialise)(Clubber *this);
};

/* Function prototypes */

/* Player prototypes */
void player_initialise(Player *this, int id, char label, int cardsLeft);
void player_write_msg(Player *this, const char *format, ...);
Message *player_read_msg(Player *this);
bool player_setup_comms(Player *this, int in, int out);

/* Card prototypes */
bool card_valid_face(char face);
bool card_valid_suit(char suit);
bool card_valid(Card *this);
void card_read(Card *this, char *line);
CardFace card_get_face(char type);
CardSuit card_get_suit(char suit);
char card_get_suit_char(CardSuit suit);
char card_get_face_char(CardFace face);
char *get_card_string(Card *this);
int card_compare(Card *this, Card *cmp, bool *same);
void card_initialise(Card *this);
bool card_equals(Card *this, char *cmp);
void card_become_next(Card *this);

/* Message prototypes */
void message_initialise(Message *this);

/* Command prototypes */
void command_initialise(Command *this); 
bool command_get_type(Command *this);
void command_validate(Command *this);

/* Deck prototypes */
void deck_initialise(Deck *this);
void deck_print(Deck *this);

/* Hand prototypes */
void hand_initialise(Hand *this, int cards);
char *hand_get_string(Hand *this);
bool hand_contains(Hand *this, Card *card);

#endif
