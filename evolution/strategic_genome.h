#ifndef STRATEGIC_GENOME_H
#define STRATEGIC_GENOME_H

typedef struct {
    float aggression_bias;            // 0.5–2.0
    float threat_weight;              // 0–1
    float edge_avoidance_weight;      // 0–1
    float flank_percentage;           // 0–1
    float flanking_bias;              // 0.5–2.0
} StrategicGenome;

#endif