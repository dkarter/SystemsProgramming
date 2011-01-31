#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "poker.h"

/* converts a hand (of 5 cards) to a string representation, and stores it in the
 * provided buffer. The buffer is assumed to be large enough.
 */
void hand_to_string (hand_t hand, char *handstr) {
    char *p = handstr;
    int i;
    char *val, *suit;
    for (i=0; i<5; i++) {
        if (hand[i].value < 10) {
            *p++ = hand[i].value + '0';
        } else {
            switch(hand[i].value) {
            case 10: *p++ = 'T'; break;
            case 11: *p++ = 'J'; break;
            case 12: *p++ = 'Q'; break;
            case 13: *p++ = 'K'; break;
            case 14: *p++ = 'A'; break;
            }
        }
        switch(hand[i].suit) {
        case DIAMOND: *p++ = 'D'; break;
        case CLUB: *p++ = 'C'; break;
        case HEART: *p++ = 'H'; break;
        case SPADE: *p++ = 'S'; break;
        }
        if (i<=3) *p++ = ' ';
    }
    *p = '\0';
}

/* converts a string representation of a hand into 5 separate card structs. The
 * given array of cards is populated with the card values.
 */
void string_to_hand (const char *handstr, hand_t hand) {
    const char *p = handstr;
    int i;
    int handValue;
    for (i=0; i<5; i++) {

	//handle Char representation
	//of valued bigger than 10
	if (*p == 'T') {
	    hand[i].value = 10;
	} else if (*p == 'J') {
	    hand[i].value = 11;
	} else if (*p == 'Q') {
	    hand[i].value = 12;
	} else if (*p == 'K') {
	    hand[i].value = 13;
 	} else if (*p == 'A') {
	    hand[i].value = 14;
	} else {
            handValue = atoi(p);
            hand[i].value = handValue;
	}
	

	//handle suit
	switch (*++p) {
	    case 'D':
		hand[i].suit = DIAMOND;
		break;
	    case 'C':
		hand[i].suit = CLUB;
		break;
	    case 'H':
		hand[i].suit = HEART;
		break;
	    case 'S':
		hand[i].suit = SPADE;
		break;
	}

        //advance string pointer to next
        //card representation
	p+=2;
     }
		

}

/* sorts the hands so that the cards are in ascending order of value (two
 * lowest, ace highest */
void sort_hand (hand_t hand) {
  //simple selection sort 
  int iPos;
  int iMin;
  int i;
  
  for (iPos = 0; iPos < 5; iPos++) {
    //assume iMin is the first element
    iMin = iPos;
    
    for (i = iPos+1; i < 5; i++) {
      if (hand[i].value < hand[iMin].value) {
	//found new minimum - remember it's index
	iMin = i;
      }
    }
    
    //iMin is the index of the minimum element, swap it with curr pos
    swap_hands(hand, iPos, iMin);
  }
}

void swap_hands (hand_t *hand, int firstIndex, int secondIndex) {
  hand_t tmp = hand[firstIndex];
  hand[firstIndex] = hand[secondIndex];
  hand[secondIndex] = tmp;
}


//return occurrences of a specific
//card value in a hand
int count_occur (int value, hand_t hand) {
  int occurrences = 0;
  int i;
  for (i=0; i<5; i++)
    if (value == hand[i].value)
      occurrences++;

  return occurrences;
}

int is_onepair (hand_t hand) {
  int i;
  for (i=0; i<5; i++)
    if (count_occur(hand[i].value, hand) >= 2)
      //we found a pair
      return 1;
  return 0;
}

int is_twopairs (hand_t hand) {
  int pair_value = 0;
  int i;

  for (i=0; i<5; i++) {
    //if it's more than a pair then ignore it
    //(it's not two unique pairs)
    if (count_occur(hand[i].value,hand) != 2)
      continue;
    
    //check if we didn't record it yet
    if (pair_value == 0)
      //record the first pair
      pair_value = hand[i].value;
    else
      //this is our second pair - bingo!
      if (pair_value != hand[i].value)
	return 1;
  }

  return 0;
}

int is_threeofakind (hand_t hand) {
  int i;
  for (i=0; i<5; i++)
    if (count_occur(hand[i].value, hand) >= 3)
      // we found at least three occurrences
      //of a same valued card
      return 1;

  return 0;
}

int is_straight (hand_t hand) {
  //we have to sort the cards for this one
  sort_hand(hand);
  int i;
  for (i = 0; i<=3; i++)
    //make sure values are consequtive
    if (hand[i].value != (hand[i+1].value - 1))
      return 0;

  //all values were consequtive
  return 1;
}

int is_fullhouse (hand_t hand) {
  int i;
  int has_pair = 0;
  int has_three = 0;
  int occurrences = 0;

  for (i=0; i<5; i++) {
    occurrences = count_occur(hand[i].value, hand);
    if (occurrences == 2)
      has_pair = 1;
    else if (occurrences == 3)
      has_three = 1;
  }
  
  //predicate returns true if has a pair and three of a kind
  return (has_pair && has_three);
}

int is_flush (hand_t hand) {
  //first we figure out the suit to check against  
  suit_t expected_suit = hand[0].suit;

  //for loop
  int i;
  
  for (i=0; i<5; i++)
    //check all cards have same suit
    if (hand[i].suit != expected_suit)
      return 0;
  
  // all cards followed the rules
  return 1;

}

int is_straightflush (hand_t hand) {
  //we have to sort the cards for this one
  sort_hand(hand);

  int i;
  for (i = 0; i<=3; i++)
    //make sure values are consequtive
    //and all cards are of same suit
    if (hand[i].value != (hand[i+1].value - 1) ||
	hand[i].suit != hand[i+1].suit)
      return 0;

  //all values were consequtive
  return 1;
}

int is_fourofakind (hand_t hand) {
  int i;
  for (i=0; i<5; i++)
    if (count_occur(hand[i].value, hand) == 4)
      // we found at least four occurrences
      //of a same valued card
      return 1;

  return 0;
}

int is_royalflush (hand_t hand) {
  //first we figure out the suit to check against  
  suit_t expected_suit = hand[0].suit;
  
  // a royal flush always starts with a 10
  int expected_value=10;
  
  //for loop
  int i;

  // then we sort the hand to make
  // it easier to check for consequtivity
  sort_hand(hand);
  
  for (i=0; i<5; i++) {
    //check that each card obeys the rules of a royal flush
    if (hand[i].value != expected_value ||
	hand[i].suit != expected_suit)
      return 0;
    
    //increase expected value with ea iteration
    expected_value++;
  }
  
  // all cards followed the rules
  return 1;
}

/* compares the hands based on rank -- if the ranks (and rank values) are
 * identical, compares the hands based on their highcards.
 * returns 0 if h1 > h2, 1 if h2 > h1.
 */
int compare_hands (hand_t h1, hand_t h2) {
  int hand1_rank = get_rank(h1);
  int hand2_rank = get_rank(h2);
  
  if (hand2_rank== hand1_rank)
    return compare_highcards(h1, h2);

    return (hand2_rank > hand1_rank);
}

int get_rank (hand_t hand, hand_t against) {
  if (is_royalflush(hand))
    return 10;
  if (is_straightflush(hand))
    return 9;
  if (is_fourofakind(hand))
    return 8;
  if (is_fullhouse(hand))
    return 7;
  if (is_flush(hand))
    return 6;
  if (is_straight(hand))
    return 5;
  if (is_threeofakind(hand))
    return 4;
  if (is_twopairs(hand))
    return 3;
  if (is_onepair(hand))
    return 2;
}
/* compares the hands based solely on their highcard values (ignoring rank). if
 * the highcards are a draw, compare the next set of highcards, and so forth.
 */
int compare_highcards (hand_t h1, hand_t h2) {
  // sort both hands first
  sort_hand(h1);
  sort_hand(h2);

  //iterate and compare
  int i;
  for (i = 4; i>=0; i--) {
    if (h1[i].value > h2[i].value) return 0;
    if (h1[i].value < h2[i].value) return 1;
  }
  

}
