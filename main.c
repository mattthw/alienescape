/*
* @Author: Matthew McCoy
* @Date:   2016-03-31 11:33:09
* @Last Modified by:   matt
* @Last Modified time: 2016-04-06 21:03:12
*/
#include "myLib.h"
#include "person.h"
#include "heart.h"
#include "bg.h"
#include "planet.h"
#include "titlescreen.h"
#include "screenwin.h"
#include "screenlose.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum GBAState {
    START,
    START_NODRAW,
    STATE2,
    STATE3,
    STATE3_NODRAW,
    STATE4,
    STATE4_NODRAW
};

//ENUM PROTOTYPE HERE, BECAUSE C DOES NOT ALLOW FORWARD REFERENCES
//TO 'ENUM' TYPES.
enum GBAState gameStart(int numLives, int init, int difficulty);

int main() {
    REG_DISPCTL = MODE3 | BG2_ENABLE;
    enum GBAState state = START;

    int difficulty = 2;
    int wait = 0;
    while(1) {
        waitForVblank();
        switch(state) {
        case START: //START SCREEN
            //draw image
            drawImage3(0, 0, 240, 160, titlescreen_data);
            //draw press start
            drawString(68, 85, "PRESS START", WHITE);
            //draw difficulty indicator
            if (difficulty == 1) {
                drawString(85, 93, "< EASY >",WHITE);
            } else if (difficulty == 2) {
                drawString(85, 93, "<NORMAL>",WHITE);
            } else if (difficulty == 3) {
                drawString(85, 93, "< HARD >",WHITE);
            } else {
                drawString(5, 90, "MODE: ",WHITE);
                char str[1];
                sprintf(str, "%d", difficulty);
                drawString(5, 90, str,WHITE);
            }
            state = START_NODRAW;
            break;
        case START_NODRAW:
            //change state if press A
            if((KEY_DOWN_NOW(BUTTON_START) || KEY_DOWN_NOW(BUTTON_A)) && wait == 0) {
                state = STATE2;
                wait = 1;
            } else if (KEY_DOWN_NOW(BUTTON_RIGHT) && difficulty <= 3 && wait == 0) {
                difficulty++;
                state = START;
                wait = 1;
            } else if (KEY_DOWN_NOW(BUTTON_LEFT) && difficulty >= 1 && wait == 0) {
                difficulty--;
                state = START;
                wait = 1;
            }
            else {
                state = START_NODRAW;
            }
            break;
        case STATE2: //GAME START
            state = gameStart(3, 1, difficulty);
            break;
        case STATE3: //WIN SCREEN
            drawImage3(0, 0, 240, 160, screenwin_data);
            // drawString(100, 85, "PRESS START", WHITE);
            delay(1);
            state = STATE3_NODRAW;
            break;
        case STATE3_NODRAW:
            //check for butotn press
            if((KEY_DOWN_NOW(BUTTON_START) || KEY_DOWN_NOW(BUTTON_A) || KEY_DOWN_NOW(BUTTON_SELECT)) && wait == 0) {
                state = START;
                wait = 1;
            } else {
                state = STATE3_NODRAW;
            }
            break;
        case STATE4: //LOSE SCREEN
            drawImage3(0, 0, 240, 160, screenlose_data);
            drawString(100, 85, "PRESS START", WHITE);
            delay(1);
            state = STATE4_NODRAW;
            break;
        case STATE4_NODRAW:
            if((KEY_DOWN_NOW(BUTTON_START) || KEY_DOWN_NOW(BUTTON_A) || KEY_DOWN_NOW(BUTTON_SELECT)) && wait == 0) {
                state = START;
                wait = 1;
            } else {
                state = STATE4_NODRAW;
            }
            break;
        }
        //don't let user hold down button
        if (!KEY_DOWN_NOW(BUTTON_START) && !KEY_DOWN_NOW(BUTTON_A) &&
            !KEY_DOWN_NOW(BUTTON_RIGHT) && !KEY_DOWN_NOW(BUTTON_LEFT)) {
            wait = 0;
        }
        if (difficulty > 3) {
            difficulty = 1;
        } else if (difficulty < 1) {
            difficulty = 3;
        }
    }
    return 0;
}

/**
 * actual game mode for Alien Escape. called recursively
 * for eachj life lost until lives = 0 or player wins
 * @param  numLives   lives player has.
 * @param  init       1 if on first loop of call, to allow player to see
 *                    flying particle locations
 * @param  difficulty 1,2, or 3
 * @return            STATE3 or STATE4
 */
enum GBAState gameStart(int numLives, int init, int difficulty) {
    MAPOBJ mapobs = defineMapElems();
    //player
    SPRITE player = {
        .x = 2, .y = (160 - PLANET_HEIGHT - PERSON_HEIGHT),
        .width = PERSON_WIDTH, .height = PERSON_HEIGHT,
        .color = (BLACK), .jumping = 0, .lives = numLives
    };
    int jumpsize = player.height * 1.65;
    //enemies
    SPRITE enemy = {
        .x = 240, .y = player.y, .width = 3, .height = 3,
        .color = (YELLOW), .jumping = 0
    };
    SPRITE flock[3];
    for (int i = 0; i < 3; i++) {
        flock[i] = enemy;
        flock[i].x = (i + 1) * 240/3;
        int range = jumpsize;
        flock[i].y = (rand() % range) + (130 - jumpsize - enemy.height);
    }
    //initial variable settings
    int destination = player.y;
    int time = 1;
    int wait = 1;
    //wait for clear
    waitForVblank();
    //DRAW BG & PLAYER
    drawImage3(0, 0, 240, 130, bg_data);
    drawImage3(0, 130, 240, 30, planet_data);
    //draw lives
    if (numLives > 3)
        numLives = 3;
    if (numLives == 3)
        drawString(5, 204, "100%", WHITE);
    else if (numLives == 2)
        drawString(5, 210, "66%", WHITE);
    else if (numLives == 1)
        drawString(5, 210, "33%", WHITE);
    else
        return STATE4; //if lives <= 0
    drawImage3(194, 5, 7, 8, &heart_data[0]);


    //game loop
    while(1) {
        //increment time used in movement math
        if (time == 999) {
            time = 1;
        } else {
            time++;
        }
        //button listeners
        if(player.jumping == 0 &&
            (KEY_DOWN_NOW(BUTTON_UP) || KEY_DOWN_NOW(BUTTON_A)) &&
            wait == 0) {
            player.jumping = 1;
            wait = 1;
            destination = player.y - jumpsize;
        }
        if(KEY_DOWN_NOW(BUTTON_RIGHT)) {
            player.x += 2;
            if(player.x > 240)
                return STATE3;
        }
        if(KEY_DOWN_NOW(BUTTON_LEFT)) {
            if(player.x - 1 > 2)
                player.x -= 2;
        }
        if (KEY_DOWN_NOW(BUTTON_SELECT)) {
            return START;
        }



        //PLAYER VERTICAL MOVEMENT
        if (player.y > destination) {
            player.y -= 2;//go up
            if (player.y <= destination || player.y - 1 < 40) {
                destination = destination + jumpsize;
            }
        } else if (player.y < destination) {
            if (player.y != destination && player.y + 1 < mapobs.y) {
               player.y += 2;
               if (player.y == destination) {
                    player.jumping = 0;
               }
            }
        }
        //player win check
        if (player.x > 240 - player.width * 1.5) {
            return STATE3;
        }



        //movement/collision for enemy 1, 2, and 3
        for (int i = 0; i < 3; i++) {
            //collision detection
            if (flock[i].x >= player.x
                && flock[i].x <= player.x + player.width
                && flock[i].y  + 1 >= player.y
                && flock[i].y <= player.y + player.height) {
                player.lives -= 1;
                if (player.lives <= 0) {
                    return STATE4;
                } else {
                    return gameStart(player.lives, 1, difficulty);
                }
                break;
            }
            //y movement
            if (flock[i].x == 0) {
                int range = player.height;
                flock[i].y = (rand() % range) + (130 - jumpsize - enemy.height);
            }
            //x movement
            if (flock[i].x == 0) {
                flock[i].x = 240;
            } else {
                if (time % (5 - difficulty) == 0 || time < difficulty) {
                    flock[i].x -= 2;
                }
            }
        }
        //set 1 of 3 to always be below player height,
        //in case rand() puts other 2 above him
        flock[1].y = 125;



        //DRAW CONTENT TO SCREEN
        waitForVblank();
        //bg, heart, and player
        drawImage3(0, 65, 240, 65, &bg_data[240*65]);
        drawImage3(player.x, player.y, PERSON_WIDTH, PERSON_HEIGHT, person_data);
        //enemies/obstacles
        for (unsigned int i = 0; i < (sizeof(flock)/sizeof(enemy)); i++) {
            if (flock[i].x > flock[i].width && flock[i].x < 240 - flock[i].width) {
                drawRect(flock[i].y,flock[i].x,flock[i].height,flock[i].width,flock[i].color);
            }
        }


        //DELAY AFTER START, DEATH, AND RESUME
        if (init) {
            delay(1);
            init = 0;
        }
        //PREVENT HOLD JUMP
        if (!(KEY_DOWN_NOW(BUTTON_UP) || KEY_DOWN_NOW(BUTTON_A))) {
            wait = 0;
        }
    }
}