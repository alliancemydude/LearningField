#ifndef TACTICAL_GENOME_H
#define TACTICAL_GENOME_H

typedef struct {
    float advance_distance;          // 8–25
    float defend_distance;            // 3–12
    float cover_radius;               // 1–8
    float cover_preference;           // 0–1
    float flanking_bias;              // 0.5–2.0
    float use_explosive_threshold;    // 1–4
    float retreat_hp_ratio;           // 0.1–0.6
    float cohesion_threshold;         // 2-8, how far a unit will be from its squad
} TacticalGenome;

#endif