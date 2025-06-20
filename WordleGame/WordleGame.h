#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <cwctype>
#include <clocale>

using namespace std;

class WordleGame {
public:
    WordleGame();
    void run();

private:
    void loadResources();
    vector<wstring> readWords(const string& filename);
    wstring getRandomWord();
    void processEvents();
    void handleInput(sf::Event& event);
	bool isUpper(sf::Uint32 unicode);
	bool isLower(sf::Uint32 unicode);
    void submitGuess();
    string evaluateGuess();
    bool wordExists(vector<wstring>& words, wstring& guess);

    void render();
	void drawScore();
    void drawHeader();
    void drawGrid();
    void drawMessage();
    void drawGameOver();

    // Константы
    const int WINDOW_HEIGHT = 800;
    const int WINDOW_WIDTH = 600;
    const int GRID_COLS = 5;
    const int MAX_GUESSES = 6;
    const int CELL_SIZE = 80;
    const int CELL_MARGIN = 10;
    const int CHARACTER_SIZE = 40;
    const int GRID_WIDTH = GRID_COLS * CELL_SIZE + (GRID_COLS - 1) * CELL_MARGIN;
    const int START_X = (WINDOW_WIDTH - GRID_WIDTH) / 2;
    const int START_Y = 150;

    // Игровое состояние
    sf::RenderWindow window;
    sf::Font font;
    vector<wstring> dictionary;
    wstring secretWord;
    vector<wstring> submittedGuesses;
    vector<string> evaluations;
    wstring currentGuess;
    int score = 0;
    bool gameOver = false;
    bool gameWon = false;

    // UI состояние
    wstring message;
    sf::Clock messageClock;
    bool showMessage = false;
};