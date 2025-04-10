/**
 * @file HeartbeatAnalyzer.h
 * @author Elijah Moore ecm0022@uah.edu
 * @date 2025-02-11
 * @brief Class for analyzing a heartbeat in real time
 * 
 * <to be read in my classic southern American accent>
 * The lord knows I have spent a lot of time reading other people's terrible 
 * code, and I always think, dang, I could have done that better. 
 * Well, now's my chance. 
 * 
 * Let me be clear, when I say better, I mean easier to use,
 * not the most optimized. 
 * My philosophy is to prioritize optimizing for future development.
 */

typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;
typedef int int32_t;

#include <string>
#include <cstdio>
#include <algorithm>

namespace HBA
{
  struct DataPoint
  {
    int32_t mag;
    u_int32_t ts;
  };
  struct HbPPG
  {
    // Pulse arrival time Measured as time from midnight in milliseconds
    u_int32_t ts;  
    // timestamp of middle of pulse relative to ts
    u_int32_t tm;  
    // timestamp of end of pulse relative to ts
    u_int32_t te;  
    // timestamp of dicrotic notch relative to ts
    u_int32_t td;  
    // pulse height
    u_int16_t ph;  
    // pulse width
    u_int16_t pw;  
    // maximum absolute value of first derivative
    u_int16_t pdmax; 
    // pulse ppg baseline
    u_int16_t pbl; 

  };
  struct HbECG
  {

    u_int32_t trr; // RPeak time Measured as time from midnight in milliseconds
    u_int16_t eph; // ECG Pulse Height
    int32_t ebl; // ECG Baseline

  };
  struct HbCombined
  {

    u_int16_t ptt; // Pulse arrival time

    HbECG *ecg; // corresponding ECG
    HbPPG *ppg; // corresponding PPG

  };

  class HeartbeatAnalyzer
  {

  public:
    HeartbeatAnalyzer(int buffer_len);
    HeartbeatAnalyzer(HeartbeatAnalyzer &&) = default;
    HeartbeatAnalyzer(const HeartbeatAnalyzer &) = default;
    HeartbeatAnalyzer &operator=(HeartbeatAnalyzer &&) = default;
    HeartbeatAnalyzer &operator=(const HeartbeatAnalyzer &) = default;
    ~HeartbeatAnalyzer();

    bool push_next_ppg(int32_t ppg, u_int32_t ts);
    bool push_next_ecg(int32_t ecg, u_int32_t ts);
    bool check_multimodal();

    DataPoint* ppg_array;
    DataPoint* ppg_dt;
    DataPoint* ecg_array;

    HbECG *hb_ecg;
    HbPPG *hb_ppg;
    HbCombined *hb_multimodal;

    bool ecg_enabled = false;

  private:
    int buf_len;
    int half_buf_len;
    int peak_seek;
    bool new_ppg = false;

    
    bool id_ecg_peak(uint16_t window);
    int32_t id_ecg_baseline();
    bool hb_ecg_manager();
    

    bool id_ppg_peak(uint16_t window);
    bool id_ppg_valley(uint16_t window);
    bool id_ppg_dt_valley(uint16_t window);
    bool hb_ppg_manager();

  };

}
