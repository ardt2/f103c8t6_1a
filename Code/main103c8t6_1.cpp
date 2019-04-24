// *****************************************************************************
//
//  Файл               : ./103c8t6_1/Code/main103c8t6.cpp
//
//  Дата правки        : 21 марта 2019 г.
//  Описание           : TODO Введи здесь описание функциональности файла
//  Лицензия           : Creative common 3.0
//
//
//  Автор Ardt2 (c) 2019
//
// *****************************************************************************
// Стремись к хорошему стилю C++ для встраиваимых систем.

// ============================================================================
#include "S103c8t6_1.h"
#include "StmPeriphery.h"


// ============================================================================
volatile uint32_t TSysTickTimer::Tick = 0; // Конструктор удален, но, слава богу,
                                           // static поле инициализируется здесь.
// ----------------------------------------------------------------------------
TSysTickTimer & SysTimer = *static_cast<TSysTickTimer *>TIM1;


// ============================================================================
TRcc & Rcc = *static_cast<TRcc *>RCC; // TODO: Если глобальных переменных станет
                                      // много, можно вынести их в отдельный файл.
// ----------------------------------------------------------------------------
TGPIO & PortA = *static_cast<TGPIO *>GPIOA;
TGPIO & PortB = *static_cast<TGPIO *>GPIOB;
TGPIO & PortC = *static_cast<TGPIO *>GPIOC;

// ------------------------------------------------------------------------
// <GPin::P6, GPin::P7, PortB>
TI2C & i2c1 = *static_cast<TI2C *>I2C1;
// ----------------------------------------------------------------------------
//TSI2C<GPin::P6, GPin::P7> Si2c1(PortB);


// ============================================================================
//TLed1637 Led1637;


// красный, красный с желтым, зеленый, зеленый мигающий, желтый, красный.
// При этом длительность сигнала "красный с желтым" рекомендуется устанавливать
// не более 2 с, длительность желтого сигнала — 3 с.
// ------------------------------------------------------------------------
struct TRoadSign1
{
    enum SignState
            { Red = 0, RedYellow, Green, GreenBlink, Yellow, };
    uint16_t SignDuration[5] =
            { 12000, 2000, 7000, 500, 3000};
    uint8_t GBlinkCount = 2;
};

// ----------------------------------------------------------------------------
TRoadSign1 RoadSign1;
TRoadSign1::SignState signstate = TRoadSign1::Red;


// ============================================================================
void InitPorts(void);

void DriveGreenLed(void);
void DriveRoadLight(void);


// ============================================================================
union
{   // Выравнивание, константы размеров? Нет, не слышали...
    __IO uint8_t bytes[22];
        uint16_t words[11];
    struct
    {
       int16_t MD;  // TODO: сдвинуть на байт.
       int16_t MC;
       int16_t MB;
       int16_t B2;
       int16_t B1;
      uint16_t AC6;
      uint16_t AC5;
      uint16_t AC4;
       int16_t AC3;
       int16_t AC2;
       int16_t AC1;
    } K;
} BmpData1;

// ----------------------------------------------------------------------------
union
{
    struct
    {
        struct
        {
            uint8_t data2; // LSB - MSB
            uint8_t data1;
            uint8_t data0;
        } D3;

        uint8_t data3; // = 0;

    } D3;

    struct
    {

        struct
        {
            uint8_t data1;
            uint8_t data0;
        } D2;

        uint8_t data3; // = 0;
        uint8_t data2; // = 0;

    } D2;

    int32_t Val;

    void Init(void) { D2.data2 = 0; D2.data3 = 0; D3.data3 = 0; }
    int32_t ShiftT(void) { return Val; }    // TODO
    int32_t ShiftP(void) { return Val; }

} BmpInt;

// ----------------------------------------------------------------------------
enum class BmpState
{
    On, Calibr, Temp, Pressr,
} BmpState = BmpState::On;

// ----------------------------------------------------------------------------
uint32_t ledtick;
uint32_t signtick;
uint32_t sectick;

uint16_t sec1 = 0;


// ===== main() ===============================================================
void main(void)
{
    // На данном этапе тактовый генератор уже должен быть настроен
    // на высокую скорость. Смотри функцию _start(); Это сделано для того, чтобы
    // инициализация проходила с той же скоростью, что и основная программа.

    // ========================================================================
    InitPorts();

    // ------------------------------------------------------------------------
    Rcc.Peryphery1on(TRcc::i2c1);

    PortB.SetupPins<GPin::P6, GPin::P7>(PinMode2::Out2MHz, PinFunct2::AF_OpenDrain);

    PortB.SetupPins<GPin::P5>(PinMode2::Out2MHz, PinFunct2::PushPull);

    PortB.PinOn<GPin::P5>();

    i2c1.Open();

    BmpInt.Init();

//    i2c1.WriteAddr(0xEEU);
    // Старт, адрес(устройство), ACK, команда(регистр), ACK, данные, ACK, ..., стоп.
//    i2c1.Write(0xEEU, 0xF4U, 0x2EU); // bmp180 - 0xEEU. Test id - 0xD0U
//    i2c1.Write(0xEEU, 0xD0U);

//    __IO uint8_t data; //
//    i2c1.FullReadOne(0xEEU, 0xD0U, data);

//    __IO uint16_t data2; //

    // Старт, адрес(устройство), ACK(ADDR), команда(регистр),
//    i2c1.Write(0xEEU, 0xF4U, 0x34U);
//    i2c1.Write(0xEEU, 0x2EU);
    // Старт, адрес(устройство), ACK(ADDR), команда(регистр), ACK, рестарт, адрес, чтение, ACK, чтение, NACK, стоп.
//    i2c1.FullReadTwo(0xEEU, 0xF6U, data2);

    // ========================================================================
    // Si2c1.Open();

//    Led1637.Setup();

    // ------------------------------------------------------------------------
    Rcc.Tim1on();
    SysTimer.Start();

    // ========================================================================
    ledtick = SysTimer.Tick + 200;
    signtick = SysTimer.Tick + 750;
    sectick = SysTimer.Tick + 1200;

    // ------------------------------------------------------------------------
    while (1)
    {
        // Стоит только какой-нибудь секции выполняться более 1 мсек и вся эта
        // многозадачность рухнет. Но есть решение.
        // TODO: прерывания I2C. Ну, это легко. )
        // --------------------------------------------------------------------
        if(sectick == SysTimer.Tick) // int sectick <= SysTimer.Tick ((
        {
            switch(BmpState)
            {
    //            Led1637.Display(sec1++);
                case BmpState::Calibr:
                    i2c1.FullReadN(0xEEu, 0xBFu, BmpData1.bytes, 22);
                    BmpState = BmpState::Temp;
                    i2c1.Write(0xEEu, 0xF4u, 0x2Eu);
                break;

                case BmpState::Temp:
                    BmpInt.Init();
                    i2c1.FullReadTwo(0xEEu, 0xF6u, BmpInt.D2.D2.data1, BmpInt.D2.D2.data0);

                    i2c1.Write(0xEEU, 0xF4u, 0xF4U); // Запуск измерения давления.

                    {
                        // TODO: Набить все эти переменные в стек - не лучшая идея.
                        __IO int32_t T;

                        int32_t X1 = (BmpInt.Val - BmpData1.K.AC6) * BmpData1.K.AC5 / 32768;
                        int32_t X2 =  BmpData1.K.MC * 2048 / (X1 + BmpData1.K.MD);
                        int32_t B5 = X1 + X2;

                        T = (B5 + 8) / 16;
                        (void)T;
                    }
                    BmpState = BmpState::Pressr;
                break;

                case BmpState::Pressr:
                    BmpInt.Init();
                    i2c1.FullReadTwo(0xEEu, 0xF6u, BmpInt.D3.D3.data1, BmpInt.D3.D3.data0);

                    i2c1.Write(0xEEu, 0xF4u, 0x2Eu); // Запуск измерения температуры.

                     // Led1637.Display(Pressure);
                    BmpState = BmpState::Temp;
                break;

                case BmpState::On:
                    i2c1.Write(0xEEu, 0xE0u, 0xB6u);
                    // __IO uint8_t tmp;
                    // i2c1.FullReadOne(0xEEu, 0xF4u, tmp);
                    BmpState = BmpState::Calibr;
                break;
                default:
                    ;
            }

            // i2c1.FullReadOne(0xEEU, 0xF6U, data);
//            i2c1.Write(0xEEU, 0xF4U);
//            i2c1.Write(0xEEU, 0x2EU);

            if(sec1 == 1000)
            {
                sec1 = 0;
            }
            sectick += 1000;
        }

        // --------------------------------------------------------------------
        DriveGreenLed();

        // --------------------------------------------------------------------
        DriveRoadLight();


    }

}


// ============================================================================
extern "C" void SysTick_Handler(void) // Вот такое объявление. Только для линковщика. )
{
    SysTimer.Tick++;
}


// ============================================================================
void InitPorts(void)
{

    // ------------------------------------------------------------------------
    // Инициализируем порты А, B, C.
    Rcc.PortOn<GPort::A, GPort::B, GPort::C>();

    PortA.ODR = 0xFFFFU; // 16 бит.
    PortB.ODR = 0x00U;
    PortC.ODR =  0xFFFFU; // Не затронет выводы, используемые для альтернативных функций.

    // ------------------------------------------------------------------------
    PortA.SetupPins
    <GPin::P0, GPin::P1, GPin::P2, GPin::P3, GPin::P4, GPin::P5>
    (PinMode2::Out2MHz, PinFunct2::OpenDrain);

    PortA.SetupPins
    <GPin::P6, GPin::P7, GPin::P8, GPin::P9, GPin::P10, GPin::P11, GPin::P12, GPin::P13, GPin::P14, GPin::P15>
    (PinMode1::Input, PinFunct1::AnalogInput);

    // ------------------------------------------------------------------------
    PortB.SetupPins
    <GPin::P12, GPin::P13, GPin::P14>
    (PinMode2::Out2MHz, PinFunct2::PushPull);

    PortB.SetupPins
    <GPin::P0, GPin::P1, GPin::P2, GPin::P3, GPin::P4, GPin::P5, GPin::P6, GPin::P7,
     GPin::P8, GPin::P9, GPin::P10, GPin::P11,      GPin::P15>
    (PinMode1::Input, PinFunct1::AnalogInput);

    // ------------------------------------------------------------------------
    // Единственный свободный пин на этом порту.
    PortC.SetupPins<GPin::P13>(PinMode2::Out2MHz, PinFunct2::OpenDrain);

}

// Если какая-нибудь задача займет больше, чем приращение счетчика,
// то такая многозадачность обвалится. ((
// ============================================================================
void DriveGreenLed(void)
{
    // Сказать, что это ОСРВ, наверное, значит ничего не сказать,
    // но многозадачность, хоть и кооперативная, здесь уже есть.
    // --------------------------------------------------------------------
    if(ledtick == SysTimer.Tick)
    {
        if(PortC.AnyOdrBitIsOn<GPin::P13>()) // ODR.13 используется как состояние.
        {
            PortC.PinOff<GPin::P13>(); // Открытый коллектор: Led -- on.
            ledtick += 100;  // Переполнение? Вместе со счетчиком.
        }
        else
        {
            PortC.PinOn<GPin::P13>();
            ledtick += 1900;
        }
    }

}


// ============================================================================
void DriveRoadLight(void)
{
    if(signtick == SysTimer.Tick)
    {
        PortB.PinOff<GPin::P12, GPin::P13, GPin::P14>();
        PortA.PinOn<GPin::P3, GPin::P4, GPin::P5>();

        signtick += RoadSign1.SignDuration[signstate];
        switch(signstate)
        {
            case TRoadSign1::Red:
                signstate = TRoadSign1::RedYellow;

                PortB.PinOn<GPin::P14>();
                PortA.PinOff<GPin::P5>();
            break;

            case TRoadSign1::RedYellow:
                signstate = TRoadSign1::Green;

                PortB.PinOn<GPin::P13, GPin::P14>();
                PortA.PinOff<GPin::P3>();
            break;

            case TRoadSign1::Green:
                signstate = TRoadSign1::GreenBlink;

                PortB.PinOn<GPin::P12>();
                PortA.PinOff<GPin::P3>();
            break;

            case TRoadSign1::GreenBlink:
                if(RoadSign1.GBlinkCount % 2 == 0)
                {
                    PortB.PinOff<GPin::P12>();
                }
                else
                {
                    PortB.PinOn<GPin::P12>();
                }

                if(RoadSign1.GBlinkCount == 9)
                {
                    signstate = TRoadSign1::Yellow;
                    RoadSign1.GBlinkCount = 2;
                }

                RoadSign1.GBlinkCount++;

                PortA.PinOff<GPin::P3>();
            break;

            case TRoadSign1::Yellow:
                signstate = TRoadSign1::Red;

                PortB.PinOn<GPin::P13>();
                PortA.PinOff<GPin::P3, GPin::P4>();
            break;

            default:    // Вот так и возникают нездоровые сенсации.
                PortB.BSRR = 0x07U << 12;
                signtick += 10000;

        }
    }
}


/*
 *

 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.


#include <stdio.h>
#include <stdlib.h>

//
// Standalone STM32F1 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//
 *
// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


#pragma GCC diagnostic pop
 */

