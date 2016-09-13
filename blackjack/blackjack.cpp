#include <iostream>
#include <iomanip>
#include <iterator>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <sstream>
#include <cctype>

std::string GetInput() {
	std::string input;
	std::getline(std::cin, input);
	return input;
}
std::string GetInputLower() {
	std::string input;
	std::getline(std::cin, input);
	std::transform(input.begin(), input.end(), input.begin(), std::tolower);
	return input;
}

const int kNumberOfValues = 13;
const int kNumberOfSuits = 4;
const int kNumberOfCardsInDeck = kNumberOfSuits * kNumberOfValues;
const char *kCardValues[kNumberOfValues] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
const char *kCardSuits[kNumberOfSuits] = {"\x05", "\x04", "\x03", "\x06"};

enum class CardValue {
	Ace,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
	Nine,
	Ten,
	Jack,
	Queen,
	King
};

enum class CardSuit {
	Clubs,
	Diamonds,
	Hearts,
	Spades
};

class Card {
public:
	Card(CardValue value, CardSuit suit) : value(value), suit(suit), flipped(true) {}
	CardValue value;
	CardSuit suit;
	bool flipped;
};

std::ostream& operator<<(std::ostream& os, const Card& card) {
	os << "\xda\xc4\xc4\xc4\xc4\xc4\xc4\xbf\n";
	if(card.flipped) {
		const char* value = kCardValues[static_cast<int>(card.value)];
		const char* suit = kCardSuits[static_cast<int>(card.suit)];
		os << std::resetiosflags(std::ios::adjustfield) << std::setiosflags(std::ios::left);
		os << "\xb3" << std::setw(6) << value << "\xb3\n";
		os << std::resetiosflags(std::ios::adjustfield);
		os << "\xb3" << suit << "     \xb3\n";
		os << "\xb3      \xb3\n";
		os << "\xb3      \xb3\n";
		os << "\xb3     " << suit << "\xb3\n";
		os << "\xb3" << std::setw(6) << value << "\xb3\n";
	} else {
		for(int i = 0; i < 6; ++i) {
			os << "\xb3      \xb3\n";
		}
	}
	os << "\xc0\xc4\xc4\xc4\xc4\xc4\xc4\xd9\n";
	return os;
}

using Cards = std::vector<Card>;

int CalculateCards(const Cards& cards) {
	int value = 0, aces = 0;
	for(const auto& card : cards) {
		if(!card.flipped) {
			continue;
		}
		switch(card.value) {
		case CardValue::Ace: ++value; ++aces; break;
		case CardValue::Jack:
		case CardValue::Queen:
		case CardValue::King: value += 10; break;
		default: value += static_cast<int>(card.value) + 1; break;
		}
	}
	if(aces > 0 && value <= 11) {
		value += 10;
	}
	return value;
}

std::ostream& operator<<(std::ostream& os, const Cards& cards) {
	std::vector<char> screen(80 * 8 + 1, ' ');
	screen[80 * 8] = 0;
	int cardIndex = 0;
	for(const auto& card : cards) {
		std::stringstream ss;
		ss << card;
		std::string to;
		for(int i = 0; std::getline(ss, to); ++i) {
			std::copy(to.begin(), to.end(), screen.begin() + i * 80 + cardIndex * 4);
		}
		++cardIndex;
	}
	for(int i = 1; i <= 8; ++i) {
		screen[80 * i - 1] = '\n';
	}
	os << &screen[0];
	os << "Highest possible value: " << CalculateCards(cards) << '\n';
	return os;
}

void CreateDecks(Cards& cards, int numberOfDecks) {
	if(cards.capacity() < numberOfDecks * kNumberOfCardsInDeck * 1u) {
		cards.reserve(numberOfDecks * kNumberOfCardsInDeck);
	}
	for(int decks = 0; decks < numberOfDecks; ++decks) {
		for(int suit = 0; suit < kNumberOfSuits; ++suit) {
			for(int value = 0; value < kNumberOfValues; ++value) {
				cards.push_back(Card(static_cast<CardValue>(value), static_cast<CardSuit>(suit)));
			}
		}
	}
	static auto randomEngine = std::default_random_engine(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
	std::shuffle(cards.begin(), cards.end(), randomEngine);
}

class Player {
	static int counter;
public:
	Player(std::string name) : id(counter++), money(100), name(name) {}
	int id;
	int money;
	int value;
	std::string name;
	Cards hand;

	void Deal(Cards& cards, int numberOfCards) {
		if(cards.size() < numberOfCards * 1u) {
			std::cout << "Not enough cards\n";
			CreateDecks(cards, 3);
		}
		for(auto i = 0; i < numberOfCards; ++i) {
			auto card = cards.back();
			hand.push_back(card);
			cards.pop_back();
		}
		value = CalculateCards(hand);
	}
};
int Player::counter = 0;

using Players = std::vector<Player>;

std::ostream& operator<<(std::ostream& os, const Player& player) {
	os << player.name << " have:\n" << player.hand << '\n';
	return os;
}

int GetBet(const Player& player) {
	int bet;
	for(;;) {
		std::cout << "Account balance: " << player.money << "\n";
		std::cout << "How much do you want to bet? 0 to leave.\n";
		try {
			auto input = GetInput();
			bet = std::stoi(input);
			if(player.money < bet) {
				std::cout << "You don't have enough money.\n\n";
			} else {
				break;
			}
		} catch(std::invalid_argument&) {
			std::cout << "Invalid input\n\n";
		}
	}
	return bet;
}

void PlayerChoice(Player& player, Cards& cards) {
	while(player.value < 21) {
		std::cout << "Hit or Stay? ";
		auto input = GetInputLower();
		if(input == "hit") {
			player.Deal(cards, 1);
			std::cout << player;
		} else if(input == "stay") {
			break;
		} else {
			std::cout << "Invalid value\n";
		}
	}
}

enum class State {
	YouLose,
	YouWin,
	BlackJack,
	Push
};
State CheckPlayer(const Player& player, const Player& dealer) {
	int playerValue = player.value;
	if(playerValue > 21) {
		std::cout << "Busted, you lose\n\n";
		return State::YouLose;
	}

	int dealerValue = dealer.value;
	if(dealerValue == 21 && dealer.hand.size() == 2) {
		std::cout << "Dealer got a blackjack, you lose\n\n";
	} else if(dealerValue > 21) {
		std::cout << "Dealer got busted, you win\n\n";
		return State::YouWin;
	} else if(dealerValue == playerValue) {
		std::cout << "Push!\n\n";
		return State::Push;
	} else if(dealerValue > playerValue) {
		std::cout << "Dealer wins\n\n";
	} else {
		if(playerValue == 21) {
			std::cout << "Blackjack! You win!\n\n";
			if(player.hand.size() == 2) {
				return State::BlackJack;
			}
		} else {
			std::cout << "You win!\n\n";
		}
		return State::YouWin;
	}
	return State::YouLose;
}

int main() {
	Player player{"Player"}, dealer{"Dealer"};

	Cards cards;
	CreateDecks(cards, 4);
	std::cout << "Blackjack!, do you play it?!\n";
	for(;;) {
		if(player.money == 0) {
			std::cout << "You have no money left\n";
			break;
		}
		int bet = GetBet(player);
		if(bet == 0) {
			std::cout << "Thank you come again!\n";
			break;
		}
		player.hand.clear();
		dealer.hand.clear();
		player.Deal(cards, 2);
		dealer.Deal(cards, 2);

		dealer.hand[1].flipped = false;
		std::cout << player;
		std::cout << dealer;
		PlayerChoice(player, cards);
		dealer.hand[1].flipped = true;

		std::cout << dealer;

		while(dealer.value < 17) {
			dealer.Deal(cards, 1);
			std::cout << dealer;
		}

		switch(CheckPlayer(player, dealer)) {
		case State::YouLose: player.money -= bet; break;
		case State::YouWin: player.money += bet; break;
		case State::BlackJack: player.money += static_cast<int>(std::floor(bet * 1.5f));
		case State::Push: break;
		}
	}
}