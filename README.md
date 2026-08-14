<p align="center">
  <img src="logo.png" width="190" alt="Pause Menu Studio logo">
</p>

<h1 align="center">Pause Menu Studio</h1>

<p align="center">
  <strong>A complete Geometry Dash pause-menu editor built directly into the game.</strong><br>
  Move, resize, group, and hide controls, save named layouts,<br>
  and add configurable cards for the level, music, coins, and difficulty.
</p>

<p align="center">
  <a href="https://github.com/BANANCHIKIREAL/pause-menu-studio/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/BANANCHIKIREAL/pause-menu-studio?display_name=tag&style=for-the-badge&color=8b5cf6"></a>
  <img alt="Geometry Dash 2.2081" src="https://img.shields.io/badge/Geometry%20Dash-2.2081-22c55e?style=for-the-badge">
  <img alt="Geode 5.9.0" src="https://img.shields.io/badge/Geode-5.9.0-38bdf8?style=for-the-badge">
  <img alt="Windows and Android" src="https://img.shields.io/badge/platform-Windows%20%7C%20Android-2563eb?style=for-the-badge&logo=android">
  <img alt="Experimental" src="https://img.shields.io/badge/status-experimental-f97316?style=for-the-badge">
</p>

<p align="center">
  <a href="#-features">Features</a> •
  <a href="#-quick-start">Quick start</a> •
  <a href="#%EF%B8%8F-controls">Controls</a> •
  <a href="#-compatibility">Compatibility</a> •
  <a href="#-known-limitations">Known limitations</a> •
  <a href="#%EF%B8%8F-building-from-source">Building</a>
</p>

> [!WARNING]
> **Pause Menu Studio is experimental.** It is usable, but the pause menu is also modified by many other mods and texture packs. Some combinations may cause visual glitches, moved controls, or saved node paths that no longer match. Save a named layout before updating mods or making a large redesign.

## ✨ Features

### 🎨 Visual editor

- Move almost any reachable pause-menu element with a mouse or touchscreen.
- Automatically enter **PLATFORMER EDIT MODE** in platformer levels, including the play-time display and platformer pause buttons.
- Automatically enter **CREATOR EDIT MODE** for levels launched from your saved level editor, including the level-editor pause button.
- Use one compact bottom toolbar instead of separate controls scattered around the screen.
- Use the compact contextual panel beside the selection for the MOVE lock, exact scaling, advanced transforms, local reset, and Hide.
- A click only selects an element. Enable **MOVE**, then drag or use the arrow keys; press MOVE again to lock it.
- Repeated clicks at the same cursor position cycle through overlapping blocks from front to back.
- Resize selected buttons and logical blocks with a touch-friendly slider.
- Open the gear control to edit rotation, opacity, and drawing layer precisely.
- Use arrow keys for precise movement.
- Use `Ctrl` for multi-selection with a shared orange outline.
- Move multiple selected blocks together.
- Optional 5-unit grid snapping without a bright overlay.
- Move the **EDIT / DONE** button itself.
- Collapse the bottom toolbar with its eye button while keeping Edit Mode, block selection, movement, scaling, and contextual controls active.
- Restore the toolbar with the compact eye button at the lower-right edge on desktop or Android.
- Enter **Preview Mode** to inspect a clean, read-only pause menu without editor chrome.
- Keep the selected block clear while the surrounding pause menu is softly dimmed.

### ↩️ History and reset

- Undo with `Ctrl + Z` or the **UNDO** button.
- Redo with `Ctrl + Y` or the **REDO** button.
- Choose an Undo/Redo history limit from 10 to 200 actions in the mod settings.
- Reset the selected block with `R`.
- Reset only the selected block's scale.
- Use a full **RESET** with a confirmation prompt.
- The editor warns before leaving a level with unsaved Edit Mode changes.

### 💾 Named layouts

- Save multiple pause-menu designs under custom names.
- Update any existing layout for the current gameplay mode directly from the Save window.
- Optionally keep normal, platformer, creator, and platformer-creator layouts separate, with independent active layouts, positions, and Hidden Blocks bins.
- Select and apply a saved design through **LAYOUTS**.
- Store position, scale, rotation, opacity, drawing layer, hidden state, and information-card presence together.
- Recreate cards stored in a layout even when their normal settings are disabled.
- Keep active-layout cards available after reopening the pause menu.
- Preserve temporarily unavailable dynamic blocks instead of deleting their records.
- A full Reset removes layout-only cards and restores the normal card settings.
- Browse clean layout cards with names, saved dates, and management actions.
- Rename or duplicate a layout without overwriting its original snapshot.

### 🗑️ Hidden Blocks

- `Delete` sends the selected block to a persistent recycle bin. `Backspace` is not bound to removal.
- The recycle bin stores the block's complete transform before hiding it.
- Restore a hidden block through the **Hidden Blocks** window.
- Browse hidden controls in a two-column icon grid with large Restore buttons.
- Before opening the list, v2.0.7+ audits the active layout and invisible controls previously managed by Pause Menu Studio.
- If a missing record can be linked reliably to a saved layout or managed node, it is returned to the list automatically.

### 🧩 Logical groups

Some visual controls are made from several technical nodes. Pause Menu Studio attempts to treat them as one logical block:

- the Jukebox/NONGD music panel, disc, title, artist, **MORE**, and delete controls;
- the Better Volume **Music** section;
- the Better Volume **SFX** section;
- the level-title card;
- the difficulty card, rating flame, and star reward;
- the user-coin card.

This lets you move a complete widget instead of accidentally selecting a `%` label, an internal sprite, or one piece of a frame.

### ⬆️ In-game updates

Version 4 adds an **UPDATES** button to Edit Mode. It checks the public
[Pause Menu Studio GitHub Releases feed](https://github.com/BANANCHIKIREAL/pause-menu-studio/releases)
for a newer stable or prerelease version.

- The game asks for confirmation before downloading anything.
- Geometry Dash checks the feed while its loading screen is visible. A banner announces a newer version, and a red **!** stays above **UPDATES** in Edit Mode.
- Download progress is shown inside Geometry Dash.
- When GitHub publishes a SHA-256 digest, the downloaded bytes must match it.
- The package must contain the Pause Menu Studio mod ID, the advertised version, and compatible Geometry Dash/Geode targets.
- The currently working package is kept as a temporary backup while the new package is staged.
- The running code is never hot-swapped. The new version loads only after restarting Geometry Dash.
- If the server is unavailable or a release has no `.geode` asset, the installed version is left untouched.

## 🚀 Quick start

1. Install [Geode](https://geode-sdk.org/) for Geometry Dash.
2. Download the `.geode` file from [Releases](https://github.com/BANANCHIKIREAL/pause-menu-studio/releases/latest).
3. Install the package for your platform:
   - **Windows:** place it in `Geometry Dash/geode/mods`.
   - **Android:** place it in `/storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods/`, or import it through Geode's mod installer.
4. Make sure the required **NONGD/Jukebox** dependency (`fleym.nongd >= 3.6.2`) is installed.
5. Fully restart Geometry Dash.
6. Open any level and pause the game.
7. Press **EDIT**, select an element, enable **MOVE**, and drag it.
8. Press **DONE** to apply the current editing session.
9. Press **SAVE** if you want to store a named layout.

> [!TIP]
> If one block is hidden behind another, click the same point repeatedly without moving the cursor. Selection cycles through the logical blocks under that point.

## ⌨️ Controls

| Action | Key / button | Notes |
|---|---|---|
| Enter Edit Mode | **EDIT** | The button itself can also be moved |
| Platformer Edit Mode | Automatic in a platformer level | Uses separate positions, layouts, and Hidden Blocks |
| Creator Edit Mode | Automatic in a level launched from your level editor | The level-editor pause button is a complete editable block |
| Leave Edit Mode | **DONE** | Applies the current editing session |
| Select a block | Click / tap | A single press does not move it |
| Unlock / lock movement | Context **MOVE** button | Each block remembers its own on/off state while the editor is open |
| Drag a block | Enable MOVE, then click / touch + movement | Dragging starts after a small movement threshold |
| Hide / restore the bottom toolbar | Toolbar eye / small lower-right eye | Only the toolbar is hidden; ordinary pause-menu blocks remain editable |
| Select the next overlapping block | Repeated click / tap at the same point | No object movement is required |
| Add a block to selection | `Ctrl + click` | Requires a keyboard; multi-selection uses an orange outline |
| Precise movement | `←` `↑` `↓` `→` | Optional physical-keyboard shortcut; moves by 1 game unit |
| Faster movement | `Shift + arrow` | Optional physical-keyboard shortcut; moves by 5 game units |
| Undo | `Ctrl + Z` / **UNDO** | Android users can use the visible button |
| Redo | `Ctrl + Y` / **REDO** | Android users can use the visible button |
| Reset selected block | `R` | Restores its original position, scale, rotation, opacity, and layer |
| Hide selected block | `Delete` | Sends it to Hidden Blocks; Backspace does nothing |
| Resize | Context scale slider or numeric field | Touch-friendly range from `0.15×` to `3.5×` |
| Reset size | Context reset button | Does not delete or hide the block |
| Advanced transform | Context gear button | Edits rotation, opacity, and drawing layer |
| Save a layout | **SAVE** | Uses a custom name |
| Select a layout | **LAYOUTS** | Applies transforms, cards, and Hidden Blocks |
| Open recycle bin | Trash button | Lists and restores hidden blocks |
| Full reset | **RESET** with no selection | Always asks for confirmation |
| Preview the menu | Preview toolbar button | Hides editor UI for a clean, read-only view |
| Return from Preview | **RETURN** | Restores the current Edit Mode session |
| Rename / duplicate layout | **REN / COPY** | Available on each Saved Layout card |

## 🖼️ Menu styles

Three base styles are available:

| Style | Intended use |
|---|---|
| **Classic** | Standard and recommended; selected by default |
| **Compact** | A denser arrangement |
| **Showcase** | Extra space for information cards |

Each style uses its own saved positions. With **Separate layouts by mode** enabled, normal, platformer, creator, and platformer-creator menus also use separate position namespaces, so a menu with additional controls cannot overwrite another pause-menu variant.

## 🪪 Information cards

Cards can be enabled separately in the mod settings.

| Card | Content | Default |
|---|---|---|
| **Level name** | Level title and creator | Off |
| **Music** | The full song widget: title, artist, SongID, size, and available actions | Off |
| **Coins** | Native user-coin icons | Off |
| **Difficulty** | Geometry Dash or compatible difficulty face, native star icon, and optional live Globed player count | Off |
| **Demonlist position** | An available AREDL/Pemonlist rank for an eligible demon | On when data is available |

## ⚙️ Settings

| Setting | Purpose | Default |
|---|---|---|
| **Enable Pause Menu Studio** | Enables or disables the entire mod | `On` |
| **Menu style** | Selects Classic, Compact, or Showcase | `Classic` |
| **Level name** | Shows the level card | `Off` |
| **Music** | Shows the music card | `Off` |
| **Coins** | Shows the coin card | `Off` |
| **Difficulty** | Shows the difficulty card | `Off` |
| **Demonlist position** | Shows an available demonlist rank | `On` |
| **Globed player count** | Shows the live total for the current Globed level session inside Difficulty | `On` |
| **Edit mode on open** | Automatically enables the editor when the pause menu opens | `Off` |
| **Snap to grid** | Rounds the released position to a 5-unit grid | `Off` |
| **Separate layouts by mode** | Isolates positions, named layouts, active layouts, and Hidden Blocks for each pause-menu mode | `On` |
| **Advanced transform controls** | Adds rotation, opacity, and drawing-layer editing | `On` |
| **Undo history size** | Keeps between 10 and 200 editor actions | `50` |
| **Check on startup** | Checks GitHub Releases while Geode loads | `On` |
| **Loading-screen banner** | Shows the available-version banner during loading | `On` |
| **Update notification sound** | Plays the bundled calm two-note chime once when the loading banner appears | `On` |
| **Update badges** | Shows a red `!` above Edit and Updates | `On` |

## 🤝 Compatibility

The minimum versions below come directly from [`mod.json`](mod.json).

| Mod | ID | Minimum version | Integration |
|---|---|---:|---|
| **NONGD / Jukebox** | `fleym.nongd` | `3.6.2` | Required dependency; uses the mod's real music widget |
| **Gold User Coins** | `colon.gold_user_coins` | `2.0.4` | Compatible colored/gold user coins |
| **Demons in Between** | `hiimjustin000.demons_in_between` | `1.7.0` | Additional difficulty faces |
| **GDDP Integration** | `minemaker0430.gddp_integration` | `1.1.16` | Additional demon progression visuals |
| **Integrated Demonlist** | `hiimjustin000.integrated_demonlist` | `1.7.13` | AREDL/Pemonlist placement for eligible demons |
| **GodlikeFaces** | `adyagd.godlikefaces` | `1.1.10` | Compatible rating flames and difficulty faces |
| **Better Volume** | `nwo5.better_volume` | `2.1.4` | Music and SFX are selected as two complete logical blocks |
| **Better Escape** | `ecuet.better-escape` | `1.0.1` | Alternative exit behavior and `Shift + Esc` compatibility |
| **Globed** | `dankmeme.globed2` | `2.1.2` | Live current-level player count inside the Difficulty card |

> [!IMPORTANT]
> Integrations depend on the node hierarchy and internal IDs of other mods. If another mod changes when a control is created, its ID, or its parent hierarchy, Pause Menu Studio may stop recognizing that block until compatibility is updated.

### Texture packs

Pause Menu Studio attempts to move outer button containers without overwriting their animated internal sprites. However, a texture pack that moves or recreates the **entire button node** may compete with the editor for its position. Include the texture-pack name and a short video in a bug report when this happens.

## 🗃️ Layout and Hidden Blocks storage

- Position, scale, rotation, supported opacity, and drawing layer are stored using a node path inside `PauseLayer`.
- Position keys include the selected menu style, gameplay mode, and applied-layout generation. Existing normal-mode keys retain their legacy format.
- A named layout stores the complete supported transform, hidden state, and known information-card presence.
- Named layouts are tagged as normal, platformer, creator, or platformer-creator and appear only in their matching editor mode.
- When mode separation is enabled, every menu variant has its own active layout and Hidden Blocks storage.
- Hidden Blocks also stores the block's complete supported restore transform.
- If a dynamic node is temporarily absent, its record remains stored instead of being deleted.
- Opening Hidden Blocks audits the active layout and invisible nodes previously managed by the editor.

This reduces lost controls, but it cannot guarantee a path match after another mod radically changes its UI hierarchy.

## ⚠️ Known limitations

Pause Menu Studio is intentionally marked **experimental**.

1. **Every mod combination cannot be covered.** Integrations are written for the IDs and minimum versions declared in `mod.json`, but I cannot confirm every possible combination. Newer versions may change their internal UI.
2. **Dynamic controls can be recreated.** Jukebox, Better Volume, and some texture packs rebuild UI after opening the pause menu or changing a song. Known controls are stabilized repeatedly, but a rare brief jump or a new incompatibility may still occur.
3. **Old layouts may contain unavailable paths.** This is expected when a layout was saved with a mod that is currently disabled. The record is kept and may become available when that mod returns.
4. **Technical invisible nodes are not recovered without evidence.** Automatic Hidden Blocks recovery only uses controls previously managed by Pause Menu Studio or marked hidden in the active layout. This avoids adding unrelated service nodes from other mods.
5. **Only the declared platforms are supported.** The current manifest targets Geometry Dash `2.2081` and Geode `5.9.0` on Windows, Android32, and Android64. iOS and macOS are not declared or built.
6. **Android runtime coverage is limited.** Android32 and Android64 packages compile successfully, but the current release has not been tested on every phone, Android version, launcher version, or touch configuration.
7. **Extreme scale and overlap remain the user's choice.** The editor helps select overlapping blocks but intentionally does not prohibit unusual designs.

## 🧯 Troubleshooting

### A block disappeared

1. Open **Hidden Blocks** so v2.0.7+ can run its missing-record audit.
2. Check whether the mod that owns the block is enabled.
3. Reopen the pause menu so dynamic mods can recreate their nodes.
4. Do not use a full Reset before saving logs and screenshots.

### A layout looks wrong

1. Check whether the same Classic, Compact, or Showcase style is selected.
2. Compare the enabled mod set with the setup used when the layout was saved.
3. Select the affected block and press `R` for a local reset.
4. Use a full **RESET** only after confirming the prompt.

### The game crashed

Create a [Bug report](https://github.com/BANANCHIKIREAL/pause-menu-studio/issues/new?template=bug-report.yml) and attach:

- the file from `Geometry Dash/geode/crashlogs`;
- `latest.log` from `Geometry Dash/geode/logs`;
- your Pause Menu Studio version;
- the full enabled-mod list;
- your texture-pack name;
- exact actions immediately before the crash;
- a screenshot or video for visual problems.

## 🐞 Reporting a bug

Before creating an issue:

1. Fully restart Geometry Dash and reproduce the problem again.
2. Check whether it follows the same action sequence.
3. If possible, disable other mods in groups to find the conflicting mod.
4. Keep the crashlog and `latest.log`.
5. Use the provided **Bug report** form so the required reproduction information is included.

[![Report a bug](https://img.shields.io/badge/Report%20a%20bug-GitHub%20Issues-ef4444?style=for-the-badge&logo=github)](https://github.com/BANANCHIKIREAL/pause-menu-studio/issues/new?template=bug-report.yml)

## 🛠️ Building from source

### Requirements

- Geometry Dash `2.2081`;
- [Geode SDK](https://docs.geode-sdk.org/getting-started/);
- CMake `3.21` or newer, as declared in [`CMakeLists.txt`](CMakeLists.txt);
- a `GEODE_SDK` environment variable pointing to the SDK directory.

For **Windows**, install Visual Studio 2022 Build Tools with the C++ toolchain. For **Android**, install a recent Android NDK supported by the current Geode SDK; this project was verified with NDK `30.0.15729638` and Clang `21.0.0`.

### Commands

Open **Developer PowerShell for VS 2022** in the project root:

```powershell
$env:GEODE_SDK = 'C:\path\to\GeodeSDK'
cmake -S . -B build-local -DCMAKE_BUILD_TYPE=Release
cmake --build build-local --config Release --parallel
```

The package is created at:

```text
build-local/bananchikireal.pause-menu-studio.geode
```

The project uses C++23 and `setup_geode_mod`.

For Android, use the Geode CLI from the project root:

```powershell
geode build -p android32 --ndk 'C:\path\to\Android\Sdk\ndk\30.0.15729638' --config Release
geode build -p android64 --ndk 'C:\path\to\Android\Sdk\ndk\30.0.15729638' --config Release
```

The packages are created in `build-android32` and `build-android64`. The repository's GitHub Actions workflow also builds Windows, Android32, and Android64 and combines the platform artifacts.

## 🧱 Project structure

```text
pause-menu-studio/
├── src/
│   ├── main.cpp                  # PauseLayer, editor, cards, and integrations
│   ├── HiddenBlocks.*            # Persistent recycle-bin storage
│   ├── HiddenBlocksPopup.*       # Hidden-block restoration UI
│   ├── LayoutProfiles.*          # Named-layout serialization
│   ├── LayoutProfilePopups.*     # SAVE and LAYOUTS popups
│   └── BlockIcons.*              # Native GD icon capture for Trash
├── mod.json                      # Manifest, dependencies, and settings
├── CMakeLists.txt                # Build configuration
├── changelog.md                  # Version history
├── about.md                      # In-game Geode About page
└── logo.png                      # Mod icon
```

## 📜 Version history

The complete history is available in [`changelog.md`](changelog.md).

### v4.5.0

- Added a compact live Globed player-count badge to the Difficulty card.
- Included the local player in the total and refresh the count every half second while the Globed level session is active.
- Added an enabled-by-default **Globed player count** setting and kept Globed an optional dependency.

### v4.4.2

- Restored the original saved-layout action tiles after the replacement panel sprite rendered as detached strips without a texture pack.
- Added rounded clipping to the original Rename, Copy, and Delete tiles without changing their colors, proportions, or icons.
- Made the Saved Layouts window shrink to the number of visible profiles instead of leaving a large empty fixed-height area.

### v4.4.1

- Replaced the long alternating-color updater text with compact rounded release-note cards.
- Added a short title and grouped white description to every update item.
- Fixed Shift+Esc exit crashes involving the custom Jukebox song card and NONGD music callbacks.
- Fixed oversized custom close icons in every Pause Menu Studio popup.
- Rebuilt the Apply Layout and Update Layout folder symbols and polished the saved-layout action buttons.

### v4.4.0

- Replaced the remaining editor action icons with a unified white, transparent icon set instead of Geometry Dash sprite frames.
- Added custom Edit, Done, Transform, Restore, Rename, Copy, Close, navigation, Apply Layout, and Update Layout icons.
- Apply Layout and Update Layout use the exact same folder silhouette as Layouts, with a clean contour gap behind their internal symbol.
- Replaced the random Trash-card previews with one neutral custom block icon so entries stay visually consistent.
- Added reusable custom icon-button components to keep popup actions aligned and readable.

### v4.3.2

- Added an original, bundled two-note update chime for the loading-screen update notification.
- The sound plays only once per game launch and only when an available-update banner is actually shown.
- Added an **Update notification sound** setting so the chime can be disabled without disabling update checks or badges.

### v4.3.1

- Corrected the v4.3.0 toolbar behavior: the editor toolbar itself is no longer selectable, movable, scalable, or stored as a layout block.
- The toolbar eye now collapses only the toolbar while Edit Mode remains fully active.
- Ordinary pause-menu blocks can still be selected, moved, resized, hidden, reset, and edited while the toolbar is collapsed.
- Added a compact lower-right eye button that restores the toolbar without leaving Edit Mode or replacing the current selection.

### v4.3.0

- The complete bottom editor toolbar is now a selectable logical block.
- Its position, scale, rotation, opacity, and drawing layer can be edited with the same contextual controls as pause-menu blocks.
- Toolbar transforms persist independently for each configured menu style and separated gameplay mode.
- Hiding the toolbar reveals a compact **SHOW TOOLBAR** recovery handle outside the panel, including on Android.
- Editor entrance and exit animations now travel from the toolbar's saved position instead of resetting it to the default bottom edge.

### v4.2.0

- The Save Layout window now offers separate **Save New** and **Update Existing** actions.
- Existing layouts for the current gameplay mode are displayed in a scrollable list and can be replaced after confirmation.
- Updating preserves the full current layout state, including transforms, information cards, and Trash contents.
- The new-name field no longer opens the Android keyboard until the player selects it.

### v4.1.7

- Fixed the in-game updater showing `Release notes were not provided for this update` for manifests that contain release notes.
- Update details are now preserved before the download callback takes ownership of the update package data.

### v4.1.6

- Replaced the Save, Layouts, Trash, Undo, and Redo toolbar graphics with a matching pure-white icon set.
- Save now uses a folder with a downward arrow, while Layouts uses a clean open folder.
- Undo and Redo use mirrored versions of the same curved-arrow design for perfect visual consistency.
- All five icons use transparent `128x128` PNG resources with consistent padding and sizing.

### v4.1.5

- Replaced unauthenticated GitHub REST API update checks with a small first-party `update.json` manifest hosted on the mod website.
- Update checks no longer consume GitHub's IP-based 60-request hourly REST API allowance.
- The manifest validates the release URL, version, and required SHA-256 digest before offering a download.

### v4.1.4

- The in-game updater now displays the selected GitHub Release's `What's New & Fixed` notes before downloading.
- Release notes are readable in a dedicated scrollable window, with Markdown links and formatting converted to game-safe text.
- The update is installed only after the player explicitly presses `DOWNLOAD`.

### v4.1.3

- Editor shortcuts are now suspended while typing or while a modal dialog is open.
- Layout names can safely contain `R` and other shortcut keys without changing the selected block.
- Resized Geometry Dash buttons now retain their edited size after being clicked.

### v4.1.2

- Replaced the texture-sensitive update badge panel with a plain bright-red `!`.
- Positioned the mark on the Edit button's upper-right edge so it moves together with the button.

### v4.1.1

- Replaced the large loading-screen update banner with a compact animated toast at the top of the screen.
- The toast slides in, remains visible briefly, and leaves upward without covering Geode's loading text.

### v4.1.0

- Added optional mode-separated layouts for normal, platformer, creator, and creator-platformer pause menus.
- Added precise rotation, opacity, and drawing-layer controls with persistence through Undo/Redo, named layouts, and Hidden Blocks.
- Added independent startup-check, loading-banner, and update-badge settings.
- Added a configurable Undo history limit from 10 to 200 actions.

### v4.0.4

- Fixed the startup update check on Geode's already active loading screen.
- Added the update `!` directly above the pause-menu Edit button as well as the Updates toolbar button.
- Moved the update banner away from Geode's loading-status labels.

### v4.0.3

- Fixed the stock pause-menu heading of official levels changing to the active Jukebox NONG/remix title after resuming and pausing again.
- The fix restores the real level name only on the PauseLayer title, so Jukebox song selection and metadata remain untouched.

### v4.0.2

- Checks GitHub Releases during the Geometry Dash loading screen and shows a visible update banner when a newer version exists.
- Keeps a red `!` above the Updates control until the update is installed.
- Makes the selected block's scale and movement-lock status substantially easier to read.

### v4.0.1

- Replaces the Updates, Layouts, and Move controls with clean white transparent icons supplied for the new interface.
- Moves the selected-block divider out of the Reset control.

### v4.0.0

- Added the secure GitHub Releases updater with in-game checks, download progress, validation, rollback protection, and optional immediate restart.
- Added a dedicated Updates control and rebuilt toolbar spacing so all eight actions and group separators have clear gaps.

### v3.3.0-beta.2

- Polished the beta editor UI after in-game testing: stable panel textures, readable labels, a compact mode badge, separated inspector actions, and a clean Edit button without a square background.

### v3.3.0-beta.1

- Introduces the new layered GD-style editor dock and animated contextual inspector.
- Adds live Ready/Unsaved, scale, movement-lock, and gameplay-mode feedback.
- Adds individual action tiles, grouped controls, mode-colored accents, and polished editor entrance/exit animation.

### v3.2.1

- Added Creator Edit Mode for levels launched from the Geometry Dash level editor.
- Added complete movement, scaling, Reset, Hide/Trash, restore, and named-layout support for the level-editor pause button.
- Isolated creator and platformer-creator positions and saved state from the other pause-menu variants.

### v3.2.0

- Shrunk the selection contextual panel (width 420 -> 400, height 78 -> 68) and tightened its internal spacing so it covers less of the screen.
- Added automatic Platformer Edit Mode for platformer-only pause controls and the play-time display.
- Isolated platformer positions, named layouts, active layout state, and Hidden Blocks from normal-mode data.

### v3.1.0

- Removes Saved Layout thumbnail previews, including all icons, dots, and the preview box.
- Gives layout names more horizontal room in the simplified cards.

### v3.0.3

- Stores normalized world-space block centers for layout thumbnails, so icons appear in their real pause-menu positions on every aspect ratio.
- Stops legacy layouts without preview coordinates from drawing broken lower-left icon piles; resave them to generate a new preview.
- Paginates Hidden Blocks four entries at a time so a large Trash remains usable.

### v3.0.2

- Makes Delete the only removal key and keeps Backspace unbound.
- Keeps the focused scale field editable instead of restoring its formatted value after every Backspace press.
- Remembers FREE MOVE separately for each block during the current editor session.
- Adds custom white artwork to both Reset controls and View, and removes the orange 1.00 slider marker.
- Captures real object sprite frames for Trash, repairs old Trash previews, and separates REN/COPY.
- Stores real object sprite frames in newly saved layout previews; legacy layouts use neutral dots instead of guessed icons.

### v3.0.1

- Makes MOVE a real lock/unlock toggle and adds exact numeric scale input.
- Adds a smooth Edit Mode exit, wider control spacing, rounded GD frames, and dedicated Reset/View icons.
- Saves the complete Trash state and icon types per named layout.
- Replaces dot-only previews with miniature Geometry Dash icons.

### v3.0.0

- Rebuilds Edit Mode with a unified bottom toolbar and contextual selection panel.
- Adds Preview Mode, animated feedback, focused selection shading, corner handles, and logical-block labels.
- Adds visual Saved Layout cards with previews, dates, Rename, Duplicate, Apply, and Delete.
- Adds a visual Hidden Blocks grid with native GD icons and larger Restore actions.
- Adds a touch-friendly scale slider and larger Android hit targets.

### v2.2.0

- Adds Android32 and Android64 support for Geometry Dash 2.2081.
- Uses the existing touch editor for tap-to-select and drag-to-move controls.
- Adds automated Windows and Android builds through GitHub Actions.

### v2.1.0

- Captures the original PlayLayer level name so a Jukebox NONG/cover title cannot replace an official level title after reopening the pause menu.
- Disables Coins and Difficulty cards by default.
- Stores card presence in named layouts and recreates layout cards even when their normal settings are disabled.
- Removes layout-only cards during a full Reset.
- Audits Hidden Blocks and preserves temporarily unavailable dynamic nodes.

## 🤍 Author

Developed by **BANANCHIKIREAL**<br>
GitHub: [@BANANCHIKIREAL](https://github.com/BANANCHIKIREAL)

If the mod is useful, consider starring the repository. It helps show that continued development is worthwhile.

---

<p align="center">
  <strong>Build a pause menu that looks exactly the way you want.</strong><br>
  <sub>And please report bugs—experimental software becomes stable through clear, reproducible reports.</sub>
</p>
