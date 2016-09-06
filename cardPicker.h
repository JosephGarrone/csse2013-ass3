/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Card Picker header
 *
 * */

#include "structs.h"

#ifndef CARD_PICKER_H
#define CARD_PICKER_H

/* Function prototypes */
Hand *sort_hand(Hand *hand);
int get_lead_card(Hand *hand, char *lastClub);
int get_play_card(Hand *hand, CardSuit suit, bool last);
Hand *hand_without_index(Hand *hand, int index);
bool get_lowest(Hand *hand, int *index, CardSuit suit);
bool get_highest(Hand *hand, int *index, CardSuit suit);

#endif
