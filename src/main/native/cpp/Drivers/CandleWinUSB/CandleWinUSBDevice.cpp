/*
 * Copyright (c) 2019 REV Robotics
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of REV Robotics nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef _WIN32

#include "rev/Drivers/CandleWinUSB/CandleWinUSBDevice.h"
#include "rev/RevUSBUtils.h"

#include <iostream> //TODO: Remove
#include <thread>

#define CANDLE_DEFAULT_CHANNEL 0
#define CANDLE_DEFAULT_FLAGS    (candle_device_mode_flags_t)(CANDLE_MODE_NORMAL | CANDLE_MODE_PAD_PKTS_TO_MAX_PKT_SIZE)

namespace rev {
namespace usb {

static void close_candle_dev(candle_handle hDev)
{
    candle_dev_close(hDev);
    candle_dev_free(hDev);
}

CandleWinUSBDevice::CandleWinUSBDevice(candle_handle hDev) :
    m_thread(hDev)
{
    m_handle = hDev;
    if (candle_dev_open(hDev) == false) {
        std::cout << candle_error_text(candle_dev_last_error(hDev)) << std::endl;
        close_candle_dev(hDev);
        throw "Failed to open device!";
    }

    if (candle_channel_start(hDev, CANDLE_DEFAULT_CHANNEL, CANDLE_DEFAULT_FLAGS) == false) {
        std::cout << candle_error_text(candle_dev_last_error(hDev)) << std::endl;
        close_candle_dev(hDev);
        throw "Failed to start device channel 0!";
    }
    
    m_descriptor = candle_dev_get_path(m_handle);
    m_name = candle_dev_get_name(m_handle);
    m_thread.Start();
}

CandleWinUSBDevice::~CandleWinUSBDevice()
{
    m_thread.Stop();
    candle_channel_stop(m_handle, CANDLE_DEFAULT_CHANNEL);
    close_candle_dev(m_handle);
}

std::string CandleWinUSBDevice::GetName() const
{
    return m_name;
}


std::wstring CandleWinUSBDevice::GetDescriptor() const
{
    return m_descriptor;
}

int CandleWinUSBDevice::GetId() const
{
    return 0;
}

CANStatus CandleWinUSBDevice::SendCANMessage(const CANMessage& msg, int periodMs)
{
    m_thread.EnqueueMessage(msg, periodMs);
    return CANStatus::kOk;
}

CANStatus CandleWinUSBDevice::RecieveCANMessage(CANMessage& msg, uint32_t messageMask, uint32_t& timestamp)
{
    return CANStatus::kOk;
}

CANStatus CandleWinUSBDevice::OpenStreamSession()
{
    return CANStatus::kOk;
}
CANStatus CandleWinUSBDevice::CloseStreamSession()
{
    return CANStatus::kOk;
}
CANStatus CandleWinUSBDevice::ReadStreamSession()
{
    return CANStatus::kOk;
}

CANStatus CandleWinUSBDevice::GetCANStatus()
{
    return CANStatus::kOk;
}

bool CandleWinUSBDevice::IsConnected()
{
    return true;
}


} // namespace usb
} // namespace rev

#else
typedef int __ISOWarning__CLEAR_;
#endif // _WIN32