Task 6: Gettin' Fancy  -  Nathan Wildofsky

I added the ability to scale and offset textures within each material,
then I made each material editable at runtime with ImGui.

uvScale and uvOffset are member variables in the Material class that get passed
from C++ to the pixel shader within the ExternalData buffer.

I manipulated the usage of these values so that a higher uvScale means a magnified texture,
and so a positive uvOffset.x scrolls to the right and a positive uvOffset.y scrolls up.