#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

// ==== i2c default pins =====
#define I2C_SCL1_PIN       6   // PB6
#define I2C_SDA1_PIN       7   // PB7

// ===== MC7RE-V2 SBUS (external inverter required) =====
#define RC_SBUS_RX_PIN PA10
#define RC_SBUS_TX_PIN PA9 // Reserved by USART1; leave unconnected

// ===== LED PIN ======
#define BUILTIN_LED_PORT GPIOC
#define BUILTIN_LED_PIN  13

// ===== BUZZER (PC13) =====
#define BUZZER_PORT       GPIOB
#define BUZZER_PIN        12

#define VBAT_PORT        GPIOB
#define VBAT_PIN         0

// ===== MOTORS (PA0–PA3) =====
#define MOTOR_PWM_PORT    GPIOA
#define MOTOR1_PWM_PIN    0  // TIM2_CH1 (Front left)
#define MOTOR2_PWM_PIN    1  // TIM2_CH2 (front right)
#define MOTOR3_PWM_PIN    2  // TIM2_CH3 (back left)
#define MOTOR4_PWM_PIN    3  // TIM2_CH4 (back right)

// ===== Function Prototypes =====
void Buzzer_Init(void);
uint16_t readVbat(void);
void MotorPWM_TIM2_Init(void);
void used_gpio_init(void);

#endif // PIN_CONFIG_H
