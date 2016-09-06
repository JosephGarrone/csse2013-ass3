/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Server header
 *
 * */

#include "structs.h"
#include "errors.h"

#ifndef SERVER_H
#define SERVER_H

/* Function prototypes */
void sigint_handler(int signal);
bool read_win_score(Hub *hub, char *raw);
bool check_file_readable(char *filename);
bool valid_deck_file(Hub *hub, char *filename);
bool start_players(Hub *hub, char **paths);
bool start_player(Hub *hub, int id, char *path);
ServerCode play_game(Hub *hub);
bool process_play(Hub *hub, Card *card);
void send_end(Hub *hub);
void check_graceful_exits(Hub *hub);
bool read_cards(Deck *deck, char *line);
int graceful_exit(Hub *hub, int code);
Hub *hub_initialise(int numPlayers);
bool add_deck(Hub *hub, Deck **deck, bool init);
void change_deck(Hub *hub);
void print_deal(Hub *hub);
ServerCode send_cards(Hub *hub);
void allocate_cards(Hub *hub);
void cleanup_round(Hub *hub);
ServerCode play_round(Hub *hub);
ServerCode play_leader(Hub *hub);
ServerCode play_followers(Hub *hub);
ServerCode play_followers_before(Hub *hub);
ServerCode play_followers_after(Hub *hub);
ServerCode notify_trick_over(Hub *hub);
void notify_followers(Hub *hub, char *played);
void move_next_player(Hub *hub);
bool end_of_game(Hub *hub);
void print_scores(Hub *hub);
ServerCode notify_game_over(Hub *hub);
void print_winners(Hub *hub);

#endif
