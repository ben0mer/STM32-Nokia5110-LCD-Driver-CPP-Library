/***********************************************************
 * Author: Ömer Gökyer
 * GitHub: https://github.com/your_username/your_repository
 * 
 * This is a Nokia 5110 84x48 LcdDriver Library.
 * C++ project
 ************************************************************/

#include "main.h"
#include <Project/projectMain.h>
#include <Project/LcdDriver.hpp>

LcdDriver lcd; 

void print_examples(int num) {

    switch (num)
    {
    case 0:
        lcd.clear();
        lcd.put_char_xy(main_gui, 0, 0);
        break;
    
    case 1:
        lcd.clear();
        lcd.put_char_xy(menu_gui, 0, 0);
        lcd.put_char_xy(arrow_char, 6, 20);
        lcd.print_buffer("Omer", 20, 20, Default);
        lcd.invert(true);
        lcd.print_buffer("ben0mer", 21, 6, FontDefault);
        lcd.print_buffer("Gokyer", 21, 36, FontDefault);
        lcd.invert(false);
        break;

    case 2:
        lcd.clear();
        lcd.put_char_xy(menu_gui, 0, 0);
        lcd.put_char_xy(arrow_char, 6, 20);
        lcd.print_buffer("ben0mer", 20, 20, Default);
        lcd.invert(true);
        lcd.print_buffer("GitHub", 21, 6, FontDefault);
        lcd.print_buffer("Omer", 21, 36, FontDefault);
        lcd.invert(false);
        break;

    case 3:
        lcd.clear();
        lcd.put_char_xy(menu_gui, 0, 0);
        lcd.put_char_xy(arrow_char, 6, 20);
        lcd.print_buffer("GitHub", 20, 20, Default);
        lcd.invert(true);
        lcd.print_buffer("Gokyer", 21, 6, FontDefault);
        lcd.print_buffer("ben0mer", 21, 36, FontDefault);
        lcd.invert(false);
        break;

    case 4:
        lcd.clear();
        lcd.put_char_xy(menu_gui, 0, 0);
        lcd.put_char_xy(arrow_char, 6, 20);
        lcd.print_buffer("Gokyer", 20, 20, Default);
        lcd.invert(true);
        lcd.print_buffer("Omer", 21, 6, FontDefault);
        lcd.print_buffer("GitHub", 21, 36, FontDefault);
        lcd.invert(false);
        break;
    }
}

void projectMain()
{

    lcd.set_pin(GPIOB, GPIO_PIN_14, "RST");
    lcd.set_pin(GPIOB, GPIO_PIN_13, "CE");
    lcd.set_pin(GPIOB, GPIO_PIN_12, "DC");
    lcd.set_pin(GPIOB, GPIO_PIN_10, "DIN");
    lcd.set_pin(GPIOB, GPIO_PIN_11, "CLK");

    lcd.init();

    while (true)
    {
        print_examples(0);
        HAL_Delay(2000);
        print_examples(1);
        HAL_Delay(2000);
        print_examples(2);
        HAL_Delay(2000);
        print_examples(3);
        HAL_Delay(2000);
        print_examples(4);
        HAL_Delay(2000);
        print_examples(1);
        HAL_Delay(2000);
    }
}
