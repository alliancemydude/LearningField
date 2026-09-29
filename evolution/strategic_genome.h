#ifndef STRATEGIC_GENOME_H
#define STRATEGIC_GENOME_H

typedef struct {
    float aggression_bias;            // 0.5–2.0
    float aggression_curve;           // 0.5–6.0
    float target_priority;            // 0–1
    float ally_proximity;             // 2–8
    float threat_weight;
    float focus_fire_tendency;
    float flank_left_weight;
    float flank_right_weight;
    float breakthrough_weight;
    float fallback_weight;
    float edge_avoidance_weight;
} StrategicGenome;

#endif