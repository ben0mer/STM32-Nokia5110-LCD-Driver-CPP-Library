
/**
 * @file LcdDriver.hpp
 * @brief This file contains the declaration of the LcdDriver class.
 * 
 * The LcdDriver class provides functions to control an LCD display.
 * It supports setting pins, initializing the display, clearing the screen,
 * printing text, drawing lines, and more.
 * 
 * @author Ömer Gökyer
 * @date [28.08.2024]
 * @version 1.0
 */

#include <stdint.h>
#include <string.h>

#include "stm32f4xx_hal.h"
#include "font.h"
#include "custom_char.h"

class LcdDriver {

    public:

        /**
         * @brief Sets the pin for a specific function in the LCD driver.
         * 
         * This function allows you to set the pin for a specific function in the LCD driver.
         * The available pin names are "RST", "CE", "DC", "DIN", and "CLK".
         * 
         * @param PORT The GPIO port to which the pin belongs.
         * @param PIN The pin number.
         * @param pinName The name of the pin function. It can be "RST", "CE", "DC", "DIN", or "CLK".
         */
        void set_pin(GPIO_TypeDef* PORT, uint16_t PIN, const char* pinName){

            if (strcmp(pinName, "RST") == 0){

                pins.RSTPORT = PORT;
                pins.RSTPIN = PIN;
            }
            else if (strcmp(pinName, "CE") == 0){

                pins.CEPORT = PORT;
                pins.CEPIN = PIN;
            }
            else if (strcmp(pinName, "DC") == 0){

                pins.DCPORT = PORT;
                pins.DCPIN = PIN;
            }
            else if (strcmp(pinName, "DIN") == 0){

                pins.DINPORT = PORT;
                pins.DINPIN = PIN;
            }
            else if (strcmp(pinName, "CLK") == 0){

                pins.CLKPORT = PORT;
                pins.CLKPIN = PIN;
            }
        }

        /**
         * @brief Initializes the LCD driver.
         * 
         * This function initializes the LCD driver by performing the following steps:
         * 1. Resets the LCD by writing a low level signal to the reset pin, then a high level signal.
         * 2. Sends extended commands to the LCD.
         * 3. Sets the LCD Vop (Contrast).
         * 4. Sets the temperature coefficient.
         * 5. Sets the LCD bias mode.
         * 6. Sends basic commands to the LCD.
         * 7. Sets the LCD display to normal mode.
         * 8. Clears the LCD display.
         * 9. Sets the inverttext flag to false.
         * 
         * @note This function assumes that the necessary GPIO pins and HAL library have been properly configured.
         * 
         * @usage
         * LcdDriver lcd;
         * lcd.init();
         */
        void init(){

            HAL_GPIO_WritePin(pins.RSTPORT, pins.RSTPIN, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(pins.RSTPORT, pins.RSTPIN, GPIO_PIN_SET);

            write(0x21, LCD_COMMAND); // LCD extended commands
            write(0xB8, LCD_COMMAND); // Set LCD Vop(Contrast)
            write(0x04, LCD_COMMAND); // Set temp coefficent
            write(0x12, LCD_COMMAND); // LCD bias mode
            write(0x20, LCD_COMMAND); // LCD basic commands
            write(LCD_DISPLAY_NORMAL, LCD_COMMAND); // LCD normal

            clear();
            inverttext = false;
        }

        /**
         * @brief Clears the LCD display by setting all pixels to 0x00.
         * 
         * This function clears the LCD display by writing 0x00 to each pixel in the buffer.
         * The buffer is then updated with the new pixel values.
         * 
         * @note This function does not update the physical LCD display. To update the display,
         *       the `updateDisplay()` function should be called after clearing the buffer.
         * 
         */
        void clear(){

            for (uint16_t i = 0; i < LCD_SIZE; i++){

                write(0x00, LCD_DATA);
                buffer[i] = 0x00;
            }
        }

        /**
         * @brief Prints a string on the LCD display using the buffer.
         * 
         * This function prints a string on the LCD display using the buffer and the specified font data.
         * The string is printed starting from the specified position (x, y) on the LCD display.
         * The font data is provided as a 2D array of type T with dimensions N x M.
         * 
         * @param str The string to be printed.
         * @param x The starting x position on the LCD display.
         * @param y The starting y position on the LCD display. This parameter should be in the range of 0 to 48.
         * @param fontData The font data used for printing the string. It should be a 2D array of type T with dimensions N x M.
         * 
         * @note This function assumes that the necessary GPIO pins and HAL library have been properly configured.
         * 
         * @usage
         * lcd.print_buffer("Hello World!", 0, 0, fontData);
         * lcd.refresh_screen();
         */
        template <typename T, size_t N, size_t M>
        void print_buffer(const char* str, uint8_t x, uint8_t y, const std::array<std::array<T, N>, M>& fontData) {

            uint8_t char_width = N;

            while (*str) {

                uint8_t shift_value = y%8; 
                uint8_t affected_rows = find_affected_rows(y, fontData[0].size());
                uint8_t bit_count = count_bits(affected_rows);  /* Find how many bits are 1 in affected_rows */
                uint8_t new_data[char_width * bit_count]{0x00};

                char c = *str;
                if (c < ' ' || c > '~') { c = ' '; } // Unsupported character
                
                shift_data(fontData[c - ' '].data(), char_width, new_data, shift_value, bit_count);
                write_to_buffer(x, affected_rows, (new_data), char_width, bit_count, false);
                refresh_screen();

                x += N; // Move to the next character position
                str++;
            }
        }

        /**
         * @brief Prints a string on an LCD display using a custom font.
         * 
         * This function prints a string on an LCD display using a custom font. The font data is provided as a 2D array of type T with dimensions N and M.
         * 
         * @param str The string to be printed.
         * @param x The x-coordinate of the starting position.
         * @param y The y-coordinate of the starting position. This parameter should be in the range of 0 to 5.
         * @param fontData The 2D array of font data.
         * 
         * @tparam T The type of the font data.
         * @tparam N The number of columns in the font data.
         * @tparam M The number of rows in the font data.
         */
        template <typename T, size_t N, size_t M>
        void print(const char* str, uint8_t x, uint8_t y, const std::array<std::array<T, N>, M>& fontData) {

            while (*str) {

                char c = *str;
                if (c < ' ' || c > '~') {

                    c = ' '; // Unsupported character
                }
                
                const std::array<T, N>& charData = fontData[c - ' '];
                int shift = get_shift_value(fontData);
                for (; shift >= 0; shift -= 8) {

                    setXY(x, static_cast<uint8_t>(y + (shift / 8)));
                    for (size_t i = 0; i < N; i++) {

                        uint8_t data = static_cast<uint8_t>((charData[i] >> shift) & 0xFF);
                        write(data, LCD_DATA);
                        
                    }
                }
                x += N; // Move to the next character position
                str++;
            }
        }

        /**
         * @brief Sets the X and Y coordinates of the LCD cursor.
         * 
         * This function sets the X and Y coordinates of the LCD cursor. The X coordinate represents the column number,
         * while the Y coordinate represents the row number. The function also updates the internal cursor position
         * variables (_cursor_x and _cursor_y) accordingly.
         * 
         * @param x The X coordinate of the cursor.
         * @param y The Y coordinate of the cursor.
         */
        void setXY(uint8_t x, uint8_t y){

            write(LCD_SETYADDR | y, LCD_COMMAND);
            write(LCD_SETXADDR | x, LCD_COMMAND);
            _cursor_x = x;
            _cursor_y = y;
        }
/*
        template <size_t N>
        void put_char(const unsigned char (*c)[N], uint8_t x, uint8_t y){
                        
            setXY(x, y);
            for (size_t i = 0; i < N; i++){

                write((*c)[i], LCD_DATA);                
            }
        }
*/


        /**
         * @brief Puts a character on the LCD screen at the specified coordinates.
         * 
         * This function takes a custom character and its coordinates (x, y) as input and displays it on the LCD screen.
         * The character is shifted vertically by the value of y modulo 8. The affected rows are determined by finding the rows
         * in the LCDHEIGHT/8 range that are affected by the character. The affected rows are represented by a bit value of 1 in
         * the affected_rows variable. The number of bits that are set to 1 in affected_rows is counted using the count_bits function.
         * A new data array is created with a size of c.char_width * bit_count, initialized with 0x00. The character data is then
         * shifted using the shift_data function, and the resulting data is written to the LCD buffer using the write_to_buffer function.
         * Finally, the screen is refreshed to display the updated content.
         * 
         * @tparam custom_char The type of the custom character.
         * @param c The custom character to be displayed.
         * @param x The x-coordinate of the character on the LCD screen.
         * @param y The y-coordinate of the character on the LCD screen.
         */
        template <typename custom_char>
        void put_char_xy(custom_char c , uint8_t x, uint8_t y){

            /* Defining local verables */
            uint8_t shift_value = y%8; 
            uint8_t affected_rows = find_affected_rows(y, c.char_height); /* Find the affected rows using c.char_height and y in LCDHEIGHT/8 rows. Affected rows bit will be 1 in affeced_rows variable. */
            uint8_t bit_count = count_bits(affected_rows); /* Find how many bits are 1 in affected_rows */
            uint8_t new_data[c.char_width * bit_count]{0x00};

            shift_data(c.data, c.char_width, new_data, shift_value, bit_count);
            write_to_buffer(x, affected_rows, new_data, c.char_width, bit_count, false);

            refresh_screen();

        }

        /**
         * @brief Clears a specified area on the LCD screen.
         *
         * This function clears a rectangular area on the LCD screen starting from the specified coordinates (x, y) with the given width and height.
         * It iterates over each pixel within the specified area and sets them to the inactive state.
         *
         * @param x The x-coordinate of the top-left corner of the area.
         * @param y The y-coordinate of the top-left corner of the area.
         * @param width The width of the area.
         * @param height The height of the area.
         */
        void clear_area(uint8_t x, uint8_t y, uint8_t width, uint8_t height){

            for (uint8_t i = 0; i < height; i++){

                for (uint8_t j = 0; j < width; j++){

                    set_pixel(x + j, y + i, false);
                }
            }
        }

        /**
         * @brief Writes data to the buffer of the LCD driver.
         * 
         * This function writes the specified data to the buffer of the LCD driver. The data is written starting from the specified x position and affects the specified rows in the buffer. The data is provided as an array of bytes, with each byte representing a column of pixels. The number of affected rows, the width of each character, the number of bits per column, and the character mode are also specified as parameters.
         * 
         * @param x The starting x position in the buffer.
         * @param affected_rows The bitmask representing the affected rows in the buffer.
         * @param new_data Pointer to the array of new data to be written.
         * @param char_width The width of each character in pixels.
         * @param bit_count The number of bits per column.
         * @param char_mode The character mode flag.
         * 
         * @note The function assumes that the buffer and other necessary variables are already defined and accessible.
         * @note The function modifies the buffer based on the provided parameters.
         */
        void write_to_buffer(int x, uint8_t affected_rows, const uint8_t* new_data, int char_width, uint8_t bit_count, bool char_mode) { 

            int new_data_row = bit_count - 1;
            for ( int i = 0; i < bit_count; i++){

                    volatile uint8_t row = 0;
                    for ( int j = 7; j > -1; j--){

                        if (affected_rows & (1 << j)){

                            row = j;
                            // reset the bit
                            affected_rows &= ~(1 << j);
                            break;
                        }
                    }
                    
                    for (int j = 0; j < char_width; j++) {
                        
                        if (inverttext && char_mode) {

                            buffer[x + (row * LCD_WIDTH) + j] |= ~(new_data[j + (new_data_row * char_width)]);
                        }
                        if (inverttext && !char_mode) {

                            buffer[x + (row * LCD_WIDTH) + j] &= ~(new_data[j + (new_data_row * char_width)]);
                        }
                        else {
                            buffer[x + (row * LCD_WIDTH) + j] |= new_data[j + (new_data_row * char_width)];
                        }
                    }
                    new_data_row--;
                }
        }

        /**
         * @brief Shifts the data in the given array by the specified shift value.
         * 
         * This function is used to shift the data in the input array by the specified shift value. The shifted data is stored in the output array.
         * 
         * @param data The input array containing the data to be shifted.
         * @param char_width The width of each character in the data array.
         * @param new_data The output array to store the shifted data.
         * @param shift_value The number of bits to shift the data by.
         * @param bit_count The number of bits in each character of the data array.
         * 
         * @note The function assumes that the input and output arrays have sufficient memory allocated.
         * 
         * @return None.
         */
        void shift_data(const uint8_t* data, uint8_t char_width, uint8_t* new_data, uint8_t shift_value, uint8_t bit_count){  

            if ( shift_value == 0 ){

                for (int i = 0; i < char_width; i++) {

                    for (int j = 0; j < bit_count; j++) {

                        new_data[i + (j * char_width)] = data[i + (j * char_width)];
                    }
                }
                return;
            }
    
            else if ( bit_count != 1 ){

                for (int i = 0; i < char_width; i++) {

                    for (int j = bit_count - 1; j > 0; j--) {

                        new_data[i + (j * char_width)] |= (data[i + ((j - 1) * char_width)] >> (8 - shift_value));
                        new_data[i + ((j - 1) * char_width)] |= (data[i + ((j - 1) * char_width)] << shift_value);
                    }
                }
                return;
            }
            else {

                // 8 bitlik verileri shift_value kadar kaydır
                for (int i = 0; i < char_width; i++) {

                    new_data[i] = (data[i] >> shift_value);
                }
                return;
            }
            
        }

        /**
         * @brief Refreshes the screen by writing the contents of the buffer to the LCD.
         * 
         * This function sets the X and Y address of the LCD and then writes the contents of the buffer
         * to the LCD in a row-by-row manner. The buffer is a 1-dimensional array representing the pixels
         * of the LCD screen. The function iterates over each row and writes the corresponding pixels to
         * the LCD.
         * 
         * @note This function assumes that the `setXY` and `write` functions are properly implemented
         * and accessible within the scope of this function.
         */
        void refresh_screen(){

            setXY(LCD_SETXADDR, LCD_SETYADDR);
            for(int i = 0; i < 6; i++){

                for(int j = 0; j < LCD_WIDTH; j++){
                write(buffer[(i * LCD_WIDTH) + j], LCD_DATA);
                }
            }
        }

        /**
         * @brief Inverts the display mode of the LCD driver.
         * 
         * This function allows the user to invert the display mode of the LCD driver.
         * When the mode is set to true, the display will be inverted. When the mode is set to false,
         * the display will be normal.
         * 
         * @param mode The display mode to set. True for inverted display, false for normal display.
         */
        void invert(bool mode){

            inverttext = mode;
        }

        /**
         * @brief Sets the value of a pixel at the specified coordinates.
         *
         * This function sets the value of a pixel at the specified coordinates in the LCD buffer.
         * The pixel is represented by a boolean value, where true represents an "on" pixel and false represents an "off" pixel.
         *
         * @param x The x-coordinate of the pixel.
         * @param y The y-coordinate of the pixel.
         * @param value The value to set for the pixel (true for "on", false for "off").
         */
        void set_pixel(uint8_t x, uint8_t y, bool value){

            if(value){

                buffer[x + (y / 8) * LCD_WIDTH] |= 1 << (y % 8);
            }
            else{

                buffer[x + (y / 8) * LCD_WIDTH] &= ~(1 << (y % 8));
            }
        }

        /**
         * @brief Draws a horizontal line on the LCD screen.
         * 
         * This function draws a horizontal line on the LCD screen starting from the specified coordinates (x, y) with the specified length (l).
         * The line is drawn by setting the corresponding bits in the buffer array.
         * 
         * @param x The x-coordinate of the starting point of the line.
         * @param y The y-coordinate of the starting point of the line.
         * @param l The length of the line to be drawn.
         */
        void draw_H_line(int x, int y, int l){
            
            int by, bi;
            if ((x>=0) && (x<LCD_WIDTH) && (y>=0) && (y<LCD_HEIGHT)){

                for (int cx=0; cx<l; cx++){

                    by=((y/8)*84)+x;
                    bi=y % 8;
                    buffer[by+cx] |= (1<<bi);
                }
            }
        }

        /**
         * @brief Draws a vertical line on the LCD screen.
         * 
         * This function draws a vertical line on the LCD screen starting from the specified coordinates (x, y) and extending downwards for a given length.
         * 
         * @param x The x-coordinate of the starting point of the line.
         * @param y The y-coordinate of the starting point of the line.
         * @param l The length of the line to be drawn.
         * 
         * @note The function only draws the line if the starting coordinates (x, y) are within the valid range of the LCD screen (0 <= x < 84, 0 <= y < 48).
         */
        void draw_V_line(int x, int y, int l){

            if ((x>=0) && (x<84) && (y>=0) && (y<48)){

                for (int cy=0; cy<= l; cy++){

                    set_pixel(x, y+cy, true);
                }
            }
        }


    private:

        /**
         * @brief Reverses the bits of a given 8-bit number.
         * 
         * This function takes an 8-bit number as input and reverses the order of its bits.
         * The reversed number is then returned.
         * 
         * @param n The 8-bit number to reverse the bits of.
         * @return The reversed number with the bits in reverse order.
         */
        uint8_t reverse_bits(uint8_t n) {

            n = (n & 0xF0) >> 4 | (n & 0x0F) << 4;
            n = (n & 0xCC) >> 2 | (n & 0x33) << 2;
            n = (n & 0xAA) >> 1 | (n & 0x55) << 1;
            return n;
        }
        
        /**
         * @brief Sends a byte of data to the LCD driver.
         *
         * This function sends a byte of data to the LCD driver using the specified pins.
         * It writes the data bit by bit, starting from the most significant bit (MSB) to the least significant bit (LSB).
         *
         * @param data The byte of data to be sent.
         */
        void send(uint8_t data){

            for (int i = 0; i < 8; i++){

                HAL_GPIO_WritePin(pins.DINPORT, pins.DINPIN, (data & 0x80) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                HAL_GPIO_WritePin(pins.CLKPORT, pins.CLKPIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(pins.CLKPORT, pins.CLKPIN, GPIO_PIN_SET);
                data <<= 1;
            }
        }

        /**
         * @brief Writes data to the LCD driver.
         * 
         * This function is used to write data to the LCD driver. It takes two parameters: 
         * the data to be written and the mode of operation (LCD_COMMAND or LCD_DATA).
         * 
         * If the mode is LCD_COMMAND, the function sets the DC (Data/Command) pin to low, 
         * sets the CE (Chip Enable) pin to low, sends the data, and then sets the CE pin to high.
         * 
         * If the mode is LCD_DATA, the function sets the DC pin to high, sets the CE pin to low, 
         * sends the data, and then sets the CE pin to high.
         * 
         * @param data The data to be written to the LCD driver.
         * @param mode The mode of operation (LCD_COMMAND or LCD_DATA).
         */
        void write(uint8_t data, uint8_t mode){

            if (LCD_COMMAND == mode){

                HAL_GPIO_WritePin(pins.DCPORT, pins.DCPIN, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(pins.CEPORT, pins.CEPIN, GPIO_PIN_RESET);
                send(data);
                HAL_GPIO_WritePin(pins.CEPORT, pins.CEPIN, GPIO_PIN_SET);
            } 
            
            else if (LCD_DATA == mode){

                HAL_GPIO_WritePin(pins.DCPORT, pins.DCPIN, GPIO_PIN_SET);
                HAL_GPIO_WritePin(pins.CEPORT, pins.CEPIN, GPIO_PIN_RESET);
                send(data);
                HAL_GPIO_WritePin(pins.CEPORT, pins.CEPIN, GPIO_PIN_SET);
            }
        }
        
        /**
         * @brief Writes data to the screen at the specified position.
         * 
         * This function writes the specified data to the screen at the given position (x, row).
         * The affected_rows parameter is a bitmask indicating which rows are affected by the data.
         * The new_data parameter is a pointer to the data to be written.
         * The char_width parameter specifies the width of each character in the data.
         * The bit_count parameter specifies the number of bits in the data.
         * 
         * @param x The x-coordinate of the position to write the data.
         * @param affected_rows A bitmask indicating which rows are affected by the data.
         * @param new_data Pointer to the data to be written.
         * @param char_width The width of each character in the data.
         * @param bit_count The number of bits in the data.
         */
        void write_to_screen(int x, uint8_t affected_rows, const uint8_t* new_data, int char_width, uint8_t bit_count) {
            
            for (int i = 0; i < bit_count; i++) {
                
                uint8_t row = 0;
                for (int j = 7; j > -1; j--) {

                    if (affected_rows & (1 << j)) {

                        row = j;
                        affected_rows &= ~(1 << j);
                        break;
                    }
                }

                setXY(x, row);
                for (int j = 0; j < char_width; j++) {

                    write(new_data[j + i * char_width], LCD_DATA);
                }
            }
        }



        /* Helper function for write_buffer */
        /**
         * @brief Calculates the shift value based on the font data.
         * 
         * This function takes a font data array and determines the appropriate shift value based on the size of the font.
         * 
         * @tparam T The type of elements in the font data array.
         * @tparam N The number of columns in the font data array.
         * @tparam M The number of rows in the font data array.
         * @param fontData The font data array.
         * @return The shift value calculated based on the font data.
         */
        template <typename T, size_t N, size_t M>
        int get_shift_value(const std::array<std::array<T, N>, M>& fontData){

            if (arrays_equal(fontData, FontMega) || arrays_equal(fontData, FontHuge)) {

                return 24;
            }
            else if (arrays_equal(fontData, FontLarge)) {

                return 8;
            }
            else {

                return 0;
            }
        }
        


        /* Helper function for font array matching */
        /**
         * @brief Compares two 2D arrays for equality.
         * 
         * This function compares two 2D arrays of different types and sizes for equality. 
         * It returns true if the arrays have the same dimensions and all corresponding elements are equal, 
         * otherwise it returns false.
         * 
         * @tparam T The type of elements in the first array.
         * @tparam N The number of columns in the first array.
         * @tparam M The number of rows in the first array.
         * @tparam U The type of elements in the second array.
         * @tparam P The number of columns in the second array.
         * @tparam Q The number of rows in the second array.
         * @param a The first 2D array to compare.
         * @param b The second 2D array to compare.
         * @return true if the arrays are equal, false otherwise.
         */
        template <typename T, size_t N, size_t M, typename U, size_t P, size_t Q>
        bool arrays_equal(const std::array<std::array<T, N>, M>& a, const std::array<std::array<U, P>, Q>& b) {

            if (M != Q || N != P) {

                return false;
            }
            for (size_t i = 0; i < M; ++i) {

                for (size_t j = 0; j < N; ++j) {

                    if (a[i][j] != b[i][j]) {

                        return false;
                    }
                }
            }
            return true;
        }

        /**
         * @brief Counts the number of set bits in a given 8-bit number.
         *
         * This function counts the number of set bits (bits with value 1) in the given 8-bit number.
         *
         * @param n The 8-bit number to count the set bits.
         * @return The number of set bits in the given number.
         */
        uint8_t count_bits(uint8_t n) {

            uint8_t bit_count = 0;
            for (int i = 0; i < 8; i++) {

                if (n & (1 << i)) {

                    bit_count++;
                }
            }
            return bit_count;
        }

        /* Helper function for write_buffer */
        /**
         * @brief Finds the affected rows on an LCD display based on the given y-coordinate and character height.
         * 
         * This function calculates the rows that will be affected on an LCD display based on the given y-coordinate and character height.
         * It returns a uint8_t value representing the affected rows.
         * 
         * @param y The y-coordinate of the starting position.
         * @param char_height The height of the character.
         * @return The affected rows as a uint8_t value.
         */
        uint8_t find_affected_rows(uint8_t y, uint8_t char_height){

            volatile uint8_t affected_rows = 0;
            for ( int i = 0; i < LCD_HEIGHT; i+=8 ){

                if ( y < i + 8 && y >= i){

                    affected_rows |= 1 << (i/8); // start row
                }
                if ( (y + char_height - 1) < i + 8 && (y + char_height ) >= i){

                    affected_rows |= 1 << (i/8); // end row
                }
            }
            // set the bits between start and end row
            for ( int i = (y/8) + 1; i < (y + char_height - 1)/8; i++){

                affected_rows |= 1 << i;
            }
            return affected_rows;
        }


        struct Pins{

            GPIO_TypeDef* RSTPORT;
            GPIO_TypeDef* CEPORT;
            GPIO_TypeDef* DCPORT;
            GPIO_TypeDef* DINPORT;
            GPIO_TypeDef* CLKPORT;
            uint16_t RSTPIN;
            uint16_t CEPIN;
            uint16_t DCPIN;
            uint16_t DINPIN;
            uint16_t CLKPIN;
        };

        Pins pins;

        uint8_t LCD_COMMAND{0};
        uint8_t LCD_DATA{1};

        static const uint16_t LCD_WIDTH{84};
        static const uint16_t LCD_HEIGHT{48};
        static const uint16_t LCD_SIZE{LCD_WIDTH * LCD_HEIGHT / 8};
        uint8_t buffer[LCD_SIZE]{0x00};
        int _cursor_x{0};
        int _cursor_y{0};
        bool inverttext{false};


        uint8_t LCD_SETYADDR{0x40};
        uint8_t LCD_SETXADDR{0x80};
        uint8_t LCD_DISPLAY_BLANK{0x08};
        uint8_t LCD_DISPLAY_NORMAL{0x0C};
        uint8_t LCD_DISPLAY_ALL_ON{0x09};
        uint8_t LCD_DISPLAY_INVERTED{0x0D};

};
