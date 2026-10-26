#ifndef STRATEGIC_GENOME_H
#define STRATEGIC_GENOME_H

typedef struct {
    float aggression_bias;            // 0.5–2.0 army-wide preference for advancing or dashing
    float threat_weight;              // 0–1 lower = go for lowest HP, 1 = go for nearest enemy
    float flank_percentage;           // 0–1 percentage of units that are classified as flankers
    float flanking_bias;              // 0.5–2.0 severity of flanking, higher = da long way
    float flank_depth;                // 0-20 the higher this value, the farther behind an enemy flankers will go
    float face_threat_weight;         // 0-1, lower = never turn to face threats, 1 = always
    float flank_targeting_weight;     // 0-1, lower = ignore angles, 1 = always prefer flanks
    float cohesion_weight;            // 0-1, lower = no cohesion bonus, 1 = always when in formation
} StrategicGenome;

#endif