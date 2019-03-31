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
enum class GPIOportX : uint32_t // Можно сделать просто enum и внести в класс.
{
    PortA = RCC_APB2ENR_IOPAEN,
    PortB = RCC_APB2ENR_IOPBEN,
    PortC = RCC_APB2ENR_IOPCEN,

};

//  uint32_t PortMask(GPIOportX n) { return static_cast<uint32_t>n; };


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
        void Tim1on(void)
        {
            APB2ENR |= RCC_APB2ENR_TIM1EN;
        }
        // --------------------------------------------------------------------
        void Tim1off(void)
        {
            APB2ENR &= ~RCC_APB2ENR_TIM1EN;
        }

        // --------------------------------------------------------------------
        void PortOn(GPIOportX port)
        {
            APB2ENR |= static_cast<uint32_t>(port); // Тактирование GPIO порта
        }
        // --------------------------------------------------------------------
        void PortOff(GPIOportX port)
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
class TGpIO : public GPIO_TypeDef
{
    private:
        TGpIO() = delete;
        ~TGpIO() = delete;

        enum PinMode { OpenDrain = 0x05U, PushPull = 0x02U, };
        enum PinSpeed { _2MHz, _10Mhz, _50Mhz};

        // --------------------------------------------------------------------
        void SetPinMode(PinMode mode, uint8_t pin)
        {
            if(pin < 8)
            {
                CRL &= ~(mode << pin);
                CRL |= mode << pin;
            }
            {
                pin -= 8;
                CRH &= ~(mode << pin);
                CRH |= mode << pin;
            }
        }

        // --------------------------------------------------------------------
        void SetPinSpeed(PinSpeed speed, uint8_t pin)
        {
            if(pin < 8)
            {
                CRL &= ~(speed << pin);
                CRL |= speed << pin;
            }
            {
                pin -= 8;
                CRH &= ~(speed << pin);
                CRH |= speed << pin;
            }
        }


    // ========================================================================
    public:
        void SetPushPull(uint8_t pin)
        {
            SetPinMode(PushPull, pin);
            // CRH &= ~(mode << n);
            // CRH |= mode << n;
//          CMSIS_GPIO.MODER |= 0x03u << PinClamp(pin) * 2;     // Если пин -- константа, то это все вычисляется
//          CMSIS_GPIO.MODER &= ~(0x02u << PinClamp(pin) * 2);  // при компиляции и сводится к r.or r.and.
        }


        // --------------------------------------------------------------------
        void SetOpenDrain(uint8_t pin)
        {
            SetPinMode(OpenDrain, pin);
        }

        // --------------------------------------------------------------------
        void On(uint8_t n)
        {
            BSRR = 0x01U << n;
        }

};

// ----------------------------------------------------------------------------
extern TGpIO & PortA;
extern TGpIO & PortB;
extern TGpIO & PortC;




// ============================================================================
class TI2C : public I2C_TypeDef
{
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

// ----------------------------------------------------------------------------
// TODO



#endif //__STM_PERIPHERY_H
