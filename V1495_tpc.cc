#include "V1495_tpc.hh"
#include "MongoLog.hh"

V1495_TPC::V1495_TPC(std::shared_ptr<MongoLog>& log, std::shared_ptr<Options>& opts, int bid, int handle, unsigned int address) :
  V1495(log, opts, bid, handle, address), fControlReg(0x101E),
  fVetoOffMSBReg(0x1012), fVetoOffLSBReg(0x1010),
  fVetoOnMSBReg(0x100E), fVetoOnLSBReg(0x100C) {
  fFractionalModeActive = 0;
  fVetoOn_clk = fVetoOff_clk = 0;
}

V1495_TPC::~V1495_TPC() {}

int V1495_TPC::Arm(std::map<std::string, int>& opts) {
  int clocks_per_us = 40;
  if ((fFractionalModeActive = opts["fractional_mode_active"]) == 1) {
    fVetoOn_clk = opts["veto_on_us"] * clocks_per_us;
    fVetoOff_clk = opts["veto_off_us"] * clocks_per_us;
    if (fVetoOn_clk * fVetoOff_clk == 0) {
      fLog->Entry(MongoLog::Message, "V1495: at least one value is zero, check the config: %i/%i",
          opts["veto_on_us"], opts["veto_off_us"]);
      fFractionalModeActive = 0;
    } else {
      fLog->Entry(MongoLog::Local, "V1495 fractional mode active: %i/%i",
          opts["veto_on_us"], opts["veto_off_us"]);
    }
  } else {
    fLog->Entry(MongoLog::Local, "V1495 fractional mode inactive");
  }
  return 0;
}

int V1495_TPC::BeforeSINStart() {
  int ret = 0;
  if (fFractionalModeActive) {
    ret += WriteReg(fControlReg, 0x1);
    ret += WriteReg(fVetoOffMSBReg, (fVetoOff_clk & 0xFFFF0000) >> 16);
    ret += WriteReg(fVetoOffLSBReg, fVetoOff_clk & 0xFFFF);
    ret += WriteReg(fVetoOnMSBReg, (fVetoOn_clk & 0xFFFF0000) >> 16);
    ret += WriteReg(fVetoOnLSBReg, fVetoOn_clk & 0xFFFF);
  } else {
    ret = WriteReg(fControlReg, 0x0);
  }
  return ret;
}

int V1495_TPC::BeforeSINStop() {
  return WriteReg(fControlReg, 0x0);
}
