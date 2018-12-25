#ifndef DECK_H
#define DECK_H
#include <string>

struct card {
    char suit; // H,S,C,D hearts (червы), spades (пики), clubs (трефы), diamonds (бубны)
    int rang; // 1..13
};

class Deck
{
public:
    Deck();
    std::string takeCard(); // взять верхнюю карту
    std::string takeSeven(); //взять 7 верхних карт
private:
    card mixed_deck[52];
};

#endif // DECK_H
