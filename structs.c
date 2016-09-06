/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Structs implementaion
 *
 * */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"

/* Player functions */

/**
 * Initialises a new player struct and hooks up functions 
 * 
 * Returns a bool indicating whether all the sys calls went through
 * */
void player_initialise(Player *this, int id, char label, int cardsLeft) {
    /* Add event handlers */
    this->read = player_read_msg;
    this->write = player_write_msg;
    this->setupComms = player_setup_comms;

    this->hand = (Hand*) malloc(sizeof(Hand));
    this->hand->initialise = hand_initialise;
    this->hand->initialise(this->hand, cardsLeft);

    this->played = (Hand*) malloc(sizeof(Hand));
    this->played->initialise = hand_initialise;
    this->played->initialise(this->played, cardsLeft);

    this->maxCards = cardsLeft;
    this->started = false;
    this->running = false;
    this->isLeading = false;
    this->id = id;
    this->label = label;
}

/**
 * Variadic wrapper around vfprintf to allow for forced flushing
 * of input to the player.
 *
 * Returns nothing
 * */
void player_write_msg(Player *this, const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(this->in, format, args);
    fflush(this->in);

    va_end(args);
}

/**
 * Reads a Message from the player's output pipe and performs
 * validity checking and client died checking (EOF).
 *
 * Returns a Message with all fields set to appropriate values
 * */
Message *player_read_msg(Player *this) {
    char chr;
    int err = 0;
    char *data;
    Message *msg = (Message*) malloc(sizeof(Message));
    msg->initialise = message_initialise;
    msg->initialise(msg);

    if (!this->started) {
        chr = fgetc(this->out);
        if (chr == EOF) {
            msg->clientDied = true;
        } else if (chr == SUCCESSFUL_START) {
            msg->valid = true;
            this->started = true;
        }
    } else {
        data = read_to_eoln(this->out, &err);
         
        if (err == EOF) {
            msg->clientDied = true;
        } else if (strlen(data) > MAX_CLIENT_MESSAGE) {
            msg->valid = false;
        } else if (!card_valid_face(data[0]) || !card_valid_suit(data[1])) {
            msg->valid = false; 
        } else {
            msg->valid = true;
            msg->raw = data;
            msg->card = (Card*) malloc(sizeof(Card));
            msg->card->initialise = card_initialise;
            msg->card->initialise(msg->card);
            msg->card->read(msg->card, msg->raw);
        }    
    }

    return msg;
}

/**
 * Attempts to open and store the FILE* for the pipes inside
 * the given player.
 *
 * Returns bool indicating the success of the fdopens
 * */
bool player_setup_comms(Player *this, int in, int out) {
    this->in = fdopen(in, "w");
    this->out = fdopen(out, "r");

    if (!this->in || !this->out) {
        return false;
    }

    return true;
}

/* Card functions */

/**
 * Checks whether the given face value is valid
 *
 * Returns bool indicating the validity
 * */
bool card_valid_face(char face) {
    if (face >= NUM_START + 2 && face <= NUM_END) {
        return true;
    } else if (face == 'T' || face == 'J' || face == 'Q' || face == 'K'
            || face == 'A') {
        return true;
    }

    return false;
}

/**
 * Checks whether the given suit is valid
 *
 * Returns bool indicating the validity
 * */
bool card_valid_suit(char suit) {
    if (suit == 'S' || suit == 'C' || suit == 'D' || suit == 'H') {
        return true;
    }

    return false;
}

/**
 * Checks whether the given card is valid
 *
 * Returns bool indicating the validity
 * */
bool card_valid(Card *this) {
    return card_valid_face(this->face) && card_valid_suit(this->suit);
}

/**
 * Reads a card from a given buffer
 * 
 * Returns nothing
 * * */
void card_read(Card *this, char *line) {
    int type = line[0];
    int suit = line[1];
    
    this->face = card_get_face(type);
    this->suit = card_get_suit(suit);
}

/**
 * Converts a char to its CardFace enum value
 *
 * Doesn't handle unclean input and will default to TWO
 * in the event that it is unclean.
 *
 * Returns a CardFace matching the type given
 * */
CardFace card_get_face(char type) {
    /* 
     * Just a thought, this might be easier to have a static array
     * with indexs at the char numbers, so you could just go
     * return typeArray[tyype];
     * either way, this works good enough at the moment
     * */

    switch (type) {
        case '2':
            return TWO;
        case '3':
            return THREE;
        case '4':
            return FOUR;
        case '5':
            return FIVE;
        case '6':
            return SIX;
        case '7':
            return SEVEN;
        case '8':
            return EIGHT;
        case '9':
            return NINE;
        case 'T':
            return TEN;
        case 'J':
            return JACK;
        case 'Q':
            return QUEEN;
        case 'K':
            return KING;
        case 'A':
            return ACE;
    }

    // This code shoud never run as the input should be sterile
    return TWO;
}

/**
 * Converts a char to its CardSuit enum value
 *
 * Doesn't handle unclean input and will default to SPADES
 * in the event that it is unclean.
 *
 * Returns a CardSuit matching the suit
 * */
CardSuit card_get_suit(char suit) {
    switch (suit) {
        case 'H':
            return HEARTS;
        case 'D':
            return DIAMONDS;
        case 'C':
            return CLUBS;
        case 'S':
            return SPADES;
    }

    // THis should never be called as the input should be sterile
    return SPADES;
}

/**
 * Converts a CardSuit to its respective char representation
 *  
 * Returns a char representing the suit value
 * */
char card_get_suit_char(CardSuit suit) {
    switch (suit) {
        case HEARTS:
            return 'H';
        case DIAMONDS:
            return 'D';
        case CLUBS:
            return 'C';
        case SPADES:
            return 'S';
    }

    // This should never be called as the input should be sterile
    return 'S';
}

/**
 * Converts a CardFace to its respective char representation
 * 
 * Returns a char representing the face value
 * */
char card_get_face_char(CardFace face) {
    switch (face) {
        case TWO:
            return '2';
        case THREE:
            return '3';
        case FOUR:
            return '4';
        case FIVE:
            return '5';
        case SIX:
            return '6';
        case SEVEN:
            return '7';
        case EIGHT:
            return '8';
        case NINE:
            return '9';
        case TEN:
            return 'T';
        case JACK:
            return 'J';
        case QUEEN:
            return 'Q';
        case KING:
            return 'K';
        case ACE:
            return 'A';
    }

    // This should never be called as the input should be sterile
    return '2';
}

/**
 * Gets the string representation of a card
 * 
 * Returns a nul-terminated string containing the face and suit values of the
 * given card
 * */
char *card_get_string(Card *this) {
    char *buffer = (char*) malloc(sizeof(char) * CARD_STRING_LEN);
    buffer[0] = card_get_face_char(this->face);
    buffer[1] = card_get_suit_char(this->suit);
    buffer[2] = NUL_TERM;

    return buffer;
}

/**
 * Initialises a new card struct and hooks up functions
 *
 * Returns nothing 
 * */
void card_initialise(Card *this) {
    this->read = card_read;
    this->getString = card_get_string;
    this->equals = card_equals;
    this->compare = card_compare;
    this->becomeNext = card_become_next;
}

/**
 * Performs a by-value comparison of two cards 
 * 
 * Returns a bool indicating whether the cards are equal
 * */
bool card_equals(Card *this, char *cmp) {
    char *value = this->getString(this);
    bool equal = false;

    if (value[0] == cmp[0] && value[1] == cmp[1] && 
            strlen(value) == strlen(cmp)) {
        equal = true;
    }

    free(value);

    return equal;
}

/**
 * Compares twos cards and returns their relative values
 * IE: compare of 7C and 8C will return -1 because 7-8=-1
 * The same flag will be set if they are of matching suits.
 * If the two cards are not of matching suits, then the comparison
 * is not fully executed, and same is set to false;
 *  
 * Relative value between two cards, WRT the first card
 * */
int card_compare(Card *this, Card *cmp, bool *same) {
    if (cmp == NULL) {
        *same = false;
        return 0;
    }

    if (this->suit != cmp->suit) {
        *same = false;
        return 0;
    }

    *same = true;
    return (this->face - cmp->face);
}


/**
 * Increments the face value of a card (IE, 7C -> 8C) internally in the
 * supplied instance/variable
 *
 * Returns nothing 
 * */
void card_become_next(Card *this) {
    if (this->face != ACE) {
        this->face++;
    }
}

/* Message functions */

/**
 * Initialises a new message struct and sets initial values
 *
 * Returns nothing 
 * */
void message_initialise(Message *this) {
    this->card = (Card*) malloc(sizeof(Card));
    
    this->valid = false;
    this->clientDied = false;
}

/* Command functions */

/**
 * Initialises a new command struct and hooks up functions/initial values
 * 
 * Returns nothing
 * */
void command_initialise(Command *this) {
    this->valid = false;

    this->getType = command_get_type;
    this->validate = command_validate;
}

/**
 * Internally sets the type of the command that was received inside
 * the command struct that is given
 * 
 * Returns a bool indicating whether the command was valid
 * */
bool command_get_type(Command *this) {
    if (str_start_match(NEW_ROUND, this->data)) {
        this->data = &(this->data[strlen(NEW_ROUND)]);
        this->type = CMD_NEW_ROUND;
    } else if (str_match(NEW_TRICK, this->data)) {
        this->data = &(this->data[strlen(NEW_TRICK)]);
        this->type = CMD_NEW_TRICK;
    } else if (str_match(TRICK_OVER, this->data)) {
        this->data = &(this->data[strlen(TRICK_OVER)]);
        this->type = CMD_TRICK_OVER;
    } else if (str_match(YOUR_TURN, this->data)) {
        this->data = &(this->data[strlen(YOUR_TURN)]);
        this->type = CMD_YOUR_TURN;
    } else if (str_start_match(PLAYED, this->data)) {
        this->data = &(this->data[strlen(PLAYED)]);
        this->type = CMD_PLAYED_CARD;
    } else if (str_start_match(SCORES, this->data)) {
        this->data = &(this->data[strlen(SCORES)]);
        this->type = CMD_LIST_SCORES;
    } else if (str_match(END, this->data)) {
        this->data = &(this->data[strlen(END)]);
        this->type = CMD_END;
    } else {
        return false;
    }

    return true;
}

/**
 * Checks whether a given command is valid and stores the result internally
 * within the command
 *
 * Returns nothing 
 * */
void command_validate(Command *this) {
    if (this->getType(this)) {
        this->valid = true;
    } else {
        this->valid = false;
    }
}

/* Deck functions */

/**
 * Initialises a new deck struct and hooks up functions/initial values
 * 
 * Returns nothing
 * */
void deck_initialise(Deck *this) {
    this->print = deck_print;
    this->cards = (Card**) malloc(sizeof(Card*) * DECK_SIZE);

    this->numCards = 0;
}

/**
 * Debug function for printing a deck once it has been stored to stdout
 * 
 * Returns nothing
 * */
void deck_print(Deck *this) {
    printf("Printing deck\n");
    for (int i = 0; i < this->numCards; i++) {
        Card *card = this->cards[i];
        char *value = card->getString(card);

        printf("%s,", value);

        if ((i + 1) % (DECK_SIZE / 4) == 0) {
            printf("\n");
        }

        free(value);
    }
}

/* Hand Functions */

/**
 * Initialises a new hand struct and hooks up functions/initial values
 *
 * Returns nothing
 * */
void hand_initialise(Hand *this, int cards) {
    this->cards = (Card**) malloc(sizeof(Card*) * cards);

    this->getString = hand_get_string;
    this->contains = hand_contains;

    this->cardsLeft = 0;    
}

/**
 * Takes a hand and builds a string detailing the cards in the hand
 * 
 * Returns a nul terminated string representation of the given hand
 * */
char *hand_get_string(Hand *this) { 
    char *buffer = (char*) malloc(sizeof(char) * (this->cardsLeft - 1) \
            * CARD_STRING_LEN);

    int i = 0;

    /* Build the string with
     * n = face value
     * n+1 = suit value
     * n+2 = comma or nul terminator
     * where n is the card start index of the card in the msg */
    for (i = 0; i < this->cardsLeft; i++) {
        Card *card = this->cards[i];
        buffer[i * 3] = card_get_face_char(card->face);
        buffer[i * 3 + 1] = card_get_suit_char(card->suit);
        buffer[i * 3 + 2] = CARD_MARKER;
        
        if (i == this->cardsLeft - 1) {
            buffer[i * 3 + 2] = NUL_TERM;
        }
    }

    return buffer;
}

/*
 * Checks whether the hand contains the specified card (By value)
 * 
 * Returns a bool indicating whether the card is in the hand
 * */
bool hand_contains(Hand *this, Card *card) {
    for (int i = 0; i < this->cardsLeft; i++) {
        Card *current = this->cards[i];
        bool same;

        if (current->compare(current, card, &same) == 0 && same) {
            return false;
        }
    }

    return true;
}






