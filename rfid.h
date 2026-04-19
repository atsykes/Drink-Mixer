/* ------------------------------------------------------------
 * RC522 wiring for ATmega328P hardware SPI
 * ------------------------------------------------------------
 * MOSI -> PB3 (Arduino D11)
 * MISO -> PB4 (Arduino D12)
 * SCK  -> PB5 (Arduino D13)
 * SS/CS -> PB2 (Arduino D10)
 * RST  -> PD2 (can be changed)
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdbool.h>

#define RC522_CS_DDR   DDRB
#define RC522_CS_PORT  PORTB
#define RC522_CS_PIN   PB2

#define RC522_RST_DDR   DDRD
#define RC522_RST_PORT  PORTD
#define RC522_RST_PIN   PD2

/* ------------------------------------------------------------
 * RC522 registers
 * ------------------------------------------------------------ */
#define RC522_COMMAND_REG      0x01
#define RC522_COM_IEN_REG      0x02
#define RC522_DIV_IEN_REG      0x03
#define RC522_COM_IRQ_REG      0x04
#define RC522_DIV_IRQ_REG      0x05
#define RC522_ERROR_REG        0x06
#define RC522_STATUS1_REG      0x07
#define RC522_STATUS2_REG      0x08
#define RC522_FIFO_DATA_REG    0x09
#define RC522_FIFO_LEVEL_REG   0x0A
#define RC522_CONTROL_REG      0x0C
#define RC522_BIT_FRAMING_REG  0x0D
#define RC522_COLL_REG         0x0E
#define RC522_MODE_REG         0x11
#define RC522_TX_MODE_REG      0x12
#define RC522_RX_MODE_REG      0x13
#define RC522_TX_CONTROL_REG   0x14
#define RC522_TX_ASK_REG       0x15
#define RC522_T_MODE_REG       0x2A
#define RC522_T_PRESCALER_REG  0x2B
#define RC522_T_RELOAD_H_REG   0x2C
#define RC522_T_RELOAD_L_REG   0x2D
#define RC522_VERSION_REG      0x37

/* RC522 commands */
#define PCD_IDLE               0x00
#define PCD_CALCCRC            0x03
#define PCD_TRANSCEIVE         0x0C
#define PCD_SOFT_RESET         0x0F

/* PICC commands */
#define PICC_REQIDL            0x26
#define PICC_ANTICOLL          0x93

#define MI_OK                  0
#define MI_NOTAGERR            1
#define MI_ERR                 2

#define BAUD 9600UL
#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

void uart_init(void);
void uart_tx(char c);
void uart_print(const char *s);
void uart_print_hex8(uint8_t v);
void spi_init(void);
uint8_t spi_transfer(uint8_t data);
inline void cs_low(void);
inline void cs_high(void);
inline void rst_low(void);
inline void rst_high(void);
void rc522_write_reg(uint8_t reg, uint8_t value);
uint8_t rc522_read_reg(uint8_t reg);
void rc522_set_bitmask(uint8_t reg, uint8_t mask);
void rc522_clear_bitmask(uint8_t reg, uint8_t mask);
void rc522_antenna_on(void);
void rc522_reset(void);
void rc522_init(void);
uint8_t rc522_to_card(uint8_t command,
                            const uint8_t *send_data,
                            uint8_t send_len,
                            uint8_t *back_data,
                            uint16_t *back_len);
uint8_t rc522_request(uint8_t req_mode, uint8_t *tag_type);
uint8_t rc522_anticoll(uint8_t *ser_num);
void print_uid(const uint8_t *uid, uint8_t len);
void hardware_init(void);

