#include <stdio.h>
#include "button.h"

void setup_sensors(void) {
    button_main();
}

void app_main(void)
{
    printf("Kompass igång");
    setup_sensors();
    for (;;) {

    }
}
