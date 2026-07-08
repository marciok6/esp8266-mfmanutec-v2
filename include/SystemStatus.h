#pragma once

#include "AppTypes.h"

class SystemStatus {
 public:
  void setState(DeviceState newState);
  DeviceState getState() const;

  void setCode(int newCode);
  int getCode() const;

 private:
  DeviceState state_ = DeviceState::Unconfigured;
  int code_ = CODE_CONFIG_NOT_FOUND;
};
