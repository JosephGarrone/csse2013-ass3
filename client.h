/*
 * Joseph Garrone
 * 43541984
 *
 * Asssignment 3 Client (Clubber) declarations
 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "structs.h"

#ifndef CLIENT_H
#define CLIENT_H

/* Function prototypes */
void ack_started();
void msg_server(const char *format, ...);
bool valid_player_count(Clubber *clubber, char *raw);
bool valid_player_id(Clubber *clubber, char *raw);
void clubber_initialise(Clubber *this);
Command *await_command();
int perform_action(Clubber *clubber, Command *command);
bool store_hand(Clubber *clubber, Command *command);
bool play_new_trick(Clubber *clubber, Command *command);
bool register_trick_over(Clubber *clubber, Command *command);
bool play_card(Clubber *clubber, Command *command);
bool register_play(Clubber *clubber, Command *command);
bool list_scores(Clubber *clubber, Command *command);
void game_over(Clubber *clubber, Command *command);
void print_status(Clubber *clubber);
void print_hand(Clubber *clubber);
void print_played(Clubber *clubber, char *msg, Hand *hand);
void store_play(Clubber *clubber, Card *card);
void print_scores(Clubber *clubber);
void recalc_lowest_club(Clubber *clubber);

#endif
