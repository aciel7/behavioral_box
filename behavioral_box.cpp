#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/rand.h"

#define LE_LEVER_PIN 0
#define HE_LEVER_PIN 1
#define NOSE_POKE_PIN 2
#define REWARD_PIN 3
#define SYNC_PIN 4

enum state {
    LE_TONE = 0,
    HE_TONE,
    LE_FAILED,
    HE_FAILED,
    LE_REWARD,
    HE_REWARD
};

int main_state;

void pulse_sync(){
    gpio_put(SYNC_PIN, 1);
    sleep_ms(1);
    gpio_put(SYNC_PIN, 0);
}

void deliver_reward(){
    gpio_put(REWARD_PIN, 1);
    sleep_ms(1);
    gpio_put(REWARD_PIN, 0);
}

void pick_tone_level(){
    uint32_t rand_num = get_rand_32();
    if (rand_num % 2){
        main_state = state::LE_TONE;
        printf("picking low effort\n");
    }
    else {
        main_state = state::HE_TONE;
        printf("picking high effort\n");
        }
    sleep_ms(500);
}

void state_handler(){
    pulse_sync();
    switch(main_state) {
        case state::LE_TONE :
            printf("state=le_tone\n");
            break;
        case state::HE_TONE :
            printf("state=he_tone\n");
            break;
        case state::LE_REWARD : 
            printf("state=le_reward\n");
            deliver_reward();
            break;
        case state::HE_REWARD : 
            printf("state=he_reward\n");
            deliver_reward();
            break;
        case state::LE_FAILED :
            printf("state=le_failed\n");
            pick_tone_level();
            state_handler();
        case state::HE_FAILED :
            printf("state=he_failed\n");
            pick_tone_level();
            state_handler();
    }

}

void nosepoke_cb (){

    printf("nose_poked\n");
    if (main_state == state::HE_REWARD || main_state == state::LE_REWARD) {
        pick_tone_level();
        state_handler();
    }
}


void le_lever_cb (){
    printf("le_lever_press\n");
    switch(main_state) {
        case state::LE_TONE :
            main_state = state::LE_REWARD; 
            state_handler();
            break; 
        case state::HE_TONE :
            main_state = state::LE_FAILED;
            state_handler();
            break;
    }
}

void he_lever_cb (){
    printf("he_lever_press\n");
    switch(main_state) {
        case state::LE_TONE :
            main_state = state::HE_FAILED; 
            state_handler();
            break; 
        case state::HE_TONE :
            main_state = state::HE_REWARD;
            state_handler();
            break;
    }
}

void gpio_callback(uint gpio, uint32_t event_mask){
    if (gpio == LE_LEVER_PIN) {
        le_lever_cb();
    }
    if (gpio == HE_LEVER_PIN) {
        he_lever_cb();
    }
    if (gpio == NOSE_POKE_PIN) {
        nosepoke_cb();
    }
    sleep_ms(200);
}


void initialize_gpio(){
    //inputs
    gpio_pull_up(LE_LEVER_PIN);
    gpio_pull_up(HE_LEVER_PIN);
    gpio_pull_up(NOSE_POKE_PIN);

    gpio_init(LE_LEVER_PIN);
    gpio_init(HE_LEVER_PIN);
    gpio_init(NOSE_POKE_PIN);

    //outputs
    gpio_set_dir(REWARD_PIN, true);
    gpio_set_dir(SYNC_PIN, true);
    gpio_init(REWARD_PIN);
    gpio_init(SYNC_PIN);
    
    gpio_put(REWARD_PIN, 0);
    gpio_put(SYNC_PIN, 0);

    //interrupts
    gpio_set_irq_enabled_with_callback(LE_LEVER_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(HE_LEVER_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(NOSE_POKE_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
}


int main()
{
    stdio_init_all();
    printf("test");
    initialize_gpio();
    pick_tone_level();
    state_handler();


    while (true) {}
}
