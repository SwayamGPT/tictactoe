#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <cmath>
#include <iostream>
#include <vector>

using namespace sf;
using namespace std;

// ================= CONSTANTS & SETTINGS =================
const int WINDOW_W = 400;
const int WINDOW_H = 580; // Taller to fit new buttons
const int BOARD_OFFSET_X = 50;
const int BOARD_OFFSET_Y = 210;
const int CELL_SIZE = 100;

// CHANGE THIS TO YOUR VPS/SERVER IP FOR GLOBAL INTERNET MULTIPLAYER
const string SERVER_IP = "127.0.0.1";
const unsigned short PORT = 53000;

// ================= UI & LOGIC CLASSES =================
enum Difficulty { EASY = 30, MEDIUM = 70, HARD = 100 };

class Button {
public:
  RectangleShape shape;
  Text text;

  Button() {}
  Button(Vector2f size, Vector2f pos, string str, Font &font) {
    shape.setSize(size);
    shape.setPosition(pos);
    shape.setFillColor(Color(45, 45, 65));
    shape.setOutlineThickness(2);
    shape.setOutlineColor(Color(80, 80, 120));

    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(20);

    FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f,
                   textRect.top + textRect.height / 2.0f);
    text.setPosition(pos.x + size.x / 2.0f, pos.y + size.y / 2.0f);
  }

  void update(Vector2i mouse) {
    if (shape.getGlobalBounds().contains((Vector2f)mouse)) {
      shape.setFillColor(Color(70, 70, 100));
      shape.setOutlineColor(Color(0, 200, 255));
      shape.setScale(1.02f, 1.02f);
    } else {
      shape.setFillColor(Color(45, 45, 65));
      shape.setOutlineColor(Color(80, 80, 120));
      shape.setScale(1.f, 1.f);
    }
  }

  void setText(string str) {
    text.setString(str);
    FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f,
                   textRect.top + textRect.height / 2.0f);
  }

  bool isClicked(Vector2i mouse) {
    return shape.getGlobalBounds().contains((Vector2f)mouse);
  }

  void draw(RenderWindow &w) {
    w.draw(shape);
    w.draw(text);
  }
};

class Board {
public:
  char grid[3][3];
  vector<Vector2i> winCells;

  Board() { reset(); }

  void reset() {
    winCells.clear();
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        grid[i][j] = ' ';
  }

  bool move(int r, int c, char p) {
    if (grid[r][c] == ' ') {
      grid[r][c] = p;
      return true;
    }
    return false;
  }

  bool movesLeft() {
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (grid[i][j] == ' ')
          return true;
    return false;
  }

  char checkWin() {
    winCells.clear();
    for (int i = 0; i < 3; i++) {
      if (grid[i][0] != ' ' && grid[i][0] == grid[i][1] &&
          grid[i][1] == grid[i][2]) {
        winCells = {{i, 0}, {i, 1}, {i, 2}};
        return grid[i][0];
      }
    }
    for (int i = 0; i < 3; i++) {
      if (grid[0][i] != ' ' && grid[0][i] == grid[1][i] &&
          grid[1][i] == grid[2][i]) {
        winCells = {{0, i}, {1, i}, {2, i}};
        return grid[0][i];
      }
    }
    if (grid[0][0] != ' ' && grid[0][0] == grid[1][1] &&
        grid[1][1] == grid[2][2]) {
      winCells = {{0, 0}, {1, 1}, {2, 2}};
      return grid[0][0];
    }
    if (grid[0][2] != ' ' && grid[0][2] == grid[1][1] &&
        grid[1][1] == grid[2][0]) {
      winCells = {{0, 2}, {1, 1}, {2, 0}};
      return grid[0][2];
    }
    return ' ';
  }
};

class AI {
private:
  int minimax(Board &b, bool isMax) {
    char w = b.checkWin();
    if (w == 'O')
      return 10;
    if (w == 'X')
      return -10;
    if (!b.movesLeft())
      return 0;

    int best = isMax ? -1000 : 1000;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (b.grid[i][j] == ' ') {
          b.grid[i][j] = isMax ? 'O' : 'X';
          if (isMax)
            best = max(best, minimax(b, false));
          else
            best = min(best, minimax(b, true));
          b.grid[i][j] = ' ';
        }
      }
    }
    return best;
  }

  void randomMove(Board &b) {
    vector<Vector2i> free;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (b.grid[i][j] == ' ')
          free.push_back({i, j});

    if (!free.empty()) {
      auto m = free[rand() % free.size()];
      b.grid[m.x][m.y] = 'O';
    }
  }

  void bestMove(Board &b) {
    int bestVal = -1000, r = -1, c = -1;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (b.grid[i][j] == ' ') {
          b.grid[i][j] = 'O';
          int val = minimax(b, false);
          b.grid[i][j] = ' ';
          if (val > bestVal) {
            bestVal = val;
            r = i;
            c = j;
          }
        }
    if (r != -1 && c != -1)
      b.grid[r][c] = 'O';
  }

public:
  void playMove(Board &b, Difficulty diff) {
    int chance = rand() % 100;
    if (chance < diff)
      bestMove(b);
    else
      randomMove(b);
  }
};

// ================= MAIN GAME LOOP =================
class Game {
public:
  enum State { MENU, PLAYING, GAME_OVER, WAITING_CONNECTION };
  enum Mode { LOCAL, AI_MODE, NET_HOST, NET_CLIENT };

  State state = MENU;
  Mode mode = AI_MODE;
  Difficulty aiDiff = HARD;

  Board board;
  AI ai;

  bool playerTurn = true; // True = X's turn, False = O's turn
  string result = "";

  TcpSocket socket;
  TcpListener listener;

  Font font;
  Button btnAI, btnDiff, btnLocal, btnHost, btnJoin;

  float animScale[3][3] = {0};
  float animAlpha[3][3] = {0};
  Clock clock;

  Game() {
    // macOS App Bundles look for assets in the Resources folder. We check
    // multiple paths.
    if (!font.loadFromFile("arial.ttf"))
      if (!font.loadFromFile("../Resources/arial.ttf"))
        font.loadFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");

    float startY = 130;
    btnAI = Button({250, 45}, {75, startY}, "Play vs AI", font);
    btnDiff = Button({250, 45}, {75, startY + 60}, "AI: Hard", font);
    btnLocal = Button({250, 45}, {75, startY + 120}, "2 Player Local", font);
    btnHost = Button({250, 45}, {75, startY + 180}, "Host Server", font);
    btnJoin = Button({250, 45}, {75, startY + 240}, "Join Server", font);

    srand(time(0));
  }

  void reset() {
    board.reset();
    playerTurn = true;
    result = "";
    state = PLAYING;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++) {
        animScale[i][j] = 0;
        animAlpha[i][j] = 0;
      }
  }

  void handleClick(Vector2i m) {
    if (state == MENU) {
      if (btnAI.isClicked(m)) {
        mode = AI_MODE;
        reset();
      } else if (btnDiff.isClicked(m)) {
        if (aiDiff == HARD) {
          aiDiff = EASY;
          btnDiff.setText("AI: Easy");
        } else if (aiDiff == EASY) {
          aiDiff = MEDIUM;
          btnDiff.setText("AI: Medium");
        } else {
          aiDiff = HARD;
          btnDiff.setText("AI: Hard");
        }
      } else if (btnLocal.isClicked(m)) {
        mode = LOCAL;
        reset();
      } else if (btnHost.isClicked(m)) {
        mode = NET_HOST;
        state = WAITING_CONNECTION;
        listener.listen(PORT);
        socket.setBlocking(true); // Block just until client connects
        if (listener.accept(socket) == sf::Socket::Done) {
          socket.setBlocking(false); // Non-blocking for gameplay
          reset();
        } else
          state = MENU;
      } else if (btnJoin.isClicked(m)) {
        mode = NET_CLIENT;
        state = WAITING_CONNECTION;
        socket.setBlocking(true);
        if (socket.connect(SERVER_IP, PORT, seconds(5)) == sf::Socket::Done) {
          socket.setBlocking(false);
          reset();
        } else
          state = MENU;
      }
    } else if (state == PLAYING) {
      if (m.x >= BOARD_OFFSET_X && m.x <= BOARD_OFFSET_X + 300 &&
          m.y >= BOARD_OFFSET_Y && m.y <= BOARD_OFFSET_Y + 300) {

        int x = (m.x - BOARD_OFFSET_X) / CELL_SIZE;
        int y = (m.y - BOARD_OFFSET_Y) / CELL_SIZE;

        // Validate if it is actually the local player's turn to click
        bool canPlayLocal = false;
        if (mode == LOCAL || mode == AI_MODE)
          canPlayLocal = true;
        else if (mode == NET_HOST && playerTurn)
          canPlayLocal = true;
        else if (mode == NET_CLIENT && !playerTurn)
          canPlayLocal = true;

        if (canPlayLocal && y < 3 && x < 3 && board.grid[y][x] == ' ') {
          board.move(y, x, playerTurn ? 'X' : 'O');

          if (mode == NET_HOST || mode == NET_CLIENT) {
            sf::Packet packet;
            packet << y << x;
            socket.send(packet);
          }
          playerTurn = !playerTurn;
        }
      }
    } else if (state == GAME_OVER) {
      socket.disconnect(); // Close network cleanly
      state = MENU;
    }
  }

  void update() {
    float dt = clock.restart().asSeconds();

    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        if (board.grid[i][j] != ' ') {
          animScale[i][j] += dt * 5;
          animAlpha[i][j] += dt * 400;
          if (animScale[i][j] > 1.0f)
            animScale[i][j] = 1.0f;
          if (animAlpha[i][j] > 255.0f)
            animAlpha[i][j] = 255.0f;
        }

    if (state != PLAYING)
      return;

    // Check Networking (Receive opponent's move)
    if ((mode == NET_HOST && !playerTurn) ||
        (mode == NET_CLIENT && playerTurn)) {
      sf::Packet packet;
      if (socket.receive(packet) == sf::Socket::Done) {
        int y, x;
        if (packet >> y >> x) {
          board.move(y, x, playerTurn ? 'X' : 'O');
          playerTurn = !playerTurn;
        }
      } else if (socket.receive(packet) == sf::Socket::Disconnected) {
        result = "Opponent Disconnected.";
        state = GAME_OVER;
        return;
      }
    }

    // Check Win Condition
    char w = board.checkWin();
    if (w != ' ') {
      result = (w == 'X' ? "Player 1 (X) Wins!" : "Player 2 (O) Wins!");
      state = GAME_OVER;
      return;
    }
    if (!board.movesLeft()) {
      result = "It's a Draw!";
      state = GAME_OVER;
      return;
    }

    // Trigger AI Turn
    if (mode == AI_MODE && !playerTurn) {
      ai.playMove(board, aiDiff);
      playerTurn = true;
    }
  }

  void draw(RenderWindow &w) {
    VertexArray bg(Quads, 4);
    bg[0].position = Vector2f(0, 0);
    bg[0].color = Color(20, 20, 35);
    bg[1].position = Vector2f(WINDOW_W, 0);
    bg[1].color = Color(35, 25, 45);
    bg[2].position = Vector2f(WINDOW_W, WINDOW_H);
    bg[2].color = Color(10, 10, 20);
    bg[3].position = Vector2f(0, WINDOW_H);
    bg[3].color = Color(15, 10, 25);
    w.draw(bg);

    Text title("TIC TAC TOE", font, 36);
    title.setStyle(Text::Bold);
    title.setFillColor(Color::White);
    FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.0f, tb.top + tb.height / 2.0f);
    title.setPosition(WINDOW_W / 2.0f, 60);
    w.draw(title);

    if (state == MENU) {
      Vector2i mouse = Mouse::getPosition(w);
      btnAI.update(mouse);
      btnDiff.update(mouse);
      btnLocal.update(mouse);
      btnHost.update(mouse);
      btnJoin.update(mouse);

      btnAI.draw(w);
      btnDiff.draw(w);
      btnLocal.draw(w);
      btnHost.draw(w);
      btnJoin.draw(w);
      return;
    }

    if (state == WAITING_CONNECTION) {
      Text wait("Waiting for connection...", font, 20);
      wait.setPosition(80, 250);
      wait.setFillColor(Color(0, 200, 255));
      w.draw(wait);
      return;
    }

    // Draw Board
    RectangleShape line({300, 4});
    line.setFillColor(Color(100, 100, 140, 150));
    for (int i = 1; i < 3; i++) {
      line.setPosition(BOARD_OFFSET_X, BOARD_OFFSET_Y + i * CELL_SIZE - 2);
      w.draw(line);
      line.setSize({4, 300});
      line.setPosition(BOARD_OFFSET_X + i * CELL_SIZE - 2, BOARD_OFFSET_Y);
      w.draw(line);
      line.setSize({300, 4});
    }

    // Draw Pieces
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (board.grid[i][j] != ' ') {
          float cx = BOARD_OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2.0f;
          float cy = BOARD_OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2.0f;

          if (board.grid[i][j] == 'X') {
            RectangleShape xLine({70, 8});
            xLine.setOrigin(35, 4);
            xLine.setPosition(cx, cy);
            xLine.setScale(animScale[i][j], animScale[i][j]);
            xLine.setFillColor(Color(0, 200, 255, (int)animAlpha[i][j]));
            xLine.setRotation(45);
            w.draw(xLine);
            xLine.setRotation(-45);
            w.draw(xLine);
          } else {
            CircleShape oShape(25);
            oShape.setOrigin(25, 25);
            oShape.setPosition(cx, cy);
            oShape.setScale(animScale[i][j], animScale[i][j]);
            oShape.setFillColor(Color::Transparent);
            oShape.setOutlineThickness(8);
            oShape.setOutlineColor(Color(255, 80, 80, (int)animAlpha[i][j]));
            w.draw(oShape);
          }
        }
      }
    }

    // Draw Win Line
    if (board.winCells.size() == 3) {
      Vector2f s(BOARD_OFFSET_X + board.winCells[0].y * CELL_SIZE + 50,
                 BOARD_OFFSET_Y + board.winCells[0].x * CELL_SIZE + 50);
      Vector2f e(BOARD_OFFSET_X + board.winCells[2].y * CELL_SIZE + 50,
                 BOARD_OFFSET_Y + board.winCells[2].x * CELL_SIZE + 50);
      RectangleShape winLine({hypot(e.x - s.x, e.y - s.y) + 40, 8});
      winLine.setOrigin(20, 4);
      winLine.setPosition(s);
      winLine.setRotation(atan2(e.y - s.y, e.x - s.x) * 180 / 3.14159f);
      winLine.setFillColor(Color(0, 255, 150, 220));
      w.draw(winLine);
    }

    if (state == GAME_OVER) {
      RectangleShape overlay({WINDOW_W, WINDOW_H});
      overlay.setFillColor(Color(0, 0, 0, 180));
      w.draw(overlay);

      Text res(result, font, 28);
      res.setStyle(Text::Bold);
      res.setFillColor(Color::White);
      FloatRect rb = res.getLocalBounds();
      res.setOrigin(rb.left + rb.width / 2.0f, rb.top + rb.height / 2.0f);
      res.setPosition(WINDOW_W / 2.0f, WINDOW_H / 2.0f - 20);
      w.draw(res);

      Text prompt("Click anywhere to continue", font, 16);
      prompt.setFillColor(Color(200, 200, 200));
      FloatRect pb = prompt.getLocalBounds();
      prompt.setOrigin(pb.left + pb.width / 2.0f, pb.top + pb.height / 2.0f);
      prompt.setPosition(WINDOW_W / 2.0f, WINDOW_H / 2.0f + 40);
      w.draw(prompt);
    }
  }
};

int main() {
  RenderWindow window(VideoMode(WINDOW_W, WINDOW_H), "Tic Tac Toe Multiplayer");
  window.setFramerateLimit(60);
  Game game;

  while (window.isOpen()) {
    Event e;
    while (window.pollEvent(e)) {
      if (e.type == Event::Closed)
        window.close();
      if (e.type == Event::MouseButtonPressed &&
          e.mouseButton.button == Mouse::Left)
        game.handleClick(Mouse::getPosition(window));
    }
    game.update();
    window.clear();
    game.draw(window);
    window.display();
  }
  return 0;
}
