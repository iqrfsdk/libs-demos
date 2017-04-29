/**
 * Copyright 2016-2017 MICRORISC s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LaunchUtils.h"
#include "JsonSerializer.h"
#include "DpaTransactionTask.h"
#include "IqrfLogging.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "JsonUtils.h"
#include <vector>
#include <utility>
#include <stdexcept>

 //TODO using istream is slower according http://rapidjson.org/md_doc_stream.html

using namespace rapidjson;

INIT_COMPONENT(ISerializer, JsonSerializer)

#define CTYPE_STR "ctype"
#define TYPE_STR "type"
#define NADR_STR "nadr"
#define HWPID_STR "hwpid"
#define TIMEOUT_STR "timeout"
#define MSGID_STR "msgid"
#define REQUEST_STR "request"
#define REQUEST_TS_STR "request-ts"
#define RESPONSE_STR "response"
#define RESPONSE_TS_STR "response-ts"
#define CONFIRMATION_STR "confirmation"
#define CONFIRMATION_TS_STR "confirmation-ts"
#define CMD_STR "cmd"
#define STATUS_STR "status"

//////////////////////////////////////////
PrfCommonJson::PrfCommonJson()
{
  m_doc.SetObject();

}

void PrfCommonJson::encodeBinary(std::string& to, unsigned char* from, int len)
{
  if (len > 0) {
    std::ostringstream ostr;
    ostr << iqrf::TracerHexString(from, len, true);

    if (std::string::npos != to.find_first_of('.')) {
      to = ostr.str();
      std::replace(to.begin(), to.end(), ' ', '.');
      if (to[to.size() - 1] == '.')
        to.pop_back();
    }
    else {
      to = ostr.str();
      if (to[to.size() - 1] == ' ')
        to.pop_back();
    }
  }

}

void PrfCommonJson::encodeTimestamp(std::string& to, std::chrono::time_point<std::chrono::system_clock> from)
{
  using namespace std::chrono;

  auto fromUs = std::chrono::duration_cast<std::chrono::microseconds>(from.time_since_epoch()).count() % 1000000;
  auto time = std::chrono::system_clock::to_time_t(from);
  auto tm = *std::gmtime(&time);
  //auto tm = *std::localtime(&time);
  
  std::ostringstream os;
  os.fill('0'); os.width(6);
  os << std::put_time(&tm, "%F %T.") <<  fromUs; // << std::put_time(&tm, " %Z\n");

  to = os.str();
}


void PrfCommonJson::parseRequestJson(rapidjson::Value& val)
{
  jutils::assertIsObject("", val);

  m_has_ctype = jutils::getMemberIfExistsAs<std::string>(CTYPE_STR, val, m_ctype);
  m_has_type = jutils::getMemberIfExistsAs<std::string>(TYPE_STR, val, m_type);
  m_has_nadr = jutils::getMemberIfExistsAs<int>(NADR_STR, val, m_nadr);
  m_has_hwpid = jutils::getMemberIfExistsAs<std::string>(HWPID_STR, val, m_hwpid);
  m_has_timeout = jutils::getMemberIfExistsAs<int>(TIMEOUT_STR, val, m_timeoutJ);
  m_has_msgid = jutils::getMemberIfExistsAs<std::string>(MSGID_STR, val, m_msgid);
  m_has_request = jutils::getMemberIfExistsAs<std::string>(REQUEST_STR, val, m_requestJ);
  m_has_request_ts = jutils::getMemberIfExistsAs<std::string>(REQUEST_TS_STR, val, m_request_ts);
  m_has_response = jutils::getMemberIfExistsAs<std::string>(RESPONSE_STR, val, m_responseJ);
  m_has_response_ts = jutils::getMemberIfExistsAs<std::string>(RESPONSE_TS_STR, val, m_response_ts);
  m_has_confirmation = jutils::getMemberIfExistsAs<std::string>(CONFIRMATION_STR, val, m_confirmationJ);
  m_has_confirmation_ts = jutils::getMemberIfExistsAs<std::string>(CONFIRMATION_TS_STR, val, m_confirmation_ts);
  m_has_cmd = jutils::getMemberIfExistsAs<std::string>(CMD_STR, val, m_cmdJ);
}


std::string PrfCommonJson::encodeResponseJson(const DpaTask& dpaTask)
{
  Document::AllocatorType& alloc = m_doc.GetAllocator();
  rapidjson::Value v;
  if (m_has_ctype) {
    v.SetString(m_ctype.c_str(), alloc);
    m_doc.AddMember(CTYPE_STR, v, alloc);
  }
  if (m_has_type) {
    v.SetString(m_type.c_str(), alloc);
    m_doc.AddMember(TYPE_STR, v, alloc);
  }
  if (m_has_nadr) {
    v = m_nadr;
    m_doc.AddMember(NADR_STR, v, alloc);
  }
  if (m_has_hwpid) {
    v.SetString(m_hwpid.c_str(), alloc);
    m_doc.AddMember(HWPID_STR, v, alloc);
  }
  if (m_has_timeout) {
    v = m_timeoutJ;
    m_doc.AddMember(TIMEOUT_STR, v, alloc);
  }
  if (m_has_msgid) {
    v.SetString(m_msgid.c_str(), alloc);
    m_doc.AddMember(MSGID_STR, v, alloc);
  }
  if (m_has_request) {
    encodeBinary(m_requestJ, (unsigned char*)dpaTask.getRequest().DpaPacket().Buffer, dpaTask.getRequest().Length());
    v.SetString(m_requestJ.c_str(), alloc);
    m_doc.AddMember(REQUEST_STR, v, alloc);
  }
  if (m_has_request_ts) {
    encodeTimestamp(m_request_ts, dpaTask.getRequestTs());
    v.SetString(m_request_ts.c_str(), alloc);
    m_doc.AddMember(REQUEST_TS_STR, v, alloc);
  }
  if (m_has_confirmation) {
    encodeBinary(m_confirmationJ, (unsigned char*)dpaTask.getConfirmation().DpaPacket().Buffer, dpaTask.getConfirmation().Length());
    v.SetString(m_confirmationJ.c_str(), alloc);
    m_doc.AddMember(CONFIRMATION_STR, v, alloc);
  }
  if (m_has_confirmation_ts) {
    encodeTimestamp(m_confirmation_ts, dpaTask.getConfirmationTs());
    v.SetString(m_confirmation_ts.c_str(), alloc);
    m_doc.AddMember(CONFIRMATION_TS_STR, v, alloc);
  }
  if (m_has_response) {
    encodeBinary(m_responseJ, (unsigned char*)dpaTask.getResponse().DpaPacket().Buffer, dpaTask.getResponse().Length());
    v.SetString(m_responseJ.c_str(), alloc);
    m_doc.AddMember(RESPONSE_STR, v, alloc);
  }
  if (m_has_response_ts) {
    encodeTimestamp(m_response_ts, dpaTask.getResponseTs());
    v.SetString(m_response_ts.c_str(), alloc);
    m_doc.AddMember(RESPONSE_TS_STR, v, alloc);
  }
  if (m_has_cmd) {
    v.SetString(m_cmdJ.c_str(), alloc);
    m_doc.AddMember(CMD_STR, v, alloc);
  }
  v.SetString(m_statusJ.c_str(), alloc);
  m_doc.AddMember(STATUS_STR, v, alloc);

  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  m_doc.Accept(writer);
  return buffer.GetString();
}

/////////////////////////////////////////
//-------------------------------
PrfRawJson::PrfRawJson(rapidjson::Value& val)
{
  parseRequestJson(val);
  parseCommand(m_cmdJ);
  if (m_timeoutJ >= 0) {
    setTimeout(m_timeoutJ);
  }

  if (!m_has_request) {
    THROW_EX(std::logic_error, "Missing member: " << REQUEST_STR);
  }

  std::string buf = m_requestJ;
  if (std::string::npos != buf.find_first_of('.')) {
    buf.replace(buf.begin(), buf.end(), '.', ' ');
    m_dotNotation = true;
  }
  std::istringstream istr(buf);

  uint8_t bbyte;
  int ival;
  int i = 0;

  while (i < MAX_DPA_BUFFER) {
    if (istr >> std::hex >> ival) {
      m_request.DpaPacket().Buffer[i] = (uint8_t)ival;
      i++;
    }
    else
      break;
  }
  m_request.SetLength(i);

}

std::string PrfRawJson::encodeResponse(const std::string& errStr)
{
  {
    std::ostringstream ostr;
    int len = m_request.Length();
    ostr << iqrf::TracerHexString((unsigned char*)m_request.DpaPacket().Buffer, m_request.Length(), true);

    std::string buf(ostr.str());

    if (m_dotNotation) {
      buf.replace(buf.begin(), buf.end(), ' ', '.');
      if (buf[buf.size() - 1] == '.')
        buf.pop_back();
    }

    m_requestJ = buf;
  }

  {
    std::ostringstream ostr;
    int len = m_response.Length();
    ostr << iqrf::TracerHexString((unsigned char*)m_response.DpaPacket().Buffer, m_response.Length(), true);

    std::string buf(ostr.str());

    if (m_dotNotation) {
      buf.replace(buf.begin(), buf.end(), ' ', '.');
      if (buf[buf.size() - 1] == '.')
        buf.pop_back();
    }

    m_responseJ = buf;
    m_has_response = true;
  }

  if (m_has_confirmation)
  {
    std::ostringstream ostr;
    int len = m_confirmation.Length();
    ostr << iqrf::TracerHexString((unsigned char*)m_confirmation.DpaPacket().Buffer, m_confirmation.Length(), true);

    std::string buf(ostr.str());

    if (m_dotNotation) {
      buf.replace(buf.begin(), buf.end(), ' ', '.');
      if (buf[buf.size() - 1] == '.')
        buf.pop_back();
    }

    m_confirmationJ = buf;
  }

  m_statusJ = errStr;
  return encodeResponseJson(*this);
}

//-------------------------------
PrfThermometerJson::PrfThermometerJson(rapidjson::Value& val)
{
  parseRequestJson(val);
  setAddress((uint16_t)m_nadr);
  parseCommand(m_cmdJ);
  if (m_timeoutJ >= 0) {
    setTimeout(m_timeoutJ);
  }
}

std::string PrfThermometerJson::encodeResponse(const std::string& errStr)
{
  rapidjson::Value v;
  v = getFloatTemperature();
  m_doc.AddMember("temperature", v, m_doc.GetAllocator());

  m_statusJ = errStr;
  return encodeResponseJson(*this);
}

//-------------------------------
PrfFrcJson::PrfFrcJson(rapidjson::Value& val)
{
  parseRequestJson(val);
  parseRequestJson(val);
  setAddress((uint16_t)m_nadr);
  parseCommand(m_cmdJ);
  if (m_timeoutJ >= 0) {
    setTimeout(m_timeoutJ);
  }

  std::string frcCmd = jutils::getPossibleMemberAs<std::string>("FrcCmd", val, "");
  if (!frcCmd.empty()) {
    setFrcCommand(parseFrcCmd(frcCmd));
    m_predefinedFrcCommand = true;
  }
  else {
    std::string frcType = jutils::getMemberAs<std::string>("FrcType", val);
    uint8_t frcUser = (uint8_t)jutils::getMemberAs<int>("FrcUser", val);
    setFrcCommand(parseFrcType(frcType), frcUser);
  }
}

std::string PrfFrcJson::encodeResponse(const std::string& errStr)
{
  Document::AllocatorType& alloc = m_doc.GetAllocator();
  rapidjson::Value v;

  if (m_predefinedFrcCommand) {
    v.SetString(PrfFrc::encodeFrcCmd((FrcCmd)getFrcCommand()).c_str(), alloc);
    m_doc.AddMember("FrcCmd", v, alloc);
  }
  else {
    v.SetString(PrfFrc::encodeFrcType((FrcType)getFrcType()).c_str(), alloc);
    m_doc.AddMember("FrcType", v, alloc);

    v = (int)getFrcUser();
    m_doc.AddMember("FrcUser", v, alloc);
  }

  std::ostringstream os;
  os.setf(std::ios::hex, std::ios::basefield);
  os.fill('0');

  switch (getFrcType()) {
  case FrcType::GET_BIT2:
  {
    for (int i = 1; i <= PrfFrc::FRC_MAX_NODE_BIT2; i++) {
      os << std::setw(2) << (int)getFrcData_bit2(i) << ' ';
    }
  }
  break;

  case FrcType::GET_BYTE:
  {
    for (int i = 1; i <= PrfFrc::FRC_MAX_NODE_BYTE; i++) {
      os << std::setw(2) << (int)getFrcData_Byte(i) << ' ';
    }
  }
  break;

  case FrcType::GET_BYTE2:
  {
    for (int i = 1; i <= PrfFrc::FRC_MAX_NODE_BYTE2; i++) {
      os << std::setw(4) << (int)getFrcData_Byte2(i) << ' ';
    }
  }
  break;
  default:
    ;
  }

  std::string values(os.str());
  values.pop_back();

  v.SetString(values.c_str(), alloc);
  m_doc.AddMember("Values", v, alloc);

  m_statusJ = errStr;
  return encodeResponseJson(*this);
}

//-------------------------------
PrfIoJson::PrfIoJson(rapidjson::Value& val)
{
  parseRequestJson(val);
  parseRequestJson(val);
  setAddress((uint16_t)m_nadr);
  parseCommand(m_cmdJ);
  if (m_timeoutJ >= 0) {
    setTimeout(m_timeoutJ);
  }

  switch (getCmd()) {

  case PrfIo::Cmd::DIRECTION:
  {
    Port port = parsePort(jutils::getMemberAs<std::string>("Port", val));
    int bit = jutils::getMemberAs<int>("Bit", val);
    bool inp = jutils::getMemberAs<bool>("Inp", val);
    directionCommand(port, bit, inp);
    m_port = port;
    m_bit = bit;
    m_val = inp;
  }
  break;

  case PrfIo::Cmd::SET:
  {
    Port port = parsePort(jutils::getMemberAs<std::string>("Port", val));
    int bit = jutils::getMemberAs<int>("Bit", val);
    bool value = jutils::getMemberAs<bool>("Val", val);
    setCommand(port, bit, value);
    m_port = port;
    m_bit = bit;
    m_val = value;
  }
  break;

  case PrfIo::Cmd::GET:
  {
    Port port = parsePort(jutils::getMemberAs<std::string>("Port", val));
    int bit = jutils::getMemberAs<int>("Bit", val);
    m_port = port;
    m_bit = bit;
    getCommand();
  }
  break;

  default:
    ;
  }
}

std::string PrfIoJson::encodeResponse(const std::string& errStr)
{

  Document::AllocatorType& alloc = m_doc.GetAllocator();
  rapidjson::Value v;

  switch (getCmd()) {

  case PrfIo::Cmd::DIRECTION:
  {
    v.SetString(encodePort(m_port).c_str(), alloc);
    m_doc.AddMember("Port", v, alloc);

    v = m_bit;
    m_doc.AddMember("Bit", v, alloc);

    v = m_val;
    m_doc.AddMember("Inp", v, alloc);
  }
  break;

  case PrfIo::Cmd::SET:
  {
    v.SetString(encodePort(m_port).c_str(), alloc);
    m_doc.AddMember("Port", v, alloc);

    v = m_bit;
    m_doc.AddMember("Bit", v, alloc);

    v = m_val;
    m_doc.AddMember("Val", v, alloc);
  }
  break;

  case PrfIo::Cmd::GET:
  {
    v.SetString(encodePort(m_port).c_str(), alloc);
    m_doc.AddMember("Port", v, alloc);

    v = m_bit;
    m_doc.AddMember("Bit", v, alloc);

    v = getInput(m_port, m_bit);
    m_doc.AddMember("Val", v, alloc);
  }
  break;

  default:
    ;
  }

  m_statusJ = errStr;
  return encodeResponseJson(*this);
}

//////////////////
//-------------------------------
PrfOsJson::PrfOsJson(rapidjson::Value& val)
{
  parseRequestJson(val);
  parseRequestJson(val);
  setAddress((uint16_t)m_nadr);
  parseCommand(m_cmdJ);
  if (m_timeoutJ >= 0) {
    setTimeout(m_timeoutJ);
  }

  switch (getCmd()) {

  case Cmd::READ:
  {
  }
  break;

  default:
    ;
  }
}

std::string PrfOsJson::encodeResponse(const std::string& errStr)
{
  Document::AllocatorType& alloc = m_doc.GetAllocator();
  rapidjson::Value v;

  switch (getCmd()) {

  case Cmd::READ:
  {
  }
  break;

  default:
    ;
  }

  m_statusJ = errStr;
  return encodeResponseJson(*this);
}

///////////////////////////////////////////
JsonSerializer::JsonSerializer()
  :m_name("Json")
{
  init();
}

JsonSerializer::JsonSerializer(const std::string& name)
  : m_name(name)
{
  init();
}

void JsonSerializer::init()
{
  registerClass<PrfRawJson>(PrfRaw::PRF_NAME);
  registerClass<PrfThermometerJson>(PrfThermometer::PRF_NAME);
  registerClass<PrfLedGJson>(PrfLedG::PRF_NAME);
  registerClass<PrfLedRJson>(PrfLedR::PRF_NAME);
  registerClass<PrfFrcJson>(PrfFrc::PRF_NAME);
  registerClass<PrfIoJson>(PrfIo::PRF_NAME);
  registerClass<PrfOsJson>(PrfOs::PRF_NAME);
}

const std::string& JsonSerializer::parseCategory(const std::string& request)
{
  //TODO
  return CAT_DPA_STR;
}

std::unique_ptr<DpaTask> JsonSerializer::parseRequest(const std::string& request)
{
  std::unique_ptr<DpaTask> obj;
  try {
    Document doc;
    jutils::parseString(request, doc);

    jutils::assertIsObject("", doc);
    std::string perif = jutils::getMemberAs<std::string>("type", doc);

    obj = createObject(perif, doc);
  }
  catch (std::exception &e) {
    m_lastError = e.what();
  }
  return std::move(obj);
}

std::string JsonSerializer::parseConfig(const std::string& request)
{
  //TODO
  return "";
}

std::string JsonSerializer::getLastError() const
{
  return m_lastError;
}
