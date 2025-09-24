# Ortos II – Use Case Diagram (UML)

Below is a PlantUML use case diagram showing how the Player interacts with the game.

```plantuml
@startuml
left to right direction
skinparam packageStyle rectangle

actor Player

rectangle "Ortos II Game" as Game {
  usecase "Start Game" as UC_Start
  usecase "Move Character" as UC_Move
  usecase "Shoot Projectiles" as UC_Shoot
  usecase "Collect Items" as UC_Collect
  usecase "Fight Enemies" as UC_Fight
  usecase "Open Gate" as UC_OpenGate
  usecase "Use Gate (Teleport to Center)" as UC_UseGate
  usecase "Respawn" as UC_Respawn
  usecase "Pause / Exit" as UC_Pause
}

Player --> UC_Start
Player --> UC_Move
Player --> UC_Shoot
Player --> UC_Collect
Player --> UC_Fight
Player --> UC_Pause

UC_Fight --> UC_OpenGate : <<includes>>\nAll enemies defeated
UC_OpenGate --> UC_UseGate : <<extends>>
UC_UseGate --> UC_Respawn : <<includes>>\nOn death or next room

@enduml
```

How to view
- Use any PlantUML renderer (e.g., VS Code PlantUML extension) to preview.
- Or copy the code block into an online PlantUML server to generate an image.

Key use cases (summary)
- Start Game: Begin a new session from the title menu.
- Move Character: Navigate the map with WASD.
- Shoot Projectiles: Attack using arrow keys to aim.
- Collect Items: Pick up items encountered in rooms.
- Fight Enemies: Engage and defeat enemies in the room.
- Open Gate: Gate becomes passable after all enemies are defeated.
- Use Gate (Teleport to Center): Transition action that teleports to room center.
- Respawn: Return after death or when entering a new room.
- Pause / Exit: Pause menu interactions and quitting the game.






