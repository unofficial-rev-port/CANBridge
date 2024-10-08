#ifdef __linux__

#include "rev/Drivers/SocketCAN/SocketCANDevice.h"

#include <map>
#include <string>
#include <thread>

#include <hal/simulation/CanData.h>
#include <hal/CAN.h>

namespace rev { 
namespace usb {

SocketCANDevice::SocketCANDevice(std::string port) :
    m_thread(port) {
        m_descriptor = port;
        // TODO: Get the name of the device, for now just hardcode the name
        m_name = "SocketCAN Device";
        m_thread.Start();
    }

SocketCANDevice::~SocketCANDevice() {
    m_thread.Stop();
}

std::string SocketCANDevice::GetName() const {
    return m_name;
}

std::string SocketCANDevice::GetDescriptor() const {
    return m_descriptor;
}

int SocketCANDevice::GetNumberOfErrors() {
    return m_thread.GetNumberOfErrors();
}

int SocketCANDevice::GetId() const {
    return 0;
}

CANStatus SocketCANDevice::SendCANMessage(const CANMessage& msg, int periodMs) {
    m_thread.EnqueueMessage(msg, periodMs);
    return m_thread.GetLastThreadError();
}

CANStatus SocketCANDevice::ReceiveCANMessage(std::shared_ptr<CANMessage>& msg, uint32_t messageID, uint32_t messageMask) {
    CANStatus status = CANStatus::kTimeout;
    
    // parse through the keys, find the messges the match, and return it
    // The first in the message id, then the messages
    std::map<uint32_t, std::shared_ptr<CANMessage>> messages;
    m_thread.ReceiveMessage(messages);
    std::shared_ptr<CANMessage> mostRecent;
    for (auto& m : messages) {
        if (
            CANBridge_ProcessMask({m.second->GetMessageId(), 0}, m.first)
            && CANBridge_ProcessMask({messageID, messageMask}, m.first)
            && (!mostRecent || m.second->GetTimestampUs() > mostRecent->GetTimestampUs())
        ) {
            mostRecent = m.second;
            status = CANStatus::kOk;
        }
    }

    if (status == CANStatus::kOk) {
        msg = mostRecent;
        status = m_thread.GetLastThreadError();
    } else {
        status = CANStatus::kError;
    }
    

    return status;
}

CANStatus SocketCANDevice::OpenStreamSession(uint32_t* sessionHandle, CANBridge_CANFilter filter, uint32_t maxSize) {
    CANStatus stat = CANStatus::kOk;
    m_thread.OpenStream(sessionHandle, filter, maxSize, &stat);
    return m_thread.GetLastThreadError();
}

CANStatus SocketCANDevice::CloseStreamSession(uint32_t sessionHandle) {
    m_thread.CloseStream(sessionHandle);
    return m_thread.GetLastThreadError();
}

CANStatus SocketCANDevice::ReadStreamSession(uint32_t sessionHandle, struct HAL_CANStreamMessage* msgs, uint32_t messagesToRead, uint32_t* messagesRead) {
    m_thread.ReadStream(sessionHandle, msgs, messagesToRead, messagesRead);
    return m_thread.GetLastThreadError();
}

CANStatus SocketCANDevice::GetCANDetailStatus(float* percentBusUtilization, uint32_t* busOff, uint32_t* txFull, uint32_t* receiveErr, uint32_t* transmitErr) {
    rev::usb::CANStatusDetails details;
    m_thread.GetCANStatus(&details);
    *percentBusUtilization = 0.0f;
    *busOff = details.busOffCount;
    *txFull = details.txFullCount;
    *receiveErr = details.receiveErrCount;
    *transmitErr = details.transmitErrCount;
    return m_thread.GetLastThreadError();
}

CANStatus SocketCANDevice::GetCANDetailStatus(float* percentBusUtilization, uint32_t* busOff, uint32_t* txFull, uint32_t* receiveErr, uint32_t* transmitErr, uint32_t* lastErrorTime) {
    return GetCANDetailStatus(percentBusUtilization, busOff, txFull, receiveErr, transmitErr);
}

bool SocketCANDevice::IsConnected() {
    // Implementation
    return true;
}

} // namespace usb
} // namespace rev


#else
#endif