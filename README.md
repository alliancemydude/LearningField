# LearningField

Author: Steven Fry
Email: bomberbub@icloud.com

**Overview**
Learning Field is a program meant to simulate different learning styles in the context of a strategy game. Set what units you would like to comprise your armies into and let them battle. You will receive detailed, specific feedback on winrates, actions taken, and strategies used. To improve the capabilities of the armies, you can run training sessions. These training sessions put the unit AI through an evolution-style set of battles where your troops will learn what strategies are best for them. 

**Running the program:**
make
./battle
Follow text prompts


**Code Architecture:**
  config.h - enums, constants, shared types
  unit.h/c - unit struct, stat blocks, weapons
  unit_blueprint.h/c - base stats per unit type
  unit_genome - per-unit genomes that can be evolved
  battlefield.h/c - spawn, calculate, and run battles
  combat.h/c - execute attacks, calculate damage, evaluate morale
  movement.h/c - pathing, facing, and position updates
  ai.h/c - per-turn behavior and choices based on unit role
  queries.h/c - spatial and targeting helpers
  strategy.h/c - faction-wide strategic genomes
  composition.h/c - army-building based on point budget
  evolution.h/c - the evolution algorithm
  calibration.h/c - automated point-value discovery
  tests.c - tests for the code itself
  main.c - the menu and entry point


**Evolution Gene Categories:**
Strategic (10):
  Faction-wide aggression, targeting preference, flanking doctrine, formation behavior
Unit (5 x 8 unit types):
  Per-unit aggression, preferred engagement range, formation spacing, local force threshold
Composition (4 per faction):
  Point-budget allocation
Deployment (2 per faction):
  Deployment spacing both vertically and horizontally

**How The Evolution Works:**
  1. Two populations of n individuals each are initialized with random genomes
  2. Every individual fights another individual on the other side, averaging winrates across matches
  3. The best individuals are selected via tournament select, crossed over, and possibly mutated
  4. The cycle repeats for n generations
  5. Fitness rewards wins, damage done, and speed. Timeouts are penalized.

**Observations/What's Interesting:**
  - Faction split. The two factions, German and Roman, chase different strategies based on their unit types. Currently that means Germans prefer mobile, small strike forces and the Romans prefer larger, formation-focused combat.
  - Calibration loop. The point values are not hard-coded. They're meant to change dynamically based on the outcome of calibration and evolution, so the AI doesn't hyper-focus on certain strategies.
  - Failures. Common evolutionary program failures are a factor. Arms-races, trading off discovery for consistency, etc.

**Known Issues:**
  - Specialist units don't play well with the fitness function. Sometimes they dominate the meta because they do very well against a specific unit type.
  - There is no memory for the game, so all actions are chosen in the moment. Grand strategy is not yet a factor.
  - Basic metas are very common. Swarm tactics often dominate due to the plain numbers advantage.

**Upcoming Plans:**
  - Adding basic unit memory. Units will remember what they did in their previous turn. Eventually this will be refined to be memory for the length of the battle.
  - Hall of Fame genomes. Eventually a section for the best-performing genomes will be added. It will be biased toward abnormal, interesting results.
  - Genome result folder. .txt files will be preserved in a folder to get a historical view of strategies, to see which ones are used most often. This will wait until most genomes are set in stone.
