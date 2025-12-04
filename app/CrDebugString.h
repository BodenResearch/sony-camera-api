/* get enum string from enum value for debug */

#ifndef CRERRORSTRING_H
#define CRERRORSTRING_H

#include <CrTypes.h>
#include <CrError.h>
#include <CrDeviceProperty.h>
#include <CrCommandData.h>

std::string CrErrorString(SCRSDK::CrError error);
std::string CrWarningExtString(SCRSDK::CrError error, CrInt32 param1, CrInt32 param2, CrInt32 param3);

std::string CrCommandIdString(SCRSDK::CrCommandId id);
std::string CrDevicePropertyString(SCRSDK::CrDevicePropertyCode code);

SCRSDK::CrCommandId CrCommandIdCode(std::string name);
SCRSDK::CrDevicePropertyCode CrDevicePropertyCode(std::string name);

#endif // CRERRORSTRING_H
