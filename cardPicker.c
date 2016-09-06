/*
 * Joseph Garrone
 * 43541984
 *
 * Assignment 3 Card Picker Implementation
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "utils.h"
#include "cardPicker.h"
#include "constants.h"

/**
 * Sorts a hand in order from lowest to highest
 *
 * NOTE: This is a bad sort - doesn't free the old
 * cards memory, and it doesn't reuse the old cards
 * (which it should). If I was redoing this, it would
 * just make a new hand and copy the pointers over in a 
 * new order
 *
 * Returns a pointer to the sorted hand
 * */
Hand *sort_hand(Hand *hand) {
    Hand *sorted = (Hand*) malloc(sizeof(Hand));
    sorted->initialise = hand_initialise;
    sorted->initialise(sorted, hand->cardsLeft);

    CardSuit order[NUM_SUITS] = {SPADES, CLUBS, DIAMONDS, HEARTS};

    for (int i = 0; i < NUM_SUITS; i++) {
        int index = 0;
        while (get_lowest(hand, &index, order[i])) {
            sorted->cards[sorted->cardsLeft++] = hand->cards[index];
            hand = hand_without_index(hand, index);
        }
    }

    return sorted;
}

/**
 * Chooses the correct card to play from a hand (When leading)
 * step 1 - check if hand has lowest known club
 * step 2 - if not get highest from D H S C
 *
 * Returns the index of the card in the hand
 * */
int get_lead_card(Hand *hand, char *nextClub) {
    for (int i = 0; i < hand->cardsLeft; i++) {
        Card *card = hand->cards[i];
        if (card->equals(card, nextClub)) {
            return i;
        }
    }

    int index = 0;

    CardSuit order[NUM_SUITS] = {DIAMONDS, HEARTS, SPADES, CLUBS};
    for (int i = 0; i < NUM_SUITS; i++) {
        if (get_lowest(hand, &index, order[i])) {
            break;
        }
    }
    
    return index;
}

/**
 * Chooses the correct card to play from a hand (If not leader)
 * step 1 - if hand can follow suit play the lowest
 * step 2 - if last then play highest from H D C S
 * step 3 - otherwise play highest from C D H S
 *
 * Returns index of the card in the hand
 * */
int get_play_card(Hand *hand, CardSuit suit, bool last) {
    int index = 0;
    
    if (!get_lowest(hand, &index, suit)) {
        if (last) {
            CardSuit order[NUM_SUITS] = {HEARTS, DIAMONDS, CLUBS, SPADES};

            for (int i = 0; i < NUM_SUITS; i++) {
                if (get_highest(hand, &index, order[i])) {
                    break;
                }
            }
        } else {
            CardSuit order[NUM_SUITS] = {CLUBS, DIAMONDS, HEARTS, SPADES};

            for (int i = 0; i < NUM_SUITS; i++) {
                if (get_highest(hand, &index, order[i])) {
                    break;
                }
            }
        }
    }
    
    return index;
}

/**
 * Makes a new hand from and old hand without a given index
 *
 * Returns the new hand
 * */
Hand *hand_without_index(Hand *hand, int index) {
    Hand *newHand = (Hand*) malloc(sizeof(Hand));
    newHand->initialise = hand_initialise;
    newHand->initialise(newHand, hand->cardsLeft - 1);
    newHand->cardsLeft = hand->cardsLeft - 1;

    int newIndex = 0;

    for (int i = 0; i < hand->cardsLeft; i++) {
        if (i != index) {
            Card *card = (Card*) malloc(sizeof(Card));
            card->initialise = card_initialise;
            card->initialise(card);
            card->face = hand->cards[i]->face;
            card->suit = hand->cards[i]->suit;
            newHand->cards[newIndex++] = card;
        }
    }

    /* I realise that this is horrible for memory */
    
    return newHand;
}

/**
 * Searches a hand for its highest card in a given suit
 *
 * Returns a bool indicating whether it was found and its location 
 * within the hand in the index pointer
 * */
bool get_highest(Hand *hand, int *index, CardSuit suit) {
    Card *high;
    bool initial = true;
    bool same;
    bool found = false;

    for (int i = 0; i < hand->cardsLeft; i++) {
        Card *card = hand->cards[i];

        if (card->suit == suit) {
            if (card->compare(card, high, &same) > 0 || initial) {
                high = card;
                *index = i;

                found = true;
                initial = false;
            }
        }
    }
    
    return found;
}

/**
 * Searches a hand for its lowest card in a given suit
 *
 * Returns a bool indicating whether it was found  and its location
 * within the hand in the index pointer
 * */
bool get_lowest(Hand *hand, int *index, CardSuit suit) {
    Card *low;
    bool initial = true;
    bool found = false;
    bool same;

    for (int i = 0; i < hand->cardsLeft; i++) {
        Card *card = hand->cards[i];

        if (card->suit == suit) {
            if (initial || card->compare(card, low, &same) < 0) {
                low = card;
                *index = i;

                found = true;
                initial = false;
            }
        }
    }
    
    return found;
}
