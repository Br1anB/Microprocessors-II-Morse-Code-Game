/// Declare header files and relevant library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "assign02.pio.h"

/// Define initial constants that the level parameters will be set to at the beginning of a new level
#define LEVEL_START_SCORE 0
#define LEVEL_START_LIVES_REMAINING 3
#define SPACE_TIME 2000000
#define DASH_TIME 300000
#define IS_RGBW true
#define WS2812_PIN 28

/// Declare global variables relevant to the levels
int lives_left = -1, score, overall_score, overall_incor, start_flag = 1;

/// Global vairables used to determine the time between button press and release
int t2, t1; 

/// The is  a global variable is used to represent a word that is currently getting inputted
char word[10];

/// Word index is a global variable representing the current morse digit that is expected to be inputted
int wordIndex = 0;

/// Declare an array containing all 36 letter and number values
char * letters_and_numbers[36] = 
{
    "A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z","0","1","2","3","4","5","6","7","8","9",    
};

/// Declare an array with all 36 corresponding morse code value for each letter
char * morse_values[36] = 
{
    ".-", "-...", "-.-.", "-..", ".","..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.", "---",".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..","-----", ".----", "..---", "...--", "....-", ".....","-....", "--...", "---..", "----."
};

/// Declare an array with the 4 possible level values represented in their morse form
char* level_morse[4] = {
    "-----", ".----", "..---", "...--"
};

/// Declare an array with 38 words related to microprocessing
char *microprocessorTerms[38] = {
        "ALU", ///1
        "CPU", ///2
        "ISA", ///3
        "RISC", ///4
        "CISC", ///5
        "FPU", ///6
        "VLIW", ///7
        "SIMD", ///8
        "MIMD", ///9
        "CACHE", ///10
        "L1CACHE", ///11
        "L2CACHE", ///12
        "L3CACHE", ///13
        "PIPELINE3", ///14
        "SUPERSCALAR", ///15 
        "CLOCKCYCLE", ///16
        "INTERRUPT", ///17
        "MMU", ///18
        "GPIO", ///19
        "UART", ///20
        "I2C", ///21
        "SPI", ///22
        "DRAM", ///23
        "SRAM", ///24
        "EPROM", ///25
        "EEPROM", ///26
        "BUS", ///27
        "REGISTER", ///28
        "ACCUMULATOR", ///29
        "PROGRAM", ///30
        "MULTICORE", ///31
        "THREAD", ///32
        "VMEMORY", ///33
        "TABLE", ///34
        "TLB", ///35
        "MESI", ///36
        "MOESI", ///37
        "MOSI" ///38
};

/// Declare an array with the respective morse representations of the microprocessig related words
char *morsewords[38] = {
    ".- .-.. ..-", ///1
    "-.-. .--. ..-", ///2
    ".. ... .-", ///3
    ".-. .. ... -.-.", ///4
    "-.-. .. ... -.-.", ///5
    "..-. .--. ..-", ///6
    "...- .-.. .. .--", ///7
    "... .. -- -..", ///8
    "-- .. -- -..", ///9
    "-.-. .- -.-. .... .", ///10
    ".-.. .---- -.-. .- -.-. .... .", ///11
    ".-.. ..--- -.-. .- -.-. .... .", ///12
    ".-.. ...-- -.-. .- -.-. .... .", ///13
    ".--. .. .--. . .-.. .. -. . ...--", ///14
    "... ..- .--. . .-. ... -.-. .- .-.. .- .-.", ///15
    "-.-. .-.. --- -.-. -.- -.-. -.-- -.-. .-.. .", ///16
    ".. -. - . .-. .-. ..- .--. -", ///17
    "-- -- ..-", ///18
    "--. .--. .. ---", ///19
    "..- .- .-. -", ///20
    ".. ..--- -.-.", ///21
    "... .--. ..", ///22
    "-.. .-. .- --", ///23
    "... .-. .- --", ///24
    ". .--. .-. --- --", ///25
    ". . .--. .-. --- --", ///26
    "-... ..- ...", ///27
    ".-. . --. .. ... - . .-.", ///28
    ".- -.-. -.-. ..- -- ..- .-.. .- - --- .-.", ///29
    ".--. .-. --- --. .-. .- --", ///30
    "-- ..- .-.. - .. -.-. --- .-. .", ///31
    "- .... .-. . .- -..", ///32
    "...- -- . -- --- .-. -.--", ///33
    "- .- -... .-.. .", ///34
    "- .-.. -...", ///35
    "-- . ... ..", ///36
    "-- --- . ... ..", ///37
    "-- --- ... .." ///38
};

/// Declare all necessary functions for the game
void input();
void reset_asm();
void poll();
void print_dash();
void print_dot();
void get_t2();
void get_t1();
void determine();
int  Briancompare (char *realString, char* inputString);
void asm_gpio_set_irq(uint pin);
void setup_gpio_isr();
void level_one();
void level_two();
void level_three();
void level_four();
void level_five();
void welcomeScreen();
void endScreen();
void scoreboard();
void levelOneScreen(int index);
void levelTwoScreen(int index);
void levelThreeScreen();
void levelFourScreen();
void levelFiveScreen();
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void LED_colour_update();

/// @brief Function to initialise the watchdog and ensure it will function correctly and print out statements to console to indicate when it will reboot the game
void watchdog_init()
{
    if (watchdog_caused_reboot())
    { //  if reboot was due to watchdog timeout
        printf("\nNo input was detected for timeout period. Rebooted by watchdog\n");
    }
    if (watchdog_enable_caused_reboot())
    { // if enabling watchdog caused reboot
        printf("\nChip reboot due to watchdog enable\n");
    }
    watchdog_enable(0x7fffff, 1); 
    watchdog_update();
}


/*
 * Main entry point for the code - simply initiates the game.
 */
int main() {
    /// Set up stdio functionality
    stdio_init_all();

    /// Set up PIO functionality so the RGB LED display will function correctly
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    /// Initialise the watchdog
    watchdog_init();

    /// Ensure the watchdog is updated and ready to start counting as the game begins
    watchdog_update();

    /// Set up the interrupt flags and set button 21 up to be able to read in button presses
    asm_gpio_set_irq(21);
    setup_gpio_isr();

    /// Seed the random function to ensure random numbers are generated later in the code
    sleep_ms(1000);
    srand(time(NULL));

    /// Start the game
    for (;;) {
        /// Update the LED to indicate the game has started. On the home screen it will be blue
        LED_colour_update();

        /// Display the welcome screen
        printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
        welcomeScreen();

        /// Print in an input from the user indicating what level they desire to start playing at
        input();

        /// Depending on this input start the game from the corresponding level

            /// Begin level 1
            if (Briancompare(word, level_morse[0])) {

                level_one();
                sleep_ms(3000);
            }   
        
            /// Begin level 2
            else if (Briancompare(word, level_morse[1])) {
                
                level_two();
                sleep_ms(3000);

            } 

            /// Begin level 3
            else if (Briancompare(word, level_morse[2])) {

                level_three();
                sleep_ms(3000);

            } 
        
            /// Begin level 4
            else if (Briancompare(word, level_morse[3])) {
        
                level_four();
                sleep_ms(3000);

            } 
        
            /// There was an invalid input inputted
            else {

                endScreen();
                printf("You couldn't even input the correct level...\n\n");
                sleep_ms(3000);

            }
    }

    return(0);
}

/// @brief Function used to print a dash to console and to add a dash to the current morse value being inputted
void print_dash () {
    if (wordIndex == 10) {
        printf("word[] overflow!\n");
    } else {
        word[wordIndex] = '-';
        wordIndex++;
        printf("-");
    }
}

/// @brief Function used to print a dot to console and to add a dot to the current morse value being inputted
void print_dot () {
    if (wordIndex == 10) {
        printf("word[] overflow!\n");
    } else {
        word[wordIndex] = '.';
        wordIndex++;
        printf(".");
    }
}

/// @brief Record the time at which a individual button press is terminated
void get_t2(){
    t2 = time_us_64();
}

/// @brief Record the time at which a individual button press is initiated
void get_t1(){
    t1 = time_us_64();
}

/// @brief This function is used to determine whether a button press was a dot or a dash
void determine(){
    /// If the button was pressed for longer than the determined dash time print a dash
    if(t2 - t1 > DASH_TIME){
        print_dash();
    }

    /// Otherwise print a dot
    else{
        print_dot();
    }
}

/// @brief This function compares the more representation of the expected input with the morse input
/// @param realString The morse value inputted by the user
/// @param inputString The expected morse value of the input
/// @return Return as a boolean. 1 if the inputs were identical, 0 if they weren't
int Briancompare (char *realString, char* inputString) {
    /// If the expected input is the same as the input return 1 indicating they were the same
    if (strcmp(realString, inputString) == 0) {
        return 1;
    }

    /// Else return 0
    else {
        return 0;
    }
    // ?????????????????????????????
}

/// @brief Enable falling-edge interrupt – see SDK for detail on gpio_set_irq_enabled()
/// @param pin this indicates the pico pin that is to be set
void asm_gpio_set_irq(uint pin) {
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_RISE, true);
}

/// @brief This function prints the home screen of the game and indicates to the player how to begin a level of their chosing
void welcomeScreen() {
    /// Print the home screen of the game
    printf("\033[1;35m");
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |   _____        _________         __        _______      ____  _____  | |\n");
    printf("| |  |_   _|      |_   ___  |       /  \\      |_   __ \\    |_   \\|_   _| | |\n");
    printf("| |    | |          | |_  \\_|      / /\\ \\       | |__) |     |   \\ | |   | |\n");
    printf("| |    | |   _      |  _|  _      / ____ \\      |  __ /      | |\\ \\| |   | |\n");
    printf("| |   _| |__/ |    _| |___/ |   _/ /    \\ \\_   _| |  \\ \\_   _| |_\\   |_  | |\n");
    printf("| |  |________|   |_________|  |____|  |____| |____| |___| |_____|\\____| | |\n");
    printf("| | ____    ____      ____       _______         _______     _________   | |\n");
    printf("| ||_   \\  /   _|   .'    `.    |_   __ \\       /  ___  |   |_   ___  |  | |\n");
    printf("| |  |   \\/   |    /  .--.  \\     | |__) |     |  (__ \\_|     | |_  \\_|  | |\n");
    printf("| |  | |\\  /| |    | |    | |     |  __ /       '.___`-.      |  _|  _   | |\n");
    printf("| | _| |_\\/_| |_   \\  `--'  /    _| |  \\ \\_    |`\\____) |    _| |___/ |  | |\n");
    printf("| ||_____||_____|   `.____.'    |____| |___|   |_______.'   |_________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\n");
    printf("\033[0;36m");

    /// Display the choice of levels and indicate how to choose which level the player would like to play
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
    printf("| |                USE GP21 TO ENTER A SEQUENCE TO BEGIN!                | |\n");
    printf("| |                  \"-----\" - Level 01 - CHARS (EASY)                   | |\n");
    printf("| |                  \".----\" - Level 02 - CHARS (HARD)                   | |\n");
    printf("| |                  \"..---\" - Level 03 - WORDS (EASY)                   | |\n");
    printf("| |                  \"...--\" - Level 04 - WORDS (HARD)                   | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\033[0;37m");
}

/// @brief Function to print a game over screen once the player loses all of their lives
void endScreen() {
    /*
    /// Flag for determining whether to print a score
    int flag = 0;
    double accuracy;

    /// Calculate the player's accuracy if a game was played
    if(overall_score || incorrect_answers != 0) {
        double numerator = overall_score;
        double denominator = overall_score + incorrect_answers;
        accuracy = (numerator / denominator);
        flag = 1;
    }
    */

    /// Print the end screen
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    printf("\033[38;2;255;0;0m");
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |            ______          __       ____    ____   _________         | |\n");
    printf("| |          .' ___  |        /  \\     |_   \\  /   _| |_   ___  |        | |\n");
    printf("| |         / .'   \\_|       / /\\ \\      |   \\/   |     | |_  \\_|        | |\n");
    printf("| |         | |    ____     / ____ \\     | |\\  /| |     |  _|  _         | |\n");
    printf("| |         \\ `.___]  _|  _/ /    \\ \\_  _| |_\\/_| |_   _| |___/ |        | |\n");
    printf("| |          `._____.'   |____|  |____||_____||_____| |_________|        | |\n");
    printf("| |                                                                      | |\n");
    printf("| |            ____      ____   ____    _________     _______            | |\n");
    printf("| |          .'    `.   |_  _| |_  _|  |_   ___  |   |_   __ \\           | |\n");
    printf("| |         /  .--.  \\    \\ \\   / /      | |_  \\_|     | |__) |          | |\n");
    printf("| |         | |    | |     \\ \\ / /       |  _|  _      |  __ /           | |\n");
    printf("| |         \\  `--'  /      \\ ' /       _| |___/ |    _| |  \\ \\_         | |\n");
    printf("| |          `.____.'        \\_/       |_________|   |____| |___|        | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------'\n\n");

    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
    printf("| |                            \e[1;37mFinal Score: %02d\033[38;2;255;0;0m                           | |\n", (overall_score + overall_incor));
    printf("| |                           \e[1;37mTotal Correct: %02d\033[38;2;255;0;0m                          | |\n", overall_score);
    printf("| |                          \e[1;37mTotal Incorrect: %02d\033[38;2;255;0;0m                         | |\n", overall_incor);
    printf("| |                         \e[1;37mOverall Accuracy: %.0f%%\033[38;2;255;0;0m                        | |\n", ((float)overall_score/(overall_score + overall_incor))*100);
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    overall_incor = 0;
    overall_score = 0;
}

/// @brief Function to print a scoreboard of players current statistics
void scoreboard() {
    if (lives_left == 3) {
        printf("\033[38;2;0;255;0m");
    } else if (lives_left == 2) {
        printf("\033[38;2;255;255;0m");
    } else {
        printf("\033[38;2;255;165;0m");
    }
    printf("\n");
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
    printf("| |                           \e[1;37mCurrent Score: %02d\033[38;2;0;255;0m                          | |\n", (overall_score + overall_incor));
    printf("| |                           \e[1;37mTotal Correct: %02d\033[38;2;0;255;0m                          | |\n", overall_score);
    printf("| |                          \e[1;37mTotal Incorrect: %02d\033[38;2;0;255;0m                         | |\n", overall_incor);
    printf("| |                         \e[1;37mOverall Accuracy: %.0f%%\033[38;2;0;255;0m                        | |\n", ((float)overall_score/(overall_score + overall_incor))*100);
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
}

/**
 * @brief Prints the level screen for level 2
 * Display a hint of the morse value of the expected letter in the level screen
 * @param index The current word that the game is using as its expected input
 * @return void
*/
void levelOneScreen(int index) {
	/// if statement to change colour depending on lives
    if (lives_left == 3) {
        printf("\033[38;2;0;255;0m");
    } else if (lives_left == 2) {
        printf("\033[38;2;255;255;0m");
    } else {
        printf("\033[38;2;255;165;0m");
    }
    
    /// Print out the level one screen
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |   _____        _________    ____   ____    _________      _____      | |\n");
    printf("| |  |_   _|      |_   ___  |  |_  _| |_  _|  |_   ___  |    |_   _|     | |\n");
    printf("| |    | |          | |_  \\_|    \\ \\   / /      | |_  \\_|      | |       | |\n");
    printf("| |    | |   _      |  _|  _      \\ \\ / /       |  _|  _       | |   _   | |\n");
    printf("| |   _| |__/ |    _| |___/ |      \\ ' /       _| |___/ |     _| |__/ |  | |\n");
    printf("| |  |________|   |_________|       \\_/       |_________|    |________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| |                   ____      ____  _____    _________                 | |\n");
    printf("| |                 .'    `.   |_   \\|_   _|  |_   ___  |                | |\n");
    printf("| |                /  .--.  \\    |   \\ | |      | |_  \\_|                | |\n");
    printf("| |                | |    | |    | |\\ \\| |      |  _|  _                 | |\n");
    printf("| |                \\  `--'  /   _| |_\\   |_    _| |___/ |                | |\n");
    printf("| |                 `.____.'   |_____|\\____|  |_________|                | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------'\n");
    printf("\n");
    int hintPadL = (38 - strlen(morse_values[index]))/2 + strlen(morse_values[index])%2;
    int hintPadR = (38 - strlen(morse_values[index]))/2;
    // Hint: The morse for of 's' is ''  38
	printf("\e[0;36m");
	printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
	printf("| |                 \e[1;37mWhat is the character \'%s\' in morse?\e[0;36m                  | |\n", letters_and_numbers[index]);
	printf("| |\e[1;37m%*sHint: The morse for of \'%s\' is \'%s\'%*s\e[0;36m| |\n", hintPadL, "", letters_and_numbers[index], morse_values[index], hintPadR, "");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\n");
    printf("\n");
}

/**
 * @brief Prints the level screen for level 2
 * @param index The current word that the game is using as its expected input
 * @return void
*/
void levelTwoScreen(int index) {
    /// Depending on the amount of lives left adjusting the print text colour
    if (lives_left == 3) {
        printf("\033[38;2;0;255;0m");
    } else if (lives_left == 2) {
        printf("\033[38;2;255;255;0m");
    } else {
        printf("\033[38;2;255;165;0m");
    }
    
    /// Print the level 2 screen
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |   _____        _________    ____   ____    _________      _____      | |\n");
    printf("| |  |_   _|      |_   ___  |  |_  _| |_  _|  |_   ___  |    |_   _|     | |\n");
    printf("| |    | |          | |_  \\_|    \\ \\   / /      | |_  \\_|      | |       | |\n");
    printf("| |    | |   _      |  _|  _      \\ \\ / /       |  _|  _       | |   _   | |\n");
    printf("| |   _| |__/ |    _| |___/ |      \\ ' /       _| |___/ |     _| |__/ |  | |\n");
    printf("| |  |________|   |_________|       \\_/       |_________|    |________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| |                _________    _____  _____      ____                   | |\n");
	printf("| |               |  _   _  |  |_   _||_   _|   .'    `.                 | |\n");
	printf("| |               |_/ | | \\_|    | | /\\ | |    /  .--.  \\                | |\n");
	printf("| |                   | |        | |/  \\| |    | |    | |                | |\n");
	printf("| |                  _| |_       |   /\\   |    \\  `--'  /                | |\n");
	printf("| |                 |_____|      |__/  \\__|     `.____.'                 | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------'\n");
    printf("\n");
	printf("\e[0;36m");
	printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
	printf("| |                 \e[1;37mWhat is the character \'%s\' in morse?\e[0;36m                  | |\n", letters_and_numbers[index]);
	printf("| |                 \e[1;37mHint: You should have studied more!\e[0;36m                  | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\n");
    printf("\n");
}

/**
 * @brief Prints the level screen for level 3
 * Display a hint of the morse representation of the expected word
 * @param index The current word that the game is using as its expected input
 * @return void
*/
void levelThreeScreen(int index) {
    /// Depending on the amount of lives left adjusting the print text colour
    if (lives_left == 3) {
        printf("\033[38;2;0;255;0m");
    } else if (lives_left == 2) {
        printf("\033[38;2;255;255;0m");
    } else {
        printf("\033[38;2;255;165;0m");
    }

    /// Print the lvel 3 screen
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |   _____        _________    ____   ____    _________      _____      | |\n");
    printf("| |  |_   _|      |_   ___  |  |_  _| |_  _|  |_   ___  |    |_   _|     | |\n");
    printf("| |    | |          | |_  \\_|    \\ \\   / /      | |_  \\_|      | |       | |\n");
    printf("| |    | |   _      |  _|  _      \\ \\ / /       |  _|  _       | |   _   | |\n");
    printf("| |   _| |__/ |    _| |___/ |      \\ ' /       _| |___/ |     _| |__/ |  | |\n");
    printf("| |  |________|   |_________|       \\_/       |_________|    |________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| |  _________     ____  ____    _______       _________     _________   | |\n");
    printf("| | |  _   _  |   |_   ||   _|  |_   __ \\     |_   ___  |   |_   ___  |  | |\n");
    printf("| | |_/ | | \\_|     | |__| |      | |__) |      | |_  \\_|     | |_  \\_|  | |\n");
    printf("| |     | |         |  __  |      |  __ /       |  _|  _      |  _|  _   | |\n");
    printf("| |    _| |_       _| |  | |_    _| |  \\ \\_    _| |___/ |    _| |___/ |  | |\n");
    printf("| |   |_____|     |____||____|  |____| |___|  |_________|   |_________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------'\n");
    printf("\n");
	printf("\e[0;36m");
    /// width 70
    /// Real 32      ((70-34)-strlen(microprocessorTerms[index]))/2 + strlen(microprocessorTerms[index])%2
    ///Hint 6
    int realPadL = (41 - strlen(microprocessorTerms[index]))/2 + !(strlen(microprocessorTerms[index])%2);
    int realPadR = (41 - strlen(microprocessorTerms[index]))/2;
    int hintPadL = (62 - strlen(morsewords[index]))/2 + strlen(morsewords[index])%2;
    int hintPadR = (62 - strlen(morsewords[index]))/2;
	printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
	printf("| |\e[1;37m%*sWhat is the word \'%s\' in morse?%*s\e[0;36m| |\n", realPadL, "", microprocessorTerms[index], realPadR, "");
	printf("| |\e[1;37m%*sHint: \'%s\'%*s\e[0;36m| |\n", hintPadL, "", morsewords[index], hintPadR, "");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\n");
    printf("\n");
}

/**
 * @brief Prints the level screen for level 4
 * @param index The current word that the game is using as its expected input
 * @return void
*/
void levelFourScreen(int index) {
    /// Depending on the amount of lives left adjusting the print text colour
    if (lives_left == 3) {
        printf("\033[38;2;0;255;0m");
    } else if (lives_left == 2) {
        printf("\033[38;2;255;255;0m");
    } else {
        printf("\033[38;2;255;165;0m");
    }

    /// Print the level screen
    printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |   _____        _________    ____   ____    _________      _____      | |\n");
    printf("| |  |_   _|      |_   ___  |  |_  _| |_  _|  |_   ___  |    |_   _|     | |\n");
    printf("| |    | |          | |_  \\_|    \\ \\   / /      | |_  \\_|      | |       | |\n");
    printf("| |    | |   _      |  _|  _      \\ \\ / /       |  _|  _       | |   _   | |\n");
    printf("| |   _| |__/ |    _| |___/ |      \\ ' /       _| |___/ |     _| |__/ |  | |\n");
    printf("| |  |________|   |_________|       \\_/       |_________|    |________|  | |\n");
    printf("| |                                                                      | |\n");
    printf("| |         _________        ____      _____  _____   _______            | |\n");
    printf("| |        |_   ___  |     .'    `.   |_   _||_   _| |_   __ \\           | |\n");
    printf("| |          | |_  \\_|    /  .--.  \\    | |    | |     | |__) |          | |\n");
    printf("| |          |  _|        | |    | |    | '    ' |     |  __ /           | |\n");
    printf("| |         _| |_         \\  `--'  /     \\ `--' /     _| |  \\ \\_         | |\n");
    printf("| |        |_____|         `.____.'       `.__.'     |____| |___|        | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------'\n");
    printf("\n");
	printf("\e[0;36m");
    /// width 70
    /// Real 32      ((70-34)-strlen(microprocessorTerms[index]))/2 + strlen(microprocessorTerms[index])%2
    /// Hint 6
    int realPadL = (41 - strlen(microprocessorTerms[index]))/2 + !(strlen(microprocessorTerms[index])%2);
    int realPadR = (41 - strlen(microprocessorTerms[index]))/2;
	printf(" .-------------------------------------------------------------------------.\n");
    printf("| .----------------------------------------------------------------------. |\n");
    printf("| |                                                                      | |\n");
	printf("| |\e[1;37m%*sWhat is the word \'%s\' in morse?%*s\e[0;36m| |\n", realPadL, "", microprocessorTerms[index], realPadR, "");
	printf("| |                 \e[1;37mHint: You should have studied more!\e[0;36m                  | |\n");
    printf("| |                                                                      | |\n");
    printf("| '----------------------------------------------------------------------' |\n");
    printf(" '-------------------------------------------------------------------------' \n");
    printf("\n");
    printf("\n");
}

/// @brief This function updates the colour of the LED display depending on the current state of the game
void LED_colour_update() {
    /// In this function the colour of the LED will be set depending on the value of the global parameter lives
    if(lives_left == 3) {
        /// Turn the LED green
        put_pixel(urgb_u32(0x00, 0xFF, 0x00));
    }

    else if(lives_left == 2) {
        /// Turn the LED yellow
        put_pixel(urgb_u32(0xFF, 0xFF, 0x00));
    }

    else if(lives_left == 1) {
        /// Turn the LED orange
        put_pixel(urgb_u32(0xD2, 0x64, 0x00));
    }

    else if(lives_left == 0) {
        /// Turn the LED red
        put_pixel(urgb_u32(0xFF, 0x00, 0x00));
    }

    else {
        /// Turn the LED blue
        put_pixel(urgb_u32(0x00, 0x00, 0xFF));  
    }
}

/**
 * @brief Begins level one of the game
 *
 * This function initializes the game parameters and begins the game loop for level one. 
 * A random letter will be chosen for the player to test their Morse code knowledge. 
 * A hint showcasing the morse representation of a letter will be shown to the player
 * The player's input will be compared to the expected input and their score and remaining lives will be updated accordingly.
 * If the player answers 5 answers correctly in a row then they advance to the next level
 * If the player loses all of their lives the game will end
 *
 * @return void
*/

void level_one() {
    /// Print welcome message
    printf("\nWelcome to Level One. Good luck! \n");

    /// Declare initial level parameters
    lives_left = LEVEL_START_LIVES_REMAINING;
    score = LEVEL_START_SCORE;
    overall_score = LEVEL_START_SCORE;
    overall_incor = LEVEL_START_SCORE;

    /// Update the colour of the LED to indicate that the game has begun
    LED_colour_update();

    /// Begin the game
    for(;;) /// A forever for loop is initiated to begin the game. This for loop will continue until all lives are lost or the player is ready to move on to the next level
    {
        /// For each iteration of the game, a random number of letter will be chosen to test the player
        int current_test_value = rand() % 36;

        /// Prompt the player to enter a morse value
        levelOneScreen(current_test_value);

        /// Read the player's input
        input();

        /// Test if the input is one of the letters in the char array. If it's not indicate this with a '?'
        char letter = '?';
        for(int i = 0; i <= 36; i++) {
            if(Briancompare(word, morse_values[i])) {
                letter = letters_and_numbers[i][0];
            }
        }
        /// Convert this input from morse code into a corresponding char letter or number and print relevant information
        printf("\nInputted value was %s which is the letter/number %c ('?' symbolises the morse entry didn't correspond to a letter/ number)\n", word, letter);
        printf("Expected value was %s which is the letter/number %c\n", morse_values[current_test_value], letters_and_numbers[current_test_value][0]);

        /// Test whether this input is correct
        if(Briancompare(word, morse_values[current_test_value])) 
        {
            /// Print that this answer was correct
            printf("This answer was CORRECT!!! :D\n");

            /// Increase the score values after each correct answer
            score++;
            overall_score++;

            /// If lives_left is less than 3 increase the number of remaining lives and update the LED
            if(lives_left < 3) 
            {
                lives_left++;
                LED_colour_update();
            }

            /// If the player has answered 5 correct answers they will move on to the next level
            if(score == 5) 
            {
                /// Print out a congratulations message and advance to the next level
                printf("Congratulations! You have completed level one. You will now move on to level two. Good luck! :D\n");
                scoreboard();
                start_flag = 0;
                level_two();
                return;
            }
        }

        /// This is the functionality enacted if the input wasn't correct
        else 
        {
            /// Print that this answer was incorrect
            printf("This answer was incorrect. :(\n");

            /// Decrease the value of lives_left, update the LED colour, and then reset the score to zero
            lives_left--;
            overall_incor++;
            LED_colour_update();
            score = LEVEL_START_SCORE;

            /// If all lives have been lost finish the game;
            if(lives_left == 0) {
                endScreen();
                start_flag = 1;
                lives_left--;
                return;
            }
        }

    }
}

/**
 * @brief Begins level two of the game
 *
 * This function initializes the game parameters and begins the game loop for level two. 
 * A random letter will be chosen for the player to test their Morse code knowledge. 
 * The player's input will be compared to the expected input and their score and remaining lives will be updated accordingly.
 * If the player answers 5 answers correctly in a row then they advance to the next level
 * If the player loses all of their lives the game will end
 *
 * @return void
*/
void level_two() {
    /// Print welcome message
    printf("\nWelcome to Level Two. Good luck! \n");

    /// Declare initial level parameters
    score = LEVEL_START_SCORE;
    /// Check if this is the first level the player will play. If not don't update the overall score and lives parameters
    if(start_flag == 1)
    {
        overall_score = LEVEL_START_SCORE;
        overall_incor = LEVEL_START_SCORE;
        lives_left = LEVEL_START_LIVES_REMAINING;
        /// Update the colour of the LED to indicate that the game has begun
        LED_colour_update();
    }

    /// Begin the game
    for(;;) /// A forever for loop is initiated to begin the game. This for loop will continue until all lives are lost or the player is ready to move on to the next level
    {
        /// For each iteration of the game, a random number of letter will be chosen to test the player
        int current_test_value = rand() % 36;

        levelTwoScreen(current_test_value);

        /// Prompt the player to enter a morse value

        /// Read the player's input
        input();

        ///Test if the input is one of the letters in the char array. If it's not indicate this with a '?'
        char letter = '?';
        for(int i = 0; i <= 36; i++) {
            if(Briancompare(word, morse_values[i])) {
                letter = letters_and_numbers[i][0];
            }
        }

        /// Convert this input from morse code into a corresponding char letter or number and print relevant information
        printf("\nInputted value was %s which is the letter/number %c ('?' symbolises the morse entry didn't correspond to a letter/ number)\n", word, letter);
        printf("Expected value was %s which is the letter/number %c\n", morse_values[current_test_value], letters_and_numbers[current_test_value][0]);

        /// Test whether this input is correct
        if(Briancompare(word, morse_values[current_test_value])) 
        {
            /// Print that this answer was correct
            printf("This answer was CORRECT!!! :D\n");

            /// Increase the score values after each correct answer
            score++;
            overall_score++;

            /// If lives_left is less than 3 increase the number of remaining lives and update the LED
            if(lives_left < 3) 
            {
                lives_left++;
                LED_colour_update();
            }

            /// If the player has answered 5 correct answers they will move on to the next level
            if(score == 5) 
            {
                /// Print out a congratulations message and advance to the next level
                printf("Congratulations! You have completed level two. You will now move on to level three. Good luck! :D\n");
                scoreboard();
                start_flag = 0;
                level_three();
                return;
            }
        }

        /// This is the functionality enacted if the input wasn't correct
        else 
        {
            /// Print that this answer was incorrect
            printf("This answer was incorrect. :(\n");
            
            /// Decrease the value of lives_left, update the LED colour, and then reset the score to zero
            lives_left--;
            overall_incor++;
            LED_colour_update();
            score = LEVEL_START_SCORE;

            /// If all lives have been lost finish the game;
            if(lives_left == 0) {
                endScreen();
                lives_left--; /// Decrement lives_left so that it become -1 which will update the LED to blue
                start_flag = 1;
                return;
            }
        }

    }
}

/**
 * @brief Begins level three of the game
 *
 * This function initializes the game parameters and begins the game loop for level three. 
 * A random word relating to microprocessing will be chosen for the player to test their Morse code knowledge. 
 * The morse representation of this word will be given to the player as a hint
 * The player's input will be compared to the expected input and their score and remaining lives will be updated accordingly.
 * If the player answers 5 answers correctly in a row then they advance to the next level
 * If the player loses all of their lives the game will end
 *
 * @return void
*/
void level_three() {
    /// Print welcome message
    printf("\nWelcome to Level Three. Good luck! \n");

    /// Declare initial level parameters
    score = LEVEL_START_SCORE;
    /// Check if this is the first level the player will play. If not don't update the overall score and lives parameters
    if(start_flag == 1)
    {
        overall_score = LEVEL_START_SCORE;
        overall_incor = LEVEL_START_SCORE;
        lives_left = LEVEL_START_LIVES_REMAINING;
        /// Update the colour of the LED to indicate that the game has begun
        LED_colour_update();
    }

    /// Begin the game
    for(;;) /// A forever for loop is initiated to begin the game. This for loop will continue until all lives are lost or the player is ready to move on to the next level
    {
        /// For each iteration of the game, a random number of letter will be chosen to test the player
        int current_test_value = rand() % 38;

        levelThreeScreen(current_test_value);

        /// Prompt the player to enter a morse value
        int correct = 1;
        char buffer[100];
        int possible_word_length = 15;
        char input_word[possible_word_length];
        for(int i = 0; i < possible_word_length; i++) {
            input_word[i] = '\0';
        }
        /// Read the player's input
        for(int j = 0; j<strlen(microprocessorTerms[current_test_value]);j++){
            int index;
            input();
            
            /// Test if the input is one of the letters in the char array. If it's not indicate this with a '?'
            char letter = '?';
            for(int i = 0; i <= 36; i++) {
                if(Briancompare(word, morse_values[i])) {
                    letter = letters_and_numbers[i][0];
                }
            }
            input_word[j] = letter;
            /// Print the value of this inputted letter
            printf("\nInputted value was %c ('?' symbolises the morse entry didn't correspond to any entry)\n", letter);
            
            /// Start to insert the word, letter by letter into the global variable 'word'
            if(j == 0){
                strcpy(buffer,word);
            }

            /// If word already has a first letter, add the next letter after the current last letter of word
            else{
                strcat(buffer," ");
                strcat(buffer,word);
            }

            /// Find the position the last inputted letter should represent in our word
            for(int i = 0; i<36;i++){
                if(microprocessorTerms[current_test_value][j] == letters_and_numbers[i][0]){ 
                    index = i;
                    break;
                }
            }

            /// Check if the last entered letter of the word matches the expected letter. If not, acknowledge this and finish reading in letters
            if(!Briancompare(word,morse_values[index])){
               correct = 0;
               break;
            }

        }

        /// Convert this input from morse code into a corresponding char letter or number and print relevant information
        printf("\nInputted value was %s which is the word %s\n", buffer, input_word);
        printf("Expected value was %s which is the word %s\n" ,morsewords[current_test_value], microprocessorTerms[current_test_value]);

        /// Test whether this input is correct
        if(correct) 
        {
            /// Print that this answer was correct
            printf("\nThis answer was CORRECT!!! :D\n");

            /// Increase the score values after each correct answer
            score++;
            overall_score++;

            /// If lives_left is less than 3 increase the number of remaining lives and update the LED
            if(lives_left < 3) 
            {
                lives_left++;
                LED_colour_update();
            }

            /// If the player has answered 5 correct answers they will move on to the next level
            if(score == 5) 
            {
                /// Print out a congratulations message and advance to the next level
                printf("Congratulations! You have completed level three. You will now move on to level four. Good luck! :D\n");
                scoreboard();
                start_flag = 0;
                level_four();
                return;
            }
        }

        /// This is the functionality enacted if the input wasn't correct
        else 
        {
            /// Print that this answer was incorrect
            printf("\nThis answer was incorrect. :(\n");
            
            /// Decrease the value of lives_left, update the LED colour, and then reset the score to zero
            lives_left--;
            overall_incor++;
            LED_colour_update();
            score = LEVEL_START_SCORE;

            /// If all lives have been lost finish the game;
            if(lives_left == 0) {
                endScreen();
                lives_left--; /// Decrement lives_left so that it become -1 which will update the LED to blue
                start_flag = 1;
                return;
            }
        }

    }
}

/**
 * @brief Begins level three of the game
 *
 * This function initializes the game parameters and begins the game loop for level three. 
 * A random word relating to microprocessing will be chosen for the player to test their Morse code knowledge. 
 * The morse representation of this word will be given to the player as a hint
 * The player's input will be compared to the expected input and their score and remaining lives will be updated accordingly.
 * If the player loses all of their lives the game will end
 * The game will keep on running until all lives are lost
 * 
 * @return void
*/
void level_four() {
    /// Print welcome message
    printf("\nWelcome to Level Four. Good luck! \n");

    /// Declare initial level parameters
    score = LEVEL_START_SCORE;
    /// Check if this is the first level the player will play. If not don't update the overall score and lives parameters
    if(start_flag == 1)
    {
        overall_score = LEVEL_START_SCORE;
        overall_incor = LEVEL_START_SCORE;
        lives_left = LEVEL_START_LIVES_REMAINING;
        /// Update the colour of the LED to indicate that the game has begun
        LED_colour_update();
    }

    /// Begin the game
    for(;;) /// A forever for loop is initiated to begin the game. This for loop will continue until all lives are lost or the player is ready to move on to the next level
    {
        /// For each iteration of the game, a random number of letter will be chosen to test the player
        int current_test_value = rand() % 38;

        levelFourScreen(current_test_value);

        /// Prompt the player to enter a morse value
        int correct = 1;
        char buffer[100];
        int possible_word_length = 15;
        char input_word[possible_word_length];
        for(int i = 0; i < possible_word_length; i++) {
            input_word[i] = '\0';
        }

        /// Read the player's input
        for(int j = 0; j<strlen(microprocessorTerms[current_test_value]);j++){
            int index;
            input();
            
            /// Test if the input is one of the letters in the char array. If it's not indicate this with a '?'
            char letter = '?';
            for(int i = 0; i <= 36; i++) {
                if(Briancompare(word, morse_values[i])) {
                    letter = letters_and_numbers[i][0];
                }
            }
            input_word[j] = letter;
            /// Print the value of this inputted letter
            printf("\nInputted value was %c ('?' symbolises the morse entry didn't correspond to any entry)\n", letter);
            
            /// Start to insert the word, letter by letter into the global variable 'word'
            if(j == 0){
                strcpy(buffer,word);
            }

            /// If word already has a first letter, add the next letter after the current last letter of word
            else{
                strcat(buffer," ");
                strcat(buffer,word);
            }

            /// Find the position the last inputted letter should represent in our word
            for(int i = 0; i<36;i++){
                if(microprocessorTerms[current_test_value][j] == letters_and_numbers[i][0]){ 
                    index = i;
                    break;
                }
            }

            /// Check if the last entered letter of the word matches the expected letter. If not, acknowledge this and finish reading in letters
            if(!Briancompare(word,morse_values[index])){
               correct = 0;
               break;
            }

        }

        /// Convert this input from morse code into a corresponding char letter or number and print relevant information
        printf("\nInputted value was %s which is the word %s\n", buffer, input_word);
        printf("Expected value was %s which is the word %s\n" ,morsewords[current_test_value], microprocessorTerms[current_test_value]);
        
        /// Enact the desired functionality if the input is correct
        if(correct) 
        {
            /// Print that this answer was correct
            printf("This answer was CORRECT!!! :D\n");

            /// Increase the score values after each correct answer
            score++;
            overall_score++;

            /// If lives_left is less than 3 increase the number of remaining lives and update the LED
            if(lives_left < 3) 
            {
                lives_left++;
                LED_colour_update();
            }

            /// If the player has answered 5 correct answers they will move on to the next level
            if(score == 5) 
            {
                /// Print out a congratulations message and advance to the next level
                printf("Congratulations! You have completed the game. Great Job! :D\n");
                scoreboard();
                start_flag = 1;
                return;
            }
        }

        /// This is the functionality enacted if the input wasn't correct
        else 
        {
            /// Print that this answer was incorrect
            printf("This answer was incorrect. :(\n");
            
            /// Decrease the value of lives_left, update the LED colour, and then reset the score to zero
            lives_left--;
            overall_incor++;
            LED_colour_update();
            score = LEVEL_START_SCORE;

            /// If all lives have been lost finish the game;
            if(lives_left == 0) {
                endScreen();
                lives_left--; /// Decrement lives_left so that it become -1 which will update the LED to blue
                start_flag = 1;
                return;
            }
        }

    }
}

/**
* @brief Function to handle input from a Morse code device and convert it into a word.
* This function using uses a loop to constantly check for input and updates the word as the input is received.
* The function assumes that a space character or a certain amount of time without input indicates the end of an input.
* The function sets the wordIndex variable to 0 to start a new word, and updates the index as the input is received.
* @return None
*/
void input() {
    /// Declare necessary parameters for the function
    wordIndex = 0;
    int inputStarted = 0;
    uint64_t timething = 0;
    int pastIndex = 0;

    /// Enter the forever for loop which will keep looping until a word is determined finished by a pause of a certain length
    for(;;) {
        /// If there's no input yet just wait
        if (wordIndex == 0) {
        } 
        
        /// If an input is detected or the word has already started to be read in go here
        else {
            /// If the input has not been started set the first morse representation of the word to be the one input
            if (!inputStarted) {
                /// If true first character input, expected: wordIndex = 1
                inputStarted = 1;
                pastIndex = wordIndex;
                timething = time_us_64();
            } 
            
            /// If the input has already been started enter here
            else {
                /// if index hasnt changed
                if (pastIndex == wordIndex) {
                    /// if it has been SPACE_TIME since word update assume its space/done
                    if ((time_us_64() - timething) >= SPACE_TIME) {
                        if (wordIndex < 10) {
                            word[wordIndex] = '\0';
                        }
                        break;
                    }
                } 
                
                /// if index has changed, add another dot/dash to the word and update time for space
                else {
                    pastIndex = wordIndex;
                    timething = time_us_64();
                }
            }
        }
    }


}

/// @brief Function to adjust the RGB LED to suitably adjust its display
/// @param pixel_grb 
/// @return none
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

/**
 * @brief Convert RGB color components to a packed 32-bit unsigned integer.
 * Given the Red, Green, and Blue components of a color represented as 8-bit unsigned integers, this function returns a 32-bit unsigned integer that represents that same colour.
 * This is necessary for compatibility with other functions elsewhere in the code
 *
 * @param r The Red component of the color (0-255).
 * @param g The Green component of the color (0-255).
 * @param b The Blue component of the color (0-255).
 *
 * @return A 32-bit unsigned integer that represents the desired color in a packed format.
 */
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return  ((uint32_t) (r) << 8)  |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}
