#include "./HeartbeatAnalyzer.h"

HBA::HeartbeatAnalyzer::HeartbeatAnalyzer(int buffer_len)
{
  printf("HBA: buf_len: %d\n", buffer_len);
  this->buf_len = buffer_len;
  this->half_buf_len = buf_len / 2;
  this->ppg_array = new HBA::DataPoint[buffer_len]();
  this->ppg_dt    = new HBA::DataPoint[buffer_len]();
  this->ecg_array = new HBA::DataPoint[buffer_len]();
  

  this->hb_ppg = new HBA::HbPPG[buffer_len]();
  this->hb_ecg = new HBA::HbECG[buffer_len]();
  this->hb_multimodal = new HBA::HbCombined[buffer_len]();
}

HBA::HeartbeatAnalyzer::~HeartbeatAnalyzer()
{
  delete this->ppg_array;
  delete this->ppg_dt;
  delete this->ecg_array;

  delete this->hb_ppg;
  delete this->hb_ecg;
  delete this->hb_multimodal;
}

bool HBA::HeartbeatAnalyzer::push_next_ppg(int32_t ppg, u_int32_t ts)
{
  /*
    Notes: I don't believe the callstack for this uses any floating point operations,
    however, I still would consider calling this in an ISR a poor life decision for
    lists of reasons. You are still welcome to try, just note some embedded platforms
    might still get upset at you. ESP32s might give you the blink of shame.
  */
  //scoot down
  // memcpy(&this->ppg_array[1], &this->ppg_array[0], sizeof(DataPoint) * (this->buf_len -1));
  for (uint16_t i = 0; i < this->buf_len -1; i++) {
    this->ppg_array[(this->buf_len) - i - 1] = this->ppg_array[(this->buf_len- i) - 2];
  }
  //insert
  this->ppg_array[0].mag = ppg;
  this->ppg_array[0].ts = ts;

  // Generate Dt
  for (uint16_t i = 0; i < this->buf_len -1; i++) {
    this->ppg_dt[(this->buf_len) - i - 1] = this->ppg_dt[(this->buf_len- i) - 2];
  }
  // memcpy(&this->ppg_dt[1], &this->ppg_dt[0], sizeof(DataPoint) * (this->buf_len -1));
  this->ppg_dt[0].mag = this->ppg_array[0].mag - this->ppg_array[1].mag;
  this->ppg_dt[0].ts = this->ppg_array[0].ts;


  if (this->hb_ppg_manager()) {

    // this->hb_ppg[0].ts = this->ppg_array[this->half_buf_len].ts;

    return true;
  } else {
    return false;
  }


}

bool HBA::HeartbeatAnalyzer::id_ppg_peak(uint16_t window) {
  /*
    Determines whether the data point at the "half_buf_len" index is a peak
    window is the size of the scope to check for something that might be greater.
  */
  // find max in window
  int32_t max = INT_MIN;
  for (uint16_t i = this->half_buf_len - window; i < (this->half_buf_len + window); i ++ ) {
    if (this->ppg_array[i].mag > max) max = this->ppg_array[i].mag;
  }
  // Beware any who come seeking to optimize, this is not optional.
  // Short explanation is that this prevents double tapping.
  // If you would like to implement better logic, be my guest,
  // but you must have something to fix this.
  // Could be optimized by doing one search on the first half and another on the second
  // but 'this is left as an exercise for the reader'
  // ~ Elijah, Feb 11, 2025
  int32_t max_recheck = INT_MIN;
  for (uint16_t i = this->half_buf_len + 1; i < (this->half_buf_len + window); i ++ ) {
    if (this->ppg_array[i].mag > max_recheck) max_recheck = this->ppg_array[i].mag;
  }
  
  if (max == this->ppg_array[this->half_buf_len].mag && max != 0 && max!= max_recheck) {
    if (this->ppg_array[this->half_buf_len].mag == this->ppg_array[this->half_buf_len+1].mag) printf("FUNKY\n");
    return true;
  } else {
    return false;
  }

}

bool HBA::HeartbeatAnalyzer::id_ppg_valley(uint16_t window) {
  /*
    Determines whether the data point at the "half_buf_len" index is a valley
    window is the size of the scope to check for something that might be lesser.
  */
  // find max in window
  int32_t min = INT_MAX;
  for (uint16_t i = this->half_buf_len - window; i < (this->half_buf_len + window); i ++ ) {
    if (i == window && this->ppg_array[i].mag == min) return false;
    if (this->ppg_array[i].mag < min) min = this->ppg_array[i].mag;
  }
  int32_t min_recheck = INT_MAX;
  for (uint16_t i = this->half_buf_len + 1; i < (this->half_buf_len + window); i ++ ) {
    if (this->ppg_array[i].mag < min_recheck) min_recheck = this->ppg_array[i].mag;
  }
  if (min == this->ppg_array[this->half_buf_len].mag && min != 0 && min != min_recheck) {
    return true;
  } else {
    return false;
  }

}

bool HBA::HeartbeatAnalyzer::id_ppg_dt_valley(uint16_t window) {
  /*
    Determines whether the data point at the "half_buf_len" index is a valley
    window is the size of the scope to check for something that might be lesser.
  */
  // find max in window
  int32_t min = INT_MAX;
  for (uint16_t i = this->half_buf_len - window; i < (this->half_buf_len + window); i ++ ) {
    // if (i == window && this->ppg_dt[i].mag == min) return false;
    if (this->ppg_dt[i].mag < min) min = this->ppg_dt[i].mag;
  }
  int32_t min_recheck = INT_MAX;
  for (uint16_t i = this->half_buf_len + 1; i < (this->half_buf_len + window); i ++ ) {
    if (this->ppg_dt[i].mag < min_recheck) min_recheck = this->ppg_dt[i].mag;
  }
  if (min == this->ppg_dt[this->half_buf_len].mag && min != 0 && min != min_recheck) {
    return true;
  } else {
    return false;
  }

}

bool HBA::HeartbeatAnalyzer::hb_ppg_manager() {
  /*
   * Function manages actual heartbeats, populates the hb_ppg vector
  */

  // ppg_peak val is generally good at 10 from my testing
  // ppg_valley val is generally good at 40 from my testing
  // One should note my test data is sampled at 200 hz,
  // and if you are having to edit this, I pity you.
  // ~ Elijah, Feb 11, 2025

  // Theory: Each heartbeat can only contain two peaks max, and two valleys max
  //         However, the valley that we care about is the main one.
  // Conclusion: valleys work fine for the marking of the next heartbeat
  // Application:
  static DataPoint detected_valleys[2]; // This should be initialized to all 0s to start.
  static DataPoint detected_peaks[3]; // Also should be initialized to all 0s.
  static DataPoint prv_dt_valley; 
  static DataPoint most_recent_dt_valley;
  static int32_t prv_baseline, most_recent_baseline;
  if (this->id_ppg_valley(40)) {
    
    // memcpy(&detected_valleys[1], &detected_valleys[0], sizeof(DataPoint));
    for (uint16_t i = 0; i < 2 -1; i++) {
      detected_valleys[(2) - i - 1] = detected_valleys[(2- i) - 2];
    }
    detected_valleys[0] = this->ppg_array[this->half_buf_len];
    

    

    DataPoint pulseArrival;
    DataPoint dicroticNotch;
    if (detected_peaks[1].ts > detected_valleys[1].ts) {
      // Did the second peak in the queue come after the first valley?
      // if it came before, then it would be the pulse arrival of the beat
      // that has fully passed.
      pulseArrival = detected_peaks[2];
      dicroticNotch = detected_peaks[1];
    } else {
      // wasn't able to pick up dicrotic 
      // notch
      pulseArrival = detected_peaks[1];
      dicroticNotch.mag = 0;
      dicroticNotch.ts = 0;
    }
    // Serial.print(">o:");
    // Serial.println(pulseArrival.mag);

    // Add logic for shipping full ppg stuffs
    // memcpy(&this->hb_ppg[1], &this->hb_ppg[0], sizeof(HBA::HbPPG) * this->buf_len);
    for (uint16_t i = 0; i < this->buf_len -1; i++) {
      this->hb_ppg[(this->buf_len) - i - 1] = this->hb_ppg[(this->buf_len- i) - 2];
    }
    // Serial.print(">o:");
    // Serial.println(most_recent_dt_valley.mag);
    this->hb_ppg[0].ts = pulseArrival.ts; // ADD REAL LOGIC!
    this->hb_ppg[0].te = detected_valleys[1].ts; // timestamp of the end of the pulse
    this->hb_ppg[0].ph = pulseArrival.mag - detected_valleys[1].mag;  
    this->hb_ppg[0].pw = detected_valleys[1].mag - pulseArrival.ts;
    this->hb_ppg[0].td = dicroticNotch.ts;
    this->hb_ppg[0].tm = prv_dt_valley.ts;  
    this->hb_ppg[0].pbl = prv_baseline;
    this->hb_ppg[0].pdmax = prv_dt_valley.mag;
    this->new_ppg = true;
    prv_dt_valley = most_recent_dt_valley;
    prv_baseline = most_recent_baseline;
    return true;
  }
  
  if (this->id_ppg_peak(10)) {
    // memcpy(&detected_peaks[1], &detected_peaks[0], sizeof(DataPoint) * 2);
    for (uint16_t i = 0; i < 3 -1; i++) {
      detected_peaks[(3) - i-1] = detected_peaks[(3- i) - 2];
    }
    detected_peaks[0] = this->ppg_array[this->half_buf_len];
  }

  if (this->id_ppg_dt_valley(20)) {
    // memcpy(&detected_dt_valleys[1], &detected_dt_valleys[0], sizeof(DataPoint) * 2);
    most_recent_dt_valley = this->ppg_dt[this->half_buf_len];
    most_recent_baseline = this->ppg_array[this->half_buf_len].mag;
  }


  return false;
}

bool HBA::HeartbeatAnalyzer::push_next_ecg(int32_t ecg, u_int32_t ts)
{ 
  // memcpy(&this->ecg_array[1], &this->ecg_array[0], sizeof(DataPoint) * (this->buf_len -1));
  for (uint16_t i = 0; i < this->buf_len - 1; i++) {
    this->ecg_array[(this->buf_len) - i - 1] = this->ecg_array[(this->buf_len- i) - 2];
  }
  //insert
  this->ecg_array[0].mag = ecg;
  this->ecg_array[0].ts = ts;

  if (this->hb_ecg_manager()) {
    return true;
  } else {
    return false;
  }
}
bool HBA::HeartbeatAnalyzer::id_ecg_peak(uint16_t window) {
  int32_t max = INT_MIN;
  for (uint16_t i = this->half_buf_len - window; i < (this->half_buf_len + window); i ++ ) {
    if (this->ecg_array[i].mag > max) max = this->ecg_array[i].mag;
  }
  int32_t max_recheck = INT_MIN;
  for (uint16_t i = this->half_buf_len + 1; i < (this->half_buf_len + window); i ++ ) {
    if (this->ecg_array[i].mag > max_recheck) max_recheck = this->ecg_array[i].mag;
  }
  
  if (max == this->ecg_array[this->half_buf_len].mag && max != 0 && max!= max_recheck) {
    if (this->ecg_array[this->half_buf_len].mag == this->ecg_array[this->half_buf_len+1].mag) printf("FUNKY\n");
    return true;
  } else {
    return false;
  }
}
bool HBA::HeartbeatAnalyzer::hb_ecg_manager() {
  if (this->id_ecg_peak(49)) {
    // memcpy(&this->hb_ecg[1], &this->hb_ecg[0], sizeof(HBA::HbECG) * this->buf_len);
    for (uint16_t i = 0; i < this->buf_len - 1; i++) {
      this->hb_ecg[(this->buf_len) - i - 1] = this->hb_ecg[(this->buf_len- i) - 2];
    }
    
    this->hb_ecg[0].trr = this->ecg_array[this->half_buf_len].ts;
    this->hb_ecg[0].ebl = this->id_ecg_baseline();
    this->hb_ecg[0].eph = this->ecg_array[this->half_buf_len].mag - this->hb_ecg[0].ebl;



    return true;
  } else {
    return false;
  }
}
int32_t HBA::HeartbeatAnalyzer::id_ecg_baseline() {
  int32_t total = 0;
  for (int32_t i = this->half_buf_len +10 ; i < this->half_buf_len + 21; i++) {
    total += this->ecg_array[i].mag / 10;
  }
  return total;
}


bool HBA::HeartbeatAnalyzer::check_multimodal() {
  if (this->ecg_enabled && this->new_ppg) {
    this->new_ppg = false;
    
    // memcpy(&this->hb_multimodal[1], &this->hb_multimodal[0], sizeof(HBA::HbCombined) * (this->buf_len -1));
    for (uint16_t i = 0; i < this->buf_len - 1; i++) {
      this->hb_multimodal[(this->buf_len) - i - 1] = this->hb_multimodal[(this->buf_len- i) - 2];
    }
    this->hb_multimodal[0].ecg = &this->hb_ecg[1];
    this->hb_multimodal[0].ppg = &this->hb_ppg[0];
    this->hb_multimodal[0].ptt = hb_multimodal[0].ppg->tm - hb_multimodal[0].ecg->trr;

    return true;

  } else return false;
}