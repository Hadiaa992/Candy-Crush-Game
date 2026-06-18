#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <cstdlib>         // for rand and srand functions
#include <ctime>          //to generate diff random candies every time
#include <cmath>         //to use abs function
#include <array>        //used in sfml part to import array
#include <string>      //to handle text in code
#include <vector>      //used for sprite cache

using namespace std;

const int rows = 8;      // 8x8 grid
const int columns = 8;
const int EMPTY = -1;

// Numbrs mapped with sfml graphics to generate required candy properly
// 0-4 : base candies (5)
// 5-9 : horizontal striped
// 10-14: vertical striped 
// 15-19: wrapped 
// 20: color bomb

const int MAX_BASE_CANDY_TYPE = 4;  // candies 0..4
const int NUM_BASE_CANDIES = 5;

const int STRIPE_H_OFFSET = 5;     //these constants will be added in base colour candies(numbers) to create strip or wrapped candy of that specific base candy
const int STRIPE_V_OFFSET = 10;
const int WRAPPED_OFFSET = 15;
const int COLOR_BOMB = 20;

int totalTime = 120;
int timeLeft = 0;


void responsivePause(sf::RenderWindow& window, int ms)           //function to slow/pause the game but window events keep running
{
    const int step = 20;          //small steps
    int waited = 0;
    while (waited < ms)          //total delay 
    {
        while (auto ev = window.pollEvent())
        {
            if (ev->is<sf::Event::Closed>()) window.close();
        }
        sf::sleep(sf::milliseconds(step));
        waited += step;
        if (!window.isOpen()) break;
    }
}

int generatecandy()                                  //random generation of candies and this function returns index for each cell
{
    int index;
    index = rand() % NUM_BASE_CANDIES;
    return index;
}

void fillBoard(int board[rows][columns])            //fill the board with random candies
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < columns; j++)
        {
            board[i][j] = generatecandy();        //at each cell generateCandy function is called and the index whixh is returned by function is stored in each cell
        }
    }
}

bool areAdjacent(int r1, int c1, int r2, int c2)
{
    if ((r1 == r2 && abs(c1 - c2) == 1) || (c1 == c2 && abs(r1 - r2) == 1))
        return true;
    return false;
}

void swapCandies(int board[rows][columns], int r1, int c1, int r2, int c2)
{
    int temp = board[r1][c1];
    board[r1][c1] = board[r2][c2];
    board[r2][c2] = temp;
}

//functions to check the type of candy i.e base,wrapped,stripeH,stripeV or colorbomb
inline bool isBaseCandy(int v) { return v >= 0 && v <= MAX_BASE_CANDY_TYPE; }
inline bool isStripeH(int v) { return v >= STRIPE_H_OFFSET && v < STRIPE_H_OFFSET + NUM_BASE_CANDIES; }
inline bool isStripeV(int v) { return v >= STRIPE_V_OFFSET && v < STRIPE_V_OFFSET + NUM_BASE_CANDIES; }
inline bool isWrapped(int v) { return v >= WRAPPED_OFFSET && v < WRAPPED_OFFSET + NUM_BASE_CANDIES; }
inline bool isColorBomb(int v) { return v == COLOR_BOMB; }

//function to tell the specific candy by returning index which is later used to detect colour for sfml
inline int getBaseColor(int v)
{
    if (isBaseCandy(v)) return v;
    if (isStripeH(v)) return v - STRIPE_H_OFFSET;
    if (isStripeV(v)) return v - STRIPE_V_OFFSET;
    if (isWrapped(v)) return v - WRAPPED_OFFSET;
    return -1;
}

bool isTShapeMatch(int board[rows][columns], int r, int c)
{
    int centerBase = getBaseColor(board[r][c]);   // made the centre candy reference by using base candies indices
    if (centerBase < 0)
    {
        return false;
    }

    bool h_match_3_center = (getBaseColor(board[r][c - 1]) == centerBase) && (getBaseColor(board[r][c + 1]) == centerBase);
    bool v_match_3_center = (getBaseColor(board[r - 1][c]) == centerBase) && (getBaseColor(board[r + 1][c]) == centerBase);

    // '+' shape
    if (h_match_3_center && v_match_3_center) return true;

    // Different T-Shapes
    if (v_match_3_center && getBaseColor(board[r][c - 1]) == centerBase && getBaseColor(board[r][c + 1]) == centerBase) /////////????????
    {
        //Up-facing T 
        if (r > 0 && c > 0 && c < columns - 1 && getBaseColor(board[r + 1][c - 1]) == centerBase && getBaseColor(board[r + 1][c + 1]) == centerBase) return true;
        //Down-facing T
        if (r < rows - 1 && c>0 && c < columns - 1 && getBaseColor(board[r - 1][c - 1]) == centerBase && getBaseColor(board[r - 1][c + 1]) == centerBase) return true;
    }

    if (h_match_3_center && getBaseColor(board[r - 1][c]) == centerBase && getBaseColor(board[r + 1][c]) == centerBase)
    {
        //left-facing T
        if (c > 0 && r > 0 && r < rows - 1 && getBaseColor(board[r - 1][c + 1]) == centerBase && getBaseColor(board[r + 1][c + 1]) == centerBase) return true;
        //right-facing T
        if (c < columns - 1 && r>0 && r < rows - 1 && getBaseColor(board[r - 1][c - 1]) == centerBase && getBaseColor(board[r + 1][c - 1]) == centerBase) return true;
    }

    return false;
}

bool checkMatch(int board[rows][columns])
{
    // horizontal check for 3 same candies in complete board
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c <= columns - 3; c++)
        {
            int baseA = getBaseColor(board[r][c]);  //get base colour/index so that special candies are aslo detected
            if (baseA >= 0)
            {
                int baseB = getBaseColor(board[r][c + 1]);
                int baseC = getBaseColor(board[r][c + 2]);
                if (baseA == baseB && baseA == baseC)
                {
                    return true;
                }
            }
        }
    }

    // vertical check for 3 same candies
    for (int c = 0; c < columns; c++)
    {
        for (int r = 0; r <= rows - 3; r++)
        {
            int baseA = getBaseColor(board[r][c]);
            if (baseA >= 0)
            {
                int baseB = getBaseColor(board[r + 1][c]);
                int baseC = getBaseColor(board[r + 2][c]);
                if (baseA == baseB && baseA == baseC)
                {
                    return true;
                }
            }
        }
    }

    //T shape check 
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < columns; c++)
        {
            if (isTShapeMatch(board, r, c)) {
                return true;
            }
        }
    }

    // L shape check 
    for (int r = 0; r <= rows - 3; r++)
    {
        for (int c = 0; c <= columns - 3; c++)
        {
            // L-shape 1 (top-Left corner)
            int top_left_base = getBaseColor(board[r][c]);
            if (top_left_base >= 0)
            {
                if ((top_left_base == getBaseColor(board[r][c + 1]) && top_left_base == getBaseColor(board[r][c + 2])) && // horizontal match
                    (top_left_base == getBaseColor(board[r + 1][c]) && top_left_base == getBaseColor(board[r + 2][c])))    // vertical match
                {
                    return true;
                }
            }

            // L-shape 2 (top-Right corner)
            int top_right_base = getBaseColor(board[r][c + 2]);
            if (top_right_base >= 0)
            {
                if ((top_right_base == getBaseColor(board[r][c + 1]) && top_right_base == getBaseColor(board[r][c])) &&
                    (top_right_base == getBaseColor(board[r + 1][c + 2]) && top_right_base == getBaseColor(board[r + 2][c + 2])))
                {
                    return true;
                }
            }

            // L-shape 3 (bottom-Left corner)
            int bottom_left_base = getBaseColor(board[r + 2][c]);
            if (bottom_left_base >= 0)
            {
                if ((bottom_left_base == getBaseColor(board[r + 2][c + 1]) && bottom_left_base == getBaseColor(board[r + 2][c + 2])) &&
                    (bottom_left_base == getBaseColor(board[r + 1][c]) && bottom_left_base == getBaseColor(board[r][c])))
                {
                    return true;
                }
            }

            // L-shape 4 (bottom-Right corner)
            int bottom_right_base = getBaseColor(board[r + 2][c + 2]);
            if (bottom_right_base >= 0)
            {
                if ((bottom_right_base == getBaseColor(board[r + 2][c + 1]) && bottom_right_base == getBaseColor(board[r + 2][c])) &&
                    (bottom_right_base == getBaseColor(board[r + 1][c + 2]) && bottom_right_base == getBaseColor(board[r][c + 2])))
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool clearMatches(int board[rows][columns], int& score)
{
    bool matchFound = false;
    bool specialCandyCreated = false;
    int candiesCleared = 0;

    // arrays to mark candies that are involved in any match
    bool matched[rows][columns] = { false };
    bool createdSpecial[rows][columns] = { false };           //to avoid creation of special candy at one cell twice

    // Detect matches & create specials

    // WRAPPED (T / L shapes)
    for (int r = 0; r <= rows - 3; r++) {
        for (int c = 0; c <= columns - 3; c++) {

            // T-Shape / Plus-Shape check (Center at r+1, c+1)
            int center_r = r + 1;
            int center_c = c + 1;
            int baseColor = getBaseColor(board[center_r][center_c]);
            int wrapped_candy_value = (baseColor >= 0) ? (WRAPPED_OFFSET + baseColor) : WRAPPED_OFFSET;

            if (baseColor >= 0 && !createdSpecial[center_r][center_c]) {

                bool v_match = (getBaseColor(board[r][center_c]) == baseColor && getBaseColor(board[r + 2][center_c]) == baseColor); // Vertical 3-match
                bool h_match = (getBaseColor(board[center_r][c]) == baseColor && getBaseColor(board[center_r][c + 2]) == baseColor); // Horizontal 3-match

                bool is_t_match = false;

                // 1. Plus-Shape (+)
                if (v_match && h_match) { is_t_match = true; }

                // 2. T-Up (Horizontal base at r+2, Vertical stem at c+1)
                else if (getBaseColor(board[r + 2][c]) == baseColor && getBaseColor(board[r + 2][c + 2]) == baseColor && v_match) { is_t_match = true; matched[r + 2][c] = matched[r + 2][c + 2] = true; }

                // 3. T-Down (Horizontal base at r, Vertical stem at c+1)
                else if (getBaseColor(board[r][c]) == baseColor && getBaseColor(board[r][c + 2]) == baseColor && v_match) { is_t_match = true; matched[r][c] = matched[r][c + 2] = true; }

                // 4. T-Left (Vertical base at c+2, Horizontal stem at r+1)
                else if (getBaseColor(board[r][c + 2]) == baseColor && getBaseColor(board[r + 2][c + 2]) == baseColor && h_match) { is_t_match = true; matched[r][c + 2] = matched[r + 2][c + 2] = true; }

                // 5. T-Right (Vertical base at c, Horizontal stem at r+1)
                else if (getBaseColor(board[r][c]) == baseColor && getBaseColor(board[r + 2][c]) == baseColor && h_match) { is_t_match = true; matched[r][c] = matched[r + 2][c] = true; }

                // Process T-Shape

                if (is_t_match) {
                    matchFound = true;
                    specialCandyCreated = true;

                    // Mark the surrounding pieces for clearing (the pieces that formed the T)
                    matched[r][center_c] = matched[r + 2][center_c] = true;
                    matched[center_r][c] = matched[center_r][c + 2] = true;
                    // DO NOT mark matched[center_r][center_c] here — the center becomes the wrapped candy and should stay on board.
                    board[center_r][center_c] = wrapped_candy_value; // color-coded wrapped
                    createdSpecial[center_r][center_c] = true;
                }

            }

            // L-shape checks (corners)
            // L1: Top-Left corner (r,c)
            baseColor = getBaseColor(board[r][c]);
            wrapped_candy_value = (baseColor >= 0) ? (WRAPPED_OFFSET + baseColor) : WRAPPED_OFFSET;
            if (baseColor >= 0 && !createdSpecial[r][c])
            {
                if (getBaseColor(board[r][c + 1]) == baseColor && getBaseColor(board[r][c + 2]) == baseColor && // H-match
                    getBaseColor(board[r + 1][c]) == baseColor && getBaseColor(board[r + 2][c]) == baseColor)// V-match
                {

                    matchFound = true;
                    specialCandyCreated = true;

                    matched[r][c + 1] = matched[r][c + 2] = true;
                    matched[r + 1][c] = matched[r + 2][c] = true;

                    board[r][c] = wrapped_candy_value;
                    createdSpecial[r][c] = true;
                }
            }

            // L2: Top-Right corner (r, c+2)
            baseColor = getBaseColor(board[r][c + 2]);
            wrapped_candy_value = (baseColor >= 0) ? (WRAPPED_OFFSET + baseColor) : WRAPPED_OFFSET;
            if (baseColor >= 0 && !createdSpecial[r][c + 2])
            {
                if (getBaseColor(board[r][c]) == baseColor && getBaseColor(board[r][c + 1]) == baseColor && // H-match
                    getBaseColor(board[r + 1][c + 2]) == baseColor && getBaseColor(board[r + 2][c + 2]) == baseColor)//V-match
                {
                    matchFound = true;
                    specialCandyCreated = true;

                    matched[r][c] = matched[r][c + 1] = true;
                    matched[r + 1][c + 2] = matched[r + 2][c + 2] = true;

                    board[r][c + 2] = wrapped_candy_value;
                    createdSpecial[r][c + 2] = true;
                }
            }

            // L3: Bottom-Left corner (r+2, c)
            baseColor = getBaseColor(board[r + 2][c]);
            wrapped_candy_value = (baseColor >= 0) ? (WRAPPED_OFFSET + baseColor) : WRAPPED_OFFSET;
            if (baseColor >= 0 && !createdSpecial[r + 2][c]) {
                if (getBaseColor(board[r + 2][c + 1]) == baseColor && getBaseColor(board[r + 2][c + 2]) == baseColor && // H-match
                    getBaseColor(board[r][c]) == baseColor && getBaseColor(board[r + 1][c]) == baseColor) { // V-match

                    matchFound = true;
                    specialCandyCreated = true;

                    matched[r + 2][c + 1] = matched[r + 2][c + 2] = true;
                    matched[r][c] = matched[r + 1][c] = true;

                    board[r + 2][c] = wrapped_candy_value;
                    createdSpecial[r + 2][c] = true;
                }
            }

            // L4: Bottom-Right corner (r+2, c+2)
            baseColor = getBaseColor(board[r + 2][c + 2]);
            wrapped_candy_value = (baseColor >= 0) ? (WRAPPED_OFFSET + baseColor) : WRAPPED_OFFSET;
            if (baseColor >= 0 && !createdSpecial[r + 2][c + 2]) {
                if (getBaseColor(board[r + 2][c]) == baseColor && getBaseColor(board[r + 2][c + 1]) == baseColor && // H-match
                    getBaseColor(board[r][c + 2]) == baseColor && getBaseColor(board[r + 1][c + 2]) == baseColor) { // V-match

                    matchFound = true;
                    specialCandyCreated = true;

                    matched[r + 2][c] = matched[r + 2][c + 1] = true;
                    matched[r][c + 2] = matched[r + 1][c + 2] = true;

                    board[r + 2][c + 2] = wrapped_candy_value;
                    createdSpecial[r + 2][c + 2] = true;
                }
            }
        }
    }

    // HORIZONTAL 4+ (create horizontal stripe for 4 horiz; color bomb for >=5)
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c <= columns - 3; c++)
        {
            int baseColor = getBaseColor(board[r][c]);
            int striped_h_value = (baseColor >= 0) ? (STRIPE_H_OFFSET + baseColor) : STRIPE_H_OFFSET;

            if (baseColor >= 0 && !matched[r][c] && !createdSpecial[r][c])
            {
                int count = 0;
                while (c + count < columns && getBaseColor(board[r][c + count]) == baseColor)
                {
                    count++;
                }

                if (count >= 3)
                {
                    matchFound = true;

                    // color bomb (5+)
                    if (count >= 5)
                    {
                        specialCandyCreated = true;
                        board[r][c] = COLOR_BOMB; // First candy becomes the bomb
                        createdSpecial[r][c] = true;
                        for (int i = 1; i < count; i++)
                        {
                            matched[r][c + i] = true; // Mark the rest for clearing
                        }
                    }
                    // horizontal stripe candy (4)
                    else if (count == 4)
                    {
                        specialCandyCreated = true;
                        board[r][c] = striped_h_value; // horizontal striped candy
                        createdSpecial[r][c] = true;
                        for (int i = 1; i < count; i++)
                        {
                            matched[r][c + i] = true;
                        }
                    }
                    // basic 3 candy match
                    else if (count == 3)
                    {
                        for (int i = 0; i < count; i++) {
                            matched[r][c + i] = true;
                        }
                    }

                    c += (count - 1);           //skip the matched candies
                }
            }
        }
    }

    // VERTICAL 4+ (create vertical stripe for 4 vert; color bomb for >=5)
    for (int c = 0; c < columns; c++)
    {
        for (int r = 0; r <= rows - 3; r++)
        {
            int baseColor = getBaseColor(board[r][c]);
            int striped_v_value = (baseColor >= 0) ? (STRIPE_V_OFFSET + baseColor) : STRIPE_V_OFFSET;

            if (baseColor >= 0 && !matched[r][c] && !createdSpecial[r][c])
            {
                int count = 0;
                while (r + count < rows && getBaseColor(board[r + count][c]) == baseColor)
                {
                    count++;
                }

                if (count >= 3)
                {
                    matchFound = true;

                    // color bomb (5+)
                    if (count >= 5)
                    {
                        specialCandyCreated = true;
                        board[r][c] = COLOR_BOMB;
                        createdSpecial[r][c] = true;
                        for (int i = 1; i < count; i++)
                        {
                            matched[r + i][c] = true;
                        }
                    }
                    // vertical stripe candy (4)  
                    else if (count == 4)
                    {
                        specialCandyCreated = true;
                        board[r][c] = striped_v_value; // vertical striped candy
                        createdSpecial[r][c] = true;
                        for (int i = 1; i < count; i++)
                        {
                            matched[r + i][c] = true;
                        }
                    }
                    // basic 3 candy match  
                    else if (count == 3)
                    {
                        for (int i = 0; i < count; i++) {
                            matched[r + i][c] = true;
                        }
                    }

                    r += (count - 1);
                }
            }
        }
    }

    // Activation: expand matched by special activations
    // When a special is activated we mark all affected cells in matched[] so they'll be cleared too.
    bool processedSpecial[rows][columns] = { false };
    bool expanded = true;
    while (expanded)
    {
        expanded = false;
        for (int r = 0; r < rows; ++r)
        {
            for (int c = 0; c < columns; ++c)
            {
                if (matched[r][c] && !processedSpecial[r][c])
                {
                    int val = board[r][c];
                    // If the cell is a special candy, activate its effect
                    if (isStripeH(val))
                    {
                        // clear entire row r
                        for (int cc = 0; cc < columns; ++cc)
                        {
                            if (!matched[r][cc])
                            {
                                matched[r][cc] = true;
                                expanded = true;
                            }
                        }
                        processedSpecial[r][c] = true;                       //prevents the re_activation of same candy
                    }
                    else if (isStripeV(val))
                    {
                        // clear entire column c
                        for (int rr = 0; rr < rows; ++rr)
                        {
                            if (!matched[rr][c])
                            {
                                matched[rr][c] = true;
                                expanded = true;
                            }
                        }
                        processedSpecial[r][c] = true;
                    }
                    else if (isWrapped(val))
                    {
                        // clear 3x3 centered at r,c
                        for (int rr = max(0, r - 1); rr <= min(rows - 1, r + 1); ++rr)
                        {
                            for (int cc = max(0, c - 1); cc <= min(columns - 1, c + 1); ++cc)
                            {
                                if (!matched[rr][cc])
                                {
                                    matched[rr][cc] = true;
                                    expanded = true;
                                }
                            }
                        }
                        processedSpecial[r][c] = true;
                    }
                    else if (isColorBomb(val)) {
                        // If color bomb is activated (e.g., included in matched by other special),
                        for (int rr = 0; rr < rows; ++rr)
                        {
                            for (int cc = 0; cc < columns; ++cc)
                            {
                                if (!matched[rr][cc]) {
                                    matched[rr][cc] = true;
                                    expanded = true;
                                }
                            }
                        }
                        processedSpecial[r][c] = true;
                    }
                    else
                    {
                        // base candies when matched
                        processedSpecial[r][c] = true;
                    }
                }
            }
        }
    }

    // Clear matched cells
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < columns; c++)
        {
            if (matched[r][c])
            {
                board[r][c] = EMPTY;
                candiesCleared++;
            }
        }
    }

    score += (candiesCleared * 10) / 3;

    return matchFound || specialCandyCreated;
}

void Gravity(int board[rows][columns])
{
    for (int c = 0; c < columns; c++)
    {
        for (int r = rows - 1; r >= 0; r--)
        {
            if (board[r][c] == EMPTY)
            {
                for (int r_above = r - 1; r_above >= 0; r_above--)
                {
                    if (board[r_above][c] != EMPTY)
                    {
                        board[r][c] = board[r_above][c];
                        board[r_above][c] = EMPTY;
                        break;
                    }
                }
            }
        }
    }
}

void refillBoard(int board[rows][columns])
{
    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < columns; c++)
        {
            if (board[r][c] == EMPTY)
            {
                board[r][c] = generatecandy();
            }
        }
    }
}

int main()
{
    srand((unsigned)time(0));
    int board[rows][columns];
    fillBoard(board);

    // SFML Setup & Scaling
    const int originalTileSize = 80;
    const int padding = 5;
    const float scaleFactor = 0.6f;  // scale board down
    const int tileSize = static_cast<int>(originalTileSize * scaleFactor);

    int boardWidth = columns * tileSize + (columns - 1) * padding;
    int boardHeight = rows * tileSize + (rows - 1) * padding;

    const int windowWidth = boardWidth + 300;  // keep HUD space
    const int windowHeight = boardHeight + 40;

    // compute top-left of board to center it
    int boardStartX = (windowWidth - 300 - boardWidth) / 2;
    int boardStartY = (windowHeight - boardHeight) / 2;

    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(windowWidth, windowHeight)), "Candy Crush");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("assets/arial.ttf")) {
        cerr << "Failed to load font!" << endl;
        return -1;
    }

    // Load textures
    // 0-4: Base candies
    // 5-9: Horizontal striped (per color)
    // 10-14: Vertical striped (per color)
    // 15-19: Wrapped (per color)
    // 20: Color bomb
    constexpr size_t TEX_COUNT = 21;
    array<string, TEX_COUNT> filenames = {
        // 0-4: Base Candies
        "assets/candy0.png","assets/candy1.png","assets/candy2.png","assets/candy3.png","assets/candy4.png",

        // 5-9: Horizontal Striped (one per color)
        "assets/stripe_h_0.png","assets/stripe_h_1.png","assets/stripe_h_2.png","assets/stripe_h_3.png","assets/stripe_h_4.png",

        // 10-14: Vertical Striped (one per color)
        "assets/stripe_v_0.png","assets/stripe_v_1.png","assets/stripe_v_2.png","assets/stripe_v_3.png","assets/stripe_v_4.png",

        // 15-19: Wrapped (one per color)
        "assets/wrapped_0.png","assets/wrapped_1.png","assets/wrapped_2.png","assets/wrapped_3.png","assets/wrapped_4.png",

        // 20: Color Bomb
        "assets/colorbomb.png"
    };

    vector<sf::Texture> textures;
    for (size_t i = 0; i < TEX_COUNT; ++i) {
        sf::Texture tex;
        if (!tex.loadFromFile(filenames[i])) {
            cerr << "Failed to load texture for index " << i << ": " << filenames[i] << endl;
        }
        textures.push_back(std::move(tex));
    }

    sf::Texture bgTex;
    if (!bgTex.loadFromFile("assets/background.png")) { cerr << "Background missing\n"; }

    sf::Sprite bgSprite(bgTex);
    if (bgTex.getSize().x > 0 && bgTex.getSize().y > 0) {
        float sx = float(windowWidth) / float(bgTex.getSize().x);
        float sy = float(windowHeight) / float(bgTex.getSize().y);
        bgSprite.setScale(sf::Vector2f(sx, sy));
    }
    bgSprite.setPosition(sf::Vector2f(0.f, 0.f));

    // Sprite cache
    vector<vector<sf::Sprite>> sprites;
    for (int r = 0; r < rows; r++) {
        vector<sf::Sprite> rowSprites;
        for (int c = 0; c < columns; c++) {
            sf::Sprite sp(textures[0]);
            sp.setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
            if (textures[0].getSize().x > 0)
                sp.setScale(sf::Vector2f((float)tileSize / textures[0].getSize().x, (float)tileSize / textures[0].getSize().y));
            rowSprites.push_back(sp);
        }
        sprites.push_back(rowSprites);
    }

    // Game State
    int cursorRow = 0, cursorColumn = 0;
    int selectedRow = -1, selectedColumn = -1;
    int score = 0;
    int totalTime = 300;  // 5 minutes example
    int timeLeft = totalTime;

    sf::Clock gameClock;
    time_t startTime = time(0);
    bool running = true;
    bool validMoveMade = false;

    // Main Loop
    while (window.isOpen() && running)
    {

        timeLeft = totalTime - (time(0) - startTime);

        if (timeLeft <= 0)
        {
            while (auto ev = window.pollEvent())
            {
                if (ev->is<sf::Event::Closed>()) window.close();
            }

            window.clear();
            if (bgTex.getSize().x > 0) window.draw(bgSprite);

            sf::Text gameOver(font, "TIME'S UP!", 48);
            gameOver.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 100));
            gameOver.setFillColor(sf::Color::Red);
            window.draw(gameOver);

            sf::Text finalScore(font, "Final Score: " + to_string(score), 24);
            finalScore.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 180));
            finalScore.setFillColor(sf::Color::Red);
            window.draw(finalScore);
            window.display();

            while (window.isOpen())
            {
                while (auto ev = window.pollEvent())
                {
                    if (ev->is<sf::Event::Closed>()) window.close();
                }
                sf::sleep(sf::milliseconds(100));
            }
            break;
        }

        while (auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>()) window.close();

            if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                sf::Keyboard::Key k = keyPressed->code;
                if (k == sf::Keyboard::Key::Q) { running = false; window.close(); }
                else if (k == sf::Keyboard::Key::Up || k == sf::Keyboard::Key::W)
                {
                    if (cursorRow > 0)
                        cursorRow--;
                }
                else if (k == sf::Keyboard::Key::Down || k == sf::Keyboard::Key::S)
                {
                    if (cursorRow < rows - 1)
                        cursorRow++;
                }
                else if (k == sf::Keyboard::Key::Left || k == sf::Keyboard::Key::A)
                {
                    if (cursorColumn > 0)
                        cursorColumn--;
                }
                else if (k == sf::Keyboard::Key::Right || k == sf::Keyboard::Key::D)
                {
                    if (cursorColumn < columns - 1)
                        cursorColumn++;
                }
                else if (k == sf::Keyboard::Key::Space || k == sf::Keyboard::Key::Enter)
                {
                    if (selectedRow == -1)
                    {
                        selectedRow = cursorRow; selectedColumn = cursorColumn;
                    }
                    else
                    {
                        if (selectedRow == cursorRow && selectedColumn == cursorColumn)
                        {
                            selectedRow = selectedColumn = -1;
                        }
                        else if (areAdjacent(selectedRow, selectedColumn, cursorRow, cursorColumn))
                        {
                            swapCandies(board, selectedRow, selectedColumn, cursorRow, cursorColumn);

                            // handle color bomb swap with a base candy (existing logic)
                            int valAtCursor = board[cursorRow][cursorColumn];
                            int valAtSelected = board[selectedRow][selectedColumn];
                            if (valAtCursor == COLOR_BOMB || valAtSelected == COLOR_BOMB) {
                                // Extract the base color from the non-bomb candy
                                int targetColor = -1;
                                if (valAtCursor == COLOR_BOMB && getBaseColor(valAtSelected) >= 0)
                                {
                                    targetColor = getBaseColor(valAtSelected);
                                }
                                else if (valAtSelected == COLOR_BOMB && getBaseColor(valAtCursor) >= 0)
                                {
                                    targetColor = getBaseColor(valAtCursor);
                                }

                                if (targetColor >= 0 && targetColor <= MAX_BASE_CANDY_TYPE)
                                {
                                    for (int rr = 0; rr < rows; ++rr)
                                        for (int cc = 0; cc < columns; ++cc)
                                            if (getBaseColor(board[rr][cc]) == targetColor) board[rr][cc] = EMPTY;
                                    board[cursorRow][cursorColumn] = EMPTY;
                                    board[selectedRow][selectedColumn] = EMPTY;
                                    validMoveMade = true;
                                    Gravity(board);
                                    refillBoard(board);
                                }
                                else swapCandies(board, selectedRow, selectedColumn, cursorRow, cursorColumn);
                            }
                            else if (checkMatch(board)) validMoveMade = true;
                            else swapCandies(board, selectedRow, selectedColumn, cursorRow, cursorColumn);

                            selectedRow = selectedColumn = -1;
                        }
                    }
                }
            }
        }

        // Process Matches (with visual pauses)
        if (validMoveMade)
        {
            bool matchChain = true;
            while (matchChain)
            {
                matchChain = clearMatches(board, score);

                // update sprites (same as your Update sprites block) so visuals reflect the cleared/created specials
                for (int r = 0; r < rows; r++) {
                    for (int c = 0; c < columns; c++) {
                        int val = board[r][c];
                        if (val >= 0 && val < (int)textures.size() && textures[val].getSize().x>0) {
                            sprites[r][c].setTexture(textures[val]);
                            sprites[r][c].setScale(sf::Vector2f((float)tileSize / textures[val].getSize().x, (float)tileSize / textures[val].getSize().y));
                            sprites[r][c].setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                        }
                    }
                }

                // Draw current state and pause so player sees created specials / activated cells
                window.clear();
                if (bgTex.getSize().x > 0) window.draw(bgSprite);

                for (int r = 0; r < rows; r++) {
                    for (int c = 0; c < columns; c++) {
                        sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
                        tile.setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                        tile.setOutlineThickness(2);
                        tile.setOutlineColor(sf::Color(80, 80, 80));
                        tile.setFillColor(sf::Color(30, 30, 30, 180));
                        window.draw(tile);

                        if (board[r][c] >= 0 && board[r][c] < (int)textures.size() && textures[board[r][c]].getSize().x > 0) {
                            window.draw(sprites[r][c]);
                        }
                        else {
                            sf::Text t(font, board[r][c] == EMPTY ? "." : to_string(board[r][c]), 24);
                            t.setPosition(sf::Vector2f(tile.getPosition().x + tileSize / 3, tile.getPosition().y + tileSize / 6));
                            window.draw(t);
                        }

                        // cursor highlight
                        if (r == cursorRow && c == cursorColumn) {
                            sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                            highlight.setPosition(tile.getPosition());
                            highlight.setFillColor(sf::Color(0, 0, 0, 0));
                            highlight.setOutlineThickness(4);
                            highlight.setOutlineColor(sf::Color::Yellow);
                            window.draw(highlight);
                        }

                        // selection highlight
                        if (r == selectedRow && c == selectedColumn) {
                            sf::RectangleShape sel(sf::Vector2f(tileSize, tileSize));
                            sel.setPosition(tile.getPosition());
                            sel.setFillColor(sf::Color(0, 0, 0, 0));
                            sel.setOutlineThickness(4);
                            sel.setOutlineColor(sf::Color::Green);
                            window.draw(sel);
                        }
                    }
                }

                // HUD (draw minimal HUD so visuals match)
                sf::Text scoreText(font, "Score: " + to_string(score), 20);
                scoreText.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 200));
                scoreText.setFillColor(sf::Color::Red);
                window.draw(scoreText);

                sf::Text timeText(font, "Time left: " + to_string(timeLeft) + " s", 20);
                timeText.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 250));
                timeText.setFillColor(sf::Color::Red);
                window.draw(timeText);

                window.display();

                // pause so user sees the change (tweak ms as desired)
                responsivePause(window, 250); // 250 ms pause after clearMatches

                if (matchChain)
                {
                    Gravity(board);

                    // update sprites to reflect gravity result
                    for (int r = 0; r < rows; r++) {
                        for (int c = 0; c < columns; c++) {
                            int val = board[r][c];
                            if (val >= 0 && val < (int)textures.size() && textures[val].getSize().x>0) {
                                sprites[r][c].setTexture(textures[val]);
                                sprites[r][c].setScale(sf::Vector2f((float)tileSize / textures[val].getSize().x, (float)tileSize / textures[val].getSize().y));
                                sprites[r][c].setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                            }
                        }
                    }

                    // draw gravity result and pause
                    window.clear();
                    if (bgTex.getSize().x > 0) window.draw(bgSprite);

                    for (int r = 0; r < rows; r++) {
                        for (int c = 0; c < columns; c++) {
                            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
                            tile.setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                            tile.setOutlineThickness(2);
                            tile.setOutlineColor(sf::Color(80, 80, 80));
                            tile.setFillColor(sf::Color(30, 30, 30, 180));
                            window.draw(tile);

                            if (board[r][c] >= 0 && board[r][c] < (int)textures.size() && textures[board[r][c]].getSize().x > 0) {
                                window.draw(sprites[r][c]);
                            }
                            else {
                                sf::Text t(font, board[r][c] == EMPTY ? "." : to_string(board[r][c]), 24);
                                t.setPosition(sf::Vector2f(tile.getPosition().x + tileSize / 3, tile.getPosition().y + tileSize / 6));
                                window.draw(t);
                            }
                        }
                    }

                    window.display();
                    responsivePause(window, 200); // 200 ms pause after Gravity

                    refillBoard(board);

                    // update sprites to reflect refill
                    for (int r = 0; r < rows; r++) {
                        for (int c = 0; c < columns; c++) {
                            int val = board[r][c];
                            if (val >= 0 && val < (int)textures.size() && textures[val].getSize().x>0) {
                                sprites[r][c].setTexture(textures[val]);
                                sprites[r][c].setScale(sf::Vector2f((float)tileSize / textures[val].getSize().x, (float)tileSize / textures[val].getSize().y));
                                sprites[r][c].setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                            }
                        }
                    }

                    // draw refill result and pause
                    window.clear();
                    if (bgTex.getSize().x > 0) window.draw(bgSprite);

                    for (int r = 0; r < rows; r++) {
                        for (int c = 0; c < columns; c++) {
                            sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
                            tile.setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                            tile.setOutlineThickness(2);
                            tile.setOutlineColor(sf::Color(80, 80, 80));
                            tile.setFillColor(sf::Color(30, 30, 30, 180));
                            window.draw(tile);

                            if (board[r][c] >= 0 && board[r][c] < (int)textures.size() && textures[board[r][c]].getSize().x > 0) {
                                window.draw(sprites[r][c]);
                            }
                            else {
                                sf::Text t(font, board[r][c] == EMPTY ? "." : to_string(board[r][c]), 24);
                                t.setPosition(sf::Vector2f(tile.getPosition().x + tileSize / 3, tile.getPosition().y + tileSize / 6));
                                window.draw(t);
                            }
                        }
                    }

                    window.display();
                    responsivePause(window, 200); // 200 ms pause after refill
                }
            }
            validMoveMade = false;
        }

        // Win check
        if (score >= 10000) {
            sf::Text winText(font, "CONGRATULATIONS! YOU WON!", 36);
            winText.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 100));
            winText.setFillColor(sf::Color::Red);
            window.clear();
            if (bgTex.getSize().x > 0) window.draw(bgSprite);
            window.draw(winText);
            window.display();
            sf::sleep(sf::seconds(3));
            break;
        }

        // Update sprites
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < columns; c++) {
                int val = board[r][c];
                if (val >= 0 && val < (int)textures.size() && textures[val].getSize().x>0) {
                    sprites[r][c].setTexture(textures[val]);
                    sprites[r][c].setScale(sf::Vector2f((float)tileSize / textures[val].getSize().x, (float)tileSize / textures[val].getSize().y));
                    sprites[r][c].setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                }
            }
        }

        // Draw Everything
        window.clear();
        if (bgTex.getSize().x > 0) window.draw(bgSprite);

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < columns; c++) {
                sf::RectangleShape tile(sf::Vector2f(tileSize, tileSize));
                tile.setPosition(sf::Vector2f(boardStartX + c * (tileSize + padding), boardStartY + r * (tileSize + padding)));
                tile.setOutlineThickness(2);
                tile.setOutlineColor(sf::Color(80, 80, 80));
                tile.setFillColor(sf::Color(30, 30, 30, 180));
                window.draw(tile);

                if (board[r][c] >= 0 && board[r][c] < (int)textures.size() && textures[board[r][c]].getSize().x > 0) {
                    window.draw(sprites[r][c]);
                }
                else {
                    sf::Text t(font, board[r][c] == EMPTY ? "." : to_string(board[r][c]), 24);
                    t.setPosition(sf::Vector2f(tile.getPosition().x + tileSize / 3, tile.getPosition().y + tileSize / 6));
                    window.draw(t);
                }

                // cursor highlight
                if (r == cursorRow && c == cursorColumn) {
                    sf::RectangleShape highlight(sf::Vector2f(tileSize, tileSize));
                    highlight.setPosition(tile.getPosition());
                    highlight.setFillColor(sf::Color(0, 0, 0, 0));
                    highlight.setOutlineThickness(4);
                    highlight.setOutlineColor(sf::Color::Yellow);
                    window.draw(highlight);
                }

                // selection highlight
                if (r == selectedRow && c == selectedColumn) {
                    sf::RectangleShape sel(sf::Vector2f(tileSize, tileSize));
                    sel.setPosition(tile.getPosition());
                    sel.setFillColor(sf::Color(0, 0, 0, 0));
                    sel.setOutlineThickness(4);
                    sel.setOutlineColor(sf::Color::Green);
                    window.draw(sel);
                }
            }
        }

        // HUD
        sf::Text hudTitle(font, "CANDY CRUSH", 35);
        hudTitle.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 20));
        hudTitle.setFillColor(sf::Color::Red);
        window.draw(hudTitle);

        sf::Text instructions(font, "Use arrows to move.\nSpace/Enter to select.\nQ to quit.\nEnjoy your game.", 20);
        instructions.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 80));
        instructions.setFillColor(sf::Color::Red);
        window.draw(instructions);

        sf::Text scoreText(font, "Score: " + to_string(score), 20);
        scoreText.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 200));
        scoreText.setFillColor(sf::Color::Red);
        window.draw(scoreText);

        sf::Text timeText(font, "Time left: " + to_string(timeLeft) + " s", 20);
        timeText.setPosition(sf::Vector2f(boardStartX + boardWidth + 20, 250));
        timeText.setFillColor(sf::Color::Red);
        window.draw(timeText);

        window.display();
    }
    return 0;
}
