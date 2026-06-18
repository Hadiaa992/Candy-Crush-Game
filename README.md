# Candy-Crush-Game

Programming Fundamentals Project (Fall 2025)
Hadia Noor <br>
This is a classic match-3 puzzle game, inspired by Candy Crush, built using C++ and the SFML. <br>
Getting Started <br>
To compile and run this game, you'll need a C++ compiler and the SFML library correctly set up in your IDE.<br>
Prerequisites<br>
1.C++ Compiler: G++ or similar (required for modern C++ standards).  <br>
2.SFML Library: You must have SFML 3.1.0 or later installed. 
This project uses sfml-graphics, sfml-window, and sfml-system. <br>
3.Assets Folder: You need an assets folder in the same directory as your compiled executable containing the required font and image files (5 base, 5 horizontal striped, 5 vertical striped, 5 wrapped, 1 color bomb, and background). <br>
Playing guidelines   <br>
The goal is to match candies to score points before the time limit of 300 seconds expires. <br>
Controls <br>
Arrow keys or W, S, A, D keys are used to move cursor  <br>
Space key or Enter key is used to select and swap candies <br>
Space key or Enter key is used to deselect candies  <br>
Q key for quitting game <br>
Core Gameplay <br>
1.Selection: Use the yellow cursor to highlight a candy. Press Space/Enter to select it. <br>
2.Swap: Move to an adjacent candy and press Space/Enter again. <br>
3.Valid Move: A swap is only permanent if it results in a line of three or more identical  candies (based on base color). Invalid swaps are reversed instantly. <br>
4.Cascades: Successful matches clear the candies, trigger Gravity (candies fall), and cause new candies to Refill from the top. Any subsequent matches formed automatically result in a chain reaction, or cascade, that continues until no more matches are found.  <br>




Special Candy Mechanics   <br>
Creating matches of 4 or more candies generates powerful special items:  <br>
Special Candy Creation  <br>
Striped Candy (Horizontal or Vertical): at the match location (4 match in a row or column). <br>
Color Bomb: at the match location (5 in a row or column) <br>
Wrapped Candy: T/L shaped matches at the center or corner of the shape. <br>
Special Candy Activation <br>
Special candies trigger when they are involved in a match (when they are marked for clearing by an adjacent match or another explosion). <br>
Striped candy clears the entire row or column where it is located. <br>
Wrapped candy explodes, clearing all nine candies in a 3x3 area centered on itself. <br>
Color bomb when swapped with a regular candy, it clears all candies of that color on the board. When caught in another explosion (cascading match), it clears the entire board. <br>
the game correctly handles chain reactions where one special activating another (e.g., a Wrapped Candy exploding a Striped Candy, which then clears a line and triggers another special) through a loop that expands the list of candies to be cleared until the board is stable. <br>
Win/Lose Conditions <br>
Win: Achieve a score of 10,000 points or more. <br>
Lose: The timer reaches 0 seconds. <br>


![alt text](<Screenshot 2026-06-18 234507.png>)