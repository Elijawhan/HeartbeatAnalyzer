#include "../src/HeartbeatAnalyzer.h"
#include <cstdio>

#include "sim_ppg.h"
#include "sim_ecg.h"
#include "sim_ts.h"

int main (int argc, char *argv[]) {
  
  HBA::HeartbeatAnalyzer myHb(100);
  myHb.ecg_enabled = true;
  
  FILE* PPG_DAT = fopen("ppg_a.csv", "w");
  FILE* PPG_DT = fopen("ppg_dt.csv", "w");
  FILE* PPG_FILT = fopen("ppg_filt.csv", "w");
  FILE* ECG_DAT = fopen("ecg_a.csv", "w");
  FILE* MM_DAT = fopen("mm_a.csv", "w");

  

  for (int i = 0; i < SIM_L - 7900; i ++) {
    int32_t ppgsig = ppg[i];
    int32_t ecgsig = ecg[i];
    u_int32_t tssig = (u_int32_t) (1000 *(ts[i]));
    static int32_t ppg_filt = 0;
    static int32_t ecg_filt = 0;
    ppg_filt = (int32_t) ((ppgsig * 20 + ppg_filt * 80)/ 100);
    ecg_filt = (int32_t) ((ecgsig * 50 + ecg_filt * 50)/ 100);
    
    
    if (myHb.push_next_ppg(ppg_filt, tssig)) {
      fprintf(PPG_DAT, "%d, %d, %d, %d, %d, %d, %d, %d,\n", 
        myHb.hb_ppg[0].ts, myHb.hb_ppg[0].tm, myHb.hb_ppg[0].te, myHb.hb_ppg[0].td,
        myHb.hb_ppg[0].pw, myHb.hb_ppg[0].ph, myHb.hb_ppg[0].pbl, myHb.hb_ppg[0].pdmax);
    }

    if (myHb.push_next_ecg(ecg_filt, tssig)) {
      fprintf(ECG_DAT, "%d, %d, %d, \n", myHb.hb_ecg[0].trr, myHb.hb_ecg[0].ebl, myHb.hb_ecg[0].eph);
    }

    if (myHb.check_multimodal()) {
      fprintf(MM_DAT, "%d, %d\n", myHb.hb_multimodal[0].ecg->trr, myHb.hb_multimodal[0].ppg->tm);
    }

    fprintf(PPG_DT, "%d, %d\n", myHb.ppg_dt[0].ts, myHb.ppg_dt[0].mag);
    fprintf(PPG_FILT, "%d, %d, %d \n", tssig, ppg_filt, ecg_filt);
    
  }

  fclose(PPG_DAT);
  fclose(PPG_DT);
  fclose(ECG_DAT);
  fclose(MM_DAT);
  return 0;

}
