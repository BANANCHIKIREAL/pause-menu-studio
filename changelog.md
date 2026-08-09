# v3.0.2

- Delete is now the only keyboard shortcut that moves a selected block to Trash; Backspace no longer removes blocks.
- The numeric scale field no longer rewrites its formatted value while focused, so Backspace can edit it normally.
- FREE MOVE now remembers its enabled or disabled state separately for each selected block while the editor is open.
- Replaced both RESET controls and VIEW with custom white reset and eye artwork.
- Removed the orange 1.00 line and label from the scale slider.
- Trash now captures the actual cached sprite frame from each removed object instead of choosing an icon from its name; objects without a reusable frame get one neutral info icon.
- Saved-layout previews now store actual cached object sprite frames; old or unavailable frames use neutral dots instead of guessed icons.
- Re-captures icons for existing Trash entries when Hidden Blocks opens.
- Added a larger gap between REN and COPY and moved the layout delete button away from them.

# v3.0.1

- Added a real MOVE lock: selecting a block no longer moves it until MOVE is enabled.
- Added direct numeric scale input.
- Increased spacing between toolbar and saved-layout actions.
- Added a smooth exit animation when leaving Edit Mode.
- Replaced RESET and VIEW with dedicated Geometry Dash icons.
- Saved the exact Trash contents and their icon types inside every named layout.
- Replaced dot-only layout previews with miniature Geometry Dash block icons.
- Added rounded Geometry Dash frames to editor panels, layout cards, previews, and Trash cards.
- Improved native icon matching for settings, music, SFX, comments, coins, demons, ratings, levels, and other hidden blocks.

# v3.0.0

- Rebuilt Edit Mode around a single dark bottom toolbar with large touch targets for Undo, Redo, Save, Layouts, Hidden Blocks, Reset, and Preview.
- Added a contextual selection panel that follows the selected logical block and provides Move guidance, a live scale slider, local scale reset, and Hide.
- Added Preview Mode, which hides the editor chrome for a clean, read-only view until Return is pressed.
- Added animated editor entry, selection-panel appearance, undo/redo feedback, trash feedback, and popup-card entry.
- Added focused selection visuals with thin cyan or orange outlines, corner handles, a logical-block name, and safe dimming around—not over—the selection.
- Redesigned Saved Layouts as cards with miniature position previews, saved dates, Apply, Rename, Duplicate, and Delete actions.
- Redesigned Hidden Blocks as a two-column visual grid with native Geometry Dash icons and large Restore buttons.
- Added a touch-friendly scale slider and enlarged editor hit areas for Android while keeping physical-keyboard shortcuts available.
- Kept existing named-layout snapshots and Hidden Blocks storage compatible; older layouts appear as Legacy Layout until saved again.

# v2.2.1

- Added the official Pause Menu Studio website to the Geode mod card.
- Added BANANCHIKIREAL Badges API v1.2.4 as a required dependency for the developer badge.

# v2.2.0

- Added official Android support for Geometry Dash 2.2081 on both Android32 and Android64.
- Added a multi-platform GitHub Actions workflow that builds Windows, Android32, and Android64 packages and combines their artifacts.
- Documented Android installation, touch controls, architecture selection, and Android build commands.

# v2.1.0

- Level cards now keep the original level name captured when PlayLayer starts, so reopening the pause menu cannot replace an official level title with Jukebox's active NONG/cover title.
- Coins and Difficulty information cards are now disabled by default on new configurations.
- Named layouts now record which information cards exist, recreate theme cards even when their normal settings are disabled, and keep those cards across pause-menu reopenings.
- Full Reset now removes theme-only cards and returns card visibility to the normal mod settings.

# v2.0.7

- Hidden Blocks now audits the active layout and currently invisible Pause Menu Studio controls, returning missing managed blocks to Trash before the list opens.
- Applying a named layout no longer deletes Trash entries whose dynamic mod nodes are temporarily unavailable.
- Named layouts now store every Trash member, including hidden dynamic controls that are absent while the layout is saved.

# v2.0.6

- Fixed the PauseLayer retain cycle reintroduced by the one-second entrance-layout guard; pressing Space immediately after Esc now lets the game remove the pause menu normally.
- Position and scale edits are now staged in Edit Mode and written only when Done is pressed; leaving the level through the unsaved-changes warning discards those temporary transforms.
- Fixed Jukebox's disc being pinned by its title-dependent nong-menu origin; placement now uses the visible nong-pin center, keeping it on the music panel's top-left corner for every song title.
- Removed the one-second layout jump on pause-menu entry by reapplying saved geometry after each entrance-animation frame instead of waiting for one delayed correction.
- Fixed the confirmation popup's Exit button leaving an orphan pause layer behind: confirmed exits now use PauseLayer's complete quit path, which closes both the level and its pause UI.

# v2.0.3

- Fixed Apply resetting resized controls to their default scale when a dynamic mod node no longer matched the saved profile path.
- Named layouts now save and restore their Trash state, including grouped hidden controls; older layouts preserve the current Trash when applied.
- Replaced the default-size control icon with Geometry Dash's native editor reset icon.

# v2.0.0

- Fixed hidden blocks occasionally disappearing without a Trash entry when two sanitized node paths produced the same storage ID; Trash entries are now unique and verified before a block is hidden.
- Replaced the layout editor's text controls with native Geometry Dash icon buttons that also follow installed texture packs.
- Changed the default menu style from Showcase to Classic.
- Fixed the confirmed Exit action failing to leave the level when Better Escape is installed.
- Fixed tiny selection outlines for music and fire widgets by measuring their visible descendants instead of technical container sizes.
- Multi-selection created with Ctrl+click now uses an orange outline; single selection remains cyan.
- Stabilized the Jukebox music block after repeated pause-menu openings so its title no longer drifts onto the level-name card.
- Re-pins Jukebox's recreated disc for several frames after a song or multi-asset change, so different title lengths cannot move it.
- Repairs Better Volume slider-thumb geometry after menu-animation actions without disabling the texture pack's sprite animations.
- Repeated clicks at the same cursor position now cycle through overlapping logical blocks from front to back.

# v1.5.1

- Fixed Jukebox's disc moving between NCS and regular songs by pinning it to the music block's top-left corner after every song-info rebuild.
- Stopped old saved child offsets from being restored inside music, level, coin, and difficulty cards.
- Added an exit warning when Edit Mode is still active and the current pause-menu layout was changed.
- Added Better Escape compatibility: normal Esc still resumes, while Shift+Esc and the quit button show the edit warning before leaving the level.
- Full RESET now restores every hidden block to its default position, makes it visible, and clears the trash.
- Better Volume groups now receive one shared screen-edge correction, preventing their parts from compressing at the top or sides.
- Enlarged the mod-list logo so the emblem fills the available icon area.

# v1.5.0

- Added per-block reset with confirmation. Select a block and press R, or use RESET while a block is selected.
- Added HIDE plus Delete/Backspace shortcuts to move selected logical blocks into a persistent trash.
- Added a trash popup that restores hidden blocks to their exact position and size from before removal.
- Better Volume Music and SFX groups are hidden, restored, and reset as complete logical blocks.
- Replaced the mod logo with a new cyan-and-gold pause-menu editor emblem with transparent outer corners.

# v1.4.1

- Fixed Better Volume's Music and SFX selection overlay: each section now appears as one logical block with one outline around its visible controls instead of separate or screen-sized technical bounds.

# v1.4.0

- A click now selects an element without moving it; dragging starts only after the cursor crosses a movement threshold.
- Added Ctrl+click multi-selection and synchronized dragging for selected elements.
- Added arrow-key movement (1 unit, or 5 units while holding Shift) with Undo/Redo history.
- Added Size-, 100%, and Size+ controls with persistent scale and named-layout support.
- Better Volume's music controls and SFX controls are now treated as two complete logical groups.
- Made the Edit/Done button movable while Edit mode is active.
- Fixed the Save button escaping its layout-name popup.
- Fixed empty space in the cards row selecting and moving level, music, and difficulty cards together.
- Level Name and Music cards now default to disabled.
- Fixed official levels displaying an NA/unrated difficulty label when Geometry Dash has a calculated difficulty.

# v1.3.0

- Added named pause-menu layouts that can be saved, selected, applied, overwritten, and deleted from Edit mode.
- Moved named-layout storage and popup UI out of `main.cpp` into dedicated source files.
- Removed the opaque black corners from the mod icon and replaced them with transparency.

# v1.2.0

- Fixed RESET corrupting animated texture-pack buttons by restoring only elements actually moved by Pause Menu Studio.
- Added a confirmation popup before resetting the layout.
- Removed the PauseLayer retain cycle that could leave the pause menu alive after resuming with Space.
- Replaced per-frame position enforcement with one delayed restore after entrance animations finish.
- Added more spacing between the Undo, Redo, and Reset buttons.

# v1.1.1

- Removed the bright editor grid.
- Added undo and redo history with `Ctrl+Z`, `Ctrl+Y`, and in-editor buttons.
- Replaced the reset setting toggle with a direct `RESET` button in Edit mode.
- Reapplied saved positions every frame so menu-animation texture packs cannot overwrite custom positions.
- Explicitly updated feature state for compatibility with GodlikeFaces fire and Legendary/Mythic faces.
- Added a rounded Geometry Dash-style frame to the mod icon.

# v1.1.0

- Added Geometry Dash's real star sprite beside the star count.
- Added exact Demons In Between and GDDP difficulty faces when those mods enable them.
- Added asynchronous AREDL/Pemonlist placement from Integrated Demonlist's data sources.
- Enabled the real multi-asset `CustomSongWidget` mode so songs, SFX, total size, and the info button are shown.
- Music and information cards now move as complete logical groups in Edit mode.
- Moving a difficulty card now keeps its face, feature state, star reward, and Demonlist rank together.
- Reset incompatible v1.0.x child positions and compacted the editor status display.

# v1.0.3

- Replaced the imitation music panel with the real `CustomSongWidget` used and extended by Jukebox 3.6.2.
- The music panel now uses Jukebox's real `JB_PinDisc.png` button and native song controls.
- Added compatibility with Gold User Coins 2.0.4 by using its swapped `secretCoinUI_001.png` frame.
- Replaced the difficulty text with Geometry Dash's real `GJDifficultySprite`.
- Fixed rated levels being shown as Unrated by using `GJGameLevel::getAverageDifficulty()`.
- Reworked Edit mode selection, smooth dragging, screen-edge protection, and selection outlines.
- Added a dedicated Pause Menu Studio mod icon.

# v1.0.0

- Initial release.
- Added draggable pause-menu elements with saved positions.
- Added Classic, Compact, and Showcase presets.
- Added configurable level, music, coin, and difficulty panels.
