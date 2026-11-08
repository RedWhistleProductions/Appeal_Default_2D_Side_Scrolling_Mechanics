# Appeal_Default_2D_Side_Scrolling_Mechanics

2D side-scrolling orientation: X is horizontal, +Y is upward, and there is no Z movement.

Default ground mechanics for Appeal players and NPCs. This is an independent local
Git project; it has no remote and has not been published. Requires C++17 and Make.

From this directory, run `make` to build and `make test` to test. The Windows output
is `Appeal_Default_2D_Side_Scrolling_Mechanics.dll`; Linux produces `Appeal_Default_2D_Side_Scrolling_Mechanics.so`.
Install beneath `Resources/Modules/Appeal_Module_Mechanics/Appeal_Default_2D_Side_Scrolling_Mechanics`.
The updated Mechanics module bridge is required for selected-entity commands and
shared globals. No third-party libraries are required.

On Windows, `Packaging/Windows/Install.ps1` builds the plugin in place. Use
`-AppealResources <path>` to copy the DLL into another Appeal installation.

Configure Appeal once with:

```text
Add_Dynamic_Module Mechanics Appeal_Default_2D_Side_Scrolling_Mechanics
```

## Basic use

```text
Mechanics Select Player
Mechanics Walk
Mechanics Update Player
Mechanics Jump
Mechanics Update All
Mechanics Stand

Mechanics Select Guard
Mechanics Walk_Analog -50
Mechanics Update All
```

`Select Entity` selects an existing entity or attaches a new entity to the default
plugin. `Use Entity Plugin_Name` also selects that entity. Selection is shared by
input callbacks, so select the correct player before processing that player's
input. NPC code can use `Digital Entity Action` or `Analog Entity Action Value`
without changing the module's selection. `Release Entity` detaches the entity;
re-entering preserves its plugin settings. `Update All` updates assigned entities.

| Digital (no arguments) | Analog (one integer) | Behavior |
| --- | --- | --- |
| `Stop` | `Stop_Analog Value` | Stop horizontal movement and stand upright |
| `Move_Left` | `Move_Left_Analog Value` | Walk toward -X; signed analog values can reverse direction |
| `Move_Right` | `Move_Right_Analog Value` | Walk toward +X; signed analog values can reverse direction |
| `Stand` | `Stand_Analog Value` | Stop horizontal movement and stand upright |
| `Walk` | `Walk_Analog Value` | Move at Speed |
| `Run` | `Run_Analog Value` | Move at Speed multiplied by Run_Multiplier |
| `Jump` | `Jump_Analog Value` | Launch with Jump_Speed when grounded |
| `Duck_Stand` | `Duck_Stand_Analog Value` | Stop and duck |
| `Duck_Walk` | `Duck_Walk_Analog Value` | Duck and move at Speed multiplied by Duck_Multiplier |

These are both script commands prefixed with `Mechanics` and plugin C exports.
Digital exports take no arguments; analog exports take one `int`. The standard
plugin exports `Digital(Entity, Action)` and `Analog(Entity, Action, Value)` route
to the same behavior. `Select(Entity)` selects the entity for direct C calls.

Digital actions use 100% of the entity's configured values. Analog values are
already mapped to -100..100 by the controller/Input layer. Movement uses
`Speed * multiplier * Value / 100`; negative values reverse horizontal movement.
Analog input changes effective velocity, leaving the configured Speed intact.
Zero stops movement; nonzero values as small as 1 are accepted unchanged. There
is no dead zone, smoothing, or device mapping. Out-of-range values are ignored.
Jump uses the absolute input fraction for upward launch strength; zero does not
jump and airborne entities cannot jump again. Jumping preserves horizontal
velocity. Duck amount is the absolute input fraction, 0..1. Stand analog records
the supplied input but remains stationary and upright. Button release can call
`Stand`, or analog movement with zero, to stop horizontal motion.

## Advanced tuning

`Mechanics Set Entity Key Value` tunes an attached entity. Distances are game
units, speed is units/second, gravity is units/second squared. Every Update uses
that entity's Delta_Time; set it to your game-loop timestep when needed.

| Key | Default | Accepted values |
| --- | --- | --- |
| Speed | 4 | Nonnegative finite number |
| Run_Multiplier | 2 | Nonnegative finite number |
| Duck_Multiplier | 0.5 | Nonnegative finite number |
| Jump_Speed | 6 | Nonnegative finite number |
| Gravity | 9.8 | Nonnegative finite number |
| Delta_Time | 1/60 | Positive finite number, seconds |
| Ground_Height | 0 | Finite number |
| Position_X, Position_Y | 0 | Finite number |
| Grounded | 1 | 0 or 1; setting 1 resets vertical velocity and snaps to ground |
| Mode | Ground | String label, independent of the current action |

Invalid settings, unknown actions, and calls without an attached entity leave
state unchanged. Defaults require no Advanced setup.

## Shared globals

The module publishes `Mechanics.Selected_Entity` (string, initially empty) and
`Mechanics.<Entity>.Plugin` (string slot name, normally Default). For this plugin,
it also publishes `Mechanics.<Entity>.<Key>` for every setting above, plus:

| Key | Type | Default | Meaning |
| --- | --- | --- | --- |
| Orientation | string | 2D_Side_Scrolling | Fixed coordinate convention |
| Action | string | Stand | Stand, Walk, Run, Jump, Duck_Stand, or Duck_Walk |
| Velocity_X, Velocity_Y | float | 0 | Effective horizontal/vertical speed |
| Duck_Amount | float | 0 | Crouch blend, 0..1 |
| Input | int | 0 | Last accepted action value |

Mode is a string, Grounded a bool, and the numeric settings are floats. Globals
refresh on selection, assignment, action, Set, and Update. Release clears the
plugin, mode, action, and orientation and removes the selection when applicable; numeric
globals become inactive snapshots. `Get(Entity, Key)` is an optional C export
used by the module to read state. These globals are available for a future
Graphics consumer; this project does not change the Graphics module.

## Current scope

This plugin supplies a simple X/Y ground movement simulation with jumping and a
flat ground height. It maintains its own per-entity state. It does not move World
entities, perform environment collision detection, resize collision shapes,
drive Physics bodies, or switch Graphics animations automatically. Water,
ladder, wall, and vehicle transitions and state transfer between plugins remain
future work. Mode and Action are separate so later animation selection can use
both. Default output is quiet; `Mechanics Debug 1` enables action diagnostics.

`make test` covers the plugin API. In the Appeal checkout, run
`powershell -NoProfile -ExecutionPolicy Bypass -File Tests/Default_Mechanics_Tests.ps1` to check DLL loading,
selected actions, globals, per-entity routing, and compatibility with Test plugins.

Move_Left/Move_Right select walking gait. Use signed Run_Analog or Duck_Walk_Analog for running or duck walking toward either side. Stop and Stand stop X motion without canceling airborne Y motion. Ground_Height is applied on every grounded Update, allowing collision code to supply rising or falling ground. Other orientations use their own standalone projects: Appeal_Default_2D_Top_Down_Mechanics and Appeal_Default_3D_Mechanics.
