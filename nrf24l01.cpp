/********************************************************************************
*   nrf24l01.cpp
*       Implementation of the NRF24L01 hardware driver.
*
*   2019 | Brandon Braun | brandonbraun653@gmail.com
********************************************************************************/

/* C++ Includes */
#include <cstring>
#include <algorithm>

/* Driver Includes */
#include "nrf24l01.hpp"


namespace NRF24L
{
    static const std::array<uint8_t, MAX_NUM_PIPES> pipeRXAddressReg =
        {
            Register::RX_ADDR_P0,
            Register::RX_ADDR_P1,
            Register::RX_ADDR_P2,
            Register::RX_ADDR_P3,
            Register::RX_ADDR_P4,
            Register::RX_ADDR_P5};
    static_assert(pipeRXAddressReg.size() == MAX_NUM_PIPES, "Too many/few items in the array!");

    static const std::array<uint8_t, MAX_NUM_PIPES> pipeEnableRXAddressReg =
        {
            EN_RXADDR::P0,
            EN_RXADDR::P1,
            EN_RXADDR::P2,
            EN_RXADDR::P3,
            EN_RXADDR::P4,
            EN_RXADDR::P5};
    static_assert(pipeEnableRXAddressReg.size() == MAX_NUM_PIPES, "Too many/few items in the array!");

    static const std::array<uint8_t, MAX_NUM_PIPES> pipeRXPayloadWidthReg =
        {
            Register::RX_PW_P0,
            Register::RX_PW_P1,
            Register::RX_PW_P2,
            Register::RX_PW_P3,
            Register::RX_PW_P4,
            Register::RX_PW_P5};
    static_assert(pipeRXPayloadWidthReg.size() == MAX_NUM_PIPES, "Too many/few items in the array!");

#if defined(USING_CHIMERA)
    NRF24L01::NRF24L01(Chimera::SPI::SPIClass_sPtr spiInstance, Chimera::GPIO::GPIOClass_sPtr chipEnable)
    {
        this->spi = spiInstance;
        this->chipEnable = chipEnable;

        memset(spi_txbuff.begin(), 0, spi_txbuff.size());
        memset(spi_rxbuff.begin(), 0, spi_rxbuff.size());

        /*-------------------------------------------------
        Initialize class variables
        -------------------------------------------------*/
        addressWidth = MAX_ADDRESS_WIDTH;
        payloadSize = MAX_PAYLOAD_WIDTH;
        dynamicPayloadsEnabled = false;
        pVariant = false;
        cachedPipe0RXAddress = 0u;
    }
#endif

    bool NRF24L01::erase()
    {
        bool eraseStatus = true;

        cachedPipe0RXAddress = 0u;

        clearChipEnable();

        flushTX();
        flushRX();

        /*------------------------------------------------
        CONFIG Register
        ------------------------------------------------*/
        writeRegister(Register::CONFIG, CONFIG::Reset);
        if (readRegister(Register::CONFIG) != CONFIG::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        EN_AA Register
        ------------------------------------------------*/
        writeRegister(Register::EN_AA, EN_AA::Reset);
        if (readRegister(Register::EN_AA) != EN_AA::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        EN_RXADDR Register
        ------------------------------------------------*/
        writeRegister(Register::EN_RXADDR, EN_RXADDR::Reset);
        if (readRegister(Register::EN_RXADDR) != EN_RXADDR::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        SETUP_AW Register
        ------------------------------------------------*/
        writeRegister(Register::SETUP_AW, SETUP_AW::Reset);
        if (readRegister(Register::SETUP_AW) != SETUP_AW::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        SETUP_RETR Register
        ------------------------------------------------*/
        writeRegister(Register::SETUP_RETR, SETUP_RETR::Reset);
        if (readRegister(Register::SETUP_RETR) != SETUP_RETR::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RF_CH Register
        ------------------------------------------------*/
        writeRegister(Register::RF_CH, RF_CH::Reset);
        if (readRegister(Register::RF_CH) != RF_CH::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RF_SETUP Register
        ------------------------------------------------*/
        writeRegister(Register::RF_SETUP, RF_SETUP::Reset);
        if (readRegister(Register::RF_SETUP) != RF_SETUP::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P0 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P0, reinterpret_cast<const uint8_t *>(&RX_ADDR_P0::Reset), RX_ADDR_P0::byteWidth);

        uint64_t result_p0 = 0u;
        readRegister(Register::RX_ADDR_P0, reinterpret_cast<uint8_t *>(&result_p0), RX_ADDR_P0::byteWidth);

        if (result_p0 != RX_ADDR_P0::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P1 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P1, reinterpret_cast<const uint8_t *>(&RX_ADDR_P1::Reset), RX_ADDR_P1::byteWidth);

        uint64_t result_p1 = 0u;
        readRegister(Register::RX_ADDR_P1, reinterpret_cast<uint8_t *>(&result_p1), RX_ADDR_P1::byteWidth);

        if (result_p1 != RX_ADDR_P1::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P2 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P2, RX_ADDR_P2::Reset);
        if (readRegister(Register::RX_ADDR_P2) != RX_ADDR_P2::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P3 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P3, RX_ADDR_P3::Reset);
        if (readRegister(Register::RX_ADDR_P3) != RX_ADDR_P3::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P4 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P4, RX_ADDR_P4::Reset);
        if (readRegister(Register::RX_ADDR_P4) != RX_ADDR_P4::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_ADDR_P5 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P5, RX_ADDR_P5::Reset);
        if (readRegister(Register::RX_ADDR_P5) != RX_ADDR_P5::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        TX_ADDR Register
        ------------------------------------------------*/
        writeRegister(Register::TX_ADDR, reinterpret_cast<const uint8_t *>(&TX_ADDR::Reset), TX_ADDR::byteWidth);

        uint64_t result_tx = 0u;
        readRegister(Register::TX_ADDR, reinterpret_cast<uint8_t *>(&result_tx), TX_ADDR::byteWidth);

        if (result_tx != TX_ADDR::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P0 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P0, RX_PW_P0::Reset);
        if (readRegister(Register::RX_PW_P0) != RX_PW_P0::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P1 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P1, RX_PW_P1::Reset);
        if (readRegister(Register::RX_PW_P1) != RX_PW_P1::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P2 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P2, RX_PW_P2::Reset);
        if (readRegister(Register::RX_PW_P2) != RX_PW_P2::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P3 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P3, RX_PW_P3::Reset);
        if (readRegister(Register::RX_PW_P3) != RX_PW_P3::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P4 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P4, RX_PW_P4::Reset);
        if (readRegister(Register::RX_PW_P4) != RX_PW_P4::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        RX_PW_P5 Register
        ------------------------------------------------*/
        writeRegister(Register::RX_PW_P5, RX_PW_P5::Reset);
        if (readRegister(Register::RX_PW_P5) != RX_PW_P5::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        DYNPD Register
        ------------------------------------------------*/
        writeRegister(Register::DYNPD, DYNPD::Reset);
        if (readRegister(Register::DYNPD) != DYNPD::Reset)
        {
            eraseStatus = false;
        }

        /*------------------------------------------------
        FEATURE Register
        ------------------------------------------------*/
        writeRegister(Register::FEATURE, FEATURE::Reset);
        if (readRegister(Register::FEATURE) != FEATURE::Reset)
        {
            eraseStatus = false;
        }

        return eraseStatus;
    }

    bool NRF24L01::begin()
    {
        /*-------------------------------------------------
        Setup the MCU hardware to the correct state
        -------------------------------------------------*/
        spiInit();

        /*-------------------------------------------------
        Check we can talk to the device and reset it back to
        some known initial condition.
        -------------------------------------------------*/
        if (!isConnected())
        {
            oopsies = FailureCode::NOT_CONNECTED;
            return false;
        }

        if (!erase())
        {
            oopsies = FailureCode::COULD_NOT_ERASE;
            return false;
        }

        /*-------------------------------------------------
        Enable 16-bit CRC
        -------------------------------------------------*/
        setCRCLength(CRCLength::CRC_16);

        /*-------------------------------------------------
        Set 1500uS timeout and 3 retry attempts. Don't lower
        or the 250KBS mode will break.
        -------------------------------------------------*/
        setRetries(AutoRetransmitDelay::w1500uS, 3);

        /*-------------------------------------------------
        Check whether or not we have a P variant of the chip
        -------------------------------------------------*/
        pVariant = setDataRate(DataRate::DR_250KBPS);

        /*-------------------------------------------------
        Set datarate to the slowest, most reliable speed supported by all hardware
        -------------------------------------------------*/
        setDataRate(DataRate::DR_1MBPS);

        /*------------------------------------------------
        Default to the 5 byte wide addressing scheme
        ------------------------------------------------*/
        setAddressWidth(AddressWidth::AW_5Byte);

        /*-------------------------------------------------
        Set the default channel to a value that likely won't congest the spectrum
        -------------------------------------------------*/
        setChannel(76);

        /*-------------------------------------------------
        Clear the buffers to start with a clean slate
        -------------------------------------------------*/
        flushTX();
        flushRX();

        /*-------------------------------------------------
        Power up the module and enable PTX. Stay in standby mode by not writing CE high.
        Delay a few milliseconds to let things settle.
        -------------------------------------------------*/
        clearChipEnable();
        powerUp();
        delayMilliseconds(5);

        initialized = true;
        return initialized;
    }

    bool NRF24L01::isInitialized()
    {
        return initialized;
    }

    void NRF24L01::startListening()
    {
        if (!listening)
        {
            /*-------------------------------------------------
            If we are auto-acknowledging RX packets with a payload, make sure the TX
            FIFO is clean so we don't accidently transmit data.
            -------------------------------------------------*/
            if (registerIsBitmaskSet(Register::FEATURE, FEATURE::EN_ACK_PAY))
            {
                flushTX();
            }

            /*-------------------------------------------------
            Clear interrupt flags and transition to RX mode
            -------------------------------------------------*/
            setRegisterBits(Register::STATUS, STATUS::RX_DR | STATUS::TX_DS | STATUS::MAX_RT);
            setRegisterBits(Register::CONFIG, CONFIG::PRIM_RX);
            setChipEnable();
            currentMode = Mode::RX;

            /*------------------------------------------------
            If we clobbered the old pipe 0 listening address so we could transmit
            something, restore it back.
            ------------------------------------------------*/
            if (cachedPipe0RXAddress)
            {
                openReadPipe(0, cachedPipe0RXAddress, true);
            }

            listening = true;
            listeningPaused = false;
        }
    }

    void NRF24L01::pauseListening()
    {
        if (listening && !listeningPaused)
        {
            clearChipEnable();
            listeningPaused = true;
        }
    }

    void NRF24L01::resumeListening()
    {
        if (listeningPaused)
        {
            setChipEnable();
            listeningPaused = false;
        }
    }

    void NRF24L01::stopListening()
    {
        if (listening || listeningPaused)
        {
            /*-------------------------------------------------
            Set the chip into standby mode I
            -------------------------------------------------*/
            clearChipEnable();
            currentMode = Mode::STANDBY_I;
            delayMilliseconds(1);

            /*-------------------------------------------------
            If we are auto-acknowledging RX packets with a payload, make sure the TX FIFO is clean so
            we don't accidentally transmit data the next time we write chipEnable high.
            -------------------------------------------------*/
            if (registerIsBitmaskSet(Register::FEATURE, FEATURE::EN_ACK_PAY))
            {
                flushTX();
            }

            /*-------------------------------------------------
            Disable RX/Enable TX
            -------------------------------------------------*/
            clearRegisterBits(Register::CONFIG, CONFIG::PRIM_RX);

            /*-------------------------------------------------
            Ensure RX Pipe 0 can listen (TX only transmits/receives on pipe 0)
            -------------------------------------------------*/
            setRegisterBits(Register::EN_RXADDR, pipeEnableRXAddressReg[0]);

            listening = false;
        }
    }

    bool NRF24L01::available()
    {
        return !rxFifoEmpty();
    }

    bool NRF24L01::available(uint8_t &pipe_num)
    {
        pipe_num = 0xFF;

        /*-------------------------------------------------
        Figure out which pipe has data available
        -------------------------------------------------*/
        if (available())
        {
            pipe_num = (getStatus() >> STATUS::RX_P_NO_Pos) & STATUS::RX_P_NO_Wid;
            return true;
        }

        return false;
    }

    void NRF24L01::read(uint8_t *const buffer, size_t len)
    {
        readPayload(buffer, len);

        /*------------------------------------------------
        Clear the ISR flag bits by setting them to 1
        ------------------------------------------------*/
        uint8_t statusVal = STATUS::RX_DR | STATUS::MAX_RT | STATUS::TX_DS;
        writeRegister(Register::STATUS, statusVal);
    }

    bool NRF24L01::writeFast(const uint8_t *const buffer, uint8_t len, const bool multicast)
    {
        /*------------------------------------------------
        Don't clobber the RX if we are listening
        ------------------------------------------------*/
        if (listening)
        {
            oopsies = FailureCode::RADIO_IN_RX_MODE;
            return false;
        }

        /*-------------------------------------------------
        Wait for the FIFO to have room for one more packet
        -------------------------------------------------*/
        uint32_t startTime = millis();

        while (txFIFOFull())
        {
            /*-------------------------------------------------
            If max retries hit from a previous transmission, we screwed up
            -------------------------------------------------*/
            if (registerIsBitmaskSet(Register::STATUS, STATUS::MAX_RT))
            {
                setRegisterBits(Register::STATUS, STATUS::MAX_RT);
                oopsies = FailureCode::MAX_RETRY_TIMEOUT;
                return false;
            }

            /*------------------------------------------------
            Make sure we aren't waiting around forever
            ------------------------------------------------*/
            if ((millis() - startTime) > DFLT_TIMEOUT_MS)
            {
                oopsies = FailureCode::TX_FIFO_FULL_TIMEOUT;
                return false;
            }

            delayMilliseconds(MIN_TIMEOUT_MS);
        }

        /*------------------------------------------------
        We're free! Load the data into the FIFO and kick off the transfer
        ------------------------------------------------*/
        startFastWrite(buffer, len, multicast, true);
        return true;
    }

    void NRF24L01::openWritePipe(const uint64_t address)
    {
        /*-------------------------------------------------
        Set the receive address for pipe 0 to be equal to the transmit address. This is to allow
        reception of an ACK packet should it be sent from the receiver.
        -------------------------------------------------*/
        writeRegister(Register::RX_ADDR_P0, reinterpret_cast<const uint8_t *>(&address), addressWidth);

        /*-------------------------------------------------
        Make sure we transmit back to the same address we expect receive from
        -------------------------------------------------*/
        writeRegister(Register::TX_ADDR, reinterpret_cast<const uint8_t *>(&address), addressWidth);

        /*-------------------------------------------------
        Set a static payload length for all receptions on pipe 0. There must also be
        an equal number of bytes clocked into the TX_FIFO when data is transmitted out.
        -------------------------------------------------*/
        writeRegister(Register::RX_PW_P0, payloadSize);

#if defined(TRACK_REGISTER_STATES)
        registers.tx_addr.update(this);
        registers.rx_pw_p0.update(this);
        registers.rx_addr_p0.update(this);
#endif
    }

    bool NRF24L01::openReadPipe(const uint8_t pipe, const uint64_t address, const bool validate)
    {
        /*------------------------------------------------
        Make sure the pipe is ok
        ------------------------------------------------*/
        if (pipe >= MAX_NUM_PIPES)
        {
            oopsies = FailureCode::INVALID_PIPE;
            return false;
        }

        /*-------------------------------------------------
        Assign the address for the pipe to listen against
        -------------------------------------------------*/
        if (pipe < 2)
        {
            /*-------------------------------------------------
            Write only as many address bytes as were set in SETUP_AW
            -------------------------------------------------*/
            uint8_t addressWidth = getAddressBytes();
            writeRegister(pipeRXAddressReg[pipe], reinterpret_cast<const uint8_t *>(&address), addressWidth);

            /*------------------------------------------------
            Save the pipe 0 address because it can be clobbered by openWritePipe() and
            will need to be restored later when we start listening again.
            ------------------------------------------------*/
            if (pipe == 0)
            {
                memcpy(&cachedPipe0RXAddress, &address, addressWidth);
            }

            /*------------------------------------------------
            Optionally validate the write
            ------------------------------------------------*/
            if (validate)
            {
                uint64_t setVal = 0u;
                readRegister(pipeRXAddressReg[pipe], reinterpret_cast<uint8_t *>(&setVal), addressWidth);

                if (setVal != (address & 0xFFFFFFFFFF))
                {
                    oopsies = FailureCode::REGISTER_WRITE_FAILURE;
                    return false;
                }
            }
        }
        else
        {
            /*------------------------------------------------
            These pipes only need their LSB set
            ------------------------------------------------*/
            writeRegister(pipeRXAddressReg[pipe], reinterpret_cast<const uint8_t *>(&address), 1);

            /*------------------------------------------------
            Optionally validate the write
            ------------------------------------------------*/
            if (validate)
            {
                uint8_t setVal = 0u;
                readRegister(pipeRXAddressReg[pipe], &setVal, 1);

                if (setVal != (address & 0xFF))
                {
                    oopsies = FailureCode::REGISTER_WRITE_FAILURE;
                    return false;
                }
            }
        }

        /*-------------------------------------------------
        Let the pipe know how wide the payload will be, then turn it on
        -------------------------------------------------*/
        writeRegister(pipeRXPayloadWidthReg[pipe], payloadSize);
        setRegisterBits(Register::EN_RXADDR, pipeEnableRXAddressReg[pipe]);

#if defined(TRACK_REGISTER_STATES)
        registers.en_rxaddr.update(this);
        registers.rx_addr_p0.update(this);
        registers.rx_addr_p1.update(this);
        registers.rx_addr_p2.update(this);
        registers.rx_addr_p3.update(this);
        registers.rx_addr_p4.update(this);
        registers.rx_addr_p5.update(this);
#endif

        return true;
    }

    bool NRF24L01::isConnected()
    {
        /*------------------------------------------------
        Grab the old settings for the register
        ------------------------------------------------*/
        uint8_t old_setup = readRegister(Register::SETUP_AW);
        uint8_t illegal_setup = 0u;

        /*------------------------------------------------
        Write some new settings and record their value
        ------------------------------------------------*/
        writeRegister(Register::SETUP_AW, illegal_setup);
        uint8_t new_setup = readRegister(Register::SETUP_AW);

        /*------------------------------------------------
        We are connected if the recorded settings match what we thought
        was set. Reset back to the old settings before exiting.
        ------------------------------------------------*/
        if (new_setup == illegal_setup)
        {
            writeRegister(Register::SETUP_AW, old_setup);
            return true;
        }
        else
        {
            oopsies = FailureCode::NOT_CONNECTED;
            return false;
        }
    }

    bool NRF24L01::rxFifoFull()
    {
        uint8_t reg = readRegister(Register::FIFO_STATUS);

#if defined(TRACK_REGISTER_STATES)
        registers.fifo_status = reg;
#endif

        return reg & FIFO_STATUS::RX_FULL;
    }

    bool NRF24L01::rxFifoEmpty()
    {
        uint8_t reg = readRegister(Register::FIFO_STATUS);

#if defined(TRACK_REGISTER_STATES)
        registers.fifo_status = reg;
#endif

        return reg & FIFO_STATUS::RX_EMPTY;
    }

    bool NRF24L01::txFIFOFull()
    {
        uint8_t reg = readRegister(Register::FIFO_STATUS);

#if defined(TRACK_REGISTER_STATES)
        registers.fifo_status = reg;
#endif

        return reg & FIFO_STATUS::TX_FULL;
    }

    bool NRF24L01::txFIFOEmpty()
    {
        uint8_t reg = readRegister(Register::FIFO_STATUS);

#if defined(TRACK_REGISTER_STATES)
        registers.fifo_status = reg;
#endif

        return reg & FIFO_STATUS::TX_EMPTY;
    }

    void NRF24L01::powerUp()
    {
        /*-------------------------------------------------
        If not powered up already, do it. The worst startup delay is
        about 5mS, so just wait that amount.
        -------------------------------------------------*/
        if (!registerIsBitmaskSet(Register::CONFIG, CONFIG::PWR_UP))
        {
            writeRegister(Register::CONFIG, CONFIG::PWR_UP);
            delayMilliseconds(5);

            currentMode = Mode::STANDBY_I;

#if defined(TRACK_REGISTER_STATES)
            registers.config.update(this);
#endif
        }
    }

    void NRF24L01::powerDown()
    {
        /*-------------------------------------------------
        Force standby mode and power down the chip
        -------------------------------------------------*/
        clearChipEnable();
        clearRegisterBits(Register::CONFIG, CONFIG::PWR_UP);
        currentMode = Mode::POWER_DOWN;

#if defined(TRACK_REGISTER_STATES)
        registers.config.update(this);
#endif
    }

    void NRF24L01::startFastWrite(const uint8_t *const buffer, size_t len, const bool multicast, const bool startTX)
    {
        uint8_t payloadType = 0u;

        if (multicast)
        {
            /*-------------------------------------------------
            Transmit one packet without waiting for an ACK from the RX device. In order for
            this to work, the Features register has to be enabled and EN_DYN_ACK set.
            -------------------------------------------------*/
            enableDynamicAck();
            payloadType = Command::W_TX_PAYLOAD_NO_ACK;
        }
        else
        {
            /*-------------------------------------------------
            Force waiting for the RX device to send an ACK packet. Don't bother disabling Dynamic Ack
            (should it even be enabled) as this command overrides it.
            -------------------------------------------------*/
            payloadType = Command::W_TX_PAYLOAD;
        }

        /*-------------------------------------------------
        Write the payload to the TX FIFO and optionally start the transfer
        -------------------------------------------------*/
        writePayload(buffer, len, payloadType);

        if (startTX)
        {
            setChipEnable();
            currentMode = Mode::TX;
        }
    }

    bool NRF24L01::txStandBy()
    {
        /*-------------------------------------------------
        Wait for the TX FIFO to signal it's empty
        -------------------------------------------------*/
        while (!txFIFOEmpty())
        {
            /*-------------------------------------------------
            If we hit the Max Retries, we have a problem and the whole TX FIFO is screwed.
            Go back to standby mode and clear out the FIFO.
            -------------------------------------------------*/
            if (registerIsBitmaskSet(Register::STATUS, STATUS::MAX_RT))
            {
                setRegisterBits(Register::STATUS, STATUS::MAX_RT);
                clearChipEnable();
                flushTX();

                currentMode = Mode::STANDBY_I;
                return false;
            }
        }

        /*------------------------------------------------
        Sends the chip back to standby mode now that the FIFO is empty
        ------------------------------------------------*/
        clearChipEnable();
        currentMode = Mode::STANDBY_I;
        return true;
    }

    bool NRF24L01::txStandBy(const uint32_t timeout, const bool startTx)
    {
        /*------------------------------------------------
        Optionally start a new transfer
        ------------------------------------------------*/
        if (startTx)
        {
            stopListening();
            setChipEnable();
        }

        /*------------------------------------------------
        Prevent the user from executing the function if they haven't told
        the device to quit listening yet.
        ------------------------------------------------*/
        if (listening)
        {
            oopsies = FailureCode::RADIO_IN_RX_MODE;
            return false;
        }

        /*------------------------------------------------
        Wait for the TX FIFO to empty, retrying packet transmissions as needed.
        ------------------------------------------------*/
        uint32_t start = millis();

        while (!txFIFOEmpty())
        {
            /*------------------------------------------------
            If max retries interrupt occurs, retry transmission. The data is
            automatically kept in the TX FIFO.
            ------------------------------------------------*/
            if (registerIsBitmaskSet(Register::STATUS, STATUS::MAX_RT))
            {
                clearChipEnable();
                setRegisterBits(Register::STATUS, STATUS::MAX_RT);

                delayMilliseconds(1);
                setChipEnable();
            }

            /*------------------------------------------------
            Automatic timeout failure
            ------------------------------------------------*/
            if ((millis() - start) > timeout)
            {
                clearChipEnable();
                flushTX();
                oopsies = FailureCode::TX_FIFO_EMPTY_TIMEOUT;
                currentMode = Mode::STANDBY_I;
                return false;
            }
        }

        /*------------------------------------------------
        Transition back to Standby Mode I
        ------------------------------------------------*/
        clearChipEnable();
        currentMode = Mode::STANDBY_I;
        return true;
    }

    void NRF24L01::writeAckPayload(const uint8_t pipe, const uint8_t *const buffer, size_t len)
    {
        //TODO: Magic numbers abound in this function. Get rid of them.
        size_t size = std::min(len, static_cast<size_t>(32)) + 1u;

        spi_txbuff[0] = Command::W_ACK_PAYLOAD | (pipe & 0x07);
        memcpy(&spi_txbuff[1], buffer, size);

        beginTransaction();
        spiWrite(spi_txbuff.begin(), size);
        endTransaction();
    }

    bool NRF24L01::isAckPayloadAvailable()
    {
        uint8_t reg = readRegister(Register::FIFO_STATUS);

#if defined(TRACK_REGISTER_STATES)
        registers.fifo_status = reg;
#endif

        return !(reg & FIFO_STATUS::RX_EMPTY);
    }

    void NRF24L01::whatHappened(bool &tx_ok, bool &tx_fail, bool &rx_ready)
    {
        uint8_t statusActual = readRegister(Register::STATUS);
        uint8_t statusCleared = statusActual | STATUS::RX_DR | STATUS::TX_DS | STATUS::MAX_RT;
        writeRegister(Register::STATUS, statusCleared);

        tx_ok = statusActual & STATUS::TX_DS;
        tx_fail = statusActual & STATUS::MAX_RT;
        rx_ready = statusActual & STATUS::RX_DR;
    }

    void NRF24L01::reUseTX()
    {
        writeRegister(Register::STATUS, STATUS::MAX_RT);
        writeCMD(Command::REUSE_TX_PL);
        clearChipEnable();
        setChipEnable();
    }

    void NRF24L01::closeReadPipe(const uint8_t pipe)
    {
        if (pipe < MAX_NUM_PIPES)
        {
            uint8_t rxaddrVal = readRegister(Register::EN_RXADDR) & ~pipeEnableRXAddressReg[pipe];
            writeRegister(Register::EN_RXADDR, rxaddrVal);

#if defined(TRACK_REGISTER_STATES)
            registers.en_rxaddr = rxaddrVal;
#endif
        }
    }

    void NRF24L01::setAddressWidth(const AddressWidth address_width)
    {
        writeRegister(Register::SETUP_AW, static_cast<uint8_t>(address_width));

        switch (address_width)
        {
        case NRF24L::AddressWidth::AW_3Byte:
            addressWidth = 3;
            break;

        case NRF24L::AddressWidth::AW_4Byte:
            addressWidth = 4;
            break;

        case NRF24L::AddressWidth::AW_5Byte:
            addressWidth = 5;
            break;
        }

#if defined(TRACK_REGISTER_STATES)
        registers.setup_aw.update(this);
#endif
    }

    AddressWidth NRF24L01::getAddressWidth()
    {
        auto reg = readRegister(Register::SETUP_AW);

#if defined(TRACK_REGISTER_STATES)
        registers.setup_aw = reg;
#endif

        return static_cast<AddressWidth>(reg);
    }

    uint8_t NRF24L01::getAddressBytes()
    {
        switch (getAddressWidth())
        {
        case NRF24L::AddressWidth::AW_3Byte:
            return 3;
            break;

        case NRF24L::AddressWidth::AW_4Byte:
            return 4;
            break;

        case NRF24L::AddressWidth::AW_5Byte:
            return 5;
            break;
        }
    }

    bool NRF24L01::setRetries(const AutoRetransmitDelay delay, const uint8_t count, const bool validate)
    {
        bool returnVal = true;
        uint8_t ard = (static_cast<uint8_t>(delay) & 0x0F) << SETUP_RETR::ARD_Pos;
        uint8_t arc = (count & 0x0F) << SETUP_RETR::ARC_Pos;
        uint8_t setup_retr = ard | arc;

        writeRegister(Register::SETUP_RETR, setup_retr);

        if (validate)
        {
            returnVal = (readRegister(Register::SETUP_RETR) == setup_retr);
        }

#if defined(TRACK_REGISTER_STATES)
        if (returnVal)
        {
            registers.setup_retr = setup_retr;
        }
#endif

        return returnVal;
    }

    void NRF24L01::setStaticPayloadSize(const uint8_t size)
    {
        payloadSize = std::min(size, static_cast<uint8_t>(32));
    }

    bool NRF24L01::setChannel(const uint8_t channel, const bool validate)
    {
        writeRegister(Register::RF_CH, channel & RF_CH::Mask);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_ch.update(this);
#endif

        if (validate)
        {
            return (readRegister(Register::RF_CH) == (channel & RF_CH::Mask));
        }

        return true;
    }

    uint8_t NRF24L01::getChannel()
    {
        uint8_t ch = readRegister(Register::RF_CH);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_ch = ch;
#endif

        return ch;
    }

    uint8_t NRF24L01::getStaticPayloadSize()
    {
        return payloadSize;
    }

    uint8_t NRF24L01::getDynamicPayloadSize()
    {
        uint8_t result = MAX_PAYLOAD_WIDTH;

        if (dynamicPayloadsEnabled)
        {
            spi_txbuff[0] = Command::R_RX_PL_WID;
            spi_rxbuff[1] = Command::NOP;

            beginTransaction();
            spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), 2);
            endTransaction();

            result = spi_rxbuff[1];

            if (result > 32)
            {
                flushRX();
                delayMilliseconds(2);
                return 0;
            }
        }

        return result;
    }

    uint8_t NRF24L01::flushTX()
    {
        return writeCMD(Command::FLUSH_TX);
    }

    uint8_t NRF24L01::flushRX()
    {
        return writeCMD(Command::FLUSH_RX);
    }

    void NRF24L01::activateFeatures()
    {
        if (!featuresActivated)
        {
            spi_txbuff[0] = Command::ACTIVATE;
            spi_txbuff[1] = 0x73;

            spiWrite(spi_txbuff.begin(), 2);
            featuresActivated = true;
        }
    }

    void NRF24L01::deactivateFeatures()
    {
        if (featuresActivated)
        {
            /*-------------------------------------------------
            Sending the activation command sequence again also disables the features
            -------------------------------------------------*/
            activateFeatures();
            featuresActivated = false;
        }
    }

    void NRF24L01::enableAckPayload()
    {
        activateFeatures();
        setRegisterBits(Register::FEATURE, FEATURE::EN_ACK_PAY | FEATURE::EN_DPL);
        setRegisterBits(Register::DYNPD, DYNPD::DPL_P0 | DYNPD::DPL_P1);

        dynamicPayloadsEnabled = true;

#if defined(TRACK_REGISTER_STATES)
        registers.dynpd.update(this);
        registers.feature.update(this);
#endif
    }

    void NRF24L01::disableAckPayload()
    {
        if (featuresActivated)
        {
            clearRegisterBits(Register::FEATURE, FEATURE::EN_ACK_PAY | FEATURE::EN_DPL);
            clearRegisterBits(Register::DYNPD, DYNPD::DPL_P0 | DYNPD::DPL_P1);

            dynamicPayloadsEnabled = false;

#if defined(TRACK_REGISTER_STATES)
            registers.dynpd.update(this);
            registers.feature.update(this);
#endif
        }
    }

    void NRF24L01::enableDynamicPayloads()
    {
        /*-------------------------------------------------
        Send the activate command to enable selection of features
        -------------------------------------------------*/
        activateFeatures();

        /*-------------------------------------------------
        Enable the dynamic payload feature bit
        -------------------------------------------------*/
        setRegisterBits(Register::FEATURE, FEATURE::EN_DPL);

        /*-------------------------------------------------
        Enable dynamic payload on all pipes. This requires that
        auto-acknowledge be enabled.
        -------------------------------------------------*/
        setRegisterBits(Register::EN_AA, EN_AA::Mask);
        setRegisterBits(Register::DYNPD, DYNPD::Mask);

        dynamicPayloadsEnabled = true;

#if defined(TRACK_REGISTER_STATES)
        registers.dynpd.update(this);
        registers.en_aa.update(this);
        registers.feature.update(this);
#endif
    }

    void NRF24L01::disableDynamicPayloads()
    {
        /*-------------------------------------------------
        Disable for all pipes
        -------------------------------------------------*/
        if (featuresActivated)
        {
            clearRegisterBits(Register::DYNPD, DYNPD::Mask);
            clearRegisterBits(Register::EN_AA, EN_AA::Mask);
            clearRegisterBits(Register::FEATURE, FEATURE::EN_DPL);

            dynamicPayloadsEnabled = false;

#if defined(TRACK_REGISTER_STATES)
            registers.dynpd.update(this);
            registers.en_aa.update(this);
            registers.feature.update(this);
#endif
        }
    }

    void NRF24L01::enableDynamicAck()
    {
        activateFeatures();
        setRegisterBits(Register::FEATURE, FEATURE::EN_DYN_ACK);

#if defined(TRACK_REGISTER_STATES)
        registers.feature.update(this);
#endif
    }

    void NRF24L01::disableDynamicAck()
    {
        if (featuresActivated)
        {
            clearRegisterBits(Register::FEATURE, FEATURE::EN_DYN_ACK);

#if defined(TRACK_REGISTER_STATES)
            registers.feature.update(this);
#endif
        }
    }

    bool NRF24L01::isPVariant()
    {
        return pVariant;
    }

    void NRF24L01::setAutoAckAll(const bool enable)
    {
        if (enable)
        {
            setRegisterBits(Register::EN_AA, EN_AA::Mask);
        }
        else
        {
            clearRegisterBits(Register::EN_AA, EN_AA::Mask);
        }

#if defined(TRACK_REGISTER_STATES)
        registers.en_aa.update(this);
#endif
    }

    bool NRF24L01::setAutoAck(const uint8_t pipe, const bool enable, const bool validate)
    {
        bool returnVal = true;

        if (pipe < MAX_NUM_PIPES)
        {
            uint8_t en_aa = readRegister(Register::EN_AA);

            if (enable)
            {
                en_aa |= 1u << pipe;
            }
            else
            {
                en_aa &= ~(1u << pipe);
            }

            writeRegister(Register::EN_AA, en_aa);

            if (validate)
            {
                returnVal = (readRegister(Register::EN_AA) == en_aa);
            }

#if defined(TRACK_REGISTER_STATES)
            if (returnVal)
            {
                registers.en_aa = en_aa;
            }
#endif
        }
        else
        {
            returnVal = false;
        }

        return returnVal;
    }

    bool NRF24L01::setPALevel(const PowerAmplitude level, const bool validate)
    {
        /*-------------------------------------------------
        Merge bits from level into setup according to a mask
        https://graphics.stanford.edu/~seander/bithacks.html#MaskedMerge
        -------------------------------------------------*/
        uint8_t setup = readRegister(Register::RF_SETUP);
        setup ^= (setup ^ static_cast<uint8_t>(level)) & RF_SETUP::RF_PWR_Msk;

        writeRegister(Register::RF_SETUP, setup);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_setup = setup;
#endif

        if (validate)
        {
            return (readRegister(Register::RF_SETUP) == setup);
        }

        return true;
    }

    PowerAmplitude NRF24L01::getPALevel()
    {
        uint8_t setup = readRegister(Register::RF_SETUP);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_setup = setup;
#endif

        return static_cast<PowerAmplitude>((setup & RF_SETUP::RF_PWR) >> 1);
    }

    bool NRF24L01::setDataRate(const DataRate speed)
    {
        uint8_t setup = readRegister(Register::RF_SETUP);

        switch (speed)
        {
        case DataRate::DR_250KBPS:
            if (pVariant)
            {
                setup |= RF_SETUP::RF_DR_LOW;
                setup &= ~RF_SETUP::RF_DR_HIGH;
            }
            else
            {
                return false;
            }
            break;

        case DataRate::DR_1MBPS:
            setup &= ~(RF_SETUP::RF_DR_HIGH | RF_SETUP::RF_DR_LOW);
            break;

        case DataRate::DR_2MBPS:
            setup &= ~RF_SETUP::RF_DR_LOW;
            setup |= RF_SETUP::RF_DR_HIGH;
            break;

        default:
            break;
        }

        writeRegister(Register::RF_SETUP, setup);
        auto result = readRegister(Register::RF_SETUP);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_setup = result;
#endif

        return (result == setup);
    }

    DataRate NRF24L01::getDataRate()
    {
        uint8_t reg = readRegister(Register::RF_SETUP);

#if defined(TRACK_REGISTER_STATES)
        registers.rf_setup = reg;
#endif

        return static_cast<DataRate>(reg & (RF_SETUP::RF_DR_HIGH | RF_SETUP::RF_DR_LOW));
    }

    void NRF24L01::setCRCLength(const CRCLength length)
    {
        uint8_t config = readRegister(Register::CONFIG) & ~(CONFIG::CRCO | CONFIG::EN_CRC);

        switch (length)
        {
        case CRCLength::CRC_8:
            config |= CONFIG::EN_CRC;
            config &= ~CONFIG::CRCO;
            break;

        case CRCLength::CRC_16:
            config |= CONFIG::EN_CRC | CONFIG::CRCO;
            break;

        default:
            break;
        }

        writeRegister(Register::CONFIG, config);

#if defined(TRACK_REGISTER_STATES)
        registers.config = config;
#endif
    }

    CRCLength NRF24L01::getCRCLength()
    {
        CRCLength result = CRCLength::CRC_DISABLED;

        uint8_t config = readRegister(Register::CONFIG) & (CONFIG::CRCO | CONFIG::EN_CRC);
        uint8_t en_aa = readRegister(Register::EN_AA);

        if ((config & CONFIG::EN_CRC) || en_aa)
        {
            if (config & CONFIG::CRCO)
            {
                result = CRCLength::CRC_16;
            }
            else
            {
                result = CRCLength::CRC_8;
            }
        }

#if defined(TRACK_REGISTER_STATES)
        registers.config = config;
        registers.en_aa = en_aa;
#endif

        return result;
    }

    void NRF24L01::disableCRC()
    {
        uint8_t disable = readRegister(Register::CONFIG) & ~CONFIG::EN_CRC;
        writeRegister(Register::CONFIG, disable);
    }

    void NRF24L01::maskIRQ(const bool tx_ok, const bool tx_fail, const bool rx_ready)
    {
        uint8_t config = readRegister(Register::CONFIG);

        config &= ~(CONFIG::MASK_MAX_RT | CONFIG::MASK_TX_DS | CONFIG::MASK_RX_DR);
        config |= (tx_fail << CONFIG::MASK_MAX_RT_Pos) | (tx_ok << CONFIG::MASK_TX_DS_Pos) | (rx_ready << CONFIG::MASK_RX_DR_Pos);

        writeRegister(Register::CONFIG, config);

#if defined(TRACK_REGISTER_STATES)
        registers.config = config;
#endif
    }

    uint8_t NRF24L01::readRegister(const uint8_t reg, uint8_t *const buf, size_t len)
    {
        if (len > MAX_PAYLOAD_WIDTH)
        {
            len = MAX_PAYLOAD_WIDTH;
        }

        spi_txbuff[0] = (Command::R_REGISTER | (Command::REGISTER_MASK & reg));
        memset(&spi_txbuff[1], Command::NOP, len);

        /*------------------------------------------------
        Read the data out, adding 1 byte for the command instruction
        ------------------------------------------------*/
        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), len + 1);
        endTransaction();

        memcpy(buf, &spi_rxbuff[1], len);

        /* Return only the status code of the chip. The register values will be in the rx buff */
        return spi_rxbuff[0];
    }

    uint8_t NRF24L01::readRegister(const uint8_t reg)
    {
        size_t txLength = 2;
        spi_txbuff[0] = (Command::R_REGISTER | (Command::REGISTER_MASK & reg));
        spi_txbuff[1] = Command::NOP;

        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), txLength);
        endTransaction();

        /* Current register value is in the second byte of the receive buffer */
        return spi_rxbuff[1];
    }

    uint8_t NRF24L01::writeRegister(const uint8_t reg, const uint8_t *const buf, size_t len)
    {
        if (len > MAX_PAYLOAD_WIDTH)
        {
            len = MAX_PAYLOAD_WIDTH;
        }

        spi_txbuff[0] = (Command::W_REGISTER | (Command::REGISTER_MASK & reg));
        memcpy(&spi_txbuff[1], buf, len);

        /*------------------------------------------------
        Write the data out, adding 1 byte for the command instruction
        ------------------------------------------------*/
        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), len + 1);
        endTransaction();

        /* Status code is in the first byte of the receive buffer */
        return spi_rxbuff[0];
    }

    uint8_t NRF24L01::writeRegister(const uint8_t reg, const uint8_t value)
    {
        size_t txLength = 2;
        spi_txbuff[0] = (Command::W_REGISTER | (Command::REGISTER_MASK & reg));
        spi_txbuff[1] = value;

        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), txLength);
        endTransaction();

        /* Status code is in the first byte of the receive buffer */
        return spi_rxbuff[0];
    }

    uint8_t NRF24L01::writePayload(const uint8_t *const buf, size_t len, const uint8_t writeType)
    {
        if ((writeType != Command::W_TX_PAYLOAD_NO_ACK) && (writeType != Command::W_TX_PAYLOAD))
        {
            return 0u;
        }

        /*-------------------------------------------------
        Calculate the number of bytes that do nothing. When dynamic payloads are enabled, the length
        doesn't matter as long as it is less than the max payload width (32).
        -------------------------------------------------*/
        len = std::min(len, payloadSize);
        uint8_t blank_len = static_cast<uint8_t>(dynamicPayloadsEnabled ? 0 : (payloadSize - len));
        size_t size = len + blank_len + 1;

        /*-------------------------------------------------
        Format the write command and fill the rest with zeros
        -------------------------------------------------*/
        memset(spi_txbuff.begin(), 0xff, spi_txbuff.size());

        spi_txbuff[0] = writeType;                  /* Write command type */
        memcpy(&spi_txbuff[1], buf, len);           /* Payload information */
        memset(&spi_txbuff[len + 1], 0, blank_len); /* Null out the remaining buffer space */

        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), size);
        endTransaction();

        return spi_rxbuff[0];
    }

    uint8_t NRF24L01::readPayload(uint8_t *const buffer, size_t len)
    {
        uint8_t status = 0u;

        /*-------------------------------------------------
        The chip enable pin must be low to read out data
        -------------------------------------------------*/
        pauseListening();

        /*-------------------------------------------------
        Cap the data length
        -------------------------------------------------*/
        len = std::min(len, MAX_PAYLOAD_WIDTH);

        /*-------------------------------------------------
        Calculate the number of bytes that do nothing. This is important for
        fixed payload widths as the full width must be read out each time.
        -------------------------------------------------*/
        uint8_t blank_len = static_cast<uint8_t>(dynamicPayloadsEnabled ? 0 : (payloadSize - len));
        size_t size = len + blank_len;

        /*-------------------------------------------------
        Format the read command and fill the rest with NOPs
        -------------------------------------------------*/
        spi_txbuff[0] = Command::R_RX_PAYLOAD;
        memset(&spi_txbuff[1], Command::NOP, size);
        memset(spi_rxbuff.begin(), 0, spi_rxbuff.size());

        /*-------------------------------------------------
        Read out the payload. The +1 is for the read command.
        -------------------------------------------------*/
        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), size + 1);
        endTransaction();

        status = spi_rxbuff[0];
        memcpy(buffer, &spi_rxbuff[1], len);

        /*------------------------------------------------
        Clear (by setting) the RX_DR flag to signal we've read data
        ------------------------------------------------*/
        setRegisterBits(Register::STATUS, STATUS::RX_DR);

        /*-------------------------------------------------
        Reset the chip enable back to the initial RX state
        -------------------------------------------------*/
        resumeListening();

        return status;
    }

    uint8_t NRF24L01::getStatus()
    {
        return writeCMD(Command::NOP);
    }

    FailureCode NRF24L01::getFailureCode()
    {
        FailureCode temp = oopsies;
        oopsies = FailureCode::CLEARED;
        return temp;
    }

#if defined(USING_CHIMERA)
    void NRF24L01::spiInit()
    {
        spi->setChipSelectControlMode(Chimera::SPI::ChipSelectMode::MANUAL);
        chipEnable->setMode(Chimera::GPIO::Drive::OUTPUT_PUSH_PULL, false);
        chipEnable->setState(Chimera::GPIO::State::LOW);

        spi->setChipSelect(Chimera::GPIO::State::LOW);
        spi->setChipSelect(Chimera::GPIO::State::HIGH);
    }

    size_t NRF24L01::spiWrite(const uint8_t *const tx_buffer, size_t len)
    {
        spi->writeBytes(tx_buffer, len, false);
        return len;
    }

    size_t NRF24L01::spiRead(uint8_t *const rx_buffer, size_t len)
    {
        spi->readBytes(rx_buffer, len, false);
        return len;
    }

    size_t NRF24L01::spiWriteRead(const uint8_t *const tx_buffer, uint8_t *const rx_buffer, size_t len)
    {
        spi->readWriteBytes(tx_buffer, rx_buffer, len, false);
        return len;
    }

    void NRF24L01::beginTransaction()
    {
        spi->setChipSelect(Chimera::GPIO::State::LOW);
    }

    void NRF24L01::endTransaction()
    {
        spi->setChipSelect(Chimera::GPIO::State::HIGH);
    }

    void NRF24L01::setChipEnable()
    {
        chipEnable->setState(Chimera::GPIO::State::HIGH);
    }

    void NRF24L01::clearChipEnable()
    {
        chipEnable->setState(Chimera::GPIO::State::LOW);
    }

    bool NRF24L01::getChipEnableState()
    {
        Chimera::GPIO::State state;
        chipEnable->getState(state);
        return static_cast<bool>(state);
    }

    void NRF24L01::delayMilliseconds(uint32_t ms)
    {
        Chimera::delayMilliseconds(ms);
    }

    uint32_t NRF24L01::millis()
    {
        return Chimera::millis();
    }
#endif

    uint8_t NRF24L01::writeCMD(const uint8_t cmd)
    {
        size_t txLength = 1;
        spi_txbuff[0] = cmd;

        beginTransaction();
        spiWriteRead(spi_txbuff.begin(), spi_rxbuff.begin(), txLength);
        endTransaction();

        return spi_rxbuff[0];
    }

    bool NRF24L01::registerIsBitmaskSet(const uint8_t reg, const uint8_t bitmask)
    {
        return (readRegister(reg) & bitmask) == bitmask;
    }

    bool NRF24L01::registerIsAnySet(const uint8_t reg, const uint8_t bitmask)
    {
        return readRegister(reg) & bitmask;
    }

    void NRF24L01::clearRegisterBits(const uint8_t reg, const uint8_t bitmask)
    {
        uint8_t regVal = readRegister(reg);
        regVal &= ~bitmask;
        writeRegister(reg, regVal);
    }

    void NRF24L01::setRegisterBits(const uint8_t reg, const uint8_t bitmask)
    {
        uint8_t regVal = readRegister(reg);
        regVal |= bitmask;
        writeRegister(reg, regVal);
    }
}
