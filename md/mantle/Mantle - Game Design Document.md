
`v1.0 — Concept`

> **Genre:** Geological Survival / 3D Voxel Sandbox 
> **Perspective:** First-person, fully 3D 
> **Platform:** PC 
> **References:** Minecraft, Noita, Factorio, Subnautica, Rain World, Don't Starve, Made in Abyss, RimWorld, Dwarf Fortress
> **Modding:** essential, data-driven from day one

---

## 1. Vision

### The Feeling

You remember coming back to base after a long mining run — pockets full, barely alive, and your shelter waiting with familiar light and silence. You built that place. It's yours.

Mantle is about that feeling, but deeper and more intense.

You live underground. There is no surface, no sun, no familiar landmarks. There are only layers — each deeper than the last, each more hostile and more rewarding. You build bases, carve paths downward, and return. Every return to base is a small victory over a world that never asked you to be there.

The world is not trying to kill you. It simply exists by its own rules.

> _Understand this world well enough to live inside it._

### The Concept

Mantle is a 3D voxel survival game set entirely underground. The world is a living geological, physical and chemical systems — rock conducts heat, gas pockets ignite, water erodes soft stone, ecosystems evolve independently of the player.

Progression moves downward through biomes of increasing geological hostility, culminating in the Mantle itself. The player builds a network of bases — one per layer — connected by infrastructure they design and maintain. Reaching the next layer is an engineering problem. Going back up is expensive.

---

## 2. Design Pillars

### Knowledge is the only defense

The world is fully simulated — no scripted events. If something went wrong, there was a cause that could have been noticed and prevented. A player who understands the geology, chemistry, and ecology of their biome is safe. A player who doesn't will lose their base to consequences they didn't see coming. This is the Noita philosophy applied to underground survival.

### The world is the challenge

No enemies in the traditional sense. Geology, chemistry, and ecosystem pressure are the primary threats. Creatures have their own agendas and will only engage the player if disturbed. The danger comes from the environment — and from the player's own decisions within it.

### Base as home, depth as journey

The player's emotional anchor is their base — a place they built, secured, and made safe. Depth is where you go on expedition. The tension between a hostile unknown below and a familiar shelter above is the core loop. Every descent is a calculated risk. Every return is a reward.

### Simulate, don't script

Every geological event, ecological shift, and structural failure follows from the simulation. Magma rises because pressure changed. Fungal overgrowth spreads because conditions became favorable. The player can always trace cause and effect — and with enough knowledge, prevent anything bad from happening.

### Experiment, don't explain

The game never tells the player what to do or how the world works. It creates conditions in which experimentation is the natural response.

The world is consistent and physical. Every material behaves according to its properties in every context — always. This consistency is what makes experimentation meaningful: a result observed once is a result that can be relied upon. The player is not guessing. They are doing science.

The game does not reward experimentation with notifications, achievements, or unlocks. The reward is understanding — and understanding compounds.

> _Nothing is explained. Everything can be figured out._

### Engineering with intention

Automation exists, but Mantle is not a pure automation game. The player must assess their environment before building — a contraption that works in one biome may fail catastrophically in a another. Every engineering decision is contextual. Think less Factorio, more a hybrid: automate thoughtfully, adapt constantly.

### Moddable from day one

Every material, reaction, contraption, biome, and creature is defined in data files. The base game is the first content pack. New biomes are the primary mod type.

---

## 3. Core Loop

Every biome follows the same fundamental cycle:

**Explore** — survey the biome, read its geology, locate resources and potential paths downward. The environment is the first thing to understand — moving before understanding is how players lose bases.

**Extract** — gather resources while managing environmental risk: structural collapse, gas pockets, flooding, heat. Extraction is never passive.

**Build** — expand and reinforce the base. Every contraption placed is a response to a specific survival problem the biome presents. The base grows around the threats.

**Connect** — establish infrastructure linking this layer to the one above: lift shafts, transport pipes, a safe return route. Until this is done, the player is stranded. This step is what makes a base feel truly established — not just survivable, but permanent.

**Solve** — find the engineering solution that makes the next layer reachable. Each biome has a specific geological barrier to overcome before descent is possible.

**Descend** — drop to the next layer and begin again.

Returning to a previous layer is optional — never required, always possible. It costs ascent resources and time, but the infrastructure built during Connect makes it viable. A player who wants to revisit an old base, recover stored resources, or simply check on what changed while they were gone can do so.

---

## 4. Player Progression

### Progression Model

Player progression operates on two levels simultaneously.

**Knowledge is the primary soft block.** The next layer is rarely physically unreachable — it is survivable only by a player who understands what they are walking into. A player who descends without understanding the geology, chemistry, and ecology of the layer below will lose their base to consequences they didn't see coming. There is no tutorial. Understanding comes from observation and experiment.

**Gear is the hard block.** Certain barriers cannot be crossed without specific tools or equipment — a rock formation that requires a drill that doesn't exist yet, a temperature that kills without thermal protection. Gear sets the minimum requirement. Knowledge determines whether you survive after meeting it.

### Crafting

Crafting is station-based. Recipes define what components are needed — not what materials to use. The player chooses the materials. The environment decides if the choice was correct.

#### Example: 
A pump has a recipe. The recipe asks for a casing, a rotor, a seal. The player builds it from whatever materials they have. If that pump is moving water, almost anything works. If it is moving magma, a casing with a low melting point will fail. The pump degrades, deforms, or breaks — and the player understands why.

There is no "correct" material for any component — only suitable and unsuitable. Suitability is discovered through experimentation, not through a recipe list. A contraption built from unsuitable materials signals failure immediately and visibly. A contraption built from well-matched materials runs without intervention.

This applies to every contraption. A base built entirely from materials suited to its biome is more stable, more efficient, and more durable than one built from whatever was available. Early-layer materials are never obsolete — depth adds new properties, it does not replace old ones. See section 5.

### Material Analysis

Materials are found everywhere — lying on cave floors, composing the walls and ground of each biome. Every material has physical properties: melting point, density, thermal conductivity, structural strength, chemical reactivity. These properties determine how a material behaves in any given environment and how well it performs in a crafted component.

Material properties are not displayed. They are discovered through experimentation — in the field, or at the experiment bench available at every base. Observations can be recorded in the field journal. The journal is the player's own document; the game contributes nothing to it.

### Composite Components

A component can be built from more than one material. A pump casing can be an outer shell of one material bonded to an inner lining of another. Each layer contributes its properties to the whole. A heat-resistant outer shell paired with a chemically inert inner lining produces a casing that neither melts nor corrodes — something neither material could achieve alone.

This is the primary reason early-layer materials remain permanently valuable. Deep materials handle extremes. Shallow materials often have properties deep materials lack entirely: low thermal conductivity, high flexibility, chemical neutrality, low density. A composite component built from both outperforms one built from either.
High level materials give special properties with some drawbacks, low level are good for simple usage, but can't handle extreme cases.

### Degradation as immediate feedback

A contraption built from unsuitable materials does not fail after a timer. It begins degrading immediately — visibly, audibly, observably. Smoke, warping geometry, unusual sounds, erratic behavior. The more unsuitable the material, the faster the signal.

A contraption built from well-matched materials requires no maintenance. It runs indefinitely until the environment itself changes.

Material selection is a decision made once, correctly — not a maintenance schedule. The player's goal is to understand the environment well enough to choose right the first time. Degradation is not punishment; it is the world's answer to a wrong question.

### Why early materials stay relevant

No material becomes obsolete because depth does not replace properties — it adds new ones. 
#### Example: 
Clay is an excellent thermal insulator. Obsidian conducts heat rapidly. These are opposite properties. In a one biome, a player who needs to keep something cool needs clay more than obsidian.

#### Specific examples / edge cases:

- A heat exchanger in a hot temperature biome requires a thermally conductive inner surface and an insulating outer shell. Obsidian handles the inner surface. Clay handles the outer. Neither works alone.
- A pressure seal requires both structural density and chemical resistance to mineral-rich water. Dense ore from Crystal Vaults provides the density; a reactive-inert mineral from Lush Caves provides the resistance.
- A lift shaft in extreme-pressure zones requires a material that is both lightweight and structurally strong under compression — a combination found only in composite layering of mid-depth and shallow materials.

The player does not know this in advance. They discover it by building, watching what fails, and understanding why.

---

## 5. The Field Journal

The player has a field journal from the start of the game. It is empty.

The journal is a free-form notebook — the player writes whatever they want, however they want. There are no prompts, no automatic entries, no system-generated notes. The game never tells the player to open it.

Journal should be in a form of canvas, where player can write, draw or paste and connect icons or some images. It should be entire player's knowledge database. New page is created when player find new material, but it will be just empty. Player should experiment, take notes, and connect different entries with each other.

The journal exists because the world is complex enough that memory alone is insufficient. A player who experiments seriously will want to record what they found. A player who doesn't keep notes will rediscover things the hard way. Both are valid — the journal is a tool, not a requirement.

The journal is the player's map of the world's logic. No two players' journals look the same. This is intentional.
This provides really unique first time experience for playing through the game, but it also not a barrier for re-run or challenge run, because player already knows almost all that he need.

### The experiment approach

Player can create experiment bench — a small controlled environment where materials can be tested in isolation. The player can expose a material to heat, pressure, water, or chemical contact and observe how it behaves without building a full contraption.

The bench does not display numbers or properties. It shows behavior: the material cracks, softens, expands, reacts, holds. The player observes and draws their own conclusions.

The bench is the game's only concession to structured experimentation. Everything else is learned in the field — by building real things in real conditions and watching what happens.

---

## 6. Resources

### Raw Materials

The foundation of all crafting. Raw materials are physical substances with measurable properties — melting point, density, conductivity, hardness. They are found everywhere: in cave walls, on cave floors, in ore veins. Their properties determine what they are good for, not their name or category.

Two different stones may look similar but behave differently under heat or load. Discovering these differences is part of the game.

### Intermediate Components

Raw materials are processed at crafting stations into intermediate components — refined forms that recipes call for. A raw ore becomes a metal rod. A soft mineral becomes an insulating panel. This processing step is where material choice matters: the properties of the raw input carry through into the component and into the final contraption.

Components are necessary steps between raw extraction and finished gear or contraptions.

### Fluids

Water, magma, gas, and other fluids are resources as much as they are hazards. They cannot be carried in inventory directly — moving and storing fluids requires infrastructure: tanks, pipes, pumps. Fluid handling becomes available once the player has built the appropriate stations and contraptions.

### Rare Minerals

Scarce, biome-specific, and qualitatively different from ordinary raw materials. Rare minerals — gemstones and similar — do not substitute for common materials. They augment them. Embedding a rare mineral into a crafted item grants it properties it could not otherwise have: unusual thermal behavior, structural resonance, chemical inertness beyond what standard materials allow.

They are not required for progression. They are the difference between a base that works and a base that works exceptionally well.

---

## 7. Failure States

### Death

On death the player respawns at their base with full HP. All inventory and equipment drops at the point of death and remains there permanently — it will not despawn, decay, or be destroyed by the environment. Returning to recover it is the player's problem.

Gear is a soft requirement. A player who understands their biome can reach their corpse without equipment. A player who doesn't will need to craft replacements first and return prepared. Both are valid approaches.

An optional **Keep Inventory** mode is available for players who want to remove this penalty. A separate **Hardcore** mode offers permadeath for players who want the opposite.

### Base Loss

A base can be partially or fully destroyed by the simulation — flooding, structural collapse, magma intrusion, creature activity. There is no automatic recovery, no rollback, no safety net. If a base is lost, it is lost.

In practice, some situations are recoverable — a flooded base can in theory be drained, a collapsed section rebuilt. Whether it is worth the effort is the player's judgment call. Sometimes building a new base is the right answer.

This is not a design failure. It is the intended consequence of a world that simulates without mercy. An experienced player recognizes warning signs before disaster strikes — rising water levels, stress fractures, unusual creature migration patterns. Losing a base to something that could have been prevented is information. The world does not punish randomly. It punishes ignorance.

### Infrastructure Failure

Individual contraptions degrade if built from materials unsuited to their environment. The signal is immediate and observable — smoke, warping geometry, erratic behavior — not a slow timer. The more unsuitable the material, the faster the degradation. A contraption built from well-matched materials requires no maintenance and runs indefinitely.

Infrastructure failure is never sudden without prior signals. The simulation always leaves traces. A player paying attention will notice degradation before catastrophic failure occurs.

---

## 8. The World

### No surface

The game begins underground. The surface does not exist — or is unreachable. It may be the subject of a future DLC or campaign extension, but is not part of the base game.

### Structure

The world is **infinite horizontally** within each layer and **finite vertically** — a fixed number of depth layers, each with its own biome, geology, and ecology. The bottom is the Mantle: a definitive endpoint and the climax of the campaign.

Each layer is large enough to explore, settle, and build in without feeling constrained. The transition between layers is not automatic — it requires the player to physically find and engineer a path downward and upward.

### Procedural generation

Each layer generates procedurally using geologically coherent noise and simulation passes. Every cave, ore vein, aquifer, and gas pocket exists for a physical reason. Every world seed is a unique geological problem.

### Material simulation

All solid materials are voxel-based and fully destructible with appropriate tools. Fluids, gases, and smoke are a separate volumetric simulation layer — realistic-looking, not blocky.

- **Heat conduction** — stone conducts slowly, metal quickly, ice melts near magma
- **Structural stress** — weight propagates through connected voxels; unsupported structures collapse
- **Chemical reactions** — declarative reaction tables define what happens when materials combine
- **Fluid dynamics** — water, lava, and gas fill spaces volumetrically and behave physically

### The world lives without you

The simulation continues whether the player is present or not. Ecosystems evolve slowly over time. A base left unattended is not safe — not because the game decided to punish you, but because the conditions around it kept changing. Creatures may move in. Fungal networks may spread. Water may find a new path.

Changes accumulate slowly — noticeable over hours of real time, not minutes. A base abandoned mid-session returns roughly as the player left it. A base abandoned across multiple sessions may have shifted meaningfully. The world does not wait, but it does not rush.

This timescale is intentional. It preserves the feeling that the player's work persists, while keeping the world genuinely alive. A base is not a save state.

---

## 9. Biomes

Each biome is a complete underground ecosystem — distinct geology, chemistry, flora, and creature ecology. Players can settle in any biome and face its specific survival puzzle. The biome choice for a primary base is entirely up to the player — there is no single correct starting point.

### Example biomes:

| Biome              | Geology                                          | Flora / Fauna                                       | Primary Hazard                            |
| ------------------ | ------------------------------------------------ | --------------------------------------------------- | ----------------------------------------- |
| **Lush Caves**     | Soft limestone, clay, underground rivers         | Bioluminescent flora, cave fish, grazing herbivores | Flooding, erosion, unstable ceilings      |
| **Crystal Vaults** | Quartz veins, resonant stone, dry atmosphere     | Sparse crystal flora, echo-sensitive creatures      | Gas pockets, resonance collapses          |
| **Fungal Warrens** | Porous basalt, spore-rich air, mycelial networks | Vast fungal colonies, decomposer creatures          | Toxic spores, spreading fungal overgrowth |
| **Thermal Vents**  | Igneous rock, obsidian seams, high heat          | Extremophile flora, heat-adapted fauna              | Magma intrusions, pressure bursts         |
| **The Mantle**     | Pure igneous, extreme pressure and heat          | Almost nothing survives here                        | Everything — final frontier               |

---

## 10. Movement and Vertical Progression

### The cost of ascent — _Made in Abyss_ model

Descending is free. Ascending is not.

The deeper the layer, the more severe the cost of returning upward without proper infrastructure. At shallow depths, unassisted ascent causes minor HP loss. At mid-depths it becomes dangerous. At deep layers, unassisted ascent is lethal.

This is not a scripted punishment — it is a physical property of depth: pressure, heat, and atmospheric conditions make return increasingly hostile to the human body.

### Infrastructure solves the problem

The player's answer to expensive ascent is engineering: lift shafts, pressurized tunnels, thermal insulation systems. Each layer may require a different solution depending on its specific geological conditions. A simple rope lift works in Lush Caves. It will not work near Thermal Vents.

Building this infrastructure is a major engineering goal at each depth layer — and completing it is what makes a base feel truly established.

### Item transport is cheap, player transport is expensive

Resources move freely between bases through pipes, chutes, and automated systems. The player does not. This asymmetry is intentional — it creates a natural incentive to build bases at every layer rather than commuting from a single home base.

---

## 11. Base Network

### Philosophy

The player doesn't build one base. They build infrastructure. A network of bases — one per layer, connected by transport systems — each adapted to the hostile conditions of its biome.

The primary base (the player's home) can be in any biome. There is no correct choice. A player who settles in one biome will face different challenges than one who settles in different biome, but neither choice is wrong.

### Bases degrade through simulation

A base does not deteriorate on its own. But the world around it keeps moving. Creatures may migrate through and settle. Geological events may shift water flow or crack walls. Fungal networks may colonize abandoned rooms. None of this is scripted — it follows from the simulation. A player who understands their biome can predict and prevent it.

### Conservation

The player can invest resources to stabilize a base before leaving — sealing tunnels, maintaining lighting, running automated drainage or pressure systems. A well-stabilized base resists the simulation: sealed spaces don't flood, lit corridors deter creature migration, pressurized rooms block atmospheric intrusion.

Stabilization is an engineering problem, not a menu option. The player decides what to seal, what to power, what to leave exposed. A base stabilized against flooding may still be colonized by fungal growth if the spore pressure wasn't considered. Understanding the specific risks of each biome is what makes stabilization effective.

A player who leaves a base without preparation is making a choice, not a mistake — they may return to find it changed, damaged, or partially reclaimed. Sometimes this is recoverable. Sometimes it isn't. The simulation does not distinguish between attended and unattended — it simply continues.

---

## 12. Building System

### Physics-based construction

Every placed voxel has mass. Structural stress propagates through connected materials. Unsupported spans collapse, weak materials fail under load, improperly sealed spaces flood. Building in high-pressure zones, near thermal vents, or in unstable geology requires different engineering approaches than building in a stable cave. The geology of the biome directly constrains what you can build and how.

### Contraption-focused building

Your base is not a house. It is a life support system.

Everything you place serves a function — pumps keep water out, heat exchangers make a thermal zone survivable, gates seal off a tunnel before a pressure burst reaches your storage. The base grows organically around the problems the biome presents. You don't build a shelter and then add contraptions to it — the contraptions _are_ the shelter.

Decoration exists, but it is secondary. The primary driver of every building decision is survival.

### Contraptions

Simple machines, each doing one thing — combinable into complex emergent systems.
### Examples:

- **Pumps** — move fluids
- **Valves** — control flow
- **Pressure plates** — detect weight
- **Heat exchangers** — transfer temperature
- **Gates** — open and close
- **Lift shafts** — vertical player transport
- **Chutes and pipes** — item transport between bases
### Destruction

All voxels are destructible with appropriate tools. Explosions propagate structurally. A blast in a gas-filled tunnel behaves like a real explosion. Tool effectiveness varies by material.

---

## 13. Creatures and Ecology

There are no enemies in the traditional sense. Creatures have their own agendas — grazing, hunting each other, migrating, defending territory. Some neutral species will engage the player only if disturbed. Some hostile species will purposefully seek and hunt down player.

Creatures can be killed. But killing has consequences — removing most herbivores from a biome changes flora growth patterns, which changes the atmosphere, which may affect other creatures. Because the ecosystem is fully simulated, cascading effects are possible and not always predictable. This is the _RimWorld_ model: systems interact in ways the designer didn't explicitly script.

Experienced players learn creature behavior patterns and coexist. Accidentally triggering a conflict feels like the player's fault — a consequence of a choice, not unfair difficulty.

The world is alive whether the player is present or not. Ecosystems evolve slowly over time following the _Rain World_ model applied to underground geology.

---

## 14. Progression

### The Descent

Mantle is a single journey downward. The world is structured so that going deeper is the natural and inevitable direction: resources grow richer, geology grows more hostile, and the Mantle pulls you forward.

The player is never told where to go. But the world makes the answer obvious.

Each biome is a self-contained engineering problem. The player must understand the geology, chemistry, and ecology of their current layer before the next one becomes survivable — not because a gate blocks the path, but because descending underprepared means losing everything to consequences that could have been anticipated. Knowledge and infrastructure together determine when descent is possible. Neither alone is enough.

**Example arc:**

1. **Lush Caves** — learn the fundamentals: material properties, water management, basic contraptions, first base
2. **Crystal / Fungal layer** — gas management, complex contraption chains, ecosystem navigation
3. **Thermal layer** — heat management, extreme structural engineering, magma systems
4. **The Mantle** — all systems simultaneously, climax event

Reaching the Mantle is the endpoint. It is a definitive moment — not a soft fade.

### After the Mantle

The world does not end. The player's base network remains. New geological phenomena emerge at extreme depths. The journey is over — but the place they built is still theirs.

### Replayability

Every world seed is a unique geological problem. The systems are fixed, the layout always differs. Each run is a different journey through the same world.

### Pacing

Each biome is designed to take approximately 15–25 hours on a first playthrough — comparable to a single planet in Factorio. A complete run through all four biomes to The Mantle is roughly 60–100 hours.

The pacing within each biome is consistent: explore, establish, solve, descend. There is no artificial acceleration deeper into the game. More end-game biome takes as long as first biome because the engineering problems are proportionally harder, not because the loop is shorter.

The first biome is the exception. It is a place where the player learns how the world works — material experimentation, base-building fundamentals, the cost of ascent. This onboarding is built into the environment, not into a tutorial system. As a result, the first biome may feel slower than later ones for an experienced player on a second run. This is intentional.

---

## 15. Modding

Mantle is designed as a highly moddable game. The base game is the first content pack (Base mod).

**All of the following are data-driven and moddable from day one:**

- Materials and physical properties
- Chemical reaction tables
- Contraption types
- Biome definitions (geology, flora, fauna, hazards, generation rules)
- Creature ecology and behavior patterns
- World generation parameters

The goal is full modding support comparable to _Minecraft_ — the extent to which core simulation systems can be modified will depend on implementation, but the ambition is maximum openness. New biomes are the primary mod type: each one a complete world addition with new materials, interactions, flora, and creatures.

---

## 16. Visual Style

- **Small-voxel rendering** for solid geometry — finer grain than Minecraft, closer to Teardown aesthetic
- **Advanced shader pipeline** — dynamic lighting, shadows, ambient occlusion, subsurface scattering
- **Volumetric layer** for fluids, gases, and smoke — realistic-looking, not blocky
- **Bioluminescent** flora and creatures provide natural light sources underground

---

## 17. Audio Design

Sound in Mantle is information before it is atmosphere.

The player spends most of their time in enclosed spaces with no visual horizon. Sound becomes the primary way the world communicates distance, material, and threat. A crack in a ceiling sounds different from a crack in a wall. Flowing water has a direction. Gas hissing from a fissure has pressure. A player who listens carefully knows more about what's around the corner than one who doesn't.

### Sound as simulation output

Every physical event produces sound that follows from its cause. Structural stress produces creaking proportional to load. Fluid movement produces sound proportional to volume and velocity. Thermal expansion produces ticking and popping in metal contraptions under heat. These are not ambient sound effects — they are direct outputs of the simulation, carrying the same information the player could derive from observation.

A contraption beginning to degrade sounds wrong before it looks wrong. The player who notices the change has time to act. The player who ignores it loses the contraption.

### Silence as signal

Deep biomes are not quiet by default — they have their own ambient pressure, geological hum, and ecological texture. A sudden silence in a normally active space means something changed. Creatures stopped moving. A gas flow stopped. A fungal network went dormant. Silence is worth paying attention to.

### Music

Sparse and non-intrusive. The score responds to depth and situation but never competes with environmental audio. The player should always be able to hear the world over the music. Tension is built by the world, not underscored by the soundtrack.

---

## 18. UI/UX Philosophy

The interface reflects the game's core principle: the player earns information through engagement with the world, not through menus.

### Show less, mean more

The HUD is minimal by default. No material property readouts, no threat indicators, no waypoints. What the player sees on screen is what their character would physically perceive — light levels, the state of visible contraptions, their own physical condition. Information that requires instruments requires the player to actually use those instruments.

This is not artificial difficulty. It is consistency. A world that simulates without explaining should have an interface that does the same.

### Diegetic where possible

Wherever possible, information is delivered through the world rather than the interface. A contraption that is degrading shows it physically — smoke, deformation, erratic movement — before any UI indicator appears. A cave filling with gas changes the visual atmosphere before a danger prompt appears. The world speaks first. The interface is a fallback, not a primary channel.

### Inventory and crafting

Practical and functional. Inventory is physical — items take space, weight affects movement in extreme conditions. Crafting is station-based: the player approaches a station, selects a recipe, chooses materials. No floating menus. No crafting from anywhere. The act of returning to base to craft is part of the loop.

### The field journal

The only non-diegetic element the player carries at all times. It is a notebook — fully player-authored, never system-generated. Its presence in the UI is deliberate: it is the one place the game acknowledges that the player is building knowledge over time. Everything else the player knows lives in their head. The journal is what they chose to write down.

---

## 19. Multiplayer

Mantle is a single-player game. There are no plans for multiplayer. The simulation-first design and the personal nature of the journey make it a deliberate single-player experience.

---

## 20. Mods — World Composition

The base game ships with a fixed layer order. But the world structure is fully moddable — new layers can be added by the community, each a complete biome with its own geology, ecology, and hazards. Players can compose a custom world from any combination of base game and mod layers, arra nged in whatever order they choose. The descent is the same — the world it leads through is up to the player.

---

## 21. Lore and Narrative

Mantle has no story in the traditional sense. There is no protagonist — the player character has no name, no backstory, no arc. They are a presence in the world, not a person. The player should see themselves, not a character.

The world has lore — but it is delivered entirely through environmental storytelling. The geology of a biome tells its history. The behavior of creatures tells you what the ecosystem has been through. The deeper you go, the more the world speaks — not through cutscenes or dialogue, but through what you find and what you observe.

There are no sharp twists. No revelations designed to surprise. The focus is gameplay. Everything the world has to say, it says through being itself.

### Environmental storytelling — how it works in practice

The world communicates through physical evidence. The player reads it by paying attention.

A vein of obsidian cutting through limestone tells a story of ancient heat — magma that pushed up through softer rock and cooled. The player who understands this knows that obsidian here means a heat source was once nearby, and may still be below. This is geology as narrative and as practical information simultaneously.

A cave system with no living herbivores but abundant flora tells a different story — something displaced the grazers, recently enough that the plants haven't been touched. The predator is probably still here. The player who reads this pauses before going deeper.

Crystal formations that grow toward a specific wall suggest airflow — something is circulating gas from that direction. Could be a gas pocket. Could be a natural ventilation shaft. Worth investigating before mining through.

Fungal networks that suddenly stop at a geological boundary didn't spread there voluntarily — something in the rock chemistry repels them. That boundary material might be useful. A player who wonders why will find out.

None of this is explained. The world is simply built with enough physical consistency that a player who pays attention can read it. The deeper the player goes, the more complex the layering of geological events becomes — and the more richly the world's history can be reconstructed by someone who knows how to look.

---
## MVP v0.1 build content
The smallest playable version of Mantle should contain:

- One biome
- Three basic materials
- One gas — flammable cave gas
- Structural physics — weight and stress propagation, collapse
- Four contraptions — pump, valve, pressure plate, gate
- Basic ascent cost — HP loss on unassisted upward movement
- One lift shaft contraption — basic vertical transport solution
- Basic tool destruction — pickaxe hardness tiers
- Procedural world generation — one biome, geologically coherent
- No creatures in v0.1 — environment is sufficient threat