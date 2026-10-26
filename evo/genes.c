#include "genes.h"

const GeneInfo GERMAN_GENES[] = {
    // Strategy (0-6)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    {"strategy.flank_depth",              0.0f, 20.0f, 4.0f},
    {"strategy.face_threat_weight",       0.0f,  1.0f, 0.0f},
    {"strategy.flank_targeting_weight",   0.0f,  1.0f, 0.0f},
    {"strategy.cohesion_weight",          0.0f,  1.0f, 0.0f},

    {"barbarian.aggression",      0.0f,  1.0f, 0.5f},
    {"barbarian.preferred_range", 0.0f, 1.0f, 0.5f},
    {"barbarian.spacing_x",       1.0f,  8.0f, 3.0f},
    {"barbarian.spacing_y",       1.0f,  8.0f, 3.0f},
    {"barbarian.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"elite_barbarian.aggression",      0.0f,  1.0f, 0.5f},
    {"elite_barbarian.preferred_range", 0.0f, 1.0f, 0.5f},
    {"elite_barbarian.spacing_x",       1.0f,  8.0f, 3.0f},
    {"elite_barbarian.spacing_y",       1.0f,  8.0f, 3.0f},
    {"elite_barbarian.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"javelin_thrower.aggression",      0.0f,  1.0f, 0.5f},
    {"javelin_thrower.preferred_range", 0.0f, 1.0f, 0.5f},
    {"javelin_thrower.spacing_x",       1.0f,  8.0f, 3.0f},
    {"javelin_thrower.spacing_y",       1.0f,  8.0f, 3.0f},
    {"javelin_thrower.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"german_cavalry.aggression",      0.0f,  1.0f, 0.5f},
    {"german_cavalry.preferred_range", 0.0f, 1.0f, 0.5f},
    {"german_cavalry.spacing_x",       1.0f,  8.0f, 3.0f},
    {"german_cavalry.spacing_y",       1.0f,  8.0f, 3.0f},
    {"german_cavalry.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"composition.barbarian_share",        0.0f, 1.0f, 0.5f},
    {"composition.elite_barbarian_share",  0.0f, 1.0f, 0.5f},
    {"composition.javelin_thrower_share",  0.0f, 1.0f, 0.5f},
    {"composition.german_cavalry_share",         0.0f, 1.0f, 0.5f}
};
const size_t GERMAN_GENE_COUNT = sizeof(GERMAN_GENES) / sizeof(GeneInfo);

const GeneInfo ROMAN_GENES[] = {
    // Strategy (0-6)
    {"strategy.aggression_bias",          0.5f,  2.0f, 1.2f},
    {"strategy.threat_weight",            0.0f,  1.0f, 0.5f},
    {"strategy.flank_percentage",         0.0f,  1.0f, 0.2f},
    {"strategy.flanking_bias",            0.5f,  2.0f, 1.0f},
    {"strategy.deploy_horizontal_spread", 0.0f, 10.0f, 6.0f},
    {"strategy.deploy_vertical_offset",   0.0f, 10.0f, 4.0f},
    {"strategy.flank_depth",              0.0f, 20.0f, 4.0f},
    {"strategy.face_threat_weight",       0.0f,  1.0f, 0.0f},
    {"strategy.flank_targeting_weight",   0.0f,  1.0f, 0.0f},
    {"strategy.cohesion_weight",          0.0f,  1.0f, 0.0f},

    {"legionare.aggression",      0.0f,  1.0f, 0.5f},
    {"legionare.preferred_range", 0.0f, 1.0f, 0.5f},
    {"legionare.spacing_x",       1.0f,  8.0f, 3.0f},
    {"legionare.spacing_y",       1.0f,  8.0f, 3.0f},
    {"legionare.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"centurion.aggression",      0.0f,  1.0f, 0.5f},
    {"centurion.preferred_range", 0.0f, 1.0f, 0.5f},
    {"centurion.spacing_x",       1.0f,  8.0f, 3.0f},
    {"centurion.spacing_y",       1.0f,  8.0f, 3.0f},
    {"centurion.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"archer.aggression",      0.0f,  1.0f, 0.5f},
    {"archer.preferred_range", 0.0f, 1.0f, 0.5f},
    {"archer.spacing_x",       1.0f,  8.0f, 3.0f},
    {"archer.spacing_y",       1.0f,  8.0f, 3.0f},
    {"archer.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"roman_cavalry.aggression",      0.0f,  1.0f, 0.5f},
    {"roman_cavalry.preferred_range", 0.0f, 1.0f, 0.5f},
    {"roman_cavalry.spacing_x",       1.0f,  8.0f, 3.0f},
    {"roman_cavalry.spacing_y",       1.0f,  8.0f, 3.0f},
    {"roman_cavalry.local_force_ratio", 0.5f,  2.0f, 1.0f},

    {"composition.legionare_share",        0.0f, 1.0f, 0.5f},
    {"composition.centurion_share",  0.0f, 1.0f, 0.5f},
    {"composition.archer_share",  0.0f, 1.0f, 0.5f},
    {"composition.roman_cavalry_share",         0.0f, 1.0f, 0.5f}
};
const size_t ROMAN_GENE_COUNT = sizeof(ROMAN_GENES) / sizeof(GeneInfo);