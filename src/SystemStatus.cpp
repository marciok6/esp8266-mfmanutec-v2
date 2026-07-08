#include "../include/SystemStatus.h"

void SystemStatus::setState(DeviceState newState) { state_ = newState; }

DeviceState SystemStatus::getState() const { return state_; }

void SystemStatus::setCode(int newCode) { code_ = newCode; }

int SystemStatus::getCode() const { return code_; }
