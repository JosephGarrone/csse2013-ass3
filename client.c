/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Client (Clubber) implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "errors.h"
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "client.h"
#include "clientConstants.h"
#include "cardPicker.h"

int main(int argc, char **argv) {
    /* The same as equals, but allows for future changes */
    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        return trigger_error(CLIENT_INVALID_ARGS, false);
    }

    Clubber *clubber = (Clubber*) malloc(sizeof(Clubber));
    if (!valid_player_count(clubber, argv[ARG_NUM_PLAYERS])) {
        return trigger_error(CLIENT_INVALID_PLAYER_COUNT, false);
    }

    if (!valid_player_id(clubber, argv[ARG_PLAYER_NAME])) {
        return trigger_error(CLIENT_INVALID_PLAYER_ID, false);
    }

    ack_started();

    clubber->initialise = clubber_initialise;
    clubber->initialise(clubber);

    clubber->score = (char*) malloc(sizeof(char) * clubber->numPlayers *
            INT32_LEN);
    
    for (int i = 0; i < clubber->numPlayers; i++) {
        clubber->score[2 * i] = '0';
        clubber->score[2 * i + 1] = (i < clubber->numPlayers - 1) ? ',' : '\0';
    }

    while (1) {
        Command *command = await_command();
        if (command->valid) {
            int code = perform_action(clubber, command);
           
            if (code != CLIENT_SUCCESS) {
                return trigger_error(code, false);
            } else {
                print_status(clubber);
            }
        } else {
            return trigger_error(CLIENT_INVALID_MESSAGE, false);
        }
    }
}

/**
 * Initialises a new Clubber and hooks up functions / sets initial values
 *
 * Returns nothings
 * */
void clubber_initialise(Clubber *this) {
    /* Abuse integer division to get the correct # of cards */
    this->cardsLeft = DECK_SIZE / this->numPlayers;

    this->hand = (Hand*) malloc(sizeof(Hand)); 
    this->hand->initialise = hand_initialise;
    this->hand->initialise(this->hand, this->cardsLeft);

    this->clubs = (Hand*) malloc(sizeof(Hand));
    this->spades = (Hand*) malloc(sizeof(Hand));
    this->diamonds = (Hand*) malloc(sizeof(Hand));
    this->hearts = (Hand*) malloc(sizeof(Hand));

    this->clubs->initialise = hand_initialise;
    this->spades->initialise = hand_initialise;
    this->diamonds->initialise = hand_initialise;
    this->hearts->initialise = hand_initialise;

    this->clubs->initialise(this->clubs, DECK_SIZE / NUM_SUITS);
    this->spades->initialise(this->spades, DECK_SIZE / NUM_SUITS);
    this->diamonds->initialise(this->diamonds, DECK_SIZE / NUM_SUITS);
    this->hearts->initialise(this->hearts, DECK_SIZE / NUM_SUITS);

    this->nextClub = (Card*) malloc(sizeof(Card));
    this->nextClub->initialise = card_initialise;
    this->nextClub->initialise(this->nextClub);
    this->nextClub->read(this->nextClub, TWO_CLUBS);

    this->last = false;
    this->plays = 0;
    this->lastPlayed = (Card*) NULL;
}

/**
 * Checks whether a valid player count has been given and stores it
 * within clubber
 * 
 * Returns bool indicating validity of player count
 * */
bool valid_player_count(Clubber *clubber, char *raw) {
    char *err;
    int playerCount = strtol(raw, &err, BASE_TEN);

    if (*err != '\0') {
        return false;
    }

    if (playerCount < MIN_PLAYER_COUNT || playerCount > MAX_PLAYER_COUNT) {
        return false;
    }

    clubber->numPlayers = playerCount;

    return true;
}

/**
 * Checks whether a valid player id has been given and stores it
 * within clubber
 *
 * Returns bool indicating validity of player id
 * */
bool valid_player_id(Clubber *clubber, char *raw) {
    if (strlen(raw) != 1) {
        return false;
    }

    if (raw[0] < ASCII_UPPER_START || raw[0] > ASCII_UPPER_END) {
        return false;
    }

    if (raw[0] >= ASCII_UPPER_START + clubber->numPlayers) {
        return false;
    }

    clubber->name = raw[0];

    return true;
}

/**
 * Sends the process started message to the server
 *
 * Returns nothing
 * */
void ack_started() {
    msg_server("%c", SUCCESSFUL_START);
}

/**
 * Sends messages to the server and ensures that the pipe is flushed
 * correctly. Accepts the same arguments as printf via the use of
 * variadic arguments
 *
 * Returns nothing
 * */
void msg_server(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vfprintf(stdout, format, args);
    fflush(stdout);

    va_end(args);
}

/**
 * Waits for a command from the server and then parses it
 *
 * Returns the command that was received
 * */
Command *await_command() {
    Command *command = (Command*) malloc(sizeof(Command));
    command->initialise = command_initialise;
    command->initialise(command);
   
    int err = 0;
    char *data = read_to_eoln(stdin, &err);
   
    if (err == EOF) {
        exit(trigger_error(CLIENT_PIPE_CLOSED, false));
    } else {
        fprintf(stderr, "From hub:%.20s\n", data);

        command->data = data;
        command->validate(command);
    }

    return command;    
}

/**
 * Determines which action to take depending on the command
 * that was received from the server
 *
 * Returns the code of an error (As an int)
 * */
int perform_action(Clubber *clubber, Command *command) {    
    Error *error = get_error(CLIENT_SUCCESS, false);
    bool success = false;
    
    switch (command->type) {
        case CMD_NEW_ROUND:
            clubber->initialise(clubber);
            success = store_hand(clubber, command);
            clubber->hand = sort_hand(clubber->hand);
            clubber->nextClub->read(clubber->nextClub, TWO_CLUBS);
            break;
        case CMD_NEW_TRICK:
            success = play_new_trick(clubber, command);
            break;
        case CMD_TRICK_OVER:
            success = register_trick_over(clubber, command);
            break;
        case CMD_YOUR_TURN:
            success = play_card(clubber, command);
            break;
        case CMD_PLAYED_CARD:
            success = register_play(clubber, command);
            break;
        case CMD_LIST_SCORES:
            success = list_scores(clubber, command);
            break;
        case CMD_END:
            game_over(clubber, command);
            break;
    }

    if (!success) {
        free(error);
        error = get_error(CLIENT_INVALID_MESSAGE, false);
    }

    return error->code;
}

/**
 * Parses the raw message received from the server, and contained
 * within command and attempts to read a valid hand from it.
 *
 * Returns a bool indicating whether the hand was read successfully
 * and whether it was valid
 * */
bool store_hand(Clubber *clubber, Command *command) {
    char *line = command->data;
    Hand *hand = clubber->hand;
    char buffer[CARD_LENGTH];
    int pos = 0;
    
    /* Check to make sure there are no extra characters */
    if (strlen(line) % CARD_LENGTH > 1) {
        return false;
    }

    for (int i = 0; i <= strlen(line); i++) {
        /* If at a comma and have read face,suit,delimiter */
        if ((line[i] == CARD_MARKER || line[i] == NUL_TERM) &&
                pos == CARD_LENGTH) {
            if (card_valid_face(buffer[0]) && card_valid_suit(buffer[1])) {
                /* Check to make sure there isn't an invalid character */
                if (i == strlen(line) && line[i] != NUL_TERM) {
                    return false;
                } else if (i < strlen(line) && line[i] != CARD_MARKER) {
                    return false;
                }

                Card *card = (Card*) malloc(sizeof(Card));
                card->initialise = card_initialise;
                card->initialise(card);
                card->read(card, buffer);
                hand->cards[hand->cardsLeft++] = card;

                if (hand->cardsLeft > DECK_SIZE / clubber->numPlayers) {
                    return false;
                }

                buffer[0] = NUL_TERM;
                buffer[1] = NUL_TERM;
                pos = 0;
            } else {
                return false;
            }
        } else if (pos < CARD_LENGTH) {
            buffer[pos++] = line[i];
        }
    }

    if (hand->cardsLeft != clubber->cardsLeft) {
        return false;
    }

    return true;
}

/**
 * Parses the raw message received from the server and then
 * responds with the card that it will be leading. Logs the played
 * card in the clubber and removes it from the current hand
 *
 * Returns bool indicating success (See perform_action logic)
 * */
bool play_new_trick(Clubber *clubber, Command *command) {
    Card *nextClub = clubber->nextClub;
    clubber->plays = 0;
    int index = get_lead_card(clubber->hand, nextClub->getString(nextClub));

    Card *card = clubber->hand->cards[index];
    char *msg = card->getString(card);
    clubber->hand = hand_without_index(clubber->hand, index);

    store_play(clubber, card);
    clubber->lastPlayed = card;
    
    msg_server("%c%c\n", msg[0], msg[1]);
    return true;
}

/**
 * Parses the raw message received from the server and logs the 
 * trick as being over by setting clubber properties
 *
 * Returns bool indicating success (See perform_action logic)
 * */
bool register_trick_over(Clubber *clubber, Command *command) {
    clubber->plays = 0;
    clubber->last = false;
    return true;
}

/**
 * Parses the raw message received from the server and chooses
 * the correct card to play as it is the clubber's turn
 *
 * Returns bool indicating success (See perform_action logic)
 * */
bool play_card(Clubber *clubber, Command *command) {
    int index = get_play_card(clubber->hand, clubber->suitLed, clubber->last);

    Card *card = clubber->hand->cards[index];
    char *msg = card->getString(card);
    clubber->hand = hand_without_index(clubber->hand, index);

    store_play(clubber, card);
    clubber->lastPlayed = card;

    if (card->suit == CLUBS) {
        recalc_lowest_club(clubber);
    }

    msg_server("%c%c\n", msg[0], msg[1]);

    return true;
}

/**
 * Parses the raw message received from the server and registers
 * the card as being played.
 *
 * Returns bool indicating success (See perform_action logic)
 * */
bool register_play(Clubber *clubber, Command *command) { 
    if (clubber->plays == 0) {
        clubber->suitLed = card_get_suit(command->data[1]);
    }

    clubber->plays++;

    Card *played = (Card*) malloc(sizeof(Card));
    played->initialise = card_initialise;
    played->initialise(played);
    played->read(played, command->data);

    /* Check if its the card we just played, so we dont have duplicates */
    bool same;
    bool match = played->compare(played, clubber->lastPlayed, &same);

    if (!(same && !match)) {
        store_play(clubber, played);
    }

    /* Make sure we know which club is the new lowest club */
    if (played->suit == CLUBS) {
        recalc_lowest_club(clubber);
    }

    if (clubber->plays == clubber->numPlayers - 1) {
        clubber->last = true;
    }
    return true;
}

/**
 * Parses the raw message received from the server and stores 
 * the current scores of the players
 *
 * Returns bool indicating success (See perform_action logic)
 * */
bool list_scores(Clubber *clubber, Command *command) {
    clubber->score = command->data;
    return true;
}

/**
 * Exit the game
 *
 * Returns nothing
 * */
void game_over(Clubber *clubber, Command *command) {
    exit(CLIENT_SUCCESS);
}

/**
 * Looks through the played clubs and finds the lowest
 * playable club that the clubber has
 *
 * Returns nothing (nextClub is set inside the clubber)
 * */
void recalc_lowest_club(Clubber *clubber) {
    Card *current = clubber->nextClub;

    for (int i = 0; i < clubber->clubs->cardsLeft; i++) {
        Card *cmp = clubber->clubs->cards[i];
        bool same;
        
        /* If the current lowest club has been played */
        if (current->compare(current, cmp, &same) == 0 && same) {
            current->becomeNext(current);
            clubber->nextClub = current;

            if (clubber->nextClub->face == ACE) {
                break;
            }

            recalc_lowest_club(clubber);
            break;
        } else {
            // We have the lowest playable club
        }
    }
}

/**
 * Outputs the current game status on receipt of a valid server command
 *
 * Returns nothing
 * */
void print_status(Clubber *clubber) {
    print_hand(clubber);
    
    clubber->spades = sort_hand(clubber->spades);
    clubber->clubs = sort_hand(clubber->clubs);
    clubber->diamonds = sort_hand(clubber->diamonds);
    clubber->hearts = sort_hand(clubber->hearts);

    print_played(clubber, SPADE_PLAYED, clubber->spades);
    print_played(clubber, CLUB_PLAYED, clubber->clubs);
    print_played(clubber, DIAMOND_PLAYED, clubber->diamonds);
    print_played(clubber, HEART_PLAYED, clubber->hearts);

    print_scores(clubber);
}

/**
 * Outputs the current scores
 *
 * Returns nothing
 * */
void print_scores(Clubber *clubber) {
    int offset = strlen(SCORE);
    char message[offset + clubber->numPlayers * INT32_LEN];

    strcpy(message, SCORE);
    strcpy(&message[offset], clubber->score);
    
    message[offset + strlen(clubber->score)] = NUL_TERM;

    fprintf(stderr, "%s\n", message);
}

/**
 * Outputs the current hand of the clubber
 *
 * Returns nothing
 * */
void print_hand(Clubber *clubber) {
    int offset = strlen(HAND);
    char message[offset + clubber->hand->cardsLeft * CARD_STRING_LEN];
    strcpy(message, HAND);

    for (int i = 0; i < clubber->hand->cardsLeft; i++) {
        Card *card = clubber->hand->cards[i];

        strcpy(&message[offset + 3 * i], card->getString(card));
        message[offset + 3 * i + 2] = ',';

        if (i == clubber->hand->cardsLeft - 1) {
            message[offset + 3 * i + 2] = NUL_TERM;
        }
    }
    fprintf(stderr, "%s\n", message);
}

/**
 * Outputs the cards currently present in the supplied hand - use for
 * printing the cards from the 4 suits that are no longer in play
 *
 * Returns nothing
 * */
void print_played(Clubber *clubber, char *msg, Hand *hand) {
    int offset = strlen(CLUBBER_PLAYED) + strlen(msg);
    char message[offset + hand->cardsLeft * 2];
    strcpy(message, CLUBBER_PLAYED);
    strcpy(&message[strlen(CLUBBER_PLAYED)], msg);

    for (int i = 0; i < hand->cardsLeft; i++) {
        Card *card = hand->cards[i];

        message[offset + i * 2] = card_get_face_char(card->face);
        message[offset + i * 2 + 1] = (i < hand->cardsLeft - 1) ? ',' : '\0';
    }

    fprintf(stderr, "%s\n", message);
}

/**
 * Stores the card that was just played in its respective hand
 * 
 * Returns nothing
 * */
void store_play(Clubber *clubber, Card *card) {
    switch (card->suit) {
        case SPADES:
            clubber->spades->cards[clubber->spades->cardsLeft++] = card;
            break;
        case CLUBS:
            clubber->clubs->cards[clubber->clubs->cardsLeft++] = card;
            break;
        case DIAMONDS:
            clubber->diamonds->cards[clubber->diamonds->cardsLeft++] = card;
            break;
        case HEARTS:
            clubber->hearts->cards[clubber->hearts->cardsLeft++] = card;
            break;
    }
}
