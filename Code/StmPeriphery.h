//******************************************************************************
//
// Файл         : ./103c8t6_1/Code/StmPeriphery.h
//
// Дата         : 21 марта 2019 г.
// Описание     : TODO Заголовочный файл StmPeriphery.h
// Назначение   : Содержит определения типов, переменных и функций,
//                которые используются во всех файлах проекта.
//
// Лицензия     : Creative common 3.0
//
// Автор Ardt2 (c) 2019
//
//******************************************************************************

// Определи макрос, чтобы не допустить повторное включение заголовочного файла.
// Для обеспечения уникальности имени макроса, произведи его от имени файла.
#ifndef __STM_PERIPHERY_H
#define __STM_PERIPHERY_H


// ============================================================================
enum class GPort : uint32_t // Можно сделать просто enum и внести в класс.
{
    A = RCC_APB2ENR_IOPAEN,
    B = RCC_APB2ENR_IOPBEN,
    C = RCC_APB2ENR_IOPCEN,

};
enum class APB2 : uint32_t // Можно сделать просто enum и внести в класс.
{
    PortAenable = RCC_APB2ENR_IOPAEN,
    PortBenable = RCC_APB2ENR_IOPBEN,
    PortCenable = RCC_APB2ENR_IOPCEN,

};
//  uint32_t PortMask(GPIOportX n) { return static_cast<uint32_t>n; };

//
// ----------------------------------------------------------------------------
class TRcc: public ::RCC_TypeDef
{
    private:
        TRcc() = delete;
        ~TRcc() = delete;


    // ========================================================================
    public:
        typedef enum // Анонимный enum
        {
            i2c1 = RCC_APB1ENR_I2C1EN,
        } TPeriphery1;


    // ========================================================================
    public:
        template<GPort... port>
        inline void PortOn(void) // Без явного разворачивания (inline)
        {                         // не развернется при -Og или -O0
//            APB2ENR |= SetBits<(uint32_t)port...>();
            APB2ENR |= SetBits17<(uint32_t)port...>();
        }

    // ------------------------------------------------------------------------
#define BITMASK 0x01    // Макроопределение здесь гарантирует нам, что константа
#define MASKWIDTH 1     // не будет перенесена компилятором в память. Брать от
                        // неё указатель мы не собираемся и у нас есть #undef.
                        // (Поменяем потом на const uint32_t и посмотрим.)
    private:
        // Функциональное пролистывание (fold) пакета параметров рекурсией. С++ 11/14.
        // Более известное как распаковка параметров.
        template<uint32_t bitmask>
        inline constexpr uint32_t SetBits(void)
        {
            // Немного избыточная проверка, ведь GPort это enum (а, кстати, bitmask и не бит).
            // static_assert(bitmask < 16, "Превышена разрядность.");
            return bitmask;
        }
        template<uint32_t bitmask1, uint32_t bitmask2, uint32_t... bitmask>
        inline constexpr uint32_t SetBits(void)
        {
            return SetBits<bitmask1>() | SetBits<bitmask2, bitmask...>();
        }

        // --------------------------------------------------------------------
        // Функциональное пролистывание (fold) пакета параметров рекурсией. С++ 17.
        template<uint8_t... bitmask>
        inline constexpr uint32_t SetBits17(void)
        {
            return (bitmask | ...); // Можно и справа налево ... | bit
        }


#undef BITMASK
#undef MASKWIDTH


    // ========================================================================
    public:
        void Tim1on(void)
        {
            APB2ENR |= RCC_APB2ENR_TIM1EN; // В принципе, для регистров RCC (и не
            // только) тоже есть (должна быть) пара BRR/BSRR, но вычисление её
            //  адреса где-то глубоко в документации на ARM Cortex.
        }
        // --------------------------------------------------------------------
        void Tim1off(void)
        {
            APB2ENR &= ~RCC_APB2ENR_TIM1EN;
        }

        // --------------------------------------------------------------------
        void PortOn(GPort port)
        {
            APB2ENR |= static_cast<uint32_t>(port); // Тактирование GPIO порта
        }
        // --------------------------------------------------------------------
        void PortOff(GPort port)
        {
            APB2ENR &= ~static_cast<uint32_t>(port);
        }

        // --------------------------------------------------------------------
        void Peryphery1on(TPeriphery1 p1)
        {
            APB1ENR |= p1;
        }

};

// Укажи здесь "extern" объявления общих переменных.
// ----------------------------------------------------------------------------
extern TRcc & Rcc;


// ============================================================================
class TSysTickTimer : public TIM_TypeDef
{
    public:
        static volatile uint32_t Tick;

    // ========================================================================
    private:
        TSysTickTimer() = delete;
        ~TSysTickTimer() = delete;

    // ========================================================================
    public:
        void Start(void)
        {
            Rcc.Tim1on();
            SysTick_Config(SystemCoreClock / 1000);
        }
        // --------------------------------------------------------------------

};

// ----------------------------------------------------------------------------
extern TSysTickTimer & SysTimer;


// ============================================================================
enum class GPin : uint8_t
{
    P0 = 0U, P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13, P14, P15,
};
enum class PinFunct1 : uint32_t
{
    // 0x11 - Резерв. ODR - Подтяжка верх/земля.
    AnalogInput = 0x00U << 2U, FloatInput = 0x01U << 2U, PullInput = 0x02U << 2U,
};
enum class PinMode1 : uint32_t // << 2
{
    Input = 0x00U,
};
enum class PinFunct2 : uint32_t // << 2
{
    PushPull = 0x00U << 2U, OpenDrain = 0x01U << 2U, AF_PushPull = 0x02U << 2U, AF_OpenDrain = 0x03U << 2U,
};
enum class PinMode2 : uint32_t
{
    Out2MHz = 0x02U, Out10Mhz = 0x01U, Out50Mhz = 0x03U,
};

// ----------------------------------------------------------------------------
class TGPIO : public GPIO_TypeDef
{
    private:
        TGPIO() = delete;
        ~TGPIO() = delete;


    // ========================================================================
    public:
        // Соответствие пар mode/funct проще сделать на типизации, чем функционально.
        template<GPin... pins> // GPin pin1,
        void SetupPins(PinMode1 mode, PinFunct1 funct)
        {
            ClearSetBits<(uint8_t)pins...>((uint32_t)funct | (uint32_t)mode);
        }
        // --------------------------------------------------------------------
        template<GPin... pins> // GPin pin1,
        void SetupPins(PinMode2 mode, PinFunct2 funct)
        {
            ClearSetBits<(uint8_t)pins...>((uint32_t)funct | (uint32_t)mode);
        }


        // ====================================================================
//        const uint32_t _2bitsmask = (uint32_t)0x03U;  // Обязательно используй прагму размещения,
//        const uint32_t _4bitsmask = (uint32_t)0x0FU;  // иначе константа не определится. (( Фигня.
// #define _2bitsmask (uint32_t)0x03U
// #define _4bitsmask (uint32_t)0x0FU

#define _MASKWIDTH 4U       // Лучше макроопределением, а потом undefine.
#define _4BITSMASK 0x0FU    // Потом проверим с const uint32_t.
// ----------------------------------------------------------------------------
// Повторение - мать учения. http://scrutator.me/post/2017/08/11/cpp17_lang_features_p1.aspx

    public:
        template<uint8_t ... bits>
        inline void ClearSetBits(uint32_t mask) //
        {
            static_assert(sizeof...(bits) > 0, "Не указаны биты.");
            static_assert(sizeof...(bits) <= 16, "Указано количество бит более разрядности (> 16).");
            // bits должен быть от константного типа (enum), чтобы это работало.
            //static_assert(!hasduplicates<bits...>(), "Найдены повторяющиеся биты.");
            if(sizeof...(bits) == 0) return;

            if(haslowbits<bits...>())
            {
                // uint32_t tmp = ;
                CRL &= ~lowbits<bits...>(_4BITSMASK); // tmp;
                // tmp =
                CRL |= lowbits<bits...>(mask); // tmp;
            }

            else if(hashighbits<bits...>())
            {
                CRH &= ~highbits<bits...>(_4BITSMASK);
                CRH |= highbits<bits...>(mask);
            }
        }

    public:
//        constexpr uint32_t createmask(uint32_t mask) { return mask; }
//        template<uint8_t ... bits>
//        inline void clearsetbits(const uint32_t mask) //
//        {
//            static_assert(sizeof...(bits) > 0, "Не указаны биты.");
//            static_assert(sizeof...(bits) <= 16, "Указано количество бит более разрядности (> 16).");
//            // bits должен быть от константного типа (enum), чтобы это работало.
//            //static_assert(!hasduplicates<bits...>(), "Найдены повторяющиеся биты.");
//
//            if(haslowbits<bits...>())
//            {
//                // uint32_t tmp = ;
//                CRL &= ~lowbits<bits...>(_4BITSMASK); // tmp;
//                //tmp = ;
//                CRL |= lowbits<bits...>(createmask(mask));
//            }
//
//            if(hashighbits<bits...>())
//            {
//                CRH &= ~highbits<bits...>(_4BITSMASK);
//                CRH |= highbits<bits...>(mask);
//            }
//        }

        template<uint8_t ... bit>
        inline constexpr bool hasduplicates(void) //
        {
            // return (... == bit) != true; // Не так просто. )) Но написать функционально
                                            // поиск повторений в массиве не так уж и
            return false;                   // сложно, наверное. Но нудно.
        }

        // Перелистывание (folding), сначала было в стиле C++ 14 написано, как рекурсия
        // шаблонной ф-ции. Но на C++ 17 это проще, несомненно и устраняет побочный
        // (не устраняет и эффект вызван другим)
        // эффект отдельного разворачивания замыкающего вызова рекурсии.
        // (Более известно как распаковка пакета параметров.)
        // --------------------------------------------------------------------
        template<uint8_t... bit>
        inline constexpr bool haslowbits(void)   //
        {
            return (... + (bit < 8 ? bit + 1 : 0)) != 0;       //
        }
        template<uint8_t... bit>
        inline uint32_t constexpr lowmask(void)
        {
            return (... & (bit < 8 ? _4BITSMASK << bit - 8 * _MASKWIDTH : 0xFFFFFFFFU));
        }
        template<uint8_t... bit>
        inline constexpr uint32_t lowbits(uint32_t mask)
        {
            return (... | (bit < 8 ? mask << bit * _MASKWIDTH : 0));
        }

        // --------------------------------------------------------------------
        template<uint8_t... bit>
        inline constexpr bool hashighbits(void)
        {
            return (... + (bit >= 8 ? bit : 0)) != 0;
        }
        template<uint8_t... bit>
        inline uint32_t constexpr highmask(void)
        {
            return (... & (bit >= 8 ? _4BITSMASK << (bit - 8) * _MASKWIDTH : 0xFFFFFFFFU));
        }
        template<uint8_t... bit>
        inline uint32_t constexpr highbits(uint32_t mask)
        {
            return (... | (bit >= 8 ? mask << (bit - 8) * _MASKWIDTH : 0U));
        }


    // ========================================================================
    public:


    // ========================================================================
    private:

        // --------------------------------------------------------------------


#undef _MASKWIDTH
#undef _4BITSMASK
};


// ----------------------------------------------------------------------------
extern TGPIO & PortA;
extern TGPIO & PortB;
extern TGPIO & PortC;


// ============================================================================
class TI2C : public I2C_TypeDef // I2C прям вот не хочется делать, но тут без
{                               // вариантов.
    private:
        TI2C() = delete;
        ~TI2C() = delete;

    // ========================================================================
    public:
        // Ниже приведена необходимая последовательность работы в режиме master.
        //  * Указание текущей тактовой частоты шины [APB1] FPCLK1, к которой подключена I2C
        //    в регистре I2C_CR2 для правильной работы устройства I2C. (Тактовая
        //    частота шины должна быть не менее 2 МГц в режиме Sm и 4 МГц в режиме Fm).
        //  * Настройка регистра управления тактированием (I2C_CCR).
        //  * Настройка регистра времени нарастания (I2C_TRISE).
        //  * Установка бита I2C_ENABLE (PE) в регистре I2C_CR1 для включения I2C.
        //  * Установка бита start в регистре I2C_CR1 для запуска передачи.
        void Open()
        {
            // Rcc.Peryphery1on();   // Тактирование I2C1

            CR1 = 0;
            CR1 |= I2C_CR1_ACK | I2C_CR1_START | I2C_CR1_STOP;

            CR2 |= 36U;     // FREQR 36 МГц (100100) TPCLK1 = 1 / (FREQR * 1e6)  ≈ 28 нсек
                            // FSCL = 400 кГц TSCL = 1 / FSCL = Thigh + Tlow = 25 * CCR * TPCLK1
                            // CCR = 1 / (25 * TPCLK1 * FSCL) ≈ 1 / (25 * 400e6 * 28e-9) ≈ 3
            // 11:0
            CCR |= 3U;      // Thigh = 9 * CCR * TPCLK1
                            // Tlow = 16 * CCR * TPCLK1
                            //
            CCR |= I2C_CCR_FS | I2C_CCR_DUTY;   // Fast mode и duty (9/16) (Если не ставить,
                                                // то можно разогнать I2C).
            TRISE |= 8U;     // 200 нсек / 28 нсек ≈ 7. (7 + 1). Reset value 0x20, можно и просто TRISE = 8U;

            CR1 |= I2C_CR1_PE;  // Periphery enabled. (Померяю, сколько получится).
        }

        // --------------------------------------------------------------------
        void Write(uint8_t data)
        {

        }
        // --------------------------------------------------------------------
        void Write(uint8_t addr, uint8_t data)
        {

        }

};

// ----------------------------------------------------------------------------
extern TI2C & i2c1;



// ============================================================================
class TEUSART : public ::USART_TypeDef // Надо будет написать ostream для него.
{                                      //

    private:
        TEUSART() = delete;
        ~TEUSART() = delete;


    // ========================================================================
    public:
        void Open()
        {

        }

        // --------------------------------------------------------------------


};



// ============================================================================

// ----------------------------------------------------------------------------
// TODO



// ============================================================================
#endif //__STM_PERIPHERY_H


// Плюшкин уехал в другой файл. Здесь немного осталось его запасов.
// --------------------------------------------------------------------
//        void Test(Pin::TPin pin)
//        {
//            volatile uint32_t tmp;
//            tmp = _4bitsmask;
//            tmp = _4bitsmask << pin;
//            tmp = ~(_4bitsmask << pin);
//            tmp = 0xFFFFFFFFU;
//            tmp &= ~(_4bitsmask << pin);
//
//            tmp = _4BITSMASK;
//            tmp = 0;
//        }


