# <cy>Pause Menu Studio</c> <cg>v4.1.3</c>

<cp>A complete visual pause-menu editor for Geometry Dash, built directly into the game.</c>

Move, resize, group, and hide pause-menu elements. Save multiple named layouts and add information cards for the level, music, coins, and difficulty.

---

## <co>IMPORTANT: EXPERIMENTAL</c>

Pause Menu Studio is currently <co>experimental</c>. Other mods and texture packs can modify the pause menu at the same time, so some combinations may cause visual glitches, shifted controls, or unmatched saved blocks.

<cy>Save a named layout before making a large redesign.</c> If you find a reproducible bug, keep your crashlog, latest.log, mod list, and exact reproduction steps ready.

## <cg>QUICK START</c>

1. Open any level and pause the game.
2. Press <cy>EDIT</c>.
3. Click an element to select it.
4. Press <cy>MOVE</c> to unlock movement.
5. Drag it with a mouse or touchscreen, or move it with the arrow keys.
5. Press <cg>DONE</c> to apply the current editing session.
6. Use <cp>SAVE</c> to store the layout under your own name.

The first click only selects a block. Movement stays locked until <cy>MOVE</c> is enabled. Press MOVE again to lock the selection. Each block remembers whether MOVE was enabled or disabled while this editor session remains open.

## <cb>CONTROLS</c>

- <cy>Click / tap</c> - select a block.
- <cy>MOVE</c> - toggle whether the selected block may move.
- <cy>Click / touch + drag</c> - move the selected block while MOVE is enabled.
- <cy>Repeated click / tap at the same point</c> - select the next overlapping block.
- <cy>Ctrl + click</c> - add another block to the selection.
- <cy>Arrow keys</c> - move by 1 unit.
- <cy>Shift + arrow keys</c> - move by 5 units.
- <cy>Ctrl + Z</c> - undo.
- <cy>Ctrl + Y</c> - redo.
- <cy>R</c> - reset the selected block.
- <cy>Delete</c> - move a block to Hidden Blocks. Backspace is intentionally not bound.
- <cy>Scale slider / numeric field</c> - resize the selection from 0.15 to 3.50.
- <cy>UNDO / REDO</c> - use the visible history buttons.
- <cy>RESET</c> - reset with a confirmation prompt.
- <cy>Preview</c> - hide the editor interface for a clean, read-only view.
- <cy>RETURN</c> - return from Preview without leaving Edit Mode.

Multi-selection uses an <co>orange</c> outline. A single selected block uses a <cb>cyan</c> outline.

## <cp>NAMED LAYOUTS</c>

- <cy>SAVE</c> stores the current layout under a custom name.
- <cy>LAYOUTS</c> opens the saved-layout list.
- A layout stores position, scale, rotation, supported opacity, drawing layer, Hidden Blocks, and the information cards that were present.
- The <cy>Separate layouts by mode</c> setting controls whether normal, platformer, creator, and platformer-creator menus use independent saved state.
- A card stored in a layout appears even when its normal mod setting is disabled.
- Cards from the active layout remain available after reopening the pause menu.
- A full <cr>RESET</c> removes layout-only cards and restores the normal card settings.
- Temporarily unavailable blocks from other mods remain in saved data instead of being deleted.
- Layout cards show the layout name, saved date, and management actions without a thumbnail.
- Use <cy>REN</c> to rename a layout or <cy>COPY</c> to duplicate it.

Old layouts may not contain card-presence data if they were saved before v2.1.0 and those cards had never been moved.

## <cr>HIDDEN BLOCKS</c>

Removing something in Edit Mode works like a recycle bin:

1. Select a block.
2. Press <cy>Delete</c> or the hide button.
3. Open the trash button.
4. Select an entry to return the block to its saved transform.

Before the list opens, the mod audits the active layout and invisible elements previously managed by Pause Menu Studio. If a missing record can be recovered reliably, it is returned to Hidden Blocks automatically.

In v3.0.2, Hidden Blocks uses a visual two-column grid, stores the actual reusable sprite frame captured from each object, and restores the exact Trash state saved by that layout. If an object has no cached sprite frame, a neutral info icon is shown instead of a guessed icon.

## <cb>V4 EDITOR INTERFACE</c>

- One bottom toolbar contains Undo, Redo, Save, Layouts, Updates, Hidden Blocks, Reset, and Preview.
- A compact contextual panel follows the selected logical block.
- Platformer levels automatically use <co>PLATFORMER EDIT MODE</c> for the play-time display and additional pause controls.
- Levels opened from your saved level editor automatically use <co>CREATOR EDIT MODE</c>. The level-editor button can be moved, resized, reset, hidden, restored from Trash, and saved in creator layouts.
- Cyan marks one selected block; orange marks a multi-selection.
- Corner handles and a logical-block label make the active target clear.
- Four safe shade regions dim everything around the selection without changing another mod's node opacity.
- Editor panels animate when Edit Mode opens, a selection appears, history changes, or a block enters Trash.
- Larger hit areas and the scale slider are designed for touchscreen use on Android.
- The context-panel gear opens exact <cy>rotation</c>, <cy>opacity</c>, and <cy>drawing-layer</c> controls.

## <cg>IN-GAME UPDATES</c>

Press <cy>UPDATES</c> in Edit Mode to check the public Pause Menu Studio GitHub Releases server.

The same check can run automatically while Geometry Dash loads. Startup checking, the loading-screen banner, and the red <cr>!</c> badges can be enabled independently in the mod settings.

If a newer release is available, the mod asks before downloading it, shows progress, validates the package identity, version, compatibility, and published SHA-256 digest, then stages it for the next restart. The currently running code is never hot-swapped.

## <cl>INFORMATION CARDS</c>

- <cy>Level name</c> - level title and creator. Disabled by default.
- <cy>Music</c> - the real Jukebox/NONGD song widget. Disabled by default.
- <cy>Coins</c> - native user-coin icons. Disabled by default.
- <cy>Difficulty</c> - difficulty face and star count. Disabled by default.
- <cy>Demonlist position</c> - an available AREDL/Pemonlist rank when Integrated Demonlist provides it.

The original level name is captured when PlayLayer starts. Reopening the pause menu should no longer replace <cg>Stereo Madness</c> with the active NONG/cover title from Jukebox.

## <cj>MENU STYLES</c>

- <cy>Classic</c> - the standard style and the default choice.
- <cy>Compact</c> - a denser layout.
- <cy>Showcase</c> - extra room for information cards.

Each style uses its own saved positions.

## <cp>MOD COMPATIBILITY</c>

### <cy>Required mod</c>

- NONGD / Jukebox: <cy>fleym.nongd >= 3.6.2</c>

### <cy>Optional integrations</c>

- Gold User Coins >= 2.0.4
- Demons in Between >= 1.7.0
- GDDP Integration >= 1.1.16
- Integrated Demonlist >= 1.7.13
- GodlikeFaces >= 1.1.10
- Better Volume >= 2.1.4
- Better Escape >= 1.0.1

Better Volume is handled as two complete blocks: <cg>Music</c> and <cg>SFX</c>. Jukebox uses the real CustomSongWidget, including its disc, MORE button, and delete button.

Integrations depend on internal node IDs from other mods. I cannot confirm every possible combination of versions and texture packs.

## <co>KNOWN LIMITATIONS</c>

- Dynamic elements from Jukebox, Better Volume, and other mods may be rebuilt after opening the pause menu or changing a song.
- A texture pack that moves an entire button container can compete with the editor for its position.
- A layout saved with a currently disabled mod may contain temporarily unavailable paths.
- Extremely large controls and heavy overlap are intentionally not blocked.
- The current build targets <cy>Geometry Dash 2.2081</c> and <cy>Geode 5.9.0</c> on <cy>Windows, Android32, and Android64</c>.
- Android touch editing uses the same select, MOVE-unlock, and drag controls as the desktop editor. Keyboard shortcuts require a physical keyboard; the visible editor buttons remain available without one.
- Android packages compile successfully, but not every phone, Android version, or touch configuration has been tested.

## <cr>TROUBLESHOOTING</c>

### <cy>A block disappeared</c>

1. Open Hidden Blocks.
2. Check whether the mod that owns the block is enabled.
3. Close and reopen the pause menu.
4. Do not use a full Reset before saving screenshots and logs.

### <cy>A layout looks wrong</c>

1. Check whether Classic, Compact, or Showcase is selected.
2. Compare the currently enabled mods with the layout's original setup.
3. Select only the affected block and press R.
4. Use a full Reset only after confirming the prompt.

### <cy>The game crashed</c>

Attach:

- the crashlog from <cy>Geometry Dash/geode/crashlogs</c>;
- latest.log from <cy>Geometry Dash/geode/logs</c>;
- your enabled mods and their versions;
- your texture-pack name;
- a screenshot or video;
- the exact actions immediately before the problem.

Developed by <cy>BANANCHIKIREAL</c>

<cg>Build a pause menu that looks exactly the way you want.</c>
