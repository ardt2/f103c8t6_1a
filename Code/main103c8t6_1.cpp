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

// ============================================================================
TRcc & Rcc = *static_cast<TRcc *>RCC; // TODO: Если глобальных переменных станет
                                      // много, можно вынести их в отдельный файл.
// ----------------------------------------------------------------------------
TSysTickTimer & SysTimer = *static_cast<TSysTickTimer *>TIM1;

// ----------------------------------------------------------------------------
TGPIO & PortA = *static_cast<TGPIO *>GPIOA;
TGPIO & PortB = *static_cast<TGPIO *>GPIOB;
TGPIO & PortC = *static_cast<TGPIO *>GPIOC;


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
__IO uint32_t rega, regb;
#include"AuxVariadic.h"

// TBits TestBits;

// ----------------------------------------------------------------------------
uint32_t ledtick;
uint32_t signtick;

// ===== main() ===============================================================
void main(void)
{
    // На данном этапе тактовый генератор уже должен быть настроен
    // на высокую скорость. Смотри функцию _start(); Это сделано для того, чтобы
    // инициализация проходила с той же скоростью, что и основная программа.


    // ------------------------------------------------------------------------
    // Инициализируем порт А.
    Rcc.PortOn<GPort::A, GPort::B, GPort::C>();

    PortA.CRL |= FunctBitsTest2<GPin::P1, GPin::P3, GPin::P5>(); //  ,GPin::P10, GPin::P13>();
    PortA.CRL |= FunctBitsTest2<GPin::P1, GPin::P2, GPin::P6, GPin::P10, GPin::P13>();

    PortA.ODR = 0xFFFFU; // 16 бит.
    PortB.ODR = 0x00U;
    PortC.ODR =  0xFFFFU;

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


    // ------------------------------------------------------------------------
//    Rcc.Peryphery1on(TRcc::i2c1);


    // ------------------------------------------------------------------------
    Rcc.Tim1on();
    SysTimer.Start();


    // ========================================================================
    ledtick = SysTimer.Tick;
    signtick = SysTimer.Tick + 500;

    // ------------------------------------------------------------------------
    while (1)
    {
        // Сказать, что это ОСРВ, наверное, значит ничего не сказать,
        // но многозадачность, хоть и кооперативная, здесь уже есть.
        // --------------------------------------------------------------------
        if(ledtick == SysTimer.Tick)
        {
            if(PortC.ODR & 0x01U << 13)
            {
                PortC.BRR = 0x01U << 13;
                ledtick += 100; // Переполнение? Вместе со счетчиком.
            }
            else
            {
                PortC.BSRR = 0x01U << 13;
                ledtick += 1900;
            }
        }

        // --------------------------------------------------------------------
        if(signtick == SysTimer.Tick)
        {
            PortB.BRR = 0x07U << 12;
            PortA.BSRR = 0x07U << 3;

            signtick += RoadSign1.SignDuration[signstate];
            switch(signstate)
            {
                case TRoadSign1::Red:
                    signstate = TRoadSign1::RedYellow;
                    PortB.BSRR = 0x01U << 14;

                    PortA.BRR = 0x01U << 5;
                break;

                case TRoadSign1::RedYellow:
                    signstate = TRoadSign1::Green;
                    PortB.BSRR = 0x01U << 14;
                    PortB.BSRR = 0x01U << 13;

                    PortA.BRR = 0x01U << 3;
                break;

                case TRoadSign1::Green:
                    signstate = TRoadSign1::GreenBlink;
                    PortB.BSRR = 0x01U << 12;

                    PortA.BRR = 0x01U << 3;
                break;

                case TRoadSign1::GreenBlink:
                    if(RoadSign1.GBlinkCount % 2 == 0)
                        PortB.BRR = 0x01U << 12;
                    else
                        PortB.BSRR = 0x01U << 12;

                    if(RoadSign1.GBlinkCount == 9)
                    {
                        signstate = TRoadSign1::Yellow;
                        RoadSign1.GBlinkCount = 2;
                    }

                    RoadSign1.GBlinkCount++;

                    PortA.BRR = 0x01U << 3;
                break;

                case TRoadSign1::Yellow:
                    signstate = TRoadSign1::Red;
                    PortB.BSRR = 0x01U << 13;

                    PortA.BRR = 0x01U << 3;
                    PortA.BRR = 0x01U << 4;
                break;

                default:    // Вот так и возникают нездоровые сенсации.
                    PortB.BSRR = 0x07U << 12;
                    signtick += 10000;

            }

        }

    }
}



// ============================================================================
extern "C" void SysTick_Handler(void) // Вот такое объявление. Только для линковщика. )
{
    SysTimer.Tick++;
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

