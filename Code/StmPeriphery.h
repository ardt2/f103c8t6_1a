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


    // ========================================================================
    private:
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
        inline constexpr uint32_t lowmask(void)
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
        inline constexpr uint32_t highmask(void)
        {
            return (... & (bit >= 8 ? _4BITSMASK << (bit - 8) * _MASKWIDTH : 0xFFFFFFFFU));
        }
        template<uint8_t... bit>
        inline constexpr uint32_t highbits(uint32_t mask)
        {
            return (... | (bit >= 8 ? mask << (bit - 8) * _MASKWIDTH : 0U));
        }


    // ========================================================================
    public:
        template<GPin... pin>
        inline void PinOn(void)
        {
            BSRR = (... | ((uint32_t)0x01U << (uint8_t)pin));
        }
        // --------------------------------------------------------------------
        template<GPin... pin>
        inline void PinOff(void)
        {
            BRR =  (... | ((uint32_t)0x01 << (uint8_t)pin));
        }

        // --------------------------------------------------------------------
        template<GPin... pin>
        inline constexpr bool AnyOdrBitIsOn()
        {
            return (... && ((ODR & ((uint32_t)0x01U << (uint8_t)pin)) != 0));
        }


    private:
    // ========================================================================




        // --------------------------------------------------------------------



#undef _MASKWIDTH
#undef _4BITSMASK
};


// ----------------------------------------------------------------------------
extern TGPIO & PortA;
extern TGPIO & PortB;
extern TGPIO & PortC;


// ============================================================================
// template<GPin scl, GPin sda>
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

            CR1 = I2C_CR1_SWRST; // 0;
            while(SR1 != 0 && SR2 != 0) ; // Эту строчку смело можно заккоментировать.
            CR1 = 0;

            // 100 кГц
            // ----------------------------------------------------------------
            CR2 &= 0xffc0;    // =I2C_CR2_FREQ_Reset
            CR2 |= 36U;        // set FREQ = APB1= 36MHz

            CCR = 180U;
            TRISE = 38U;

            // 400 кГц
            // ----------------------------------------------------------------
//            CR2 |= 36U;     // FREQR 36 МГц (100100) TPCLK1 = 1 / (FREQR * 1e6)  ≈ 28 нсек
//                            // FSCL = 400 кГц TSCL = 1 / FSCL = Thigh + Tlow = 25 * CCR * TPCLK1
//                            // CCR = 1 / (25 * TPCLK1 * FSCL) ≈ 1 / (25 * 400e6 * 28e-9) ≈ 3
//            // 11:0
//            CCR |= 3U;      // Thigh = 9 * CCR * TPCLK1
//                            // Tlow = 16 * CCR * TPCLK1
//                            //
//            CCR |= I2C_CCR_FS | I2C_CCR_DUTY;   // Fast mode и duty (9/16) (Если не ставить,
//                                                // то можно разогнать I2C, ну её и так можно
//                                                //  разогнать, емкости проводников больше мешают).
//            TRISE |= 8U;     // 200 нсек / 28 нсек ≈ 7. (7 + 1). Reset value 0x20, можно и просто TRISE = 8U;

            CR1 |= I2C_CR1_PE; // Periphery enabled. (Померяю, сколько получится).
        }

        // --------------------------------------------------------------------
        void Write(uint8_t addr, uint8_t data)
        {
            CR1 |= I2C_CR1_START; // Старт.
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) ;

            DR = addr << 1U;

            while( (SR1 & I2C_SR1_ADDR) == 0 ) ;
            (void)SR2;

            DR = data;


            CR1 |= I2C_CR1_STOP;

        }

        // --------------------------------------------------------------------
        void WriteAddr(uint8_t addr)
        {
            CR1 |= I2C_CR1_START; // Старт.
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) {}

            // I2C использует 7-мь бит адреса. Младший некоторые к-ры используют
            // для указания записи и чтения для байт, идуших за адресом без старт/стопа.
            DR = addr << 1U | 0x00U; // Адрес устройства. ( | 0x01U) для чтения.

            while( (SR1 & I2C_SR1_ADDR) == 0 ) {} // Сброс флага ADDR
            (void)SR2;                            // чтением 2-х регистров состояния.

            CR1 |= I2C_CR1_STOP;

/*
            CR1 |= I2C_CR1_START; // Рестарт.
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) ;

            DR = 0x02U; // Адрес регистра.

            while( (SR1 & I2C_SR1_ADDR) == 0 ) {}
            (void)SR2;


//            if( (SR2 & I2C_SR2_TRA) != 0 )
//            {
                DR = 0xFFU; // Данные.
//            }
//            while( (SR1 & I2C_SR1_TXE) == 0 ) ;


            while( (SR1 & I2C_SR1_BTF) != 0 ) ;
            CR1 |= I2C_CR1_STOP;

*/
        }


        // --------------------------------------------------------------------
        void WriteAddr2(uint8_t addr)
        {
            __IO uint32_t tmp;



            CR1 |= I2C_CR1_START; // Старт.
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) {}

            DR = 0x02U; // Адрес устройства. ( | 0x01U) для чтения.
            while( (SR1 & I2C_SR1_ADDR) == 0 ) {}
            (void)SR2;

            DR = 0x03U;
            while( (SR1 & I2C_SR1_BTF) == 0 ) ;

            DR = 0xFFU;
            while( (SR1 & I2C_SR1_BTF) == 0 ) ;

            DR = 0xFFU;
            while( (SR1 & I2C_SR1_BTF) == 0 ) ;

            DR = 0xFFU;
            while( (SR1 & I2C_SR1_BTF) == 0 ) ;

            DR = 0xFFU;
            while( (SR1 & I2C_SR1_BTF) == 0 ) ;

//            CR1 |= I2C_CR1_STOP;

//            (void)SR1;
//            CR1 |= I2C_CR1_START; // Старт.
//            while ((I2C1->SR1 & I2C_SR1_SB) == 0) {}
//
//            DR = 0x11U; // Адрес устройства. ( | 0x01U) для чтения.
//            while( (SR1 & I2C_SR1_ADDR) == 0 ) {}
//            (void)SR2;

            CR1 |= I2C_CR1_START; // Старт.
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) {}

            CR1 |= I2C_CR1_STOP;

            tmp = 0;
        }
         // --------------------------------------------------------------------
        void WriteData(uint8_t data)
        {
           // (void)SR1;
//            (void)SR2;

//            CR1 |= I2C_CR1_START; // | I2C_CR1_ACK; // | I2C_CR1_STOP;
//            while ((I2C1->SR1 & I2C_SR1_SB) == 0) ;

           // CR1 |= I2C_CR1_STOP; // 0;I2C_SR1_TXE
//            CR1 |= I2C_CR1_PE;

           // (void)SR1;

            DR = data; // << 1;

            while( (SR1 & I2C_SR1_BTF) != 0 ) ;

            (void)SR1;
            (void)SR2;

            CR1 |= I2C_CR1_STOP;

        }

        // --------------------------------------------------------------------
        void Write(uint8_t data)
        {
            CR1 |= I2C_CR1_START; // | I2C_CR1_ACK; // | I2C_CR1_STOP;
            while ((I2C1->SR1 & I2C_SR1_SB) == 0) ;

            DR = data; // << 1;

            while( (SR1 & I2C_SR1_BTF) != 0 ) ;

            (void)SR2;

            CR1 |= I2C_CR1_STOP;

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

// ----------------------------------------------------------------------------
extern TEUSART & Eusart1;


// Программная I2C. Более универсальная, конечно.
// ============================================================================
template<GPin sclpin, GPin sdapin>
class TSI2C // : public TBaseI2C
{
    private:
        template<GPin Pin>
        class TLine
        {
            public:
                TGPIO & Port;

                PinMode1 InMode = PinMode1::Input;
                PinFunct1 InFunct = PinFunct1::FloatInput;

                PinMode2 OutMode = PinMode2::Out2MHz;
                PinFunct2 OutFunct = PinFunct2::OpenDrain;


            // Предполагается, что тактирование порта включено.
            // ================================================================
            public:
                inline TLine(TGPIO & port) : Port(port)
                {

                }
                inline ~TLine() {};

                // ------------------------------------------------------------
                inline void SetIn(void)
                {
                    Port.SetupPins<Pin>(InMode, InFunct);
                }
                // ------------------------------------------------------------
                inline void SetOut(void) // bool high = true
                {
                    Port.SetupPins<Pin>(OutMode, OutFunct);
                }
                // ------------------------------------------------------------
                inline void Rise(void)
                {
                    Port.PinOn<Pin>();
                }
                // ------------------------------------------------------------
                inline void Drop(void)
                {
                    Port.PinOff<Pin>();
                }
                // ------------------------------------------------------------
                inline bool Read(void)
                {
                    return ((uint32_t)Pin | Port.IDR) == 0;
                }
        };


        // ====================================================================
        TLine<sclpin> SCL;
        TLine<sdapin> SDA;

        const uint8_t Delay1 = 5;
        const uint8_t Delay2 = 2;


    // ========================================================================
    public:
        inline TSI2C(TGPIO & port)
            : SCL(port),
              SDA(port)
        {
            // Rcc.SysFreq;
        }
        // --------------------------------------------------------------------
        inline ~TSI2C() {};

        // --------------------------------------------------------------------
        void Open(void)
        {
            SCL.Rise();
            SDA.Rise();
            SCL.SetOut();
            SDA.SetOut();
        }

        // --------------------------------------------------------------------
        void Write(uint8_t device, uint8_t reg, uint8_t byte)
        {

        }
        // --------------------------------------------------------------------
        void Write(uint8_t device, uint8_t reg,  const uint8_t * const byte, uint8_t nbytes)
        {

        }
        // --------------------------------------------------------------------
        void Read(uint8_t device, uint8_t reg, uint8_t & byte)
        {

        }
        // --------------------------------------------------------------------
        uint8_t Read(uint8_t device, uint8_t reg, uint8_t & byte, uint8_t nbytes)
        {
            return 0;
        }

        // --------------------------------------------------------------------
        void WriteByte(uint8_t byte)
        {
            Start();
            Write8(byte);
            Ack();
            Stop();
        }
        // --------------------------------------------------------------------
        void WriteByteNoStop(uint8_t byte)
        {
            Start();
            Write8(byte);
            Ack();
        }
        // --------------------------------------------------------------------
        void WriteByteNoStartStop(uint8_t byte)
        {
            Write8(byte);
            Ack();
        }

        // --------------------------------------------------------------------
        inline void Start(void)
        {
            SDA.Drop();
            Delay(5);
            SCL.Drop();
        }
        // --------------------------------------------------------------------
        inline void Stop(void)
        {
            SCL.Rise();
            Delay(5);
            SDA.Rise();
            Delay(10);
        }

        // --------------------------------------------------------------------
        inline void Write8(uint8_t byte)
        {
            for(uint8_t i = 0; i != 8; i++)
            {
                bool ping = byte & 0x01U;
                if(ping)
                {
                    SDA.Rise();
                }
                else
                {
                    SDA.Drop();
                }

                Delay(5);
                SCL.Rise();
                Delay(5);
                SCL.Drop();

                byte >>= 1U;
            }
            Delay(5);
        }

        // --------------------------------------------------------------------
        inline void Ack(void)
        {
            SDA.Rise();

            Delay(2);
            SCL.Rise();
            Delay(5);
            SCL.Drop();

            SDA.Drop();

            Delay(5);
        }

        // --------------------------------------------------------------------
        inline void Delay(uint8_t usec)
        {
            for(__IO uint32_t t = 0; t != usec * 10; t++) ;
        }

    // ========================================================================

};

// ----------------------------------------------------------------------------
extern TSI2C<GPin::P6, GPin::P7> Si2c1;


const int8_t Seg[] =
    {0x3f,0x06,0x5b,0x4f,
     0x66,0x6d,0x7d,0x07,
     0x7f,0x6f,0x77,0x7c,
     0x39,0x5e,0x79,0x71}; //0~9,A,b,C,d,E,F

// ============================================================================
class TLed1637
{
//    static const int8_t Seg[] =
//            {0x3f,0x06,0x5b,0x4f,
//             0x66,0x6d,0x7d,0x07,
//             0x7f,0x6f,0x77,0x7c,
//             0x39,0x5e,0x79,0x71}; //0~9,A,b,C,d,E,F

    private:
        void StartCommand1(void)
        {
//            Si2c1.Start();
//            Si2c1.Write8(0x40U);
//            Si2c1.Ack();
//            Si2c1.Stop();

            Si2c1.WriteByteNoStop(0xC0U);
        }
        // --------------------------------------------------------------------
        void Write(uint8_t addr, uint8_t data)
        {
            Si2c1.WriteByteNoStop(addr);
            Si2c1.WriteByteNoStartStop(data);
            Si2c1.Stop();
        }

    // ========================================================================
    public:
        void Setup(void)
        {
            Si2c1.WriteByte(0x89U); // LedsOn();
            Si2c1.WriteByte(0x40U);
        }
        // --------------------------------------------------------------------
        void LedsOn(uint8_t bright = 1) // Минимум после подачи питания. Макс. 7
        {
            if(bright > 7) bright = 7;
            Si2c1.WriteByte(0x88U | bright);
        }
        // --------------------------------------------------------------------
        void LedsOff(void)
        {
            Si2c1.WriteByte(0x80U);
        }
        // --------------------------------------------------------------------
        void Clear(void)
        {
            StartCommand1();

            for(uint8_t i = 0; i != 4; i++)
            {
                Si2c1.WriteByteNoStartStop(0x00U); // !!! TODO: timeout
            }

            Si2c1.Stop();
        }
        // --------------------------------------------------------------------
        void Display(uint16_t n)
        {
            // StartCommand1();

            for(uint8_t i = 4; i != 0 ; i--)
            {
                // if(n == 0 && i != 3) Si2c1.Write8(0x00);
                // Si2c1.WriteByteNoStartStop(Seg[n % 10]);
                Write(0xC0 + (i - 1), Seg[n % 10]);
                n /= 10;
            }

            // Si2c1.Stop();
        }
        // --------------------------------------------------------------------
        // template<uint8_t radix>
        void DisplayHex(uint16_t n)
        {
            // StartCommand1();

            for(uint8_t i = 4; i != 0 ; i--)
            {
                //Si2c1.WriteByteNoStartStop(Seg[n % 16]);
                Write(0xC0 + (i - 1), Seg[n % 16]);
                n /= 16;
            }

            // Si2c1.Stop();
        }
};

// ----------------------------------------------------------------------------
extern TLed1637 Led1637;


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


