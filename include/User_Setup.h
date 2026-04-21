#define USER_SETUP_INFO "CUG_Badge_Setup"

// Wskazujemy sterownik z poradnika
#define ILI9341_DRIVER 

// Nasze piny z Wokwi diagram.json
#define TFT_MISO 5
#define TFT_MOSI 6
#define TFT_SCLK 4
#define TFT_CS   7
#define TFT_DC   10
#define TFT_RST  3

// Ładujemy podstawowe czcionki
#define LOAD_GLCD  
#define LOAD_FONT2 
#define LOAD_FONT4 

// Włączamy szybkie sprzętowe taktowanie (40 MHz) z poradnika
#define SPI_FREQUENCY  40000000