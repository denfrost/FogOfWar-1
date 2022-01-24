# 1. FogOfWar
This is a project to implement the fog system of the battlefield used in top-down games.
https://youtu.be/8Igz48XFkBk  



# 2. Content/Core
### 2-1. Bluerpints
Blueprints related to game instances and lobbies.

### 2-2. Maps
프로젝트에서 사용하는 모든 레벨입니다.  
DreamscapeMeadows Additional assets are required to open levels and LostRuin levels.
https://www.unrealengine.com/marketplace/ko/product/dreamscape-nature-meadows
https://www.unrealengine.com/marketplace/ko/product/modular-lost-ruins-kit  



# 3. Content/FogOfWar
### 3-1. Blueprints
 A Blueprint that references the TopDownGrid to update the fog texture.

### 3-2. Materials
A header that references the fog texture.



# 4. Content/TopDown
### 4-1. Blueprints
Blueprints for top-down gameplay.



# 5. Content/UI
### 5-1. Blueprints
All Widget Blueprints used by the project.



# 6. Source\FogOfWar\Public\FogOfWar
### 6-1. FogManager
See TopDownGrid and TopDownPC. Create and update the FogTexture.
Passes information of units owned by TopDownPC to FogTexture. Determines the visibility of units not owned by TopDownPC.

### 6-2. FogTexture
A class that creates and updates an actual texture (Texture2D). We use 2 buffers (SoruceBuffer, UpscaleBuffer) to update the texture.
Update the SourceBuffer with the data received from the FogManager. Update the UpscaleBuffer with data from the SourceBuffer. Render the texture with data from the UpscaleBuffer.


# 7. Source\FogOfWar\Public\TopDown
### 7-1. TopDownCamera
It is a phone that only has a camera component that supports top-down view. Supports basic screen movement. 

### 7-2. TopDownGameInstance, TopDownGameMode
Used to differentiate each player in multiplayer.

### 7-3. TopDownGrid
A class that creates a virtual grid map in the level.
It converts world coordinates into grid coordinates and stores the height value of each tile and world coordinates in the form of a map.

### 7-4. TopDownHUD
Used to select TopDownUnit by dragging.

### 7-5. TopDownPlayerController
Use it to identify and control the TopDownUnit you own.

### 7-6. TopDownUnit
Implement TopDownUnitInterface.
Stores team information and field of vision information. 

### 7-7. TopDownUnitInterface
This is an interface for unit selection effects and enemy identification.
