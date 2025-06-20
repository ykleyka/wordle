#include "WordleGame.h"

WordleGame::WordleGame() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"Wordle") {
    window.setFramerateLimit(60);
    setlocale(LC_ALL, "be_BY.UTF-8");
    srand(time(NULL));
    loadResources();
}

void WordleGame::run() {
    secretWord = getRandomWord();
    while (window.isOpen()) {
        processEvents();
        render();
    }
}

void WordleGame::loadResources() {
    const string filename = "5_letters_words.txt";
    const string fontname = "Montserrat-Bold.ttf";
    dictionary = readWords(filename);
    if (dictionary.empty()) throw runtime_error("Failed to load dictionary");

    sf::Image icon;
    icon.loadFromFile("icon.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    if (!font.loadFromFile(fontname)) throw runtime_error("Failed to load font");
}

vector<wstring> WordleGame::readWords(const string& filename) {
    vector<wstring> words;
    wifstream infile(filename);
    // Устанавливаем локаль для корректного чтения UTF-8
    infile.imbue(locale(locale(), new codecvt_utf8<wchar_t>));
    wstring line;
    while (getline(infile, line)) {
        words.push_back(line);
    }
    infile.close();
    return words;
}

wstring WordleGame::getRandomWord() {
    int index = rand() % dictionary.size();
    return dictionary[index];
}

void WordleGame::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }
        // Обработка ввода символов через событие TextEntered
        if (!gameOver) {
            handleInput(event);
		}
		else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            gameOver = false;
            gameWon = false;
            submittedGuesses.clear();
            evaluations.clear();
            currentGuess.clear();
            secretWord = getRandomWord();
		}
    }
}

bool WordleGame::isLower(sf::Uint32 unicode) {
	// а-ч (1072-1097), ы-і (1099-1110), ў (1118)
    return (unicode >= 1072 && unicode <= 1096) || (unicode >= 1099 && unicode <= 1110) || unicode == 1118;
}

bool WordleGame::isUpper(sf::Uint32 unicode) {
	// А-Ч (1040-1065), Ы-І (1067-1071), Ў (1038) 
    return (unicode >= 1040 && unicode <= 1064) || (unicode >= 1067 && unicode <= 1071)  || unicode == 1038;
}

void WordleGame::handleInput(sf::Event& event) {
    if (event.type == sf::Event::TextEntered && currentGuess.size() < GRID_COLS) {
        if (isUpper(event.text.unicode)) {
            currentGuess.push_back(towlower(event.text.unicode));
        }
        // 39 - апостроф
        else if (isLower(event.text.unicode) || event.text.unicode == 39) {
            currentGuess.push_back(event.text.unicode);
        }
    }
    else if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Enter) {
            submitGuess();
        }
        else if (event.key.code == sf::Keyboard::BackSpace) {
            if (!currentGuess.empty()) currentGuess.pop_back();
        }
    }
}

void WordleGame::submitGuess() {
    // Если введено ровно 5 символов – проверяем слово
    if (currentGuess.size() == GRID_COLS) {
        if (!wordExists(dictionary, currentGuess)) {
            message = L"Слова няма ў слоўніку!";
            messageClock.restart();
        }
        else {
            string eval = evaluateGuess();
            submittedGuesses.push_back(currentGuess);
            evaluations.push_back(eval);
            if (currentGuess == secretWord) {
                gameWon = true;
                gameOver = true;
                score++;
            }
            else if (submittedGuesses.size() >= MAX_GUESSES) {
                gameOver = true;
            }
            currentGuess = L"";
        }
    }
    else {
        message = L"Увядзіце 5 літар!";
        messageClock.restart();
    }
}

bool WordleGame::wordExists(vector<wstring>& words, wstring& guess) {
    return find(words.begin(), words.end(), guess) != words.end();
}


// Функция оценки угадывания:
// 'X' – буква угадана правильно и в нужной позиции,
// 'Y' – буква присутствует, но в другой позиции,
// '_' – буквы нет в слове.
string WordleGame::evaluateGuess() {
    std::string result(GRID_COLS, '_');          
    std::vector<bool> used(GRID_COLS, false);   

    for (int i = 0; i < GRID_COLS; ++i) {
        if (currentGuess[i] == secretWord[i]) {
            result[i] = 'X';
            used[i] = true;
        }
    }

    for (int i = 0; i < GRID_COLS; ++i) {
        if (result[i] == '_') { 
            for (int j = 0; j < GRID_COLS; ++j) {
                if (!used[j] && currentGuess[i] == secretWord[j]) {
                    result[i] = 'Y';
                    used[j] = true; 
                    break; 
                }
            }
        }
    }
    return result;
}

void WordleGame::render() {
    window.clear(sf::Color(30, 30, 30));
    drawScore();
    drawHeader();
    drawGrid();
    drawMessage();
	if (gameOver) {
		drawGameOver();
	}
    window.display();
}

void WordleGame::drawScore() {
	sf::Text scoreText(sf::String(L"Рахунак: " + to_wstring(score)), font, 25);
	scoreText.setFillColor(sf::Color::White);
	scoreText.setPosition(10, 10);
	window.draw(scoreText);
}

void WordleGame::drawHeader() {
    sf::Text title(sf::String(L"Wordle"), font, 40);
    title.setFillColor(sf::Color::White);
    title.setPosition(window.getSize().x / 2 - title.getGlobalBounds().width / 2, 50);
    window.draw(title);

    sf::Text instruction(sf::String(L"Угадайце 5-літарнае слова"), font, 20);
    instruction.setFillColor(sf::Color::White);
    instruction.setPosition(window.getSize().x / 2 - instruction.getGlobalBounds().width / 2, 100);
    window.draw(instruction);
}

void WordleGame::drawGrid() {
    const sf::Color gray(100, 100, 100);
    const sf::Color thisRow(180, 180, 180);
    const sf::Color thisRowEmpty(50, 50, 50);
    // Отрисовка сетки для попыток
    for (int row = 0; row < MAX_GUESSES; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            int x = START_X + col * (CELL_SIZE + CELL_MARGIN);
            int y = START_Y + row * (CELL_SIZE + CELL_MARGIN);

            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(x, y);
            cell.setOutlineThickness(2);
            cell.setOutlineColor(sf::Color::White);

            // Если строка уже введена, назначаем цвет ячейки в зависимости от оценки
            if (row < submittedGuesses.size()) {
                char eval = evaluations[row][col];
                if (eval == 'X') {
                    cell.setFillColor(sf::Color::Green);  // зелёный
                }
                else if (eval == 'Y') {
                    cell.setFillColor(sf::Color::Yellow);  // жёлтый
                }
                else {
                    cell.setFillColor(gray); // серый
                }
            }
            else if (row == submittedGuesses.size() && col < currentGuess.size()) {
                cell.setFillColor(thisRow); // текущая вводимая строка
            }
            else {
                cell.setFillColor(thisRowEmpty);  // пустая клетка
            }

            window.draw(cell);

            // Отрисовка буквы в ячейке, если она есть
            wstring letter = L"";
            if (row < submittedGuesses.size()) {
                letter = wstring(1, submittedGuesses[row][col]);
            }
            else if (row == submittedGuesses.size() && col < currentGuess.size()) {
                letter = wstring(1, currentGuess[col]);
            }
            if (!letter.empty()) {
                letter[0] = towupper(letter[0]);

                sf::Text letterText(sf::String(letter), font, CHARACTER_SIZE);
                letterText.setFillColor(sf::Color::White);

                // Выравниваем букву, устанавливая origin по центру тела текста
                sf::FloatRect textBounds = letterText.getLocalBounds();
                letterText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                    textBounds.top + textBounds.height / 2.0f);

                // Устанавливаем позицию по центру ячейки
                letterText.setPosition(x + CELL_SIZE / 2.0f, y + CELL_SIZE / 2.0f);
                window.draw(letterText);
            }
        }
    }
}

void WordleGame::drawMessage() {
    if (messageClock.getElapsedTime().asSeconds() < 2.f) {
        sf::Text msgText(sf::String(message), font, 24);
        msgText.setFillColor(sf::Color::Red);
        msgText.setPosition(window.getSize().x / 2 - msgText.getGlobalBounds().width / 2,
            START_Y + MAX_GUESSES * (CELL_SIZE + CELL_MARGIN) + 20);
        window.draw(msgText);
    }
}

void WordleGame::drawGameOver() {
    sf::Text resultText("", font, 25);
    if (gameWon) {
        resultText.setString(sf::String(L"Вы выйгралі!"));
        resultText.setFillColor(sf::Color::Green);
    }
    else {
        resultText.setString(sf::String(L"Вы прайгралі! Сакрэтнае слова: " + secretWord));
        resultText.setFillColor(sf::Color::Red);
    }
    resultText.setPosition(window.getSize().x / 2 - resultText.getGlobalBounds().width / 2,
        START_Y + MAX_GUESSES * (CELL_SIZE + CELL_MARGIN) + 20);
    window.draw(resultText);

    sf::Text continueText(L"Націсніце Enter, каб пачаць новую гульню", font, 20);
	continueText.setFillColor(sf::Color::White);
	continueText.setPosition(window.getSize().x / 2 - continueText.getGlobalBounds().width / 2,
		START_Y + MAX_GUESSES * (CELL_SIZE + CELL_MARGIN) + 60);
	window.draw(continueText);
}