// dram_timing.h
#pragma once
#include <cstdint>
#include <string>

namespace dram {

struct DRAMTiming {
    // Core Timings
    uint16_t tCAS = 0;   // Column Address Strobe Latency
    uint16_t tRCD = 0;   // RAS to CAS Delay
    uint16_t tRP  = 0;   // Row Precharge
    uint16_t tRAS = 0;   // Row Active Time
    uint16_t tRC  = 0;   // Row Cycle Time (tRAS + tRP)

    // Bank-to-Bank and Window Timings
    uint16_t tRRD_S = 0; // Row to Row Delay (Different bank group) - SHORT
    uint16_t tRRD_L = 0; // Row to Row Delay (Same bank group) - LONG
    uint16_t tFAW   = 0; // Four Activation Window
    uint16_t tCCD_S = 0; // CAS to CAS Delay (Different bank group) - SHORT
    uint16_t tCCD_L = 0; // CAS to CAS Delay (Same bank group) - LONG

    // Write-Specific Timings
    uint16_t tBL = 4;    // Burst Length (in cycles, default BL8=4 cycles)
    uint16_t tWR  = 0;   // Write Recovery Time
    uint16_t tWTR_S = 0; // Write to Read Delay (Different bank group)
    uint16_t tWTR_L = 0; // Write to Read Delay (Same bank group)
    uint16_t tCKE  = 0;  // CKE minimum pulse width

    // Refresh Timings
    uint32_t tREFI = 0;  // Refresh Interval
    uint16_t tRFC  = 0;  // Refresh Cycle Time

    // Meta Data
    std::string name;
    uint32_t frequency_mhz = 0;
    double current_temp = 25.0; // Celsius
    double process_variation = 1.0; // 1.0 = nominal, 1.05 = 5% slow corner

    static DRAMTiming DDR4_2400_R() {
        DRAMTiming t;
        t.name = "DDR4-2400R";
        t.frequency_mhz = 1200;
        // Cycle counts for 1200MHz clock
        t.tCAS = 17; t.tRCD = 17; t.tRP = 17; t.tRAS = 39; t.tRC = 56;
        t.tRRD_S = 4; t.tRRD_L = 6; t.tFAW = 26;
        t.tCCD_S = 4; t.tCCD_L = 6;
        t.tWR = 18; t.tWTR_S = 3; t.tWTR_L = 9;
        t.tREFI = 9360; // 7.8us @ 1.2GHz
        t.tRFC = 420;
        return t;
    }

    // NEW: Apply process variation/temp scaling
    void apply_derating() {
        double factor = process_variation;
        // JEDEC: >85C requires faster refresh (3.9us instead of 7.8us)
        if (current_temp > 85.0) {
            tREFI /= 2;
        }
        // Process variation slows down the "analog" timings
        tRCD = (uint16_t)(tRCD * factor);
        tRP  = (uint16_t)(tRP * factor);
        tRAS = (uint16_t)(tRAS * factor);
        tRC  = (uint16_t)(tRC * factor);
        tRFC = (uint16_t)(tRFC * factor);
    }
};

} // namespace dram
