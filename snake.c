#include <curses.h>
#include <unistd.h>
#include <stdlib.h> 
#include <time.h>

#define INITIAL_SNAKE_LENGTH 5
#define BASE_DELAY 10000
#define MIN_DELAY 10

void draw_snake(int[], int[], int);
void move_snake(int[], int[], int, int, int, int*, int);
void generate_trophy(int*, int*, int*, int*, int, int);
void adjust_speed(int, int*);



int main() {
    int snake_length = INITIAL_SNAKE_LENGTH; // Sets initial snake length, which is defined as 5 characters.
    int snake_x[100]; //array to store the position of snake position x
    int snake_y[100]; //array to store the position of snake position y
    int direction; //initial direction of the snake is randomized
    int ch;
    int trophy_x, trophy_y, trophy_value, trophy_time; // initializes the variables needed for trophy
    int trophy_counter = 0; // initalizes trophy counter to 0.
    time_t start_time, current_time;

    //initializing some of curses functions
    initscr(); //initialize screen
    start_color(); //initialize color
    init_pair(1, COLOR_GREEN, COLOR_BLACK); //color the background black and the snake green
    init_pair(2, COLOR_RED, COLOR_BLACK);   //color the background black and the border red
    init_pair(3, COLOR_BLUE, COLOR_BLACK); //color the background black and the trophies blue
    noecho(); //dont echo user input
    curs_set(0); //hide cursor
    keypad(stdscr, TRUE); //initialize the ability to use keyboard for input
    nodelay(stdscr, TRUE);
    timeout(100);

//getting terminal size
    int max_y, max_x; //variables to get the length of the window
    getmaxyx(stdscr, max_y, max_x); //use curses function to initialize the width and height of the games window
    int WIDTH = max_x - 2;
    int HEIGHT = max_y - 2;
    //calculate the border perimeter
    int border_perimeter = 2 * (WIDTH - 2) + 2 * (HEIGHT - 2);
    int half_border_perimeter = border_perimeter / 2;
    int snake_perimeter = 0;

    srand(time(NULL)); //random number generator for random spawn sequences 
//calculate the coordinates of the middle of the terminal window
    int middle_x = max_x / 2;
    int middle_y = max_y / 2;
//initialize the snakes position in the middle of the terminal
    for (int i = 0; i < INITIAL_SNAKE_LENGTH; i++){
        snake_x[i] = middle_x - i;
        snake_y[i] = middle_y;
    }

//generate initial trophy
    generate_trophy(&trophy_x, &trophy_y, &trophy_value, &trophy_time, WIDTH, HEIGHT);
    start_time = time(NULL);

//randomize intial directions 
    int random_direction = rand() % 3;
//statement to set the direction base don random number
    switch (random_direction) {
        case 0:
            direction = KEY_UP; //moves the snake upward if the random number is 0
            break;
        case 1:
            direction = KEY_DOWN; //moves the snake downward if the random number is 1
            break;
        case 2:
            direction = KEY_RIGHT; //moves the snake to the right if the random number is 2
            break;
    }

//loop to run the game
    while ((ch = getch()) != 'x') { //while the user hasn't quit the game by pressing x, continue the game
        clear(); //clears the screen

        //draw the snake and border
        draw_snake(snake_x, snake_y, snake_length);
        //draw the trophy
        attron(COLOR_PAIR(3));
        mvprintw(trophy_y, trophy_x, "%d", trophy_value);
        attroff(COLOR_PAIR(3));
        //drawing the width of the border
        for (int i = 0; i < WIDTH; i++) {
            attron(COLOR_PAIR(2)); //coloring the width of snake games border red by turning on the attribute func
            mvprintw(0, i, "_");
            mvprintw(HEIGHT - 1, i, "_");
            attroff(COLOR_PAIR(2));
        }
        //drawing the height of the border
        for (int i = 1; i < HEIGHT - 1; i++) {
            attron(COLOR_PAIR(2)); //coloring the height of snake games border red by turning on the attribute func
            mvprintw(i, 0, "|");
            mvprintw(i, WIDTH - 1, "|");
            attroff(COLOR_PAIR(2));
        }

        //move the snake
        move_snake(snake_x, snake_y, direction, WIDTH, HEIGHT, &snake_length, trophy_counter);

        //generates a new trophy if the old one has expired
        current_time = time(NULL);
        if (current_time - start_time >= trophy_time){
            generate_trophy(&trophy_x, &trophy_y, &trophy_value, &trophy_time, WIDTH, HEIGHT);
            start_time = current_time;
        }

        //check if snake touched trophy
        if (snake_x[0] == trophy_x && snake_y[0] == trophy_y) {
            //increase the snake's length
            for (int i = snake_length - 1; i < snake_length + trophy_value - 1; i++) {
                snake_x[i] = snake_x[snake_length - 1];
                snake_y[i] = snake_y[snake_length - 1];
            }
            snake_length += trophy_value;
            trophy_counter += 1;
            generate_trophy(&trophy_x, &trophy_y, &trophy_value, &trophy_time, WIDTH, HEIGHT);
        }

    //checks if the snake has reached more than half of the borders perimeter
        snake_perimeter = 2 * snake_length;
        if (snake_perimeter > half_border_perimeter) {
            endwin();
            printf("Congratulations! You won the game by reaching more than half of the border's perimeter! You collected %d trophies.\n", trophy_counter);
            return 0;
        }


        refresh(); //refresh the screen

        //changes the direction of the snake based on user input
        switch (ch) {
            case KEY_UP: //the up arrow key will make the snake go up
                direction = KEY_UP;
                break;
            case KEY_DOWN: //the down arrow key will make the snake go down
                direction = KEY_DOWN;
                break;
            case KEY_LEFT: //the left arrow key will make the snake go left
                direction = KEY_LEFT;
                break;
            case KEY_RIGHT: //the right arrow key will make the snake go right
                direction = KEY_RIGHT;
                break;
        }

        //adjust game speed based on snake length to make the snake movement speed seem to change based on length
        int sleep_time = BASE_DELAY;
        adjust_speed(snake_length, &sleep_time);
        usleep(sleep_time); //pauses the screen to look like an animation
    }

    endwin(); //ends the game
    return 0;
}

//function to draw the snakes
void draw_snake(int x[], int y[], int length) {
    for (int i = 0; i < length; i++) {
        attron(COLOR_PAIR(1)); //turn on snakes color attribute
        mvprintw(y[i], x[i], "*"); //prints the snake
        attroff(COLOR_PAIR(1)); //turn off snakes color attribute
    }
}

//controls how the snake moves based on its position
void move_snake(int x[], int y[], int direction, int WIDTH, int HEIGHT, int* snake_length, int trophy_counter) {
    int new_x = x[0];
    int new_y = y[0];

//update snakes head position base don direction
    switch (direction) {
        case KEY_UP:
            new_y--; //if the user input is up, decrease the vertical position (y) to move up
            break;
        case KEY_DOWN:
            new_y++; //if the user input is down, increase the vertical position (y) to move down
            break;
        case KEY_LEFT:
            new_x--; //if the user input is left, decrease the vertical position (x) to move left
            break;
        case KEY_RIGHT:
            new_x++; //if the user input is right, increase the horizontal position (x) to move right
            break;
    }

    //check if the snake hits the border
    if (new_x == 0 || new_x == WIDTH - 1 || new_y == 0 || new_y == HEIGHT - 1) {
        //snake hits the border, game over
        endwin();
        printf("Game Over: Snake hit the border. You collected %d trophies\n", trophy_counter);
        printf("");
        exit(0);
    }

    //checks if the snake hits itself
    for (int i = 1; i < *snake_length; i++) {
        if (new_x == x[i] && new_y == y[i]) {
            //snake hits itself, game over
            endwin();
            printf("Game Over: Snake hit itself. You collected %d trophies.\n", trophy_counter);
            exit(0);
        }
    }

    //move the snake's body
    for (int i = *snake_length; i > 0; i--) {
        x[i] = x[i - 1];
        y[i] = y[i - 1];
    }

    //update the position of the snake's head
    x[0] = new_x;
    y[0] = new_y;
}

//generates trophy with random time properties
void generate_trophy(int* x, int* y, int* value, int* timer, int WIDTH, int HEIGHT) {
    *x = rand() % (WIDTH - 2) + 1; //random x position within the borders
    *y = rand() % (HEIGHT - 2) + 1; //random y position within the borders
    *value = rand() % 9 + 1; //random trophy value between 1 and 9
    *timer = rand() % 15 + 1; //random expiration time between 1 and 9 seconds
}

//function to adjust gaem speed based on hte snakes length
void adjust_speed(int snake_length, int* sleep_time){
    *sleep_time = BASE_DELAY - (snake_length * 1000);
//ensure sleep time doesnt go below minimum delay
    if (*sleep_time < MIN_DELAY){
        *sleep_time = MIN_DELAY;
    }
}

