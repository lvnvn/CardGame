#include "deck.h"
#include <random>
#include <set>
#include <iostream>
#include <time.h>

Deck::Deck()
{
    std::set<int> used_cards;
    card next;

    for(int i = 0; i < 52; i++)
    {
        int tmp = rand()%52;
        while(used_cards.count(tmp) != 0)
            tmp =  rand()%52;
        used_cards.insert(tmp);
        int suit = tmp / 13;
        if(suit == 0)
            next.suit = 'H';
        else if(suit == 1)
            next.suit = 'S';
        else if(suit == 2)
            next.suit = 'C';
        else if(suit == 3)
            next.suit = 'D';
        next.rang = tmp % 13;
        //std::cout << "\n" << next.suit << " " << next.rang << "\n";
        mixed_deck[i].suit = next.suit;
        mixed_deck[i].rang = next.rang;
    }
}

std::string Deck::takeSeven()
{
    std::string result = "";
    int first = 0;
    while(mixed_deck[first].rang == -1)
        first++;

    for(int i = first; i < first+7; i++)
    {
        result = result + std::to_string(mixed_deck[i].rang) + std::string(1,mixed_deck[i].suit) + ' ';
        mixed_deck[i].rang = -1;
    }
    return result;
}

std::string Deck::takeCard()
{
    int first = 0;
    while(mixed_deck[first].rang == -1 && first < 52)
        first++;
    if(first == 52) // колода закончилась
        return "none";
    std::string result =  std::to_string(mixed_deck[first].rang) + std::string(1,mixed_deck[first].suit);
    mixed_deck[first].rang = -1;
    return result;
}
