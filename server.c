/*
 *
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Server (Clubhub) implementation
 *
 * */

/**
 * NOTE TO TUTOR:
 *
 * Might make marking/finding functions easier:
 * structs.h - Contains all the struct declarations and prototypes for them
 * structs.c - Contains all struct functions
 * utils.h - Contains prototypes for utils.c
 * utils.c - Contains some handy utils that may be reused in ass4
 * errors.c - Contains my overimplemented error utility (More of an exercise)
 * errors.h - Contains error structs and error enums (Reusable for ass4)
 * constants.h - Constants for use in both client and server
 * server_constants.h - Constants for use in server side
 * client_constants.h - Constants for use in client side
 * card_picker.c - Deals with the card picking logic for hands
 * card_picker.h - Function prototypes for said c file
 *
 * A note on how I've implemented structs. I felt like making it somewhat
 * pythonic (I guess), it terms of replicating the whole __self__ type thing
 * i understand it might be easier to not actually store the functions
 * in the structs themselves, as thats only really handy if you have
 * different functions at different times.
 *
 * Additional note: I use variadic functions in my code. Style.sh doesn't like
 * this, so you may need to manually comment them out. I may or may not write
 * a script (~s4354198/public/var-style.sh), that will comment those lines
 * out and run style.sh - feel free to copy it out and use it, or run it 
 * in place as you should have +x on it.
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "utils.h"
#include "server.h"
#include "errors.h"
#include "structs.h"
#include "constants.h"
#include "serverConstants.h"
#include "cardPicker.h"

/* Global hub variable required for cleaning up nicely when 
 * SIGINT is caught */
Hub *sigHub;

int main(int argc, char **argv) {
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);

    if (argc < MIN_ARGS || argc > MAX_ARGS) {
        return trigger_error(SERVER_INVALID_ARGS, true);
    }

    Hub *hub = hub_initialise(argc - NON_PLAYER_ARGS);
    sigHub = hub;
   
    if (!read_win_score(hub, argv[ARG_WINSCORE])) {
        return trigger_error(SERVER_INVALID_WIN_SCORE, true);
    }

    if (!check_file_readable(argv[ARG_DECK])) {
        return trigger_error(SERVER_DENIED_DECK_FILE, true);   
    }
    
    if (!valid_deck_file(hub, argv[ARG_DECK])) {
        return trigger_error(SERVER_INVALID_DECK_FILE, true);
    } 

    if (!start_players(hub, &argv[ARG_PROG1])) {
        return trigger_error(SERVER_CHILD_PROCESS_FAILED, true);
    }

    ServerCode exitCode = play_game(hub);

    return graceful_exit(hub, exitCode);
}

/**
 * Handler for the SIGINT signal
 * Ensures that a graceful exit is called on receipt of this signal
 *
 * Returns nothing
 * */
void sigint_handler(int signal) {
    exit(graceful_exit(sigHub, SERVER_SIGINT_RECEIVED));
}

/**
 * Send the end message to all the players
 *
 * Returns nothing
 * */
void send_end(Hub *hub) {
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->started) {
            player->write(player, "%s\n", END);
        }
    }
}

/**
 * Check which clubbers have exited nicely, and kill those who haven't
 *
 * Returns nothing
 * */
void check_graceful_exits(Hub *hub) {
    int status;
    int exitCode = 0;

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->started) {
            Message *msg = player->read(player);
            
            if (!msg->clientDied) {
                /* Check if died/ended */
                int result = waitpid(player->pid, &status, WNOHANG);
                
                kill(player->pid, SIGKILL);
                result = waitpid(player->pid, &status, WNOHANG);
               
                /* result is -1 if the process doesn't exist. If the kill
                 * worked, then result would be the zombie process id, 
                 * therefore, the first waitpid clears the zombie leaving 
                 * the next one to catch a pid if the process is still
                 * active, or -1 if it actually already died. */
                if (hub->allStarted && result != -1) {
                    fprintf(stderr, "Player %c terminated due to signal %d\n",
                            player->label, WTERMSIG(status));
                }
            } else {
                /* They died nicely so reap, change status and print exit
                 * info if applicable */
                waitpid(player->pid, &status, WNOHANG);
                player->running = false;
                exitCode = WEXITSTATUS(status);
                if (hub->allStarted && exitCode) {
                    fprintf(stderr, "Player %c exited with status %d\n", 
                            player->label, exitCode);
                }
            }        
        }
    }
}

/**
 * Gracefully shutdown the clubbers and the server
 *
 * Returns int which will match a ServerCode error
 * */
int graceful_exit(Hub *hub, int code) {
    send_end(hub);
    sleep(KILL_INTERVAL);
    check_graceful_exits(hub);
    return trigger_error(code, true);
}

/**
 * Resets all necessary variables at the end of the round
 *
 * Returns nothing
 * */
void cleanup_round(Hub *hub) {
    /* Reset the played struct for each player */
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        player->played = (Hand*) malloc(sizeof(Hand*));
        player->played->initialise = hand_initialise;
        player->played->initialise(player->played, player->maxCards);
    }
}

/**
 * Attempt to play an entire game
 *
 * Returns ServerCode indicating success or errors if encountered
 * */
ServerCode play_game(Hub *hub) {
    int error;
    
    while (1) {
        change_deck(hub);
        allocate_cards(hub);

        error = send_cards(hub);
        if (error != SERVER_SUCCESS) {
            return error;
        }
        
        print_deal(hub);

        error = play_round(hub);
        if (error != SERVER_SUCCESS) {
            return error;
        }

        cleanup_round(hub);

        print_scores(hub);

        if (end_of_game(hub)) {
            error = notify_game_over(hub);
            if (error != SERVER_SUCCESS) {
                return error;
            }
            break;
        }
    }

    print_winners(hub);

    return error;
}

/**
 * Check whether the winscore threshold has been meet or exceeded
 *
 * Returns bool indicating if the game is over
 * */
bool end_of_game(Hub *hub) {
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->score >= hub->winScore) {
            return true;
        }
    }

    return false;
}

/**
 * Prints the current scores to stdout
 *
 * Returns nothing
 * */
void print_scores(Hub *hub) {
    int pos = strlen(SCORES);
    char message[pos + BUFFER_SIZE];
    strcpy(message, SCORES);
    
    /* Build the message */
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        char *score = int_to_str(player->score);

        strcpy(&message[pos], score);
        
        pos += strlen(score);

        if (i != hub->numPlayers - 1) {
            message[pos++] = ',';
        }
    }
    message[pos] = '\0';

    printf("%s\n", message);
}

/**
 * Notifies the players that the game has ended naturally
 *
 * Returns ServerCode indicating the success or any errors encountered
 * */
ServerCode notify_game_over(Hub *hub) {
    int pos = strlen(SCORES);
    char message[pos + BUFFER_SIZE];
    strcpy(message, SCORES);

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        char *score = int_to_str(player->score);

        strcpy(&message[pos], score);
        
        pos += strlen(score);

        if (i != hub->numPlayers - 1) {
            message[pos++] = ',';
        }
    }
    message[pos] = '\0';

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        player->write(player, "%s\n", message);
    }

    return SERVER_SUCCESS;
}

/**
 * Attempts to play through an entire round with the players
 *
 * Returns ServerCode indicating the success or any errors
 * encountered
 * */
ServerCode play_round(Hub *hub) {
    ServerCode error;

    while (hub->currentDeck->numCards > 0) {
        error = play_leader(hub);
        if (error != SERVER_SUCCESS) {
            return error;
        }

        error = play_followers(hub);
        if (error != SERVER_SUCCESS) {
            return error;
        }

        error = notify_trick_over(hub);
        if (error != SERVER_SUCCESS) {
            return error;
        }

        move_next_player(hub);
    }
    
    return SERVER_SUCCESS;
}

/**
 * Notifies the players that the trick has eneded
 *
 * Returns ServerCode indicating the success
 * */
ServerCode notify_trick_over(Hub *hub) {
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        player->write(player, "%s\n", TRICK_OVER);
    }
    
    return SERVER_SUCCESS;
}

/**
 * Ask the leader to play a card to start the trick
 *
 * Returns ServerCode indicating the success or any errors
 * encountered
 * */
ServerCode play_leader(Hub *hub) {
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->isLeading) {
            player->write(player, "%s\n", NEW_TRICK);

            Message *msg = player->read(player);

            /* Check that the server didnt die*/
            if (msg->clientDied) {
                return SERVER_CHILD_DIED;
            } else if (!msg->valid) {
                return SERVER_CHILD_INVALID_COMM;
            } else if (!process_play(hub, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }

            /* Check that it was a valid play */
            if (!player->played->contains(player->played, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }
            
            player->lastPlayed = msg->card;
            player->played->cards[player->played->cardsLeft++] = msg->card;
    
            hub->currentSuit = player->lastPlayed->suit;

            printf("Player %c led %s\n", player->label, msg->raw);

            notify_followers(hub, msg->raw);
        }
    }

    return SERVER_SUCCESS;
}

/**
 * Play through all the other plays in the correct order to
 * complete the round
 *
 * Returns ServerCode indicating the success or any errors
 * encountered
 * */
ServerCode play_followers(Hub *hub) {
    ServerCode error = play_followers_after(hub);
    if (error != SERVER_SUCCESS) {
        return error;
    }

    return play_followers_before(hub);
}

/**
 * Play each of the players that comes before the leader
 * IE: Leader is player B, so A plays
 *
 * Returns a ServerCode indicating the success or any errors
 * encountered
 * */
ServerCode play_followers_before(Hub *hub) {
    bool foundLeader = false;

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->isLeading) {
            foundLeader = true;
        }

        /* Stop when we hit the leader */
        if (foundLeader) {
            break;
        }

        if (!player->isLeading) {
            player->write(player, "%s\n", YOUR_TURN);

            Message *msg = player->read(player);

            /* Check that the msg was valid etc */
            if (msg->clientDied) {
                return SERVER_CHILD_DIED;
            } else if (!msg->valid) {
                return SERVER_CHILD_INVALID_COMM;
            } else if (!process_play(hub, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }

            /* Check that it was a legal play */
            if (!player->played->contains(player->played, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }
            
            player->lastPlayed = msg->card;
            player->played->cards[player->played->cardsLeft++] = msg->card;
            
            printf("Player %c played %s\n", player->label, msg->raw);

            notify_followers(hub, msg->raw);
        }
    }

    return SERVER_SUCCESS;
}

/**
 * Play each of the players that comes after the leader 
 * IE: Leader is player B, so C then D plays
 *
 * see play_followers_before for when A gets to play
 *
 * Returns a ServerCode indicating the success or any errors
 * */
ServerCode play_followers_after(Hub *hub) {
    bool foundLeader = false;

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->isLeading) {
            foundLeader = true;
        }

        /* Don't do anything until the player in question is
         * after the leader */
        if (!foundLeader) {
            continue;
        }

        if (!player->isLeading) {
            player->write(player, "%s\n", YOUR_TURN);

            Message *msg = player->read(player);

            /* Check that the message was valid */
            if (msg->clientDied) {
                return SERVER_CHILD_DIED;
            } else if (!msg->valid) {
                return SERVER_CHILD_INVALID_COMM;
            } else if (!process_play(hub, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }

            /* Check that the play was valid */
            if (!player->played->contains(player->played, msg->card)) {
                return SERVER_CHILD_INVALID_PLAY;
            }
            
            player->lastPlayed = msg->card;
            player->played->cards[player->played->cardsLeft++] = msg->card;
            
            printf("Player %c played %s\n", player->label, msg->raw);

            notify_followers(hub, msg->raw);
        }
    }

    return SERVER_SUCCESS;
}

/**
 * Send a played notification to all the players when a 
 * card is played
 *
 * Returns nothing
 * */
void notify_followers(Hub *hub, char *played) {
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        player->write(player, "%s%s\n", PLAYED, played);
    }
}

/** 
 * Rotate the deck that is currently in use
 *
 * Returns nothing
 * */
void change_deck(Hub *hub) {
    hub->currentDeck = hub->decks[hub->playingDeck++];
    hub->currentDeck->numCards = DECK_SIZE;

    if (hub->playingDeck == hub->deckCount) {
        hub->playingDeck = 0;
    }
}

/**
 * Allocates the cards on the server side (Doesn't send any to 
 * the clubbers)
 *
 * Returns nothing
 * */
void allocate_cards(Hub *hub) {
    int player = 0;
    int dealt = 0;
    int i = 0;

    while (i < hub->currentDeck->numCards) {
        Card *card = hub->currentDeck->cards[i];
        Hand *hand = hub->players[player % hub->numPlayers]->hand;

        /* Dont deal the 2D if there are 3 players */
        if (hub->numPlayers == 3 && card->equals(card, TWO_DIAMONDS)) {
            i++;
            continue;
        }

        if (player % hub->numPlayers == 0 && i) {
            dealt++;
        }
        
        hand->cards[dealt] = card;
        hand->cardsLeft = dealt + 1;

        i++;
        player++;
    }

    if (hub->numPlayers == 3) {
        hub->currentDeck->numCards--;
    }
}

/**
 * Sends a message to a clubber giving them their hand
 *
 * Returns ServerCode indicating the success
 * */
ServerCode send_cards(Hub *hub) {
    int offset = strlen(NEW_ROUND);
    /* msg + # of cards times 3 chars (face, suit, comma) - the last comma */
    char message[offset + (DECK_SIZE / hub->numPlayers) * 3 - 1];
    strcpy(message, NEW_ROUND);

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];
        Hand *hand = player->hand;
   
        /* Construct the message */    
        for (int j = 0; j < hand->cardsLeft; j++) {
            char *value = hand->cards[j]->getString(hand->cards[j]);

            message[j * 3 + offset] = value[0];
            message[j * 3 + 1 + offset] = value[1];

            if (j < hand->cardsLeft - 1) {
                message[j * 3 + 2 + offset] = CARD_MARKER;
            }

            free(value);
        }
        
        player->write(player, "%s\n", message);
    }

    return SERVER_SUCCESS;
}

/**
 * Prints the hands that have been dealt to players
 *
 * Returns nothing
 * */
void print_deal(Hub *hub) {
    /* msg + # of cards times 3 chars (face, suit, comma) - the last comma */
    char message[(DECK_SIZE / hub->numPlayers) * 3 - 1];

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];
        player->hand = sort_hand(player->hand);
        Hand *hand = player->hand;

        /* Build each players message */
        for (int j = 0; j < hand->cardsLeft; j++) {
            char *value = hand->cards[j]->getString(hand->cards[j]);

            message[j * 3] = value[0];
            message[j * 3 + 1] = value[1];

            if (j < hand->cardsLeft - 1) {
                message[j * 3 + 2] = CARD_MARKER;
            } else {
                message[j * 3 + 2] = NUL_TERM;
            }

            free(value);
        }

        printf("Player (%c): %s\n", player->label, message);
    }
}

/**
 * Updates information pertaining to cards remaining
 * and how many points the trick is worth.
 *
 * Returns bool indicating if it was process successfully
 * (Originally this was going to check if the card played
 * matched the clubber's logic, but this isn't needed, thus
 * it always returns true)
 * */
bool process_play(Hub *hub, Card *card) {
    hub->currentDeck->numCards--;

    if (card->suit == CLUBS) {
        hub->trickPoints++;
    }

    return true;
}

/**
 * Figures out which played should be the next to play
 * based on the cardsd that were played in the last trick
 *
 * Returns nothing, the leader flag is set inside the leading
 * player
 * */
void move_next_player(Hub *hub) {
    Player *leader; 
    Card *highest;

    /* Find the card that was lead */
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (!player->isLeading) {
            continue;
        }

        leader = player;
        highest = player->lastPlayed;
    }

    /* See if a higher card was played that matched
     * the suit that was led. If it did, they're the next
     * leader */
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];
        Card *last = player->lastPlayed;

        player->isLeading = false;
        bool same;

        if (last->suit != hub->currentSuit) {
            continue;
        } else if (i == 0) {
            leader = player;
            highest = last;
        } else if (last->compare(last, highest, &same) > 0) {
            leader = player;
            highest = last;
        }
    }

    leader->isLeading = true;
    leader->score += hub->trickPoints;

    hub->trickPoints = 0;
}

/**
 * Prints the winners of the game to stdout
 *
 * Returns nothing
 * */
void print_winners(Hub *hub) {
    int offset = strlen(WINNERS);
    int winners = 0;
    int minScore = hub->winScore;
    /* winners len + num players * (space,label) + nul term */
    char message[offset + hub->numPlayers * 2 + 1];
    strcpy(message, WINNERS);

    /* Find the lowest score */
    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->score < minScore) {
            minScore = player->score;
        }
    }

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = hub->players[i];

        if (player->score == minScore) {
            message[offset + winners * 2] = ' ';
            message[offset + winners * 2 + 1] = player->label;
            winners++;
        }
    }

    message[offset + winners * 2] = NUL_TERM;

    printf("%s\n", message);
}

/**
 * Initialise a hub and set initial values
 *
 * Returns an initialised hub 
 * */
Hub *hub_initialise(int numPlayers) {
    Hub *hub = (Hub*) malloc(sizeof(Hub));
    hub->players = (Player**) malloc(sizeof(Player*) * numPlayers);
    hub->numPlayers = numPlayers;

    /* Allocate enough room for 1 deck - we will use realloc later  */
    hub->decks = (Deck**) malloc(sizeof(Deck*));
    hub->currentPlayer = 0;
    hub->playingDeck = 0;
    hub->currentPlayer = 0;
    hub->deckCount = 0;
    hub->trickPoints = 0;
    hub->allStarted = false;

    for (int i = 0; i < hub->numPlayers; i++) {
        Player *player = (Player*) malloc(sizeof(Player));
        player->initialise = player_initialise;
        player->initialise(player, i, ASCII_UPPER_START + i, 
                DECK_SIZE / numPlayers);
        hub->players[i] = player;
    }

    return hub;
}

/**
 * Check if the win score argument was valid
 * 
 * Return bool indicting if winscore is valid
 * */
bool read_win_score(Hub *hub, char *raw) {
    char *err;
    int winScore = strtol(raw, &err, BASE_TEN);
    
    if (*err != '\0') {
        return false;
    }

    if (winScore < MIN_WIN_SCORE) {
        return false;
    }

    hub->winScore = winScore;

    return true;
}

/**
 * Check if the program is able to open the file for reading
 *
 * Return bool indicating the file is available for reading
 * */
bool check_file_readable(char *filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        return false;
    }

    return true;
}

/**
 * Reads all the decks from a file and performs validation
 * checking on the deck(s)
 *
 * Returns bool indicating validity of the decks
 * */
bool valid_deck_file(Hub *hub, char *filename) {
    char *line;
    int err = 0;
    bool lastLine = false;
    FILE *file = fopen(filename, "r");

    Deck *deck = (Deck*) malloc(sizeof(Deck));
    deck->initialise = deck_initialise;
    deck->initialise(deck);

    line = read_to_eoln(file, &err);
    while (!lastLine) {
        lastLine = (err == EOF);
        /* We hit a comment */
        if (line[0] == COMMENT_MARKER) {
            /* If this is a comment on the last line, then add the deck */
            if (lastLine && !add_deck(hub, &deck, false)) {
                return false;
            }
        /* We hit a lone fullstop on its own, so add the deck */
        } else if (line[0] == DECK_MARKER && 
                strlen(line) == DECK_MARKER_LEN) {
            if (!add_deck(hub, &deck, true)) {
                return false;
            }
        /* We hit an empty line and its the last line so add deck */
        } else if (strlen(line) == 0 && lastLine) {
            if (!add_deck(hub, &deck, false)) {
                return false;
            }
        /* We hit an empty line so do nothing*/
        } else if (strlen(line) == 0) {
            // Do nothing
        /* Must be a line that we should try to read */
        } else if (read_cards(deck, line)) {
            /* If it was the last line, then the deck must be done so add it*/
            if (lastLine && !add_deck(hub, &deck, false)) {
                return false;
            }
        } else {
            return false;
        }

        line = read_to_eoln(file, &err);
    }
    
    return true;
}

/**
 * Add the deck to the list of decks stored in hub
 *
 * Returns bool indicating the validity of the deck
 * */
bool add_deck(Hub *hub, Deck **deck, bool init) {
    if ((*deck)->numCards != DECK_SIZE) {
        return false;
    }

    hub->decks = (Deck**) realloc(hub->decks, 
            sizeof(Deck*) * (hub->deckCount + 1));
    hub->decks[hub->deckCount++] = *deck;

    if (init) {
        (*deck) = (Deck*) malloc(sizeof(Deck));
        (*deck)->initialise = deck_initialise;
        (*deck)->initialise(*deck);
    }

    return true;
}

/**
 * Read the cards from a line into a deck
 *
 * Returns bool indicating validity of the line
 * */
bool read_cards(Deck *deck, char *line) {
    char buffer[CARD_LENGTH];
    int pos = 0;

    for (int i = 0; i <= strlen(line); i++) {
        /* If at a coimma and have read face,suit,delimiter */
        if ((line[i] == CARD_MARKER || line[i] == NUL_TERM) && 
                pos == CARD_LENGTH) {
            if (card_valid_face(buffer[0]) && card_valid_suit(buffer[1])) {
                /* Check to make sure there isnt an invalid character */
                if (i == strlen(line) && line[i] != NUL_TERM) {
                    return false;
                } else if (i < strlen(line) && line[i] != CARD_MARKER) {
                    return false;
                }

                Card *card = (Card*) malloc(sizeof(Card));
                card->initialise = card_initialise;
                card->initialise(card);
                card->read(card, buffer);

                deck->cards[deck->numCards++] = card;

                if (deck->numCards > DECK_SIZE) {
                    return false;
                }

                buffer[0] = NUL_TERM;
                buffer[1] = NUL_TERM;
                pos = 0;
            } else {
                return false;
            }
        } else if (line[i] == CARD_MARKER && i < strlen(line)) {
            /* Encountered a comma not in the right position */
            return false;
        } else if (pos < CARD_LENGTH) {
            buffer[pos++] = line[i];
        }
    }

    return true;
}

/**
 * Starts all the child processes in order
 *
 * Returns bool indicating whether all starts were successful
 **/
bool start_players(Hub *hub, char **paths) {
    for (int i = 0; i < hub->numPlayers; i++) {
        if (!start_player(hub, i, paths[i])) {
            return false;
        } else {
            hub->players[i]->running = true;
        }

        if (i == 0) {
            hub->players[i]->isLeading = true;
        }
    }

    hub->allStarted = true;

    return true;
}

/**
 * Attempt to pipe, dup and exec a child process. Store said pipes
 * for use later.
 *
 * Returns bool indicating whether the child process started successfully 
 * */
bool start_player(Hub *hub, int id, char *path) {
    Player *player = hub->players[id];
    int clientIn[2];
    int clientOut[2];

    if (pipe(clientIn) || pipe(clientOut)) {
        return false;
    }

    player->pid = fork();
    
    if (player->pid == -1) {
        return false;
    } else if (player->pid) {
        /* Close unneeded pipes on serverside */
        close(clientIn[SOCKET_READ]);
        close(clientOut[SOCKET_WRITE]);

        player->setupComms(player, clientIn[SOCKET_WRITE], 
                clientOut[SOCKET_READ]);

        /* Wait for the dash for each player, before starting the next */
        if (!player->read(player)->valid) {
            return false;    
        }
    } else {
        close(clientIn[SOCKET_WRITE]);
        close(clientOut[SOCKET_READ]);

        int devNull = open(DEV_NULL, O_WRONLY);
        dup2(clientIn[SOCKET_READ], STDIN);
        dup2(clientOut[SOCKET_WRITE], STDOUT);

        /* Here we throw away the client's stderr. We can't just leave 
           it in the buffer, or it will overflow at either 4k or 64k */
        dup2(devNull, STDERR);

        char numPlayers[10];
        char label[10];

        sprintf(numPlayers, "%d", hub->numPlayers);
        sprintf(label, "%c", ASCII_UPPER_START + id);

        execl(path, path, numPlayers, label, EXEC_END_ARGS);
    }

    return true;
}

































