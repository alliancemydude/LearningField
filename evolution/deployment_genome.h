#ifndef DEPLOYMENT_GENOME_H
#define DEPLOYMENT_GENOME_H

typedef struct {
    float deploy_horizontal_spread;   // 0–10
    float deploy_vertical_offset;     // 0–10
    float deploy_flanking_bias;       // -5–5
} DeploymentGenome;

#endif