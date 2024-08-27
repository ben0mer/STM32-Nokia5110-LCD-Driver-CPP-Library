# Nokia 5110 LCD Driver Library

This repository contains a C++ library for controlling a Nokia 5110 84x48 LCD display using an STM32 microcontroller. The library provides various functions to initialize the display, clear the screen, add custom char, print text, draw lines, and more.

## Features

- Pin Configuration: Easily configure GPIO pins for the LCD display.
- Display Initialization: Initialize the LCD with various settings including contrast, bias mode, and temperature coefficient.
- Text Display: Print text on the screen using custom fonts.
- Graphics Drawing: Draw horizontal and vertical lines, custom characters, and more.
- Screen Management: Clear the screen, refresh the display, and invert text.

## Installation

1. Clone the repository:
    ```sh
    git clone https://github.com/ben0mer/ProjectName.git
    ```

2. Include the necessary files in your project:
    - `Project/LcdDriver.hpp`
    - `Project/font.h`
    - `Project/custom_char.h`

3. Ensure that the STM32 HAL library is properly configured in your project.

## Usage

1. Include the `LcdDriver.hpp` header file in your main project file:
    ```cpp
    #include <Project/LcdDriver.hpp>
    ```

2. Create an instance of the `LcdDriver` class and initialize the LCD:
    ```cpp
    LcdDriver lcd;
    lcd.init();
    ```

3. Use the provided functions to control the LCD display:
    ```cpp
    lcd.clear();
    lcd.print("Hello, STM32!", 0, 0, fontData);
    ```

4. You can check the `Project/projectMain.cpp` file for more example and for my GUI example.


## API Reference

### `LcdDriver`

#### Methods

- `void setPin(GPIO_TypeDef* PORT, uint16_t PIN, const char* name)`
  - Sets the pin for a specific function in the LCD driver.

- `void init()`
  - Initializes the LCD driver.

- `void clear()`
  - Clears the LCD display by setting all pixels to 0x00.

- `void invert(bool mode)`
  - Inverts the display mode of the LCD driver.

- `uint8_t count_bits(uint8_t n)`
  - Counts the number of set bits in a given 8-bit number.

- `uint8_t reverse_bits(uint8_t n)`
  - Reverses the bits of a given 8-bit number.

- `template <typename T, size_t N, size_t M, typename U, size_t P, size_t Q> bool arrays_equal(const std::array<std::array<T, N>, M>& a, const std::array<std::array<U, P>, Q>& b)`
  - Compares two 2D arrays for equality.

- `void write(uint8_t data, uint8_t mode)`
  - Writes data to the LCD driver.

- `void send(uint8_t data)`
  - Sends a byte of data to the LCD driver.

- `uint8_t find_affected_rows(uint8_t y, uint8_t char_height)`
  - Finds the affected rows on an LCD display based on the given y-coordinate and character height.


Author: Ömer Gökyer